#include <iostream>
#include <fstream>
#include <ns3/flow-monitor-helper.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("CsmaMulticastExample");

int main(int argc, char *argv[])
{
       LogComponentEnable("CsmaMulticastExample", LOG_LEVEL_INFO);
       Config::SetDefault("ns3::CsmaNetDevice::EncapsulationMode", StringValue("Dix")); //封装
       CommandLine cmd;
       cmd.Parse(argc, argv);

       // 创建节点
       NS_LOG_INFO("Create nodes.");
       NodeContainer c;
       c.Create(7);
       NodeContainer c0 = NodeContainer(c.Get (0), c.Get (1), c.Get (2), c.Get (3));
       NodeContainer c1 = NodeContainer(c.Get (3), c.Get (4), c.Get (5), c.Get (6));
 
       // 网络拓扑
       NS_LOG_INFO("Build Topology.");
       CsmaHelper csma;
       csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(5000000)));
       csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));
       NetDeviceContainer nd0 = csma.Install(c0); 
       NetDeviceContainer nd1 = csma.Install(c1);

       // 协议栈
       NS_LOG_INFO("Add IP Stack.");
       InternetStackHelper internet;
       internet.Install(c);

       // IP地址
       NS_LOG_INFO("Assign IP Addresses.");
       Ipv4AddressHelper ipv4Addr;
       ipv4Addr.SetBase("10.1.1.0", "255.255.255.0");
       ipv4Addr.Assign(nd0);
       ipv4Addr.SetBase("10.1.2.0", "255.255.255.0");
       ipv4Addr.Assign(nd1);
       Ipv4Address multicastSource("10.1.1.1");
       Ipv4Address multicastGroup("225.1.2.4");
 
       // 路由
       NS_LOG_INFO("Configure multicasting.");
       Ipv4StaticRoutingHelper multicast;
       Ptr<Node> multicastRouter = c.Get(3); 
       Ptr<NetDevice> inputIf = nd0.Get(3); 
       NetDeviceContainer outputDevices;
       outputDevices.Add(nd1.Get(0));
       multicast.AddMulticastRoute(multicastRouter, multicastSource, multicastGroup, inputIf, outputDevices);
       Ptr<Node> sender = c.Get(0); 
       Ptr<NetDevice> senderIf = nd0.Get(0);
       multicast.SetDefaultMulticastRoute (sender, senderIf);
       //Ptr<Node> sender1 = c.Get(3);
       //Ptr<NetDevice> senderIf1 = nd1.Get(3);
       //multicast.SetDefaultMulticastRoute (sender, senderIf);

       // 应用
       NS_LOG_INFO("Create Applications.");
       uint16_t multicastPort = 9;
       OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(multicastGroup, multicastPort)));
       onoff.SetConstantRate(DataRate("255b/s"));
       onoff.SetAttribute("PacketSize", UintegerValue(128));
       ApplicationContainer app = onoff.Install(c0.Get(0));
       app.Start(Seconds (1.));
       app.Stop(Seconds (10.));
       PacketSinkHelper sink("ns3::UdpSocketFactory", 
       InetSocketAddress(Ipv4Address::GetAny(), multicastPort)); 
       ApplicationContainer sinkC = sink.Install(c1.Get(3));
       sinkC.Start(Seconds (1.0));
       sinkC.Stop(Seconds (12.0));

       // 跟踪
       NS_LOG_INFO("Configure Tracing.");
       AsciiTraceHelper ascii;
       csma.EnableAsciiAll(ascii.CreateFileStream("csma.tr"));
       csma.EnablePcapAll("csma", true);
 
       // 启动
       NS_LOG_INFO("Run Simulation.");
       Simulator::Run();
       Simulator::Destroy();
       NS_LOG_INFO("Done.");
}