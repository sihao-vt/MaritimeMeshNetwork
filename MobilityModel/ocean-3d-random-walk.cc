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
	//This attribute is different from Bounds in random-walk-2d-mobility-model
	//(0.0 0.0) is position set at time 0;
	//The range should always be positive
    .AddAttribute ("Bounds",
                   "Bounds of the area to cruise.",
                   RectangleValue (Rectangle (0.0, 1.0, 0.0, 1.0)),
                   MakeRectangleAccessor (&Ocean3dRandomWalk::m_bounds),
                   MakeRectangleChecker ())
    .AddAttribute ("Time",
                   "Change current direction and speed after moving for this delay.",
                   TimeValue (Seconds (0.1)),
                   MakeTimeAccessor (&Ocean3dRandomWalk::m_time),
                   MakeTimeChecker ())
    .AddAttribute ("Direction",
                   "A random variable used to pick the direction (radians).",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6.283184]"),
                   MakePointerAccessor (&Ocean3dRandomWalk::m_direction),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Speed",
                   "A random variable used to pick the speed (m/s).",
                   StringValue ("ns3::UniformRandomVariable[Min=1|Max=2]"),
                   MakePointerAccessor (&Ocean3dRandomWalk::m_speed),
                   MakePointerChecker<RandomVariableStream> ())
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
	.AddAttribute("WindSpeed",
	              "Simulation windspeed.",
				  DoubleValue(0),
				  MakeDoubleAccessor (&Ocean3dRandomWalk::m_windspeed),
				  MakeDoubleChecker<double> ())
	.AddAttribute("TimeSteps",
	              "Time step between two simulated ocean surface.",
				  DoubleValue(0.02),
				  MakeDoubleAccessor (&Ocean3dRandomWalk::m_timestep),
				  MakeDoubleChecker<double> ())
	.AddAttribute("Steps",
	              "How many ocean surface instances have been simulated.",
				  UintegerValue(2000),
				  MakeUintegerAccessor (&Ocean3dRandomWalk::m_steps),
				  MakeUintegerChecker<uint64_t> ())
				  ;
  return tid;
}

void
Ocean3dRandomWalk::DoInitialize (void)
{
  InitPath();
  DoInitializePrivate ();
  MobilityModel::DoInitialize ();
}

void
Ocean3dRandomWalk::DoInitializePrivate (void)
{
  m_helper.Update ();
  double speed = m_speed->GetValue ();
  double direction = m_direction->GetValue ();
  Vector vector (std::cos (direction) * speed,
                 std::sin (direction) * speed,
                 0.0);
  
  Vector position = m_helper.GetCurrentPosition();
  position.z=GetHeight(Simulator::Now(),position.x,position.y);
  m_helper.SetPosition(position);
  m_helper.SetVelocity (vector);
  m_helper.Unpause ();
  
  Time delayLeft;
  delayLeft = m_time;
  DoWalk (delayLeft);
}

void
Ocean3dRandomWalk::DoWalk (Time delayLeft)
{
  Vector position = m_helper.GetCurrentPosition ();
  Vector speed = m_helper.GetVelocity ();
  Vector nextPosition = position;
  nextPosition.x += speed.x * delayLeft.GetSeconds ();
  nextPosition.y += speed.y * delayLeft.GetSeconds ();
  m_event.Cancel ();
  if (m_bounds.IsInside (nextPosition))
    {
      m_event = Simulator::Schedule (delayLeft, &Ocean3dRandomWalk::DoInitializePrivate, this);
    }
  else
    {
      nextPosition = m_bounds.CalculateIntersection (position, speed);
      Time delay = Seconds ((nextPosition.x - position.x) / speed.x);
      m_event = Simulator::Schedule (delay, &Ocean3dRandomWalk::Rebound, this,
                                     delayLeft - delay);
    }
 NotifyCourseChange ();
}

void
Ocean3dRandomWalk::Rebound (Time delayLeft)
{
  m_helper.UpdateWithBounds (m_bounds);
  Vector position = m_helper.GetCurrentPosition ();
  Vector speed = m_helper.GetVelocity ();
  switch (m_bounds.GetClosestSide (position))
    {
    case Rectangle::RIGHT:
    case Rectangle::LEFT:
      speed.x = -speed.x;
      break;
    case Rectangle::TOP:
    case Rectangle::BOTTOM:
      speed.y = -speed.y;
      break;
    }
  position.z=GetHeight(Simulator::Now(),position.x,position.y);
  m_helper.SetPosition(position);

  m_helper.SetVelocity (speed);
  m_helper.Unpause ();
  DoWalk (delayLeft);
}

void
Ocean3dRandomWalk::DoDispose (void)
{
  MobilityModel::DoDispose ();
}
Vector
Ocean3dRandomWalk::DoGetPosition (void) const
{
  m_helper.UpdateWithBounds (m_bounds);
  return m_helper.GetCurrentPosition ();
}
void
Ocean3dRandomWalk::DoSetPosition (const Vector &position)
{
  //Set the rectangle bounds for this mobility model
  m_bounds=Rectangle(position.x+m_bounds.xMin,position.x+m_bounds.xMax,position.y+m_bounds.yMin,position.y+m_bounds.yMax);
  NS_ASSERT_MSG((m_bounds.xMin>=0 && m_bounds.yMin>=0), "The sides of rectangle bounds should always be positive");
  NS_ASSERT (m_bounds.IsInside (position));
  m_helper.SetPosition (position);
  Simulator::Remove (m_event);
  m_event = Simulator::ScheduleNow (&Ocean3dRandomWalk::DoInitializePrivate, this);
}
Vector
Ocean3dRandomWalk::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}
int64_t
Ocean3dRandomWalk::DoAssignStreams (int64_t stream)
{
  m_speed->SetStream (stream);
  m_direction->SetStream (stream + 1);
  return 2;
}

