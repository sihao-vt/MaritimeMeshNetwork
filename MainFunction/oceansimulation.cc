#include"ns3/core-module.h"
#include"ns3/wifi-module.h"
#include"ns3/mobility-module.h"
#include"ns3/internet-module.h"
#include<string>
#include"ns3/ocean-olsr-helper.h"
#include"ns3/ipv4-static-routing-helper.h"
#include"ns3/ipv4-list-routing-helper.h"
#include"ns3/network-module.h"
#include"ns3/mesh-module.h"
#include"ns3/mesh-helper.h"
#include"ns3/netanim-module.h"
#include"ns3/applications-module.h"
#include"ns3/olsr-helper.h"
#include<iostream>
#include<sstream>
#include<fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OceanNetwork");

int main (int argc,char *argv[])
{
  std::string m_filename = "src/mobility/model/ocean_data_1024_2000_15_0.10_600.txt";
  //std::string m_filename = "src/mobility/model/data.txt";
  //std::string m_filename = "src/mobility/model/ocean_data_64_80_25_0.10_200.txt";
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
  uint32_t m_xSize=2;
  uint32_t m_ySize=1;
  double m_step=10000.0;
  double m_randomStart=0.1;
  double m_totalTime=40;
  double m_packetInterval=0.1;
  uint32_t m_nIfaces=1;
  bool m_chan=true;
  bool m_oceanProtocol=true;
  std::string m_stack("ns3::Dot11sStack");
  std::string m_root("ff:ff:ff:ff:ff:ff");
  std::string m_animFile("OceanFile.xml");
  bool m_pcap = true;

  CommandLine cmd;
  cmd.AddValue("frequency","The center frequency(in whitespace band)",m_centerFreq);
  cmd.AddValue("packetsize","The size of packets", m_packetSize);
  cmd.AddValue("x-size","Number of nodes in a row grid", m_xSize);
  cmd.AddValue("y-size","Number of rows in grid",m_ySize);
  cmd.AddValue("step","Size of edge in our grid(meters)", m_step);
  cmd.AddValue("start", "Maximum random start delay for beacon jitter(sec)", m_randomStart);
  cmd.AddValue("time", "Simulation time(sec)", m_totalTime);
  cmd.AddValue("packet-interval", "Interval between packets in UDP ping(sec)", m_packetInterval);
  cmd.AddValue("interfaces", "Number of radio interfaces used by each mesh point", m_nIfaces);
  cmd.AddValue("channels", "Use different frequency channel for different interfaces", m_chan);
  cmd.AddValue("stack", "Type of protocol stack. ns3::Dot11sStack by default", m_stack);
  cmd.AddValue("root", "Mac address of root mesh point in HWMP", m_root);
  cmd.AddValue("switch", "switch between ocean protocol or regular protocol", m_oceanProtocol);
  cmd.AddValue("pcap", "Enable PCAP traces on interfaces", m_pcap);
  cmd.Parse(argc,argv);

  NodeContainer node1;
  NodeContainer node2;
  node1.Create(1);
  node2.Create(1);
  NodeContainer nodes;
  nodes.Add(node1);
  nodes.Add(node2);
  //NodeContainer nodes;
  //nodes.Create(m_xSize*m_ySize);
  YansWifiChannelHelper channel;
  //channel=YansWifiChannelHelper::Default();
  channel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  channel.AddPropagationLoss ("ns3::OceanPropagationModel",
			      "PatchSize",UintegerValue(m_patchSize),
			      "WindSpeed",DoubleValue(m_windSpeed),
	                      "MeshSize", UintegerValue(m_meshSize),
			      "TimeSteps",DoubleValue(m_timeStep),
			      "Steps", UintegerValue(m_steps),
			      "FileName", StringValue(m_filename),
                              "Frequency",DoubleValue(m_centerFreq));//propagation model

  YansWifiPhyHelper phy = YansWifiPhyHelper::Default();
  phy.Set("Frequency",UintegerValue(m_centerFreq));//center frequency
  //phy.Set("ChannelWidth",UintegerValue(20));//Bandwidth
  //phy.Set("ChannelNumber",UintegerValue(20));//Number of Channel
  phy.Set("TxGain",DoubleValue(30));//Tx gain
  phy.Set("RxGain",DoubleValue(10));//Rx gain
  //phy.Set("ShortGuardEnabled",BooleanValue(true));
  //phy.Set("GreenfieldEnabled",BooleanValue(true));
  phy.SetChannel(channel.Create());
  
  MeshHelper mesh;
  mesh = MeshHelper::Default();
  if(!Mac48Address(m_root.c_str()).IsBroadcast())
  {
    mesh.SetStackInstaller(m_stack, "Root", Mac48AddressValue(Mac48Address(m_root.c_str())));
  }
  else
  {
    mesh.SetStackInstaller(m_stack);
  }

  if(m_chan)
  {
    mesh.SetSpreadInterfaceChannels(MeshHelper::SPREAD_CHANNELS);
  }
  else
  {
    mesh.SetSpreadInterfaceChannels(MeshHelper::ZERO_CHANNEL);
  }
  mesh.SetMacType("RandomStart", TimeValue(Seconds(m_randomStart)));
  mesh.SetNumberOfInterfaces(m_nIfaces);

  NetDeviceContainer devices=mesh.Install(phy,nodes);
  
  //Some attributes should be set after install function above, like CenterFrequency. All PHY layer be set later.
  //Config::Set("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/$ns3::WifiPhy/Frequency",UintegerValue(710));

  MobilityHelper mobility;
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX",DoubleValue(50.0),
				"MinY",DoubleValue(50.0),
				"DeltaX",DoubleValue(m_step),
				"DeltaY",DoubleValue(m_step),
				"GridWidth",UintegerValue(m_xSize),
				"LayoutType",StringValue("RowFirst"));
  
  mobility.SetMobilityModel("ns3::Ocean3dRandomWalk",
                            "Bounds",RectangleValue(Rectangle(-10,10,-10,10)),
			"Speed",StringValue("ns3::UniformRandomVariable[Min=0.0|Max=0.0]"),
			"FileName",StringValue(m_filename),
			"MeshSize",UintegerValue(m_meshSize),
			"PatchSize",UintegerValue(m_patchSize),
			"WindSpeed",DoubleValue(m_windSpeed),
			"TimeSteps",DoubleValue(m_timeStep),
		        "Steps",UintegerValue(m_steps));

  //mobility.Install(nodes);
  mobility.Install(node1);
  
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                "MinX",DoubleValue(2000.0),
				"MinY",DoubleValue(9800.0),
				"DeltaX",DoubleValue(m_step),
				"DeltaY",DoubleValue(m_step),
				"GridWidth",UintegerValue(m_xSize),
				"LayoutType",StringValue("RowFirst"));
  mobility.Install(node2);


  if (m_pcap)
    phy.EnablePcapAll(std::string ("oceanP"));
 
  if(m_oceanProtocol==true)
  {
    OceanOlsrHelper oceanOlsr;
    Ipv4StaticRoutingHelper staticRouting;
    Ipv4ListRoutingHelper list;
    list.Add(staticRouting, 0);
    list.Add(oceanOlsr, 10);
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
    list.Add(olsr, 10);
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
  UdpEchoClientHelper echoClient(interfaces.GetAddress(0), 9);//remote server ip and port number
  echoClient.SetAttribute("MaxPackets", UintegerValue((uint32_t)(m_totalTime*(1/m_packetInterval)    )));
  echoClient.SetAttribute("Interval", TimeValue(Seconds (m_packetInterval)));
  echoClient.SetAttribute("PacketSize", UintegerValue(m_packetSize));
  ApplicationContainer clientApps = echoClient.Install(nodes.Get(m_xSize*m_ySize-1));
  clientApps.Start(Seconds(0.0));
  clientApps.Stop (Seconds(m_totalTime));

  Simulator::Stop(Seconds(m_totalTime));
  //AnimationInterface anim(m_animFile);

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
