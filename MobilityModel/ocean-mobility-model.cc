#include <fstream>
#include "ocean-mobility-model.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <cmath>
#include "ns3/uinteger.h"
#include "ns3/node.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (OceanMobilityModel);

TypeId
OceanMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::OceanMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<OceanMobilityModel> ()
    .AddAttribute("FileName",
	              "The position of the file and file name.",
		          StringValue("src/mobility/model/data.txt"),
		          MakeStringAccessor(&OceanMobilityModel::m_filename),
		          MakeStringChecker())
    .AddAttribute("MeshSize",
	              "The meshsize of simulated wave area.",
		          UintegerValue(1024),
		          MakeUintegerAccessor (&OceanMobilityModel::m_meshsize),
		          MakeUintegerChecker<uint64_t> ())			  
    .AddAttribute("PatchSize",
	              "The patchsiize of simulated wave area.",
		          UintegerValue(2000),
		          MakeUintegerAccessor (&OceanMobilityModel::m_patchsize),
		          MakeUintegerChecker<uint64_t> ())
    .AddAttribute("TimeSteps",
	              "Time step between two simulated ocean surface.",
		          DoubleValue(0.1),
		          MakeDoubleAccessor (&OceanMobilityModel::m_timestep),
		          MakeDoubleChecker<double> ())
    .AddAttribute("Steps",
	              "How many ocean surface instances have been simulated.",
		          UintegerValue(600),
		          MakeUintegerAccessor (&OceanMobilityModel::m_steps),
		          MakeUintegerChecker<uint64_t> ())
	;
  return tid;
}

void
OceanMobilityModel::DoInitialize (void)
{
  InitPath();
  GetHeight();
}

void
OceanMobilityModel::DoDispose (void)
{
  MobilityModel::DoDispose ();
}

Vector
OceanMobilityModel::DoGetPosition (void) const
{
  return m_position;
}

void
OceanMobilityModel::DoSetPosition (const Vector &position)
{
  m_position = position;
  NotifyCourseChange();
}

double
OceanMobilityModel::GetHeight(void)
{
    Time simulation_time = Simulator::Now();
    double time=simulation_time.GetSeconds();
    uint16_t T=time/m_timestep;
    double result;

    result =(m_path[T]*((T+1)*m_timestep-time)+m_path[T+1]*(time - T * m_timestep))/m_timestep;

    m_position.z=result;

    NotifyCourseChange();
		
	return result;
}

void
OceanMobilityModel::InitPath(void)
{
	std::ifstream infile;
	m_path.resize(m_steps);
	uint32_t index=this->GetObject<Node>()->GetId();
	uint16_t nop=5;
	infile.open(m_filename, std::ifstream::in);
	if(!infile) NS_FATAL_ERROR("No such file!");
	double temp;
	for(uint16_t i=0;i<nop;i++)
		infile>>temp;
	uint32_t pass=index*m_steps;
	for(uint32_t i=0;i<pass;i++)
		infile>>temp;
	for(uint32_t i=0;i<m_steps;i++)
	{
		infile>>m_path[i];
	}
	infile.close();
}

OceanMobilityModel::OceanMobilityModel() {}
OceanMobilityModel::~OceanMobilityModel() {}

Vector
OceanMobilityModel::DoGetVelocity(void) const
{ return Vector (0.0,0.0,0.0);}

double
OceanMobilityModel::GetPredictedHeight(Time t)
{
	Time simulation_time=Simulator::Now();
	double time=simulation_time.GetSeconds();
	double predict_time=time+t.GetSeconds();
	double result;
	uint16_t T=predict_time/m_timestep;
	
	result =(m_path[T]*((T+1)*m_timestep-predict_time)+m_path[T+1]*(predict_time - T * m_timestep))/m_timestep;

	return result;
}


}
