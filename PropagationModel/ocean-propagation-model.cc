#include <fstream>
#include"ocean-propagation-model.h"
#include"ns3/log.h"
#include"ns3/mobility-model.h"
#include"ns3/boolean.h"
#include"ns3/double.h"
#include"ns3/string.h"
#include"ns3/simulator.h"
#include"ns3/pointer.h"
#include<cmath>
#include"ns3/uinteger.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(OceanPropagationModel);

TypeId
OceanPropagationModel::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::OceanPropagationModel")
    .SetParent<PropagationLossModel>()
	.SetGroupName("Propagation")
	.AddConstructor<OceanPropagationModel>()
	.AddAttribute("PatchSize",
	              "The patchsize of simulation(m).",
				  UintegerValue(2000),
				  MakeUintegerAccessor(& OceanPropagationModel::m_patchSize),
				  MakeUintegerChecker<uint32_t>())
    .AddAttribute("MeshSize",
	              "The meshsize of simulated wave area.",
				  UintegerValue(1024),
				  MakeUintegerAccessor (&OceanPropagationModel::m_meshSize),
				  MakeUintegerChecker<uint64_t> ())			  
	.AddAttribute("TimeSteps",
	              "Time step between two simulated ocean surface.",
				  DoubleValue(0.02),
				  MakeDoubleAccessor (&OceanPropagationModel::m_timeStep),
				  MakeDoubleChecker<double> ())
	.AddAttribute("FileName",
	              "The position of the file and file name.",
				  StringValue("src/mobility/model/data.txt"),
				  MakeStringAccessor(&OceanPropagationModel::m_filename),
				  MakeStringChecker())
	.AddAttribute("WindSpeed",
	              "The height of simulated ocean follows normal distribution, we need windspeed to calculate the variance of normal distribution",
				  DoubleValue(15),
				  MakeDoubleAccessor(& OceanPropagationModel::SetWindspeed,
				  &OceanPropagationModel::GetWindspeed),
				  MakeDoubleChecker<double>())
	.AddAttribute("Steps",
	              "How many times slots of simulation.",
				  UintegerValue(600),
				  MakeUintegerAccessor(&OceanPropagationModel::m_steps),
				  MakeUintegerChecker<uint32_t>())
	.AddAttribute("Frequency",
	              "The carrier frequency(in MHz).",
				  DoubleValue(714),
				  MakeDoubleAccessor(& OceanPropagationModel::SetFreq,
				  &OceanPropagationModel::GetFreq),
				  MakeDoubleChecker<double>())
	;
	return tid;
}

OceanPropagationModel::OceanPropagationModel(){}

void OceanPropagationModel::InitPath(){
    
	path.resize(m_steps);
    for(uint16_t i=0;i<m_steps;i++)
	  path[i].resize(m_meshSize*m_meshSize);

	std::ifstream infile;
    double tmp;
    int16_t nop=5;

	infile.open(m_filename,std::ifstream::in);

    for(uint16_t n=0; n<nop; n++) {
        infile >> tmp;
    }

    for(uint16_t r=0; r<(m_steps); r++)
    {
        for(uint32_t c=0; c<(m_meshSize*m_meshSize); c++)
        {
            infile >> path[r][c];
        }
    }
	infile.close();

}

void OceanPropagationModel::SetWindspeed(double windspeed)
{
  m_windSpeed=windspeed;
}

double OceanPropagationModel::GetWindspeed()const { return m_windSpeed;}

void OceanPropagationModel::SetFreq(double freq)
{
  m_freq=freq;
  static const double c = 299792458.0;
  m_lambda=c/m_freq/1e6;
  InitPath();
}
double OceanPropagationModel::GetFreq() const{return m_freq;}

double OceanPropagationModel::DbmToW(double dbm)const
{
  double mw=std::pow(10.0,dbm/10.0);
  return mw/1000.0;
}

double OceanPropagationModel::DbmFromW(double w) const
{
  double dbm = std::log10(w*1000.0)*10;
  return dbm;
}

int64_t OceanPropagationModel::DoAssignStreams(int64_t stream){return 0;}

