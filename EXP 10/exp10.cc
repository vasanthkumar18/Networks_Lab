#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"

//Network Topology
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |
// n2   n1   n0   p0
//      srvr cli

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Exp10");

int main(int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi = 3;
  bool tracing = true;

  CommandLine cmd(__FILE__);
  cmd.AddValue("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue("tracing", "Enable pcap tracing", tracing);

  cmd.Parse(argc, argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
  {
    std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
    return 1;
  }

  if (verbose)
  {
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
  }
  Vector3D movingNodePos;
  movingNodePos.x = 130;
  movingNodePos.y = 100;
  movingNodePos.z = 0;
  int i = 30;
  while (i > 0)
  {
    NodeContainer wifiApNode; //Creating the access point Node
    wifiApNode.Create(1);

    NodeContainer wifiStaNodes; //Creating Wifi Station Nodes
    wifiStaNodes.Create(nWifi);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiHelper wifi;
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    WifiMacHelper mac;
    Ssid ssid = Ssid("ns-3-ssid");
    mac.SetType("ns3::StaWifiMac",
                "Ssid", SsidValue(ssid),
                "ActiveProbing", BooleanValue(false));

    NetDeviceContainer staDevices;
    staDevices = wifi.Install(phy, mac, wifiStaNodes);

    mac.SetType("ns3::ApWifiMac",
                "Ssid", SsidValue(ssid));

    NetDeviceContainer apDevices;
    apDevices = wifi.Install(phy, mac, wifiApNode);

    MobilityHelper mobility;

    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX", DoubleValue(100.0),
                                  "MinY", DoubleValue(100.0),
                                  "DeltaX", DoubleValue(5.0),
                                  "DeltaY", DoubleValue(10.0),
                                  "GridWidth", UintegerValue(10),
                                  "LayoutType", StringValue("RowFirst"));

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);
    mobility.Install(wifiStaNodes);

    //mobility.Install(wifiStaNodes.Get(2));
    //
    //mobility.SetMobilityModel("ns3::RandomDirection2dMobilityModel",
    //                          "Bounds", RectangleValue(Rectangle(-50000, 50000, -50000, 50000)));
    //
    //mobility.Install(wifiStaNodes.Get(0));

    InternetStackHelper stack;
    stack.Install(wifiApNode);
    stack.Install(wifiStaNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.3.0", "255.255.255.0");

    Ipv4InterfaceContainer wifiApInterfaces;
    wifiApInterfaces =
        address.Assign(apDevices);

    Ipv4InterfaceContainer wifiStaInterfaces;
    wifiStaInterfaces =
        address.Assign(staDevices);

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(wifiStaNodes.Get(1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(wifiStaInterfaces.GetAddress(1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(5000));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps =
        echoClient.Install(wifiStaNodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    if (tracing == true)
    {
      phy.EnablePcap("exp10_ap.pcap", apDevices.Get(0));
      phy.EnablePcap("exp10_cl.pcap", staDevices.Get(0));
      phy.EnablePcap("exp10_sr.pcap", staDevices.Get(1));
    }

    Simulator::Stop(Seconds(3.0));

    Ptr<ConstantPositionMobilityModel> Ap_pos = wifiApNode.Get(0)->GetObject<ConstantPositionMobilityModel>();
    if (Ap_pos != NULL)
      std::cout << "\nAP = " << Ap_pos->GetPosition();

    //Ptr<RandomDirection2dMobilityModel> pos0 = wifiStaNodes.Get(0)->GetObject<RandomDirection2dMobilityModel>();

    Ptr<ConstantPositionMobilityModel> pos0 = wifiStaNodes.Get(0)->GetObject<ConstantPositionMobilityModel>();
    if (pos0 != NULL)
    {
      movingNodePos.x += 1;

      pos0->SetPosition(movingNodePos);
      std::cout << "\nNode 0 = " << pos0->GetPosition();
    }

    Ptr<ConstantPositionMobilityModel> pos1 = wifiStaNodes.Get(1)->GetObject<ConstantPositionMobilityModel>();
    if (pos1 != NULL)
      std::cout << "\nNode 1 = " << pos1->GetPosition();

    Ptr<ConstantPositionMobilityModel> pos2 = wifiStaNodes.Get(2)->GetObject<ConstantPositionMobilityModel>();
    if (pos2 != NULL)
      std::cout << "\nNode 2 = " << pos2->GetPosition();

    Simulator::Run();
    Simulator::Destroy();
    --i;
  }
  return 0;
}
