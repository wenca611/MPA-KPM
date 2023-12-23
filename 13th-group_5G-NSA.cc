/**     ASSIGNMENT 13 - 5G NR SIMULATION IN NS-3
 * 
 * +------------------------------------------------+
 * |              Device requirements               |
 * +------------------------------------+-----------+
 * |    Device / Confing.   |   no.     |   STATUS  |
 * +------------------------+-----------+-----------+
 * |    gNodeBs             |   2x      |   DONE    |
 * |    User Equipment      |   5x      |   DONE    |
 * |    Internet Server     |   1x      |   DONE    |
 * |    4G Core             |   1x      |   DONE    | 
 * |    Internet            |   1x      |   DONE    |
 * +------------------------+-----------+-----------+
 * |    IP addressing                   |   DONE    |
 * |    IP routing                      |   DONE    |
 * +------------------------------------+-----------+
 * 
 * 
 * +------------------------------------------------+
 * |                    Mobility                    |
 * +------------------------------------+-----------+
 * |    Task                            |   STATUS  |
 * +------------------------------------+-----------+
 * |    HexagonalGridScenarioHelper     |   DONE    |
 * |    UE's mimic speed of vehicles    |   DONE    |       note: assumed speed of vehicle on the street = 50 km/h
 * +------------------------------------+-----------+
 * 
 * 
 * +------------------------------------------------+
 * |            NR Configuration (gNodeBs)          |
 * +------------------------------------+-----------+
 * |    Task                            |   STATUS  |
 * +------------------------------------+-----------+
 * |    Sub-6GHz frequency band         |   DONE    |       note: 1500 MHz (band n51)
 * |    1st numerology for BW part      |   DONE    |       note: Two numerologies for bandwidth parts
 * |    2nd numerology for BW part      |   DONE    |
 * |    Set appropriate Tx power        |   DONE    |
 * +------------------------------------+-----------+
 * 
 * 
 * +------------------------------------------------+
 * |                    Traffic                     |
 * +------------------------------------+-----------+
 * |    Task                            |   STATUS  |
 * +------------------------------------+-----------+
 * |    UEs are streaming videos        |   DONE    |       chosen: 1500 Bytes / UDP protocol (ref. RTSP)   
 * +------------------------------------+-----------+
 * 
 * 
 * +------------------------------------------------+
 * |            Simulation Requirements             |
 * +------------------------------------+-----------+
 * |    Task                            |   STATUS  |
 * +------------------------------------+-----------+
 * |    Execute simulation without err. |   DONE    | 
 * |    Data from different params.     |   DONE    |
 * |    Network observation             |   DONE    |
 * +------------------------------------+-----------+
 * |    Capture and analyze key metrics |           |
 * +------------------------------------+-----------+
 * |    throughput                      |   DONE    |
 * |    latency                         |   DONE    |
 * |    packet loss                     |   DONE    |              
 * +------------------------------------+-----------+
 * 
 * 
 * ©roup members:
 *      Adam Hora
 *      Boris Hynšt
 *      Jan Hrubý
 *      Václav Pastušek**/

// include used modules
#include "ns3/antenna-module.h"
#include "ns3/applications-module.h"
#include "ns3/buildings-module.h"
#include "ns3/config-store-module.h"
#include "ns3/core-module.h"   
#include "ns3/flow-monitor-module.h"   
#include "ns3/internet-apps-module.h"      
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/nr-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"

#include "iostream"
#include "iomanip"
#include "sys/stat.h"


// definition of used namespace
using namespace ns3;

// Function for moving hexagonal grid plot file.
bool copyFile(const std::string& sourceFile, const std::string& sourceDiagram, const std::string& destinationDiagram) 
{

  // Execute gnuplot, to create diagram.*/
  std::string command = "gnuplot " + sourceFile;                // Gnuplot execution
  int commadnResult = system(command.c_str());                  // Run command inside terminal

  // If statement in case of failure.
  if (commadnResult != 0) 
  {
      std::cerr << "Error executing GNUplot command" << std::endl;
      return false;
  }

  std::ifstream source(sourceDiagram, std::ios::binary);        // Open source file in binary mode for reading
  std::ofstream dest(destinationDiagram, std::ios::binary);     // Open destination file in binary mode for writing   
  dest << source.rdbuf();                                       // Copy contents of source file to destination file

  source.close();                                               // Close source file
  dest.close();                                                 // Close destination file
  std::remove(sourceFile.c_str());                              // Remove source file  
  std::remove(sourceDiagram.c_str());                           // Remove source diagram  

  return true;
}

