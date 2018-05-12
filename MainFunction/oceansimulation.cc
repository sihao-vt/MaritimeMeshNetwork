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
//void Rx(std::string context, const olsr::PacketHeader &header, const olsr::MessageList &messages){std::cout<<"Received"<<std::endl;}

//Callback function
//trace if routing table has changed
//void TableChange(std::string context, uint32_t size){std::cout<<"Topology Table Change "<<std::endl; }

//Callback function
//void UdpRx(std::string rx, Ptr<const Packet> packet)
//{static int count=0; std::cout<<"Tx "<<++count<<std::endl;}

//packet rate //antenna height 
int main (int argc,char *argv[])
{
  std::string m_filename = "src/mobility/model/ocean_data_1024_2000_12_0.10_610.txt";
  //std::string m_filename = "src/mobility/model/data.txt";
	std::string m_heightfile="src/mobility/model/ocean_clean_data_1024_2000_12_0.10_610.txt";
	//std::string m_heightfile="src/mobility/model/ocean_clean_data_9500_1024_2000_12_0.10_610.txt";
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
 
  uint32_t m_packetSize = 1150;
  uint32_t m_centerFreq = 600;
  uint32_t m_xSize=4;
  uint32_t m_ySize=4;
  double m_step=9950;
  double m_totalTime=60;
  double m_packetInterval=0.005;
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
  
  std::string phyMode="ErpOfdmRate12Mbps"; // if we use 802.11g
  Config::SetDefault("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue(phyMode)); 
	
  NodeContainer nodes;
  nodes.Create(m_xSize*m_ySize);

	WifiHelper wifi;
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
	//channel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(m_centerFreq*1000000.0));

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
  phy.Set("Frequency",UintegerValue(m_centerFreq));
  phy.Set("TxGain",DoubleValue(3));
  phy.Set("RxGain",DoubleValue(3));
  phy.SetChannel(channel.Create());
  
	WifiMacHelper mac;
	wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
			                         "DataMode", StringValue(phyMode),
															 "ControlMode", StringValue(phyMode));

  mac.SetType("ns3::AdhocWifiMac");

	NetDeviceContainer devices=wifi.Install(phy, mac, nodes);
  
  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX",DoubleValue(50.0),
                        				"MinY",DoubleValue(50.0),
				                        "DeltaX",DoubleValue(m_step),
				                        "DeltaY",DoubleValue(m_step),
				                        "GridWidth",UintegerValue(m_xSize),
				                        "LayoutType",StringValue("RowFirst"));
 
  mobility.SetMobilityModel("ns3::Ocean3dRandomWalk",
			                      "FileName",StringValue(m_heightfile),
			                      "MeshSize",UintegerValue(m_meshSize),
			                      "PatchSize",UintegerValue(m_patchSize),
			                      "TimeSteps",DoubleValue(m_timeStep),
		                        "Steps",UintegerValue(m_steps));
	//mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

  mobility.Install(nodes);

  if (m_pcap && m_oceanProtocol==true)
    phy.EnablePcapAll(std::string ("oceanProtocolEnable"));
  else if(m_pcap && m_oceanProtocol==false)
    phy.EnablePcapAll(std::string ("oceanProtocolDisable"));
 
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
  
	//onoff application
	/*Ipv4Address remoteaddress=interfaces.GetAddress(m_xSize*m_ySize-1);
	OnOffHelper onoff ("ns3::UdpSocketFactory", InetSocketAddress(remoteaddress, 9));
	onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1.0]"));
	onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
	onoff.SetAttribute("PacketSize", UintegerValue(m_packetSize));
	onoff.SetAttribute("DataRate", DataRateValue(m_packetSize*8/m_packetInterval));
	ApplicationContainer app1=onoff.Install(nodes.Get(0));
	app1.Start (Seconds(0.0));
	app1.Stop  (Seconds(m_totalTime));

	PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress(remoteaddress, 9));
	ApplicationContainer app2=sink.Install(nodes.Get(m_xSize*m_ySize-1));
	app2.Start (Seconds(0.0));
	app2.Stop  (Seconds(m_totalTime));*/

	//udp client application
  uint16_t port=4000;
	UdpServerHelper server(port);
	ApplicationContainer apps=server.Install(nodes.Get(m_xSize*m_ySize-1));
	apps.Start(Seconds(15.0));
	apps.Stop (Seconds(m_totalTime));

	Address serverAddress=Address(interfaces.GetAddress(m_xSize*m_ySize-1));
	UdpClientHelper client(serverAddress, port);
	client.SetAttribute("PacketSize", UintegerValue(m_packetSize));
	client.SetAttribute("MaxPackets", UintegerValue((uint32_t)(m_totalTime*(1/m_packetInterval))));
	client.SetAttribute("Interval", TimeValue(Seconds(m_packetInterval)));
	ApplicationContainer clientApps=client.Install(nodes.Get(0));
	clientApps.Start(Seconds(15.0));
	clientApps.Stop (Seconds(m_totalTime));

  Simulator::Stop(Seconds(m_totalTime));
  //AnimationInterface anim(m_animFile); // if AnimNet is necessary
	//Callback trace
  //std::string sender="/NodeList/*/$ns3::olsr::RoutingProtocol/Rx";
  //Config::Connect(sender,MakeCallback(&Rx));  
  //std::string tablechange="/NodeList/*/$ns3::olsr::RoutingProtocol/RoutingTableChanged";
	//Config::Connect(tablechange, MakeCallback(&TableChange));
	//td::string rx="NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::YansWifiPhy/PhyTxBegin";
	//Config::Connect(rx, MakeCallback(&UdpRx));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