double OceanPropagationModel::DoCalcRxPower(double txPowerDbm,
                                            Ptr<MobilityModel> a,
					    Ptr<MobilityModel> b) const
{
  //This is a two-ray model
  double distance=a->GetDistanceFrom(b);
  double antennaHeight=5.0;
  double txHeight=a->GetPosition().z+antennaHeight;
  double rxHeight=a->GetPosition().z+antennaHeight;
  if(distance<100)
  {
    double pathLoss=4*M_PI*std::sqrt(distance*distance+(rxHeight-txHeight)*(rxHeight-txHeight))/m_lambda;
	return txPowerDbm-20*std::log10(pathLoss);
  }
  else
  {
    double refangle = 0.0;
    double xref = b->GetPosition().x - a->GetPosition().x; 
    double yref = b->GetPosition().y - a->GetPosition().y; 
    //std::cout<<xref<<" "<<yref<<std::endl;
    if(xref == 0.0) {
       if(yref > 0) {refangle = M_PI/2;}
       else if(yref < 0){refangle = -M_PI/2;}
       else {NS_FATAL_ERROR("Two nodes are overlapped");}
    }
    else {
       refangle = std::atan(yref/xref);
       if((refangle >= 0.0) & (xref < 0) & (yref < 0)) refangle = refangle + M_PI;
       if((refangle <  0.0) & (xref < 0))              refangle = refangle + M_PI;
    }

    if(checkBlock(distance,txHeight,rxHeight,Simulator::Now(),a->GetPosition().x,a->GetPosition().y,refangle))
    {
      return -1000; 
    }
    else
    {
	  double d0=1;
      double EIRP_w=std::pow(10,txPowerDbm/10)/1000;
      double E0_Square=30*EIRP_w/(d0*d0);
      double d1=sqrt((txHeight-rxHeight)*(txHeight-rxHeight)+distance*distance);
      double d2=sqrt((txHeight+rxHeight)*(txHeight+rxHeight)+distance*distance);
      double delta_theta=2*M_PI*(d2-d1)/m_lambda;
      double ETOT_Square=E0_Square*(std::pow(d0/d1,2))+E0_Square*(std::pow(d0/d2,2))-
                       2*E0_Square*(d0/d1)*(d0/d2)*std::cos(delta_theta);
      double Pr_w=ETOT_Square*m_lambda*m_lambda/480/M_PI/M_PI;
      double Pr_dbm=10*std::log10(Pr_w*1000);
      //std::cout<<Pr_dbm<<std::endl;
      return Pr_dbm;
    }
  }
}

bool OceanPropagationModel::checkBlock(double distance, double txHeight, double rxHeight, Time simulation_time, 
                                       double xlength, double ylength, double refangle)const
{
	double v11,v12,v21,v22,v31,v32,v41,v42;
	double P1,P2,P3,P4,P_res;
	double time=simulation_time.GetSeconds();
	uint16_t T=time/m_timeStep;

  if(txHeight<=0 || rxHeight<=0) return true;//if any antenna is lower than 0, we assume the link is corrupted.
  
  const double radius=6371000;//earth radius
  double theta=distance/radius;
  
  double l_sq=(radius+txHeight)*(radius+txHeight)+(radius+rxHeight)*(radius+rxHeight)-2*std::cos(theta)*(radius+rxHeight)*(radius+txHeight);
  
  double phi1=std::acos((l_sq+(radius+txHeight)*(radius+txHeight)-(radius+rxHeight)*(radius+rxHeight))/2/sqrt(l_sq)/(radius+txHeight));
  
  double grid_size = (double)m_patchSize/(m_meshSize-1);
  xlength=fmod(xlength,m_patchSize);
  ylength=fmod(ylength,m_patchSize);
  
  uint16_t num=distance/grid_size;

  for(uint16_t i=0;i<num;i++)
  {
    double thetad=(i+1)*grid_size/radius;
	double h1=(radius+txHeight)*sin(phi1)/std::sin(M_PI-phi1-thetad)-radius;
    
    xlength=xlength+grid_size*(std::cos(refangle));
    ylength=ylength+grid_size*(std::sin(refangle));
    xlength=fmod(xlength<0?xlength+m_patchSize:xlength, m_patchSize);
    ylength=fmod(ylength<0?ylength+m_patchSize:ylength, m_patchSize);
    
    int16_t XAxis = xlength/grid_size;
    int16_t YAxis = ylength/grid_size;

    uint32_t c = (m_meshSize*YAxis)+XAxis; 
            v11 = path[T  ][c];
            v12 = path[T+1][c];
            v21 = path[T  ][c+1];
            v22 = path[T+1][c+1];
            v31 = path[T  ][m_meshSize+c];
            v32 = path[T+1][m_meshSize+c];
            v41 = path[T  ][m_meshSize+c+1];
            v42 = path[T+1][m_meshSize+c+1];

	P1=(v11*(m_timeStep*(T+1)-time)+v12*(time-m_timeStep*T))/m_timeStep;
	P2=(v21*(m_timeStep*(T+1)-time)+v22*(time-m_timeStep*T))/m_timeStep;
	P3=(v31*(m_timeStep*(T+1)-time)+v32*(time-m_timeStep*T))/m_timeStep;
	P4=(v41*(m_timeStep*(T+1)-time)+v42*(time-m_timeStep*T))/m_timeStep;
 
   P_res= (P1*((XAxis+1)*grid_size-xlength)*((YAxis+1)*grid_size-ylength)+
	   P3*((XAxis+1)*grid_size-xlength)*(ylength-YAxis*grid_size)+
	P2*(xlength-XAxis*grid_size)*((YAxis+1)*grid_size-ylength)+
	P4*(xlength-XAxis*grid_size)*((ylength-YAxis*grid_size)))
        /(grid_size*grid_size);
  
    if(P_res>h1){ 
        return true;
    }
  }
        
  return false;
}


}
