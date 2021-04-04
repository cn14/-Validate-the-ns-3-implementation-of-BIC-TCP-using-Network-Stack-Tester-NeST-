/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 NITK Surathkal
 *
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
 *
 * Authors: Siddharth Singh <siddharth12375@gmail.com>
 *          Chetan Agarwal <chetanag35@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

#include <iostream>
#include <fstream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/point-to-point-module.h"

#include <sys/stat.h> 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TsvwgScenarios");

uint32_t g_firstBytesReceived = 0;
uint32_t g_secondBytesReceived = 0;
uint32_t g_marksObserved = 0;
uint32_t g_dropsObserved = 0;

void
TraceFirstCwnd (std::ofstream* ofStream, uint32_t oldCwnd, uint32_t newCwnd)
{
  // TCP segment size is configured below to be 1448 bytes
  // so that we can report cwnd in units of segments
  *ofStream << Simulator::Now ().GetSeconds () << " " << static_cast<double> (newCwnd) / 1448 << std::endl;
}

void
TraceFirstRtt (std::ofstream* ofStream, Time oldRtt, Time newRtt)
{
  *ofStream << Simulator::Now ().GetSeconds () << " " << newRtt.GetSeconds () * 1000 << std::endl;
}

void
TraceSecondCwnd (std::ofstream* ofStream, uint32_t oldCwnd, uint32_t newCwnd)
{
  // TCP segment size is configured below to be 1448 bytes
  // so that we can report cwnd in units of segments
  *ofStream << Simulator::Now ().GetSeconds () << " " << static_cast<double> (newCwnd) / 1448 << std::endl;
}

void
TraceSecondRtt (std::ofstream* ofStream, Time oldRtt, Time newRtt)
{
  *ofStream << Simulator::Now ().GetSeconds () << " " << newRtt.GetSeconds () * 1000 << std::endl;
}

void
TraceFirstRx (Ptr<const Packet> packet, const Address &address)
{
  g_firstBytesReceived += packet->GetSize ();
}

void
TraceSecondRx (Ptr<const Packet> packet, const Address &address)
{
  g_secondBytesReceived += packet->GetSize ();
}

void
TraceRouterDrop (std::ofstream* ofStream, Ptr<const QueueDiscItem> item)
{
  *ofStream << Simulator::Now ().GetSeconds () << " " << std::hex << item->Hash () << std::endl;
  g_dropsObserved++;
}

void
TraceRouterQueueLength (std::ofstream* ofStream, DataRate routerLinkRate, uint32_t oldVal, uint32_t newVal)
{
  // output in units of ms
  *ofStream << Simulator::Now ().GetSeconds () << " " << std::fixed << static_cast<double> (newVal * 8) / (routerLinkRate.GetBitRate () / 1000) << std::endl;
}

void
TraceDropsFrequency (std::ofstream* ofStream, Time dropsSamplingInterval)
{
  *ofStream << Simulator::Now ().GetSeconds () << " " << g_dropsObserved << std::endl;
  g_dropsObserved = 0;
  Simulator::Schedule (dropsSamplingInterval, &TraceDropsFrequency, ofStream, dropsSamplingInterval);
}

void
TraceMarksFrequency (std::ofstream* ofStream, Time marksSamplingInterval)
{
  *ofStream << Simulator::Now ().GetSeconds () << " " << g_marksObserved << std::endl;
  g_marksObserved = 0;
  Simulator::Schedule (marksSamplingInterval, &TraceMarksFrequency, ofStream, marksSamplingInterval);
}

void
TraceFirstThroughput (std::ofstream* ofStream, Time throughputInterval)
{
  *ofStream << Simulator::Now ().GetSeconds () << " " << g_firstBytesReceived * 8 / throughputInterval.GetSeconds () / 1e6 << std::endl;
  g_firstBytesReceived = 0;
  Simulator::Schedule (throughputInterval, &TraceFirstThroughput, ofStream, throughputInterval);
}

void
TraceSecondThroughput (std::ofstream* ofStream, Time throughputInterval)
{
  *ofStream << Simulator::Now ().GetSeconds () << " " << g_secondBytesReceived * 8 / throughputInterval.GetSeconds () / 1e6 << std::endl;
  g_secondBytesReceived = 0;
  Simulator::Schedule (throughputInterval, &TraceSecondThroughput, ofStream, throughputInterval);
}

void
ScheduleFirstTcpCwndTraceConnection (std::ofstream* ofStream)
{
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&TraceFirstCwnd, ofStream));
}

