/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef CUSTOM_HEADER_NIUX_H
#define CUSTOM_HEADER_NIUX_H

#include "ns3/header.h"
#include "int-header-niux.h"//#include "ns3/int-header-niux.h"

namespace ns3 {
/**
 * \ingroup ipv4
 *
 * \brief Custom packet header
 */
class MyCustomHeader : public Header 
{
public:
  /**
   * \brief Construct a null custom header
   */
  MyCustomHeader ();
  MyCustomHeader (uint32_t _headerType);
  /**
   * \enum EcnType
   * \brief ECN Type defined in \RFC{3168}
   */

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  uint32_t brief, headerType, getInt;
  enum HeaderType{
	L2_Header = 1,
	L3_Header = 2,
	L4_Header = 4
  };

	// ppp header
  uint16_t pppProto;

  // IPv4 header
  enum FlagsE {
    DONT_FRAGMENT = (1<<0),
    MORE_FRAGMENTS = (1<<1)
  };
  uint16_t m_payloadSize; //!< payload size
  uint16_t ipid; //!< identification
  uint32_t m_tos : 8; //!< TOS, also used as DSCP + ECN value
  uint32_t m_ttl : 8; //!< TTL
  uint32_t l3Prot: 8;  //!< Protocol
  uint32_t ipv4Flags : 3; //!< flags
  uint16_t m_fragmentOffset;  //!< Fragment offset
  uint32_t sip; //!< source address
  uint32_t dip; //!< destination address
  uint16_t m_checksum; //!< checksum
  uint16_t m_headerSize; //!< IP header size

  union {
	  struct {
		  uint16_t sport;        //!< Source port
		  uint16_t dport;   //!< Destination port
		  uint32_t seq;  //!< Sequence number
		  uint32_t ack;       //!< ACK number
		  uint8_t length;             //!< Length (really a uint4_t) in words.
		  uint8_t tcpFlags;              //!< Flags (really a uint6_t)
		  uint16_t windowSize;        //!< Window size
			// checksum skiped
		  uint16_t urgentPointer;     //!< Urgent pointer

		  uint8_t optionBuf[32];     // buffer for storing raw options
      uint16_t ih_pg;
	    uint32_t ih_seq;

			uint32_t ih_seq;
			uint16_t ih_pg;

			MyIntHeader ih;
	  } tcp;
	  struct {
		  uint16_t sport;        //!< Source port
		  uint16_t dport;   //!< Destination port
		  uint16_t payload_size;
		} udp;
    struct {
      uint16_t sport;
      uint16_t dport;
      uint16_t flags; // including isOwn
	    uint16_t pg;
	    uint32_t seq;
      MyIntHeader ih;
    } ack;
  };

  uint8_t GetIpv4EcnBits (void) const;

};

} // namespace ns3


#endif /* CUSTOM_HEADER_H */

