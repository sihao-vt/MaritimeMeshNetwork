#ifndef OCEAN_OLSR_HEADER_H
#define OCEAN_OLSR_HEADER_H

#include "olsr-header.h"


namespace ns3{
namespace olsr{
class OceanMessageHeader : public MessageHeader
{
private:
  double m_currentHeight;
  double m_predictHeight;

public:
  void SetCurrentHeight(double height)
  {m_currentHeight=height;}

  double GetCurrentHeight(void) const
  {return m_currentHeight;}

  void SetPredictHeight(double height)
  {m_predictHeight=height;}

  double GetPredictHeight(void) const
  {return m_predictHeight;}

  OceanMessageHeader();
  virtual ~OceanMessageHeader();
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  
	virtual uint32_t GetSerializedSize(void) const;
	virtual void Serialize(Buffer::Iterator start) const;
	virtual uint32_t Deserialize(Buffer::Iterator start);
};
  
  typedef std::vector<OceanMessageHeader> OceanMessageList;

}
}

#endif