void
ScheduleFirstTcpRttTraceConnection (std::ofstream* ofStream)
{
  Config::ConnectWithoutContext ("/NodeList/0/$ns3::TcpL4Protocol/SocketList/0/RTT", MakeBoundCallback (&TraceFirstRtt, ofStream));
}

void
ScheduleFirstPacketSinkConnection (void)
{
  Config::ConnectWithoutContext ("/NodeList/2/ApplicationList/*/$ns3::PacketSink/Rx", MakeCallback (&TraceFirstRx));
}

void
ScheduleSecondTcpCwndTraceConnection (std::ofstream* ofStream)
{
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeBoundCallback (&TraceSecondCwnd, ofStream));
}

void
ScheduleSecondTcpRttTraceConnection (std::ofstream* ofStream)
{
  Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/RTT", MakeBoundCallback (&TraceSecondRtt, ofStream));
}

int
main (int argc, char *argv[])
{
  ////////////////////////////////////////////////////////////
  // variables not configured at command line               //
  ////////////////////////////////////////////////////////////
  std::string dir = "deepak_TcpBic_low_results/";
  mkdir("deepak_TcpBic_low_results",0777); 


  Time stopTime = Seconds (20);
  Time baseRtt = MilliSeconds (12);
  Time marksSamplingInterval = MilliSeconds (100);
  Time throughputSamplingInterval = MilliSeconds (200);
  std::string firstTcpRttTraceFile = dir+"TcpBic-first-tcp-rtt.dat";
  std::string firstTcpCwndTraceFile = dir+"TcpBic-first-tcp-cwnd.dat";
  std::string firstTcpThroughputTraceFile =dir+"TcpBic-first-tcp-throughput.dat";
  std::string secondTcpRttTraceFile = dir+"TcpBic-second-tcp-rtt.dat";
  std::string secondTcpCwndTraceFile = dir+"TcpBic-second-tcp-cwnd.dat";
  std::string secondTcpThroughputTraceFile =dir+"TcpBic-second-tcp-throughput.dat";
  std::string routerDropTraceFile =dir+"TcpBic--router-drops.dat";
  std::string routerDropsFrequencyTraceFile =dir+"TcpBic--router-drops-frequency.dat";
  std::string routerLengthTraceFile = dir+"TcpBic-router-length.dat";



  float bottleneck = 50;
  float rtt_h1_h3 = 12;
  float rtt_h2_h3 = 12;
  float packetSize = 1500;
  float bdp = bottleneck * std::min (rtt_h1_h3,rtt_h2_h3);
  int no_of_pkts = int(bdp / packetSize);

  ////////////////////////////////////////////////////////////
  // variables configured at command line                   //
  ////////////////////////////////////////////////////////////
  bool enablePcap = false;
  bool controlScenario = false;
  std::string firstTcpType = "TcpBic";
  std::string secondTcpType = "TcpBic";
  std::string routerQueueType = "fq";

  ////////////////////////////////////////////////////////////
  // Override ns-3 defaults                                 //
  ////////////////////////////////////////////////////////////
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (1448));
  // Increase default buffer sizes to improve throughput over long delay paths
  Config::SetDefault ("ns3::TcpSocket::SndBufSize",UintegerValue (8192000));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize",UintegerValue (8192000));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (10));
  Config::SetDefault ("ns3::TcpL4Protocol::RecoveryType", TypeIdValue (TcpPrrRecovery::GetTypeId ()));
  Config::SetDefault ("ns3::FifoQueueDisc::MaxSize",
                      QueueSizeValue (QueueSize (QueueSizeUnit::PACKETS, no_of_pkts)));


  ////////////////////////////////////////////////////////////
  // command-line argument parsing                          //
  ////////////////////////////////////////////////////////////
  CommandLine cmd;
  cmd.AddValue ("firstTcpType", "First TCP type (cubic, prague, or reno)", firstTcpType);
  cmd.AddValue ("secondTcpType", "Second TCP type (cubic, prague, or reno)", secondTcpType);
  cmd.AddValue ("routerQueueType", "Router queue type (dualq, fq, or codel)", routerQueueType);
  cmd.AddValue ("baseRtt", "base RTT", baseRtt);
  cmd.AddValue ("controlScenario", "control scenario (disable router bottleneck)", controlScenario);
  cmd.AddValue ("stopTime", "simulation stop time", stopTime);
  cmd.AddValue ("enablePcap", "enable Pcap", enablePcap);
  cmd.AddValue ("firstTcpRttTraceFile", "filename for rtt tracing", firstTcpRttTraceFile);
  cmd.AddValue ("firstTcpCwndTraceFile", "filename for cwnd tracing", firstTcpCwndTraceFile);
  cmd.AddValue ("firstTcpThroughputTraceFile", "filename for throughput tracing", firstTcpThroughputTraceFile);
  cmd.AddValue ("secondTcpRttTraceFile", "filename for second rtt tracing", secondTcpRttTraceFile);
  cmd.AddValue ("secondTcpCwndTraceFile", "filename for second cwnd tracing", secondTcpCwndTraceFile);
  cmd.AddValue ("secondTcpThroughputTraceFile", "filename for second throughput tracing", secondTcpThroughputTraceFile);
  cmd.AddValue ("routerDropTraceFile", "filename for router drops tracing", routerDropTraceFile);
  cmd.AddValue ("routerDropsFrequencyTraceFile", "filename for router drop frequency tracing", routerDropsFrequencyTraceFile);
  cmd.AddValue ("routerLengthTraceFile", "filename for router queue length tracing", routerLengthTraceFile);
  cmd.AddValue ("throughputSamplingInterval", "Interval at which throughput is sampled", throughputSamplingInterval);
  cmd.Parse (argc, argv);
  Time oneWayDelay = MilliSeconds (3);

  TypeId firstTcpTypeId = TcpBic::GetTypeId ();

  std::ofstream firstTcpRttOfStream;
  firstTcpRttOfStream.open (firstTcpRttTraceFile.c_str (), std::ofstream::out);
  std::ofstream firstTcpCwndOfStream;
  firstTcpCwndOfStream.open (firstTcpCwndTraceFile.c_str (), std::ofstream::out);
  std::ofstream firstTcpThroughputOfStream;
  firstTcpThroughputOfStream.open (firstTcpThroughputTraceFile.c_str (), std::ofstream::out);
  std::ofstream secondTcpRttOfStream;
  secondTcpRttOfStream.open (secondTcpRttTraceFile.c_str (), std::ofstream::out);
  std::ofstream secondTcpCwndOfStream;
  secondTcpCwndOfStream.open (secondTcpCwndTraceFile.c_str (), std::ofstream::out);
  std::ofstream secondTcpThroughputOfStream;
  secondTcpThroughputOfStream.open (secondTcpThroughputTraceFile.c_str (), std::ofstream::out);
  std::ofstream routerDropOfStream;
  routerDropOfStream.open (routerDropTraceFile.c_str (), std::ofstream::out);
  std::ofstream routerDropsFrequencyOfStream;
  routerDropsFrequencyOfStream.open (routerDropsFrequencyTraceFile.c_str (), std::ofstream::out);
  std::ofstream routerLengthOfStream;
  routerLengthOfStream.open (routerLengthTraceFile.c_str (), std::ofstream::out);

  ////////////////////////////////////////////////////////////
  // scenario setup                                         //
  ////////////////////////////////////////////////////////////
  Ptr<Node> h1 = CreateObject<Node> ();
  Ptr<Node> h2 = CreateObject<Node> ();
  Ptr<Node> h3 = CreateObject<Node> ();
  Ptr<Node> router = CreateObject<Node> ();

  // Device containers
  NetDeviceContainer h1_router;
  NetDeviceContainer h2_router;
  NetDeviceContainer h3_router;

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
  p2p.SetChannelAttribute ("Delay", TimeValue (oneWayDelay));
  h1_router = p2p.Install (h1, router);
  h2_router = p2p.Install (h2, router);

  p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("50Mbps")));
  h3_router = p2p.Install (h3, router);

  InternetStackHelper stackHelper;
  stackHelper.Install (h1);
  stackHelper.Install (h2);
  stackHelper.Install (h3);
  stackHelper.Install (router);

  // Set the per-node TCP type here
  Ptr<TcpL4Protocol> proto;
  proto = h1->GetObject<TcpL4Protocol> ();
  proto->SetAttribute ("SocketType", TypeIdValue (firstTcpTypeId));
  proto = h2->GetObject<TcpL4Protocol> ();
  proto->SetAttribute ("SocketType", TypeIdValue (firstTcpTypeId));
  proto = h3->GetObject<TcpL4Protocol> ();
  proto->SetAttribute ("SocketType", TypeIdValue (firstTcpTypeId));

  TrafficControlHelper tchFq;
  tchFq.SetRootQueueDisc ("ns3::FqCoDelQueueDisc");
  tchFq.Install (h1_router.Get (0));
  tchFq.Install (h2_router.Get (0));
  tchFq.Install (h3_router.Get (0));

  TrafficControlHelper tchFq2;
  tchFq2.SetRootQueueDisc ("ns3::FifoQueueDisc");
  tchFq2.Install (h3_router.Get (1));

  // InternetStackHelper will install a base TrafficControLayer on the node,
  // but the Ipv4AddressHelper below will install the default FqCoDelQueueDisc
  // on all single device nodes.  The below code overrides the configuration
  // that is normally done by the Ipv4AddressHelper::Install() method by
  // instead explicitly configuring the queue discs we want on each device.

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer h1_routerIfaces = ipv4.Assign (h1_router);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer h2_routerIfaces = ipv4.Assign (h2_router);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer h3_routerIfaces = ipv4.Assign (h3_router);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  ////////////////////////////////////////////////////////////
  // application setup                                      //
  ////////////////////////////////////////////////////////////

  BulkSendHelper tcp ("ns3::TcpSocketFactory", Address ());
  BulkSendHelper tcp2 ("ns3::TcpSocketFactory", Address ());

  tcp.SetAttribute ("MaxBytes", UintegerValue (0));
  tcp2.SetAttribute ("MaxBytes", UintegerValue (0));

  uint16_t firstPort = 5000;
  ApplicationContainer firstApp;
  InetSocketAddress firstDestAddress (h3_routerIfaces.GetAddress (0), firstPort);
  tcp.SetAttribute ("Remote", AddressValue (firstDestAddress));
  firstApp = tcp.Install (h1);
  firstApp.Start (Seconds (5));
  firstApp.Stop (stopTime - Seconds (1));

  ApplicationContainer firstApp2;
  uint16_t secondPort = 6000;
  InetSocketAddress secondDestAddress (h3_routerIfaces.GetAddress (0), secondPort);
  tcp2.SetAttribute ("Remote", AddressValue (secondDestAddress));
  firstApp2 = tcp2.Install (h2);
  firstApp2.Start (Seconds (5));
  firstApp2.Stop (stopTime - Seconds (1));

  Address firstSinkAddress (InetSocketAddress (Ipv4Address::GetAny (), firstPort));
  PacketSinkHelper firstSinkHelper ("ns3::TcpSocketFactory", firstSinkAddress);
  ApplicationContainer firstSinkApp;
  firstSinkApp = firstSinkHelper.Install (h3);
  firstSinkApp.Start (Seconds (5));
  firstSinkApp.Stop (stopTime - MilliSeconds (500));

  Address secondSinkAddress (InetSocketAddress (Ipv4Address::GetAny (), secondPort));
  PacketSinkHelper secondSinkHelper ("ns3::TcpSocketFactory", secondSinkAddress);
  ApplicationContainer secondSinkApp;
  secondSinkApp = secondSinkHelper.Install (h3);
  secondSinkApp.Start (Seconds (5));
  secondSinkApp.Stop (stopTime - MilliSeconds (500));

  // Setup traces that can be hooked now
  Ptr<TrafficControlLayer> tc;
  Ptr<QueueDisc> qd;

  // Setup scheduled traces; TCP traces must be hooked after socket creation
  Simulator::Schedule (Seconds (5) + MilliSeconds (100), &ScheduleFirstTcpRttTraceConnection, &firstTcpRttOfStream);
  Simulator::Schedule (Seconds (5) + MilliSeconds (100), &ScheduleFirstTcpCwndTraceConnection, &firstTcpCwndOfStream);
  Simulator::Schedule (Seconds (5) + MilliSeconds (100), &ScheduleFirstPacketSinkConnection);
  Simulator::Schedule (throughputSamplingInterval, &TraceFirstThroughput, &firstTcpThroughputOfStream, throughputSamplingInterval);
  Simulator::Schedule (Seconds (5) + MilliSeconds (100), &ScheduleSecondTcpRttTraceConnection, &secondTcpRttOfStream);
  Simulator::Schedule (Seconds (5) + MilliSeconds (100), &ScheduleSecondTcpCwndTraceConnection, &secondTcpCwndOfStream);
  Simulator::Schedule (throughputSamplingInterval, &TraceSecondThroughput, &secondTcpThroughputOfStream, throughputSamplingInterval);
  Simulator::Schedule (marksSamplingInterval, &TraceDropsFrequency, &routerDropsFrequencyOfStream, marksSamplingInterval);

  if (enablePcap)
    {
      p2p.EnablePcapAll (dir+"TcpBic-", false);
    }

  Simulator::Stop (stopTime);
  Simulator::Run ();

  firstTcpCwndOfStream.close ();
  firstTcpRttOfStream.close ();
  firstTcpThroughputOfStream.close ();
  secondTcpCwndOfStream.close ();
  secondTcpRttOfStream.close ();
  secondTcpThroughputOfStream.close ();
  routerDropOfStream.close ();
  routerDropsFrequencyOfStream.close ();
  routerLengthOfStream.close ();
}


