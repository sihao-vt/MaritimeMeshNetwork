#include <fstream>
#include "ocean-3d-random-walk.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <cmath>
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (Ocean3dRandomWalk);

TypeId
Ocean3dRandomWalk::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ocean3dRandomWalk")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<Ocean3dRandomWalk> ()
    .AddAttribute("FileName",
	          "The position of the file and file name.",
		StringValue("src/mobility/model/data.txt"),
		  MakeStringAccessor(&Ocean3dRandomWalk::m_filename),
		  MakeStringChecker())
    .AddAttribute("MeshSize",
	          "The meshsize of simulated wave area.",
		  UintegerValue(1024),
		  MakeUintegerAccessor (&Ocean3dRandomWalk::m_meshsize),
		  MakeUintegerChecker<uint64_t> ())			  
    .AddAttribute("PatchSize",
	        "The patchsiize of simulated wave area.",
		UintegerValue(2000),
		MakeUintegerAccessor (&Ocean3dRandomWalk::m_patchsize),
		MakeUintegerChecker<uint64_t> ())
    .AddAttribute("TimeSteps",
	          "Time step between two simulated ocean surface.",
		  DoubleValue(0.1),
		  MakeDoubleAccessor (&Ocean3dRandomWalk::m_timestep),
		  MakeDoubleChecker<double> ())
    .AddAttribute("Steps",
	          "How many ocean surface instances have been simulated.",
		  UintegerValue(600),
		  MakeUintegerAccessor (&Ocean3dRandomWalk::m_steps),
		  MakeUintegerChecker<uint64_t> ())
		  ;
  return tid;
}

void
Ocean3dRandomWalk::DoInitialize (void)
{
  InitPath();
  GetHeight();
}

void
Ocean3dRandomWalk::DoDispose (void)
{
  MobilityModel::DoDispose ();
}

Vector
Ocean3dRandomWalk::DoGetPosition (void) const
{
  return m_position;
}

void
Ocean3dRandomWalk::DoSetPosition (const Vector &position)
{
  m_position = position;
  NotifyCourseChange();
}

void
Ocean3dRandomWalk::GetHeight(void)
{
    Time simulation_time = Simulator::Now();
    double time=simulation_time.GetSeconds();
    uint16_t T=time/m_timestep;
    double result;

    result =(m_path[T]*((T+1)*m_timestep-time)+m_path[T+1]*(time - T * m_timestep))/m_timestep;

    m_position.z=result;
		//std::cout<<m_position.z<<std::endl;
    
    Time delay = Seconds(m_timestep);
    Simulator::Schedule(delay, &Ocean3dRandomWalk::GetHeight, this);

    NotifyCourseChange();
}

void
Ocean3dRandomWalk::InitPath(void)
{
    m_path.resize(m_steps);

    double xlength = m_position.x;
    double ylength = m_position.y;

    double xaxis=fmod(xlength, m_patchsize);
    double yaxis=fmod(ylength, m_patchsize);

	std::ifstream infile;
	int16_t nop=5;

	infile.open(m_filename,std::ifstream::in);

	double interval = (double) m_patchsize/(m_meshsize-1);
	uint16_t XAxis = xaxis / interval;
	uint16_t YAxis = yaxis / interval;
        //std::cout<<XAxis<<" "<<YAxis<<std::endl;
        uint32_t p1, p2, p3, p4;
	
	double tmp;
        for(uint16_t i=0;i<nop; i++)
	{
            infile>>tmp;
	}

	p1=YAxis*m_meshsize+XAxis;
	p2=YAxis*m_meshsize+XAxis+1;
	p3=(YAxis+1)*m_meshsize+XAxis;
	p4=(YAxis+1)*m_meshsize+XAxis+1;
        //std::cout<<p1<<p2<<p3<<p4<<std::endl;

	double h1, h2, h3, h4;
	for(uint16_t i=0;i<m_steps;i++)
	{
	  for(uint32_t j=0; j<m_meshsize*m_meshsize; j++)
	  {
	    if(j==p1-1) infile>>h1;
	    else if(j==p2-1) infile>>h2;
	    else if(j==p3-1) infile>>h3;
	    else if(j==p4-1) infile>>h4;
	    else infile>>tmp;
	  }
          double height=
	          (h1*((XAxis+1)*interval-xaxis)*((YAxis+1)*interval-yaxis)+
	          h3*((XAxis+1)*interval-xaxis)*(yaxis-YAxis*interval)+
		  h2*(xaxis-XAxis*interval)*((YAxis+1)*interval-yaxis)+
		  h4*(xaxis-XAxis*interval)*((yaxis-YAxis*interval)))
		  /(interval*interval);
	 // std::cout<<"  "<<height<<std::endl;
          m_path[i] = height;
	}

	infile.close();
}

Ocean3dRandomWalk::Ocean3dRandomWalk() {}
Ocean3dRandomWalk::~Ocean3dRandomWalk() {}

Vector
Ocean3dRandomWalk::DoGetVelocity(void) const
{ return Vector (0.0,0.0,0.0);}

double
Ocean3dRandomWalk::GetPredictedHeight(Time t) const
{
	Time simulation_time=Simulator::Now();
	double time=simulation_time.GetSeconds();
	double predict_time=time+t.GetSeconds();
	double result;
//	std::cout<<time<< " "<<predict_time<<std::endl; 
	uint16_t T=predict_time/m_timestep;
	
	result =(m_path[T]*((T+1)*m_timestep-time)+m_path[T+1]*(time - T * m_timestep))/m_timestep;

	return result;
}


}
