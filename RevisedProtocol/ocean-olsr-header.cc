#include"ocean-olsr-header.h"

namespace ns3{
namespace olsr{

NS_OBJECT_ENSURE_REGISTERED(OceanMessageHeader);

OceanMessageHeader::OceanMessageHeader()
{}

OceanMessageHeader::~OceanMessageHeader()
{}

TypeId
OceanMessageHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::olsr::OceanMessageHeader")
    .SetParent<MessageHeader>()
	.SetGroupName("Olsr")
	.AddConstructor<OceanMessageHeader>()
	;
	return tid;
}

TypeId
OceanMessageHeader::GetInstanceTypeId(void) const
{
  return GetTypeId();
}

}
}
