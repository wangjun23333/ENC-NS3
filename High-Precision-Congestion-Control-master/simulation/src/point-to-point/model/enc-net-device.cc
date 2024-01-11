/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
* Copyright (c) 2006 Georgia Tech Research Corporation, INRIA
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation;
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Author: Yuliang Li <yuliangli@g.harvard.com>
*/

#define __STDC_LIMIT_MACROS 1
#include <stdint.h>
#include <stdio.h>
//#include "ns3/enc-net-device.h"
#include "enc-net-device.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"
#include "ns3/data-rate.h"
#include "ns3/object-vector.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/assert.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/simulator.h"
#include "ns3/point-to-point-channel.h"
//#include "ns3/enc-channel.h"
#include "enc-channel.h"
#include "ns3/flow-id-tag.h"
#include "ns3/error-model.h"
#include "ns3/ppp-header.h"
#include "ns3/udp-header.h"
#include "ns3/seq-ts-header.h"
#include "ns3/pointer.h"
//#include "ns3/custom-header.h"
#include "ns3/custom-header-niux.h"

#include <iostream>

NS_LOG_COMPONENT_DEFINE("EncNetDevice");

namespace ns3 {
	
	/******************
	 * EncNetDevice
	 *****************/
	NS_OBJECT_ENSURE_REGISTERED(EncNetDevice);

	TypeId
		EncNetDevice::GetTypeId(void)
	{
		static TypeId tid = TypeId("ns3::EncNetDevice")
			.SetParent<PointToPointNetDevice>()
			.AddConstructor<EncNetDevice>()
			.AddAttribute ("TxBeQueue<Packet>", 
					"A queue to use as the transmit queue in the device.",
					PointerValue (),
					MakePointerAccessor (&EncNetDevice::m_queue),
					MakePointerChecker<Queue> ())
			.AddTraceSource ("EncEnqueue", "Enqueue a packet in the EncNetDevice.",
					MakeTraceSourceAccessor (&EncNetDevice::m_traceEnqueue))
			.AddTraceSource ("EncDequeue", "Dequeue a packet in the EncNetDevice.",
					MakeTraceSourceAccessor (&EncNetDevice::m_traceDequeue))
			.AddTraceSource ("EncDrop", "Drop a packet in the EncNetDevice.",
					MakeTraceSourceAccessor (&EncNetDevice::m_traceDrop))
			;

		return tid;
	}

	EncNetDevice::EncNetDevice()
	{
		NS_LOG_FUNCTION(this);
		m_ecn_source = new std::vector<ECNAccount>;
	}

	EncNetDevice::~EncNetDevice()
	{
		NS_LOG_FUNCTION(this);
	}

	void
		EncNetDevice::DoDispose()
	{
		NS_LOG_FUNCTION(this);

		PointToPointNetDevice::DoDispose();
	}

	void
		EncNetDevice::TransmitComplete(void)
	{
		NS_LOG_FUNCTION(this);
		NS_ASSERT_MSG(m_txMachineState == BUSY, "Must be BUSY if transmitting");
		m_txMachineState = READY;
		NS_ASSERT_MSG(m_currentPkt != 0, "EncNetDevice::TransmitComplete(): m_currentPkt zero");
		m_phyTxEndTrace(m_currentPkt);
		m_currentPkt = 0;
		DequeueAndTransmit();
	}

	void
		EncNetDevice::DequeueAndTransmit(void)
	{
		NS_LOG_FUNCTION(this);
		if (!m_linkUp) return; // if link is down, return
		if (m_txMachineState == BUSY) return;	// Quit if channel busy
		Ptr<Packet> p;
		//switch
		p = m_queue->DequeueRR(m_paused);		//this is round-robin
		if (p != 0){
			m_snifferTrace(p);
			m_promiscSnifferTrace(p);
			Ipv4Header h;
			Ptr<Packet> packet = p->Copy();
			uint16_t protocol = 0;
			ProcessHeader(packet, protocol);
			packet->RemoveHeader(h);
			FlowIdTag t;
			uint32_t qIndex = m_queue->GetLastQueue();
			//m_node->SwitchNotifyDequeue(m_ifIndex, qIndex, p);
			p->RemovePacketTag(t);
			m_traceDequeue(p, qIndex);
			TransmitStart(p);
			return;
		}else{ //No queue can deliver any packet
			NS_LOG_INFO("PAUSE prohibits send at node " << m_node->GetId());
		}
		return;
	}

