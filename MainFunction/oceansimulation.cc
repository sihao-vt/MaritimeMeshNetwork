#include"ns3/core-module.h"
#include"ns3/wifi-module.h"
#include"ns3/mobility-module.h"
#include"ns3/internet-module.h"
#include<string>
#include"ns3/ocean-olsr-helper.h"
#include"ns3/ipv4-static-routing-helper.h"
#include"ns3/ipv4-list-routing-helper.h"
#include"ns3/network-module.h"
#include"ns3/netanim-module.h"
#include"ns3/applications-module.h"
#include"ns3/olsr-module.h"
#include<iostream>
#include<sstream>
#include<fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OceanNetwork");

//Callback function
//trace tc/rx packet receive
void Rx(std::string context, const olsr::PacketHeader &header, const olsr::MessageList &messages){std::cout<<"Received"<<std::endl;}

//Callback function
//trace if routing table has changed
void TableChange(std::string context, uint32_t size)
{std::cout<<"Topology Table Change "<<std::endl; }


int main (int argc,char *argv[])
{
  //std::string m_filename = "src/mobility/model/ocean_data_1024_2000_15_0.10_610.txt";
  std::string m_filename = "src/mobility/model/ocean_data_1024_2000_12_0.10_610.txt";
  //std::string m_filename = "src/mobility/model/data.txt";
  uint16_t m_meshSize;
  uint16_t m_patchSize;
  double m_windSpeed;
  double m_timeStep;
  uint32_t m_steps;

  std::ifstream infile;//read some necessary arguments
  infile.open(m_filename,std::ifstream::in);
  if(!infile) NS_FATAL_ERROR("No such file!");
  infile>>m_meshSize;
  infile>>m_patchSize;
  infile>>m_windSpeed;
  infile>>m_timeStep;
  infile>>m_steps;
  infile.close();
 
  uint32_t m_packetSize = 1024;
  uint32_t m_centerFreq = 600;
  uint32_t m_xSize=4;
  uint32_t m_ySize=4;
  double m_step=9950;
  double m_totalTime=60;
  double m_packetInterval=0.001;
  bool m_oceanProtocol=false;
  //std::string m_animFile("OceanFile.xml");
  bool m_pcap = false;

  CommandLine cmd;
  cmd.AddValue("frequency","The center frequency(in whitespace band)",m_centerFreq);
  cmd.AddValue("packetsize","The size of packets", m_packetSize);
  cmd.AddValue("x-size","Number of nodes in a row grid", m_xSize);
  cmd.AddValue("y-size","Number of rows in grid",m_ySize);
  cmd.AddValue("step","Size of edge in our grid(meters)", m_step);
  cmd.AddValue("time", "Simulation time(sec)", m_totalTime);
  cmd.AddValue("packet-interval", "Interval between packets in UDP ping(sec)", m_packetInterval);
  cmd.AddValue("switch", "switch between ocean protocol or regular protocol", m_oceanProtocol);
  cmd.AddValue("pcap", "Enable PCAP traces on interfaces", m_pcap);
  cmd.Parse(argc,argv);
  

	//std::string phyMode="DsssRate11Mbps"; //if we use 802.11b 
  std::string phyMode="ErpOfdmRate12Mbps"; // if we use 802.11g
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode)); 

	
  NodeContainer nodes;
  nodes.Create(m_xSize*m_ySize);

	WifiHelper wifi;
	//wifi.SetStandard(WIFI_PHY_STANDARD_80211b); //802.11b
  wifi.SetStandard(WIFI_PHY_STANDARD_80211g); //802.11g

  YansWifiChannelHelper channel;
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss ("ns3::OceanPropagationModel",
			                        "PatchSize",UintegerValue(m_patchSize),
			                        "WindSpeed",DoubleValue(m_windSpeed),
	                            "MeshSize", UintegerValue(m_meshSize),
			                        "TimeSteps",DoubleValue(m_timeStep),
			                        "Steps", UintegerValue(m_steps),
			                        "FileName", StringValue(m_filename),
                              "Frequency",DoubleValue(m_centerFreq));

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
  phy.Set("Frequency",UintegerValue(m_centerFreq));
  phy.Set("TxGain",DoubleValue(12));
  phy.Set("RxGain",DoubleValue(20));
  phy.SetChannel(channel.Create());
  
	WifiMacHelper mac;
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
			                         "DataMode", StringValue(phyMode),
															 "ControlMode", StringValue(phyMode));

  mac.SetType("ns3::AdhocWifiMac");
	NetDeviceContainer devices=wifi.Install(phy, mac, nodes);
  
	//If we use 802.11n protocol(2.4 or 5GHz), we should use this statement to change
	//center frequency
	//Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::WifiPhy/Frequency", UintegerValue(600));
  
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX",DoubleValue(50.0),
                        				"MinY",DoubleValue(50.0),
				                        "DeltaX",DoubleValue(m_step),
				                        "DeltaY",DoubleValue(m_step),
				                        "GridWidth",UintegerValue(m_xSize),
				                        "LayoutType",StringValue("RowFirst"));
 
  mobility.SetMobilityModel("ns3::Ocean3dRandomWalk",
			                      "FileName",StringValue(m_filename),
			                      "MeshSize",UintegerValue(m_meshSize),
			                      "PatchSize",UintegerValue(m_patchSize),
			                      "TimeSteps",DoubleValue(m_timeStep),
		                        "Steps",UintegerValue(m_steps));
	//mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  mobility.Install(nodes);


  if (m_pcap && m_oceanProtocol==true)
    phy.EnablePcapAll(std::string ("oceanProtocolEnable0"));
  else if(m_pcap && m_oceanProtocol==false)
    phy.EnablePcapAll(std::string ("oceanProtocolDisable0"));
 
  if(m_oceanProtocol==true)
  {
    OceanOlsrHelper oceanOlsr;
    Ipv4StaticRoutingHelper staticRouting;
    Ipv4ListRoutingHelper list;
    list.Add(staticRouting, 0);
    list.Add(oceanOlsr, 100);
    InternetStackHelper internet;
    internet.SetRoutingHelper(list);
    internet.Install(nodes);
  }
  else
  {
    OlsrHelper olsr;
    Ipv4StaticRoutingHelper staticRouting;
    Ipv4ListRoutingHelper list;
    list.Add(staticRouting, 0);
    list.Add(olsr, 100);
    InternetStackHelper internet;
    internet.SetRoutingHelper(list);
    internet.Install(nodes);
  }

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0","255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  UdpEchoServerHelper echoServer(9);
  ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
  serverApps.Start(Seconds(0.0));
  serverApps.Stop(Seconds(m_totalTime));
  UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue((uint32_t)(m_totalTime*(1/m_packetInterval))));
  echoClient.SetAttribute("Interval", TimeValue(Seconds (m_packetInterval)));
  echoClient.SetAttribute("PacketSize", UintegerValue(m_packetSize));
  ApplicationContainer clientApps = echoClient.Install(nodes.Get(m_xSize*m_ySize-1));
  clientApps.Start(Seconds(0.0));
  clientApps.Stop (Seconds(m_totalTime));

  Simulator::Stop(Seconds(m_totalTime));
  //AnimationInterface anim(m_animFile); // if AnimNet is necessary
	//Callback trace
  //std::string sender="/NodeList/*/$ns3::olsr::RoutingProtocol/Rx";
  //Config::Connect(sender,MakeCallback(&Rx));  
  //std::string tablechange="/NodeList/*/$ns3::olsr::RoutingProtocol/RoutingTableChanged";
	//Config::Connect(tablechange, MakeCallback(&TableChange));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
