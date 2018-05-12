#include"ocean-olsr-header.h"

#define OCEAN_OLSR_MSG_HEADER_SIZE 16

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

uint32_t
OceanMessageHeader::GetSerializedSize(void) const
{
	uint32_t size=OCEAN_OLSR_MSG_HEADER_SIZE;
	switch (m_messageType)
	{
		case MID_MESSAGE:
			size+=m_message.mid.GetSerializedSize();
			break;
		case HELLO_MESSAGE:
			size += m_message.hello.GetSerializedSize (); 
		break;
		case TC_MESSAGE:
			size += m_message.tc.GetSerializedSize (); 
		break;
			case HNA_MESSAGE:
		size += m_message.hna.GetSerializedSize (); 
		break;
		default:
		  NS_ASSERT (false);
	}   
	  return size;
}


void
OceanMessageHeader::Serialize (Buffer::Iterator start) const
{
	Buffer::Iterator i=start;
	i.WriteU32 (uint32_t((m_predictHeight+20)*1000000));
	i.WriteU8 (m_messageType);
	i.WriteU8 (m_vTime);
	i.WriteHtonU16 (GetSerializedSize ());
	i.WriteHtonU32 (m_originatorAddress.Get ());
	i.WriteU8 (m_timeToLive);
	i.WriteU8 (m_hopCount);
	i.WriteHtonU16 (m_messageSequenceNumber);

	switch (m_messageType)
	{
		case MID_MESSAGE:
				m_message.mid.Serialize (i);
		break;
		case HELLO_MESSAGE:
				m_message.hello.Serialize (i);
		break;
		case TC_MESSAGE:
		m_message.tc.Serialize (i);
		break;
		case HNA_MESSAGE:
			m_message.hna.Serialize (i);
		break;
		default:
			NS_ASSERT (false);
	}

}



uint32_t
OceanMessageHeader::Deserialize (Buffer::Iterator start)
{
	uint32_t size;
	Buffer::Iterator i =start;
	uint32_t temp =i.ReadU32 ();
	int64_t tmp=temp;
	m_predictHeight = double(tmp)/1000000-20;
	m_messageType  = (MessageType) i.ReadU8 ();
	NS_ASSERT (m_messageType >= HELLO_MESSAGE && m_messageType <= HNA_MESSAGE);
	m_vTime  = i.ReadU8 ();
	m_messageSize  = i.ReadNtohU16 ();
	m_originatorAddress = Ipv4Address (i.ReadNtohU32 ());
	m_timeToLive  = i.ReadU8 ();
	m_hopCount  = i.ReadU8 ();
	m_messageSequenceNumber = i.ReadNtohU16 ();
	size = OCEAN_OLSR_MSG_HEADER_SIZE;
	switch (m_messageType)
	{
		case MID_MESSAGE:
			size += m_message.mid.Deserialize (i, m_messageSize - OCEAN_OLSR_MSG_HEADER_SIZE);
		break;
		case HELLO_MESSAGE:
		size += m_message.hello.Deserialize (i, m_messageSize - OCEAN_OLSR_MSG_HEADER_SIZE);
		break;
		case TC_MESSAGE:
			size += m_message.tc.Deserialize (i, m_messageSize - OCEAN_OLSR_MSG_HEADER_SIZE);
		break;
		case HNA_MESSAGE:
			size += m_message.hna.Deserialize (i, m_messageSize - OCEAN_OLSR_MSG_HEADER_SIZE);
		break;
		default:
      NS_ASSERT (false);
	}
		return size;
}


}
}