double
Ocean3dRandomWalk::GetHeight(Time simulation_time, double xlength, double ylength)
{
    xlength=fmod(xlength,m_patchsize);
    ylength=fmod(ylength,m_patchsize);

	double time=simulation_time.GetSeconds();
	uint16_t T=time/m_timestep;
	double result;
	//std::ifstream infile;
	//infile.open(m_filename,std::ifstream::in);

	double interval=(double)m_patchsize/(m_meshsize-1);
	int16_t XAxis = xlength/interval;
	int16_t YAxis = ylength/interval;
	double P1,P2,P3,P4;
	//int16_t nop=5;//totally 5 parameter at the beginning of the file, we need to skip them.
    
	//Once we want to get a height in file, we need 4 points around it, because the position is discrete not continuous
	//And also time is also discrete, so to get a value in any position and time, we totally need 8 points.
	//uint32_t P11=T*m_meshsize*m_meshsize+YAxis*m_meshsize+XAxis+nop;
	//uint32_t P12=(T+1)*m_meshsize*m_meshsize+YAxis*m_meshsize+XAxis+nop;
	//uint32_t P21=T*m_meshsize*m_meshsize+YAxis*m_meshsize+(XAxis+1)+nop;
	//uint32_t P22=(T+1)*m_meshsize*m_meshsize+YAxis*m_meshsize+(XAxis+1)+nop;
	//uint32_t P31=T*m_meshsize*m_meshsize+(YAxis+1)*m_meshsize+XAxis+nop;
	//uint32_t P32=(T+1)*m_meshsize*m_meshsize+(YAxis+1)*m_meshsize+XAxis+nop;
	//uint32_t P41=T*m_meshsize*m_meshsize+(YAxis+1)*m_meshsize+(XAxis+1)+nop;
	//uint32_t P42=(T+1)*m_meshsize*m_meshsize+(YAxis+1)*m_meshsize+(XAxis+1)+nop;
    
	//double tmp;
	double v11,v12,v21,v22,v31,v32,v41,v42;
	v11=path[T][YAxis*m_meshsize+XAxis];
	v12=path[T+1][YAxis*m_meshsize+XAxis];
	v21=path[T][YAxis*m_meshsize+XAxis+1];
	v22=path[T+1][YAxis*m_meshsize+XAxis+1];
	v31=path[T][(YAxis+1)*m_meshsize+XAxis];
	v32=path[T+1][(YAxis+1)*m_meshsize+XAxis];
	v41=path[T][(YAxis+1)*m_meshsize+XAxis+1];
	v42=path[T+1][(YAxis+1)*m_meshsize+XAxis+1];

	/*for(uint32_t i=0;i<P42+1;i++)
	{
      if(i==P11) infile>>v11;
	  else if(i==P12) infile>>v12;
	  else if(i==P21) infile>>v21;
	  else if(i==P22) infile>>v22;
	  else if(i==P31) infile>>v31;
	  else if(i==P32) infile>>v32;
	  else if(i==P41) infile>>v41;
	  else if(i==P42) infile>>v42;
	  else infile>>tmp;
	}*/
    
	//interpolation once
	P1=(v11*(m_timestep*(T+1)-time)+v12*(time-m_timestep*T))/m_timestep;
	P2=(v21*(m_timestep*(T+1)-time)+v22*(time-m_timestep*T))/m_timestep;
	P3=(v31*(m_timestep*(T+1)-time)+v32*(time-m_timestep*T))/m_timestep;
	P4=(v41*(m_timestep*(T+1)-time)+v42*(time-m_timestep*T))/m_timestep;
    //std::cout<<"Position: "<<P1<<" "<<P2 << " "<<P3<<" "<<P4<<std::endl;

    //interpolation twice
    result=(P1*((XAxis+1)*interval-xlength)*((YAxis+1)*interval-ylength)+
	        P2*((XAxis+1)*interval-xlength)*(ylength-YAxis*interval)+
			P3*(xlength-XAxis*interval)*((YAxis+1)*interval-ylength)+
			P4*(xlength-XAxis*interval)*((ylength-YAxis*interval)))/(interval*interval);
    
	//infile.close();

    return result;
}

void
Ocean3dRandomWalk::InitPath(void)
{
    path.resize(m_steps);
	for(uint16_t i=0;i<m_steps;i++)
    path[i].resize(m_meshsize*m_meshsize);

	std::ifstream infile;
	double tmp;
	int16_t nop=5;

	infile.open(m_filename,std::ifstream::in);

	for(uint16_t n=0; n<nop; n++) 
		infile >> tmp;
											       

	for(uint16_t r=0; r<(m_steps); r++)
	{   
		for(uint32_t c=0; c<(m_meshsize*m_meshsize); c++)
		{   
		    infile >> path[r][c];
		}   
	}   
	infile.close();
}


}
