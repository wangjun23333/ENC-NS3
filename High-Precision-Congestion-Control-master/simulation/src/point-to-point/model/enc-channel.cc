/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007, 2008 University of Washington
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
 */

#include "enc-channel.h"
#include "enc-net-device.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <iostream>
#include <fstream>

NS_LOG_COMPONENT_DEFINE ("EncChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (EncChannel);

TypeId 
EncChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EncChannel")
    .SetParent<Channel> ()
    .AddConstructor<EncChannel> ()
    .AddAttribute ("Delay", "Transmission delay through the channel",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&EncChannel::m_delay),
                   MakeTimeChecker ())
    .AddTraceSource ("TxRxEnc",
                     "Trace source indicating transmission of packet from the EncChannel, used by the Animation interface.",
                     MakeTraceSourceAccessor (&EncChannel::m_txrxEnc))
  ;
  return tid;
}

//
// By default, you get a channel that 
// has an "infitely" fast transmission speed and zero delay.
EncChannel::EncChannel()
  :
    PointToPointChannel ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_nDevices = 0;
}

void
EncChannel::Attach (Ptr<EncNetDevice> device)
{
  
  //std::cout << m_nDevices << " " << N_DEVICES << "\n";
  //fflush(stdout);
  NS_LOG_FUNCTION (this << device);
  NS_ASSERT_MSG (m_nDevices < N_DEVICES, "Only two devices permitted");
  NS_ASSERT (device != 0);

  m_link[m_nDevices++].m_src = device;
//
// If we have both devices connected to the channel, then finish introducing
// the two halves and set the links to IDLE.
//
  if (m_nDevices == N_DEVICES)
    {
      m_link[0].m_dst = m_link[1].m_src;
      m_link[1].m_dst = m_link[0].m_src;
      m_link[0].m_state = IDLE;
      m_link[1].m_state = IDLE;
    }


}

bool
EncChannel::TransmitStart (
  Ptr<Packet> p,
  Ptr<EncNetDevice> src,
  Time txTime)
{
  NS_LOG_FUNCTION (this << p << src);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  NS_ASSERT (m_link[0].m_state != INITIALIZING);
  NS_ASSERT (m_link[1].m_state != INITIALIZING);

  uint32_t wire = src == m_link[0].m_src ? 0 : 1;

  Simulator::ScheduleWithContext (m_link[wire].m_dst->GetNode ()->GetId (),
                                  txTime + m_delay, &EncNetDevice::Receive,
                                  m_link[wire].m_dst, p);

  // Call the tx anim callback on the net device
  m_txrxEnc (p, src, m_link[wire].m_dst, txTime, txTime + m_delay);
  return true;
}

uint32_t 
EncChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  //std::cout<<m_nDevices<<"\n";
  //std::cout.flush();


  return m_nDevices;
}

Ptr<EncNetDevice>
EncChannel::GetEncDevice (uint32_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (i < 2);
  return m_link[i].m_src;
}

Ptr<NetDevice>
EncChannel::GetDevice (uint32_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return GetEncDevice (i);
}

Time
EncChannel::GetDelay (void) const
{
  return m_delay;
}

Ptr<EncNetDevice>
EncChannel::GetSource (uint32_t i) const
{
  return m_link[i].m_src;
}

Ptr<EncNetDevice>
EncChannel::GetDestination (uint32_t i) const
{
  return m_link[i].m_dst;
}

bool
EncChannel::IsInitialized (void) const
{
  NS_ASSERT (m_link[0].m_state != INITIALIZING);
  NS_ASSERT (m_link[1].m_state != INITIALIZING);
  return true;
}

} // namespace ns3
