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
* Author: Yibo Zhu <yibzh@microsoft.com>
*/
#ifndef ENC_NET_DEVICE_H
#define ENC_NET_DEVICE_H

#include "ns3/point-to-point-net-device.h"
//#include "ns3/enc-channel.h"
#include "enc-channel.h"
#include "ns3/event-id.h"
//#include "ns3/broadcom-egress-queue.h"
#include "ns3/broadcom-egress-queue.h"
#include "ns3/custom-header-niux.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"
#include <vector>
#include<map>

namespace ns3 {

/**
 * \class EncNetDevice
 * \brief A Device for a IEEE 802.1 Network Link.
 */
class EncNetDevice : public PointToPointNetDevice 
{
public:
  static const uint32_t qCnt = 8;	// Number of queues/priorities used

  static TypeId GetTypeId (void);

  EncNetDevice ();
  virtual ~EncNetDevice ();

  /**
   * Receive a packet from a connected PointToPointChannel.
   *
   * This is to intercept the same call from the PointToPointNetDevice
   *
   * @see PointToPointNetDevice
   * @param p Ptr to the received packet.
   */
  virtual void Receive (Ptr<Packet> p);

  /**
   * Send a packet to the channel by putting it to the queue
   * of the corresponding priority class
   *
   * @param packet Ptr to the packet to send
   * @param dest Unused
   * @param protocolNumber Protocol used in packet
   */
  virtual bool SwitchSend (uint32_t qIndex, Ptr<Packet> packet, MyCustomHeader &ch);

  /**
   * Get the size of Tx buffer available in the device
   *
   * @return buffer available in bytes
   */
  //virtual uint32_t GetTxAvailable(unsigned) const;

  /**
   * TracedCallback hooks
   */
  void ConnectWithoutContext(const CallbackBase& callback);
  void DisconnectWithoutContext(const CallbackBase& callback);

  bool Attach (Ptr<EncChannel> ch);

  virtual Ptr<Channel> GetChannel (void) const;
  
  void SetQueue (Ptr<BEgressQueue> q);
  Ptr<BEgressQueue> GetQueue ();
  virtual bool IsEnc(void) const;
  void TriggerTransmit(void);

	TracedCallback<Ptr<const Packet>, uint32_t> m_traceEnqueue;
	TracedCallback<Ptr<const Packet>, uint32_t> m_traceDequeue;
	TracedCallback<Ptr<const Packet>, uint32_t> m_traceDrop;
protected:

  bool TransmitStart (Ptr<Packet> p);
  
  virtual void DoDispose(void);

  /// Reset the channel into READY state and try transmit again
  virtual void TransmitComplete(void);

  /// Look for an available packet and send it using TransmitStart(p)
  virtual void DequeueAndTransmit(void);

  /// Resume a paused queue and call DequeueAndTransmit()
  virtual void Resume(unsigned qIndex);

  /**
   * The queues for each priority class.
   * @see class Queue
   * @see class InfiniteQueue
   */
  Ptr<BEgressQueue> m_queue;

  Ptr<EncChannel> m_channel;

  bool m_paused[qCnt];	//< Whether a queue paused
  
  /* RP parameters */
  EventId  m_nextSend;		//< The next send event
  /* State variable for rate-limited queues */

  struct ECNAccount{
	  Ipv4Address source;
	  uint32_t qIndex;
	  uint32_t port;
	  uint8_t ecnbits;
	  uint16_t qfb;
	  uint16_t total;
  };

  std::vector<ECNAccount> *m_ecn_source;

public:
	void TakeDown(); // take down this device
	void UpdateNextAvail(Time t);
};

} // namespace ns3

#endif // ENC_NET_DEVICE_H