	void
		EncNetDevice::Resume(unsigned qIndex)
	{
		NS_LOG_FUNCTION(this << qIndex);
		NS_ASSERT_MSG(m_paused[qIndex], "Must be PAUSEd");
		m_paused[qIndex] = false;
		NS_LOG_INFO("Node " << m_node->GetId() << " dev " << m_ifIndex << " queue " << qIndex <<
			" resumed at " << Simulator::Now().GetSeconds());
		DequeueAndTransmit();
	}

	void
		EncNetDevice::Receive(Ptr<Packet> packet)
	{
		NS_LOG_FUNCTION(this << packet);
		if (!m_linkUp){
			m_traceDrop(packet, 0);
			return;
		}

		if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt(packet))
		{
			// 
			// If we have an error model and it indicates that it is time to lose a
			// corrupted packet, don't forward this packet up, let it go.
			//
			m_phyRxDropTrace(packet);
			return;
		}

		m_macRxTrace(packet);
		MyCustomHeader ch(MyCustomHeader::L2_Header | MyCustomHeader::L3_Header | MyCustomHeader::L4_Header);
		ch.getInt = 1; // parse INT header
		packet->PeekHeader(ch);

		//if (m_node->GetNodeType() > 0){ // switch
			packet->AddPacketTag(FlowIdTag(m_ifIndex));
			m_node->SwitchReceiveFromDevice(this, packet, ch);
		//}
		return;
	}

	bool
		EncNetDevice::Attach(Ptr<EncChannel> ch)
	{
		NS_LOG_FUNCTION(this << &ch);
		m_channel = ch;
		m_channel->Attach(this);
		NotifyLinkUp();
		return true;
	}

	bool
		EncNetDevice::TransmitStart(Ptr<Packet> p)
	{
		NS_LOG_FUNCTION(this << p);
		NS_LOG_LOGIC("UID is " << p->GetUid() << ")");
		//
		// This function is called to start the process of transmitting a packet.
		// We need to tell the channel that we've started wiggling the wire and
		// schedule an event that will be executed when the transmission is complete.
		//
		NS_ASSERT_MSG(m_txMachineState == READY, "Must be READY to transmit");
		m_txMachineState = BUSY;
		m_currentPkt = p;
		m_phyTxBeginTrace(m_currentPkt);
		Time txTime = Seconds(m_bps.CalculateTxTime(p->GetSize()));
		Time txCompleteTime = txTime + m_tInterframeGap;
		NS_LOG_LOGIC("Schedule TransmitCompleteEvent in " << txCompleteTime.GetSeconds() << "sec");
		Simulator::Schedule(txCompleteTime, &EncNetDevice::TransmitComplete, this);

		bool result = m_channel->TransmitStart(p, this, txTime);
		if (result == false)
		{
			m_phyTxDropTrace(p);
		}
		return result;
	}

	bool EncNetDevice::SwitchSend (uint32_t qIndex, Ptr<Packet> packet, MyCustomHeader &ch){
		m_macTxTrace(packet);
		m_traceEnqueue(packet, qIndex);
		m_queue->Enqueue(packet, qIndex);
		DequeueAndTransmit();
		return true;
	}

	Ptr<Channel>
		EncNetDevice::GetChannel(void) const
	{
		return m_channel;
	}

   bool EncNetDevice::IsEnc(void) const{
	   return true;
   }

   void EncNetDevice::TriggerTransmit(void){
	   DequeueAndTransmit();
   }

	void EncNetDevice::SetQueue(Ptr<BEgressQueue> q){
		NS_LOG_FUNCTION(this << q);
		m_queue = q;
	}

	Ptr<BEgressQueue> EncNetDevice::GetQueue(){
		return m_queue;
	}

	void EncNetDevice::TakeDown(){
		// TODO: delete packets in the queue, set link down
		// switch
		// clean the queue
		for (uint32_t i = 0; i < qCnt; i++) {
			m_paused[i] = false;
			while (1){
				Ptr<Packet> p = m_queue->DequeueRR(m_paused);
				if (p == 0)
					 break;
				m_traceDrop(p, m_queue->GetLastQueue());
			}
			// TODO: Notify switch that this link is down
		}
		m_linkUp = false;
	}

	void EncNetDevice::UpdateNextAvail(Time t){
		if (!m_nextSend.IsExpired() && t.GetTimeStep() < m_nextSend.GetTs()){
			Simulator::Cancel(m_nextSend);
			Time delta = t < Simulator::Now() ? Time(0) : t - Simulator::Now();
			m_nextSend = Simulator::Schedule(delta, &EncNetDevice::DequeueAndTransmit, this);
		}
	}
} // namespace ns3
