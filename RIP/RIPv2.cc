#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/animation-interface.h"
#include "ns3/netanim-module.h"
//        0    
//     B-----PC1
//    / \
//  2/   \1
//  A--3--C    PC2
//  |           |
// 4|          8|
//  D-----E-----F
//     5     7   
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RipSimpleRouting");

// 断开两个结点间的连接
void TearDownLink (Ptr<Node> nodeA, Ptr<Node> nodeB, uint32_t interfaceA, uint32_t interfaceB)
{
    nodeA->GetObject<Ipv4> ()->SetDown (interfaceA);
    nodeB->GetObject<Ipv4> ()->SetDown (interfaceB);
}

int main(int argc, char** argv)
{

    CommandLine cmd;
    cmd.Parse (argc, argv);
    Config::SetDefault ("ns3::Rip::SplitHorizon", EnumValue (RipNg::POISON_REVERSE));

    Ptr<Node> pc1 = CreateObject<Node>();
    Names::Add("Pc1Node", pc1);
    Ptr<Node> pc2 = CreateObject<Node>();
    Names::Add("Pc2Node", pc2);
    Ptr<Node> a = CreateObject<Node> ();
    Names::Add ("RouterA", a);
    Ptr<Node> b = CreateObject<Node> ();
    Names::Add ("RouterB", b);
    Ptr<Node> c = CreateObject<Node> ();
    Names::Add ("RouterC", c);
    Ptr<Node> d = CreateObject<Node> ();
    Names::Add ("RouterD", d);
    Ptr<Node> e = CreateObject<Node> ();
    Names::Add ("RouterE", e);
    Ptr<Node> f = CreateObject<Node> ();
    Names::Add ("RouterF", f);
    
    NodeContainer net1(pc1, b);
    NodeContainer net2(b, c);
    NodeContainer net3(a, b);
    NodeContainer net4(a, c);
    NodeContainer net5(a, d);
    NodeContainer net6(d, e);
    NodeContainer net7(c, e);
    NodeContainer net8(e, f);
    NodeContainer net9(f, pc2);
    NodeContainer routers(a, b, c, d, e);
    routers.Add(f);
    NodeContainer nodes(pc1, pc2);

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue (5000000));
    csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
    NetDeviceContainer ndc1 = csma.Install (net1);
    NetDeviceContainer ndc2 = csma.Install (net2);
    NetDeviceContainer ndc3 = csma.Install (net3);
    NetDeviceContainer ndc4 = csma.Install (net4);
    NetDeviceContainer ndc5 = csma.Install (net5);
    NetDeviceContainer ndc6 = csma.Install (net6);
    NetDeviceContainer ndc7 = csma.Install (net7);
    NetDeviceContainer ndc8 = csma.Install (net8);
    NetDeviceContainer ndc9 = csma.Install (net9);

    RipHelper ripRouting;
    ripRouting.ExcludeInterface(b, 1);
    ripRouting.ExcludeInterface(f, 2);
    ripRouting.SetInterfaceMetric(b, 2, 3);
    ripRouting.SetInterfaceMetric(c, 1, 3);

    Ipv4ListRoutingHelper listRH;
    listRH.Add(ripRouting, 0);

    InternetStackHelper internet;
    internet.SetIpv6StackInstall(false);
    internet.SetRoutingHelper(listRH);
    internet.Install(routers);

    InternetStackHelper internetNodes;
    internetNodes.SetIpv6StackInstall(false);
    internetNodes.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase(Ipv4Address("10.0.0.0"), Ipv4Mask("255.255.255.0"));
    Ipv4InterfaceContainer iic1 = ipv4.Assign(ndc1);
    ipv4.SetBase(Ipv4Address("10.0.1.0"), Ipv4Mask("255.255.255.0"));
    Ipv4InterfaceContainer iic2 = ipv4.Assign(ndc2);
    ipv4.SetBase(Ipv4Address("10.0.2.0"), Ipv4Mask("255.255.255.0"));
    Ipv4InterfaceContainer iic3 = ipv4.Assign(ndc3);
    ipv4.SetBase(Ipv4Address("10.0.3.0"), Ipv4Mask("255.255.255.0"));
    Ipv4InterfaceContainer iic4 = ipv4.Assign(ndc4);
    ipv4.SetBase(Ipv4Address("10.0.4.0"), Ipv4Mask("255.255.255.0"));
    Ipv4InterfaceContainer iic5 = ipv4.Assign(ndc5);
    ipv4.SetBase(Ipv4Address("10.0.5.0"), Ipv4Mask("255.255.255.0"));
    Ipv4InterfaceContainer iic6 = ipv4.Assign(ndc6);
    ipv4.SetBase(Ipv4Address("10.0.6.0"), Ipv4Mask("255.255.255.0"));
    Ipv4InterfaceContainer iic7 = ipv4.Assign(ndc7);
    ipv4.SetBase(Ipv4Address("10.0.7.0"), Ipv4Mask("255.255.255.0"));
    Ipv4InterfaceContainer iic8 = ipv4.Assign(ndc8);
    ipv4.SetBase(Ipv4Address("10.0.8.0"), Ipv4Mask("255.255.255.0"));
    Ipv4InterfaceContainer iic9 = ipv4.Assign(ndc9);

    Ptr<Ipv4StaticRouting> staticRouting;
    staticRouting = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting>(pc1->GetObject<Ipv4>()->GetRoutingProtocol());
    staticRouting->SetDefaultRoute("10.0.0.2", 1);
    staticRouting = Ipv4RoutingHelper::GetRouting<Ipv4StaticRouting>(pc2->GetObject<Ipv4>()->GetRoutingProtocol());
    staticRouting->SetDefaultRoute("10.0.8.1", 1);

    RipHelper routingHelper;
    Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>(&std::cout);
    float time_main = 0.0;
    float t = 0.0;
    for(int time_part = 0 ; time_part < 2; time_part++){
        time_main = float(29 * time_part);
        for(int time = 1; time < 10; time++){
            t = float(time);
            t += time_main;
            routingHelper.PrintRoutingTableAt(Seconds(t), a, routingStream);
            routingHelper.PrintRoutingTableAt(Seconds(t), b, routingStream);
            routingHelper.PrintRoutingTableAt(Seconds(t), c, routingStream);
            routingHelper.PrintRoutingTableAt(Seconds(t), d, routingStream);
            routingHelper.PrintRoutingTableAt(Seconds(t), e, routingStream);
            routingHelper.PrintRoutingTableAt(Seconds(t), f, routingStream);
        }
    }

    uint32_t packetSize = 1024;
    Time interPacketInterval = Seconds(1.0);
    V4PingHelper ping("10.0.8.2");

    ping.SetAttribute ("Interval", TimeValue (interPacketInterval));
    ping.SetAttribute ("Size", UintegerValue (packetSize));
    // ping的发起者pc1
    ApplicationContainer apps = ping.Install(pc1);
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(120.0));

    // 打开跟踪数据
    AsciiTraceHelper ascii;
    csma.EnableAsciiAll (ascii.CreateFileStream ("MyRip.tr"));
    csma.EnablePcapAll ("MyRip", true);

    // 定时断开C和E之间的连接
    Simulator::Schedule (Seconds (30), &TearDownLink, c, e, 3, 2);

    Simulator::Stop (Seconds (120.0));
    Simulator::Run ();
    Simulator::Destroy ();

}

