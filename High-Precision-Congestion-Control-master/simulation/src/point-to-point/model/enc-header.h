//yibo

#ifndef ENC_HEADER_H
#define ENC_HEADER_H

#include <stdint.h>
#include "ns3/header.h"
#include "ns3/buffer.h"
//#include "ns3/int-header-niux.h"
#include "ns3/int-header-niux.h"

namespace ns3 {

/**
 * \ingroup Pause
 * \brief Header for the Congestion Notification Message
 *
 * This class has two fields: The five-tuple flow id and the quantized
 * congestion level. This can be serialized to or deserialzed from a byte
 * buffer.
 */
 
class encHeader : public Header
{
public:
 
  enum {
	  FLAG_CNP = 0
  };
  encHeader (uint16_t pg);
  encHeader ();
  virtual ~encHeader ();

//Setters
  /**
   * \param pg The PG
   */
  void SetPG (uint16_t pg);
  void SetSeq(uint32_t seq);
  void SetFlags(uint16_t _flags);
  void SetSport(uint32_t _sport);
  void SetDport(uint32_t _dport);
  void SetMyIntHeader(const MyIntHeader &_ih);

//Getters
  /**
   * \return The pg
   */
  uint16_t GetPG () const;
  uint32_t GetSeq() const;
  uint16_t GetFlags() const;
  uint16_t GetSport() const;
  uint16_t GetDport() const;

  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  static uint32_t GetBaseSize(); // size without INT

private:
  uint16_t sport, dport;
  uint16_t flags;
  uint16_t m_pg;
  uint32_t m_seq; // the sequence number.
  MyIntHeader ih;
  
};

}; // namespace ns3

#endif /* QBB_HEADER */
