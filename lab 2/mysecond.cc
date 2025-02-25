/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
 
// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0
 
using namespace ns3;
 
NS_LOG_COMPONENT_DEFINE("SecondScriptExample");
 
int
main(int argc, char* argv[])
{
    bool verbose = true;
    uint32_t nCsma1 = 3;
    uint32_t nCsma2 = 3;
 
    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma1", "Number of \"extra\" CSMA1 nodes/devices", nCsma1);
    cmd.AddValue("nCsma2", "Number of \"extra\" CSMA2 nodes/devices", nCsma2);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
 
    cmd.Parse(argc, argv);
 
    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }
 
 
    NodeContainer p2pNodes;
    p2pNodes.Create(2);
 
    NodeContainer csmaNodes1, csmaNodes2;
    csmaNodes1.Add(p2pNodes.Get(0));
    csmaNodes1.Create(nCsma1);
 
    csmaNodes2.Add(p2pNodes.Get(1));
    csmaNodes2.Create(nCsma2);
 
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
 
    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);
 
    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));
 
    NetDeviceContainer csmaDevices1, csmaDevices2;
    csmaDevices1 = csma.Install(csmaNodes1);
    csmaDevices2 = csma.Install(csmaNodes2);
 
    InternetStackHelper stack;
    stack.Install(p2pNodes.Get(0));
    stack.Install(p2pNodes.Get(1));
    stack.Install(csmaNodes1);
    stack.Install(csmaNodes2);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);
 
    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces1;
    csmaInterfaces1 = address.Assign(csmaDevices1);
 
    address.SetBase("10.1.3.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces2;
    csmaInterfaces2 = address.Assign(csmaDevices2);
    UdpEchoServerHelper echoServer(21);
    ApplicationContainer serverApps = echoServer.Install(csmaNodes1.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));
 
    UdpEchoClientHelper echoClient(csmaInterfaces1.GetAddress(1), 21);
    echoClient.SetAttribute("MaxPackets", UintegerValue(2));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(3.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));
 
    ApplicationContainer clientApps = echoClient.Install(csmaNodes2.Get(1));
    clientApps.Start(Seconds(4.0));
    clientApps.Stop(Seconds(10.0));
 
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
 
    pointToPoint.EnablePcapAll("second", true);
    csma.EnablePcap("second", csmaDevices2.Get(1), true);
 
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
 