int main(int argc, char *argv[])
{
  // Terminal log printing for Client and Server Application
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    /**    For the video streaming, it was decided to use protocol RTSP (Real-Time Streaming Protocol) as a reference. 
    *     This protocol utilizes UDP packets and a total packet size of 1500 bytes wes chosen, hence the default values.**/

  // ---------------------------------------------------------------{VARIABLES}---------------------------------------------------------------
  // 5G NR parameter specifications
  uint16_t numerology = 0;

  // NR parameters
  double carrierFrequency = 1.5e9;                // Band n51 for 5G NR (1500 MHz), (EU) and type TDD => Time Division Duplex
  double bandwidth = 1e8;                         // Bandwidth of 100 MHz
  double txPower = 43.0;                          // Power of the transmission 43 dBm, found in 5G documentation.

  // Hexagonal Grid Scenario parameters specification
  uint16_t gNodeBNumber = 2;
  uint16_t endUserDevicesNumber = 5;
  uint16_t baseStationDistance = 300;             // 300 meters for real life urban area.
  uint16_t numberOfRings = 1;                     // Specification of the rings for the hexagonal scenario.

  // Traffic parameters
  uint16_t packetSize = 1400;                     // Size of the packet 1400
  uint16_t packetsPerSecond = 100;                // Packet per second

  // Simulation parameters:
  Time simTime = MilliSeconds(2000);
  Time udpAppStartTime = MilliSeconds(400);

  // Final summarization printing
  std::string simTag = "statistics.txt";
  std::string outputDir = "./13th-group_5G-NSA";
  std::string sourceHexagonalGridFilePath = "hexagonal-topology.gnuplot";                               // Path of the source file.
  std::string sourceHexagonalGridDiagramPath = "hexagonal-topology.gnuplot.pdf";                        // Path of the source diagram.
  std::string destinationHexagonalGridDiagramPath = "13th-group_5G-NSA/hexagonal-topology.gnuplot.pdf"; // Path of the destination diagram.
  bool pcapFilesGeneration = false;

  // -------------------------------------------------------------{CMD ARGUMENTS}-------------------------------------------------------------
  CommandLine cmd(__FILE__);

  // User interaction with the program and input of the parameters.
  cmd.AddValue("numerology", "Numerology 0 - 4.", numerology);  
  cmd.AddValue("carrierFrequency", "Carrier frequency in [Hz]. !Must be 0.5 - 100 GHz!", carrierFrequency);
  cmd.AddValue("bandwidth", "Bandwidth in [Hz].", bandwidth);
  cmd.AddValue("txPower", "Power of the transmitor in [dBm].", txPower);
  cmd.AddValue("gNodeBNumber", "Number of gNodeB's. !Min = 1 & Max = 37!", gNodeBNumber);
  cmd.AddValue("baseStationDistance", "Distance between individual gNodeB's [m]. !Minimum of 10!", baseStationDistance);
  cmd.AddValue("endUserDevicesNumber", "Number of end user devices.", endUserDevicesNumber);
  cmd.AddValue("packetSize", "Size of the packet payload in [Bytes].", packetSize);
  cmd.AddValue("packetsPerSecond", "Maximum packets transmitted per second.", packetsPerSecond);
  cmd.AddValue("simTime", "Time of the simulation in [sec].", simTime);
  cmd.AddValue("pcapFilesGeneration", "Enables capturing traffic into pcap files.", pcapFilesGeneration);

  // Parse the command line input.
  cmd.Parse(argc, argv);

  // Check if the parameters from user input are correct.
  NS_ABORT_IF(numerology > 5 || numerology < 0);
  NS_ABORT_IF(carrierFrequency < 0.5e9 && carrierFrequency > 100e9);
  NS_ABORT_IF(bandwidth < 0);
  NS_ABORT_IF(txPower <= 0);
  NS_ABORT_IF(gNodeBNumber < 1);
  NS_ABORT_IF(baseStationDistance < 10);
  NS_ABORT_IF(endUserDevicesNumber < 1);
  NS_ABORT_IF(packetSize <= 0 || packetSize > 2500);
  NS_ABORT_IF(packetsPerSecond <= 0);
  NS_ABORT_IF(simTime <= MilliSeconds(0));

  // Calculation of the ring number for the Hexagonal Grid Scenario.
  if (gNodeBNumber == 1)
  {
    numberOfRings = 0;
  }
  else if (gNodeBNumber > 1 && gNodeBNumber < 8)
  {
    numberOfRings = 1;
  }
  else if (gNodeBNumber > 7 && gNodeBNumber < 14)
  {
    numberOfRings = 2; 
  }
   else if (gNodeBNumber > 13 && gNodeBNumber < 20)
  {
    numberOfRings = 3; 
  } 
  else if (gNodeBNumber > 19 && gNodeBNumber < 32)
  {
    numberOfRings = 4; 
  } 
  else if (gNodeBNumber > 31 && gNodeBNumber < 38)
  {
    numberOfRings = 5; 
  }
  // If some crazy number is put in, default value is set up. (:
  else
  {
    gNodeBNumber = 2;
    numberOfRings = 1;
  }

  // Creation of the folder for saving simulation files.
  int check;
  check = mkdir("13th-group_5G-NSA",0777);

  // check if directory is created or not
  if (!check)
  {
      std::cout<<"Directory created"<<std::endl;
  }


  // ----------------------------------------------------------------{SCENARIO}---------------------------------------------------------------
  Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));     // Some legacy ompatibility or smth.
  
  HexagonalGridScenarioHelper gridScenario;                             // Creation of hexagonalgrid scenario instance.           
  
  gridScenario.SetSectorization(HexagonalGridScenarioHelper::SINGLE);   // Using SINGLE (gNB have 360 degree) instead of TRIPLE (3 gNBs for 1 site with 120 degree coverage) for better diagnostics
  gridScenario.SetNumRings(numberOfRings);                              // Set number of hexagonal grids 0 = 1 site; 1 = 7 sites 
  gridScenario.SetUtNumber(endUserDevicesNumber);                       // Set Set the ammount of the UEs

  // Scenario parameters
  gridScenario.m_bsHeight = 30.0;                                       // Set bs (gNB) height
  gridScenario.m_utHeight = 1.5;                                        // Set UE height
  gridScenario.m_minBsUtDistance = 5.0;                                 // Set minimal distance between gNB and UE
  gridScenario.m_isd = baseStationDistance;                             // Set distance between sites - Ideally would be around 300 meters, which provides the best coverage
  gridScenario.m_antennaOffset = 1.0;                                   // Set antenna offset
  gridScenario.SetSitesNumber(gNodeBNumber);                            // Set the ammount of sites and gNBs*Sectorization (We use secotorization SINGLE => 1)
  gridScenario.CreateScenarioWithMobility(Vector(9.0,9.0,0.0), 1.0);    // Creation of scenario with constatnt speed and direction, by using the provided parameters

  // -----------------------------------------------------------------{NODES}-----------------------------------------------------------------
  // UE's node containers
  NodeContainer endUserDeviceContainer;
  // For cycle to create required ammount of end user devices, which is 5 in our case. Number 0 - 4
  for (uint32_t j = 0; j < endUserDevicesNumber; j++)
  {
    //std::cout<<"For Loop"<<std::endl;
    Ptr<Node> endUserDevice = gridScenario.GetUserTerminals().Get(j);
    endUserDeviceContainer.Add(endUserDevice);
  }

  //gNodeB container;
  NodeContainer gNodeBContainer;
  for (uint32_t j = 0; j < gNodeBNumber; j++)
  {
    Ptr<Node> gNodeB = gridScenario.GetBaseStations().Get(j);
    gNodeBContainer.Add(gNodeB);
  }

  // EPCHelper creation, epcHelper is node that works as a Core part of 4G core => includes functions of SG-W, P-GW, MME and HSS.
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
  // Beamforming helper creation. Basically just advanced way of data transmission, which is pretty much alike to directional antenna.
  Ptr<IdealBeamformingHelper> beamformingHelper = CreateObject<IdealBeamformingHelper>();
  // NrHelper creation, general 5G function support.
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
  
  // Assigning of the created pointers to the nrHelper.
  nrHelper -> SetBeamformingHelper(beamformingHelper);
  nrHelper -> SetEpcHelper(epcHelper);

  // Component Carrier Creator and allBwps
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ComponentCarriersCreator;

  // Configuration of the Band, by given parameters
  CcBwpCreator::SimpleOperationBandConf bandConfinguration(carrierFrequency,
                                                          bandwidth,
                                                          (uint8_t) 1,
                                                          BandwidthPartInfo::UMi_StreetCanyon);    

  // Creation of the Operation band
  OperationBandInfo bandInormation = ComponentCarriersCreator.CreateOperationBandContiguousCc(bandConfinguration);
  allBwps = CcBwpCreator::GetAllBwps({bandInormation});

  Config::SetDefault("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue(MilliSeconds(0)));
  nrHelper -> SetChannelConditionModelAttribute("UpdatePeriod", TimeValue(MilliSeconds(0)));
  nrHelper -> SetPathlossAttribute("ShadowingEnabled", BooleanValue(false));

  // Initialization of the band for the simulation
  nrHelper -> InitializeOperationBand(&bandInormation);

  // BeamForming method (description higher with BeamforningHelper)
  beamformingHelper -> SetAttribute("BeamformingMethod", TypeIdValue(DirectPathBeamforming::GetTypeId()));

  // Core latency specification for [ms] = 0 means no delay, we are setting ideal network
  epcHelper -> SetAttribute("S1uLinkDelay", TimeValue(MilliSeconds(0)));
  Ptr<Node> pgw = epcHelper -> GetPgwNode();           // Get the PG-W node.
  Ptr<Node> sgw = epcHelper -> GetSgwNode();           // Get the SG-W node.

  // Antenna configurations for the End User Devices
  nrHelper -> SetUeAntennaAttribute("NumRows", UintegerValue(2));
  nrHelper -> SetUeAntennaAttribute("NumColumns", UintegerValue(4));
  nrHelper -> SetUeAntennaAttribute("AntennaElement", PointerValue(CreateObject<IsotropicAntennaModel>()));
  // Specification of management algorithms for End User Devices.
  nrHelper -> SetUeBwpManagerAlgorithmAttribute("GBR_MC_VIDEO", UintegerValue(0));      
  // Install of the End User Device configuration.
  NetDeviceContainer endUserDeviceNetDeviceContainer = nrHelper -> InstallUeDevice (endUserDeviceContainer, allBwps);

  // Some config update
  for (auto it = endUserDeviceNetDeviceContainer.Begin(); it != endUserDeviceNetDeviceContainer.End(); ++it)
  {
      DynamicCast<NrUeNetDevice>(*it) -> UpdateConfig();
  }

  // Antenna configurations for the gNodeB's
  nrHelper -> SetGnbAntennaAttribute("NumRows", UintegerValue(4));
  nrHelper -> SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
  nrHelper -> SetGnbAntennaAttribute("AntennaElement", PointerValue(CreateObject<IsotropicAntennaModel>()));
  // Specification of management algorithms for gNodeB's.
  nrHelper -> SetGnbBwpManagerAlgorithmAttribute("GBR_MC_VIDEO", UintegerValue(0));     
  // Install of the gNodeB configuration.
  NetDeviceContainer gNodeBNetDevice = nrHelper -> InstallGnbDevice(gridScenario.GetBaseStations(), allBwps);

  // Setting the attributes for the gNodeB's numerology and transmission power, both set up higher in the Variables part.
  double x = pow(10, txPower / 10);

  for (uint16_t i = 0; i < gNodeBNumber; i++)
  {
    nrHelper -> GetGnbPhy(gNodeBNetDevice.Get(i), 0) -> SetAttribute("Numerology", UintegerValue(numerology));
    nrHelper -> GetGnbPhy(gNodeBNetDevice.Get(i), 0) -> SetAttribute("TxPower", DoubleValue(10 * log10((bandwidth / bandwidth) * x)));
  }

  // Some config update
  for (auto it = gNodeBNetDevice.Begin(); it != gNodeBNetDevice.End(); ++it)
  {
      DynamicCast<NrGnbNetDevice>(*it)-> UpdateConfig();
  }

  // --------------------------------------------------------------{IP + INTERNET}---------------------------------------------------------------- 
  // Creation of the external server, located in the Internet.
  NodeContainer remoteServerContainer;
  remoteServerContainer.Create(1);
  Ptr<Node> remoteServer = remoteServerContainer.Get(0);

  // Creating the internet stack helper.
  InternetStackHelper internet;
  internet.Install(remoteServerContainer);

  // Connection of the remote server to the PG-W node.
  PointToPointHelper point2pointHelper;
  point2pointHelper.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s"))); // Maximum data rate
  point2pointHelper.SetDeviceAttribute("Mtu", UintegerValue(2500));                     // Maxium packet size (including headers)
  point2pointHelper.SetChannelAttribute("Delay", TimeValue(Seconds(0.000)));            // No delay => ideal network
  NetDeviceContainer internetDevices = point2pointHelper.Install(pgw, remoteServer);    // Installing the helper to the remote server and gateway

  // IPv4 addressing and static routing.
  Ipv4AddressHelper ipv4Helper;
  Ipv4StaticRoutingHelper ipv4StaticRoutingHelper;
  ipv4Helper.SetBase("1.0.0.0", "255.0.0.0");                                     // Set of IP addressing space.
  Ipv4InterfaceContainer internetIpInterfaces = ipv4Helper.Assign(internetDevices);     // Interface creation on the internetDevices.
  Ptr<Ipv4StaticRouting> remoteServerStaticRouting = ipv4StaticRoutingHelper.GetStaticRouting(remoteServer -> GetObject<Ipv4>());
  remoteServerStaticRouting -> AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);
  internet.Install(gridScenario.GetUserTerminals());

  Ipv4InterfaceContainer endUserDeviceIpInterface = epcHelper -> AssignUeIpv4Address(NetDeviceContainer(endUserDeviceNetDeviceContainer));

  // Setting the default gateway for the End User Devices
   for (uint32_t j = 0; j < gridScenario.GetUserTerminals().GetN(); ++j)
    {
        Ptr<Ipv4StaticRouting> EndUserDeviceStaticRouting = 
          ipv4StaticRoutingHelper.GetStaticRouting(gridScenario.GetUserTerminals().Get(j) -> GetObject<Ipv4>());
        EndUserDeviceStaticRouting -> SetDefaultRoute(epcHelper -> GetUeDefaultGatewayAddress(), 1);
    }

  // Connect all End User Devices to the nearest gNodeB
  nrHelper -> AttachToClosestEnb(endUserDeviceNetDeviceContainer, gNodeBNetDevice);
  
  // -----------------------------------------------------------------{TRAFFIC}-------------------------------------------------------------------
  // We want the UE's to send data to the server, so we have to create UP-Link connection between the individual EU's and the remote server.
  uint16_t upLinkPort = 49469;                                                              // Specification of the UP-Link port (dynamic port)
  ApplicationContainer endUserApplicationUpLinkStream;

  UdpServerHelper upLinkPacketSinkVideoStream(upLinkPort);                                  // Specification of the UDP server helper.
  endUserApplicationUpLinkStream.Add(upLinkPacketSinkVideoStream.Install(remoteServer));    // Installing this stream to the "listener".

 // Configuration of the traffic, using preset, and/or user input data.
  UdpClientHelper upLinkClientVideoStream;
  upLinkClientVideoStream.SetAttribute("RemotePort", UintegerValue(upLinkPort));        // Specification of the destination port.
  upLinkClientVideoStream.SetAttribute("MaxPackets", UintegerValue(0xFFFFFFFF));        // Specification of the maximum number of packets.
  upLinkClientVideoStream.SetAttribute("PacketSize", UintegerValue(packetSize));                  // Specification of the packet size.
  upLinkClientVideoStream.SetAttribute("Interval", TimeValue(Seconds(1.0 / packetsPerSecond)));   // Specification of the time between send packets.
  
  Address remoteServerIpAddress = internetIpInterfaces.GetAddress(1);                           // Obtain the IP address of the remote server.
  upLinkClientVideoStream.SetAttribute("RemoteAddress", AddressValue(remoteServerIpAddress));   // Specification of the remote server address.

  EpsBearer videoStreamBearer(EpsBearer::GBR_CONV_VIDEO);      // Specification of the bearer for the video stream traffic.                               

  // Filter for the streaming traffic.
  Ptr<EpcTft> videoStreamTrafficFilter = Create<EpcTft>();
  EpcTft::PacketFilter upLinkVideoStream;
  upLinkVideoStream.localPortStart = upLinkPort;
  upLinkVideoStream.localPortEnd = upLinkPort;
  videoStreamTrafficFilter -> Add(upLinkVideoStream);

  ApplicationContainer clientApplicationContainer;            // Installation of the client application.

  for (uint32_t i = 0; i < endUserDeviceContainer.GetN(); ++i)
  {
      Ptr<NetDevice> endUserDevice = endUserDeviceNetDeviceContainer.Get(i);
      clientApplicationContainer.Add(upLinkClientVideoStream.Install(endUserDeviceContainer.Get(i)));
      nrHelper -> ActivateDedicatedEpsBearer(endUserDevice, videoStreamBearer, videoStreamTrafficFilter);
  }

  // Deffinition of the start and stop of the simulation.
  endUserApplicationUpLinkStream.Start(udpAppStartTime);
  endUserApplicationUpLinkStream.Stop(simTime);

  // Flow monitor helper
  //nrHelper -> EnableTraces();

  FlowMonitorHelper flowMmonitorHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add(remoteServer);
  endpointNodes.Add(gridScenario.GetUserTerminals());

  Ptr<ns3::FlowMonitor> monitor = flowMmonitorHelper.Install(endpointNodes);
  monitor -> SetAttribute("DelayBinWidth", DoubleValue(0.001));
  monitor -> SetAttribute("JitterBinWidth", DoubleValue(0.001));
  monitor -> SetAttribute("PacketSizeBinWidth", DoubleValue(20));

  if (pcapFilesGeneration == true)
  {
    point2pointHelper.EnablePcapAll("13th-group_5G-NSA/Node");
  }

  // ----------------------------------------------------------------{SIMULATION}-----------------------------------------------------------------
  uint16_t coreMobilityDistance = 0;

  if (numberOfRings == 0)
  {
    coreMobilityDistance = (numberOfRings + 1) * baseStationDistance;
  }
  else
  {
    coreMobilityDistance = numberOfRings * baseStationDistance;
  }

  //Setting Cosntant position mobility model for Core parts of network.
  MobilityHelper mobilityCoreNetwork;

  //Setting exact positions on which the SG-W node will be located.
  Ptr<ListPositionAllocator> sgwPositionAlloc = CreateObject<ListPositionAllocator>();
  sgwPositionAlloc->Add(Vector(coreMobilityDistance + 50, 50, 0.0));

  // Set and install position allocator for SG-W node.
  mobilityCoreNetwork.SetPositionAllocator(sgwPositionAlloc);
  mobilityCoreNetwork.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityCoreNetwork.Install(sgw);

  //Setting exact positions on which the PG-W node will be located.
  Ptr<ListPositionAllocator> pgwPositionAlloc = CreateObject<ListPositionAllocator>();
  pgwPositionAlloc->Add(Vector(coreMobilityDistance + 170, 50, 0.0));
  
  // Set and install position allocator for PG-W node.
  mobilityCoreNetwork.SetPositionAllocator(pgwPositionAlloc);
  mobilityCoreNetwork.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityCoreNetwork.Install(pgw);

  //Setting exact positions on which the Remote Server node will be located.
  Ptr<ListPositionAllocator> remoteServerPositionAlloc = CreateObject<ListPositionAllocator>();
  remoteServerPositionAlloc -> Add(Vector(coreMobilityDistance + 180, 0, 0.0));

  // Set and install position allocator for the Remote Server node.
  MobilityHelper mobilityRemoteServer;
  mobilityRemoteServer.SetPositionAllocator(remoteServerPositionAlloc);
  mobilityRemoteServer.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityRemoteServer.Install(remoteServerContainer);
  
  // NetAnim file creation
  AnimationInterface animation("13th-group_5G-NSA/13th-group_5G-NSA.xmp");

  for (uint32_t j = 0; j < endUserDevicesNumber; j++)
  {
    // Change the name, coulour and size of the End User Devices in Netanim.
    animation.UpdateNodeDescription(endUserDeviceContainer.Get(j), "UE-"+std::to_string(j+1));     
    animation.UpdateNodeColor(endUserDeviceContainer.Get(j), 0,255,0);
    animation.UpdateNodeSize(endUserDeviceContainer.Get(j), 10,10);
  }

  // Change the name, coulour and size of the PG-W node.
  animation.UpdateNodeDescription(pgw, "PG-W");
  animation.UpdateNodeColor(pgw, 0,0,255);
  animation.UpdateNodeSize(pgw, 10,10);

  // Change the name, colour and size of the SG-W node.
  animation.UpdateNodeDescription(sgw, "SG-W");
  animation.UpdateNodeColor(sgw, 0,0,255);
  animation.UpdateNodeSize(sgw, 10,10);


  // MME calculation number of gNodeB's and end user devices + sgw and pgw
  uint16_t mme = gNodeBNumber + endUserDevicesNumber + 2; 
  // Change the name, colour and the size of the MME node.
  animation.UpdateNodeDescription(mme, "MME");
  animation.UpdateNodeColor(mme, 0,0,255);
  animation.UpdateNodeSize(mme, 10,10);

  //Change the name, colour and the size of the gNodeB nodes.
  for(uint8_t i = 0; i < gNodeBNumber; i++)
  {
    animation.UpdateNodeDescription(i, "gNodeB-"+std::to_string(i+1));
    animation.UpdateNodeColor(i, 255,0,0);
    animation.UpdateNodeSize(i, 10,10);
  }

  //Change the name, colour and the size of the remote server node.
  animation.UpdateNodeDescription(remoteServer, "Remote Server");
  animation.UpdateNodeColor(remoteServer, 255,20,147);
  animation.UpdateNodeSize(remoteServer, 10,10);

  Simulator::Stop(simTime);
  Simulator::Run();

  // Move of Hexagonal Grid File gnuplot to a group folder.

  // ----------------------------------------------------------------{STATISTICS}-----------------------------------------------------------------  
  /** This part of code takes total flow data and prints them into file for each node. We also use this data, to calculate packet loss in [%].
   *  This file is then taken and printed out to the terminal. At the end, avarage throughput and delay are added.*/

  monitor -> CheckForLostPackets();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMmonitorHelper.GetClassifier());
  FlowMonitor::FlowStatsContainer flowStatisticsContainer = monitor -> GetFlowStats();

  // Specification of the monitored parameters, namely Throughput, Delay, Latency.
  double averageFlowThroughput = 0.0;
  double averageFlowDelay = 0.0;

  std::ofstream outFile;                                    // Declaration of object outFile.
  std::string fileName = outputDir + "/" + simTag;          // Creating variable with the file name and location.
  // Opens created file for writing and if the file exists, it's content will be cleared.
  outFile.open(fileName.c_str(), std::ofstream::out | std::ofstream::trunc);
  // Error in file opening.
  if (!outFile.is_open())
  {
      std::cerr << "Can't open file " << fileName << std::endl;
      return 1;
  }

  outFile.setf(std::ios_base::fixed);
  double flowDuration = (simTime - udpAppStartTime).GetSeconds();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = flowStatisticsContainer.begin(); 
      i != flowStatisticsContainer.end(); ++i)
  {
    Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
    std::stringstream protoStream;
    protoStream << (uint16_t)t.protocol;
    protoStream.str("UDP");
    
    outFile << "\n" << "UE-" << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> "
            << t.destinationAddress << ":" << t.destinationPort << ")\t" << protoStream.str() << "\n";
    outFile << "  Tx Packets:\t" << i->second.txPackets << "\t" << "Packets\n";
    outFile << "  Rx Packets:\t" << i->second.rxPackets << "\t" << "Packets\n";
    outFile << "  PacketLoss:\t" << std::fixed << std::setprecision(2) << (double(i->second.txPackets) - double(i->second.rxPackets))
                                                                            /double(i->second.txPackets)*100 << "\t" << "% \n";
    if (i->second.rxPackets > 0)
    {
        // Measure the duration of the flow from receiver's perspective
        averageFlowThroughput += i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000;
        averageFlowDelay += 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets;
        
        outFile << "  Throughput:\t" << std::fixed << std::setprecision(3) << i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000 << "\t" << "Mbps\n";
        outFile << "  Mean delay:\t" << 1000 * i->second.delaySum.GetSeconds() / i->second.rxPackets << "\t" << "ms\n";
    }
    else
    {
        outFile << "  Throughput:  0 Mbps\n";
        outFile << "  Mean delay:  0 ms\n";
    }
  }

  double meanFlowThroughput = averageFlowThroughput / flowStatisticsContainer.size();
  double meanFlowDelay = averageFlowDelay / flowStatisticsContainer.size();

  outFile << "\n\n  Mean flow throughput:\t" << meanFlowThroughput << " Mbps\n";
  outFile << "  Mean flow delay:\t" << meanFlowDelay << " ms\n";

  outFile.close();
  std::ifstream f(fileName.c_str());

  if (f.is_open())
  {
      std::cout << f.rdbuf();
  }

  // Automatic creation of the plotted diagram from the raw data provided by hexagon scenario helper.
  copyFile(sourceHexagonalGridFilePath, sourceHexagonalGridDiagramPath, destinationHexagonalGridDiagramPath); // Copy file and delete source file. 

  Simulator::Destroy();
}


