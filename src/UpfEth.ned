//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package src;


import inet.common.MessageDispatcher;
import inet.linklayer.ppp.PppInterface;
import inet.transportlayer.udp.Udp;
import inet.node.base.NetworkLayerNodeBase;
import ned.IdealChannel;
//import simu5g.corenetwork.gtp.GtpUser;
import simu5g.corenetwork.trafficFlowFilter.TrafficFlowFilter;

import inet.applications.contract.IApp;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.linklayer.contract.IEthernetInterface;

import src.corenetwork.gtp.GtpUserEth;


module UpfEth extends NetworkLayerNodeBase // não extende UPF pq troquei o GTP // é o UPF+NSTT
{
    parameters:
        @display("bgb=1691.496,1102.572;i=abstract/switch");
        @figure[applicationLayer](type=rectangle; pos=250,6; size=1000,130; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
        @figure[applicationLayer.title](type=text; pos=1245,11; anchor=ne; text="application layer");
        @figure[transportLayer](type=rectangle; pos=250,156; size=1000,130; fillColor=#ff0000; lineColor=#808080; cornerRadius=5; fillOpacity=0.1);
        @figure[transportLayer.title](type=text; pos=1245,161; anchor=ne; text="transport layer");
        @figure[submodules];

        string nodeType = default("UPF");   // do NOT change!
        forwarding = default(true);
        multicastForwarding = default(false);

        bool hasTimeSynchronization = default(false); // enable IEEE 802.1 AS time synchronization
        clock.typename = default(hasTimeSynchronization ? "SettableClock" : ""); // enable explicit local clock model
    gates:
        // this gate is used for the UPF connected to the external data network
        // it must be left unconnected for intermediate UPFs 
        inout filterGate @loose @labels(EthFrame-conn);
    submodules:
        // modulos necessarios para ocnexão ethernet com nós externos ao 5G, e separados dos outro para q percorram o caminho certo
        ethIf: <default("LayeredEthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=1423.602,1025.082");
        }
        encapIf: <default("EthernetLayer")> like IEthernetLayer if typename != "" {
            parameters:
                //registerProtocol = true;
                @display("p=1374.894,823.608");
        }


        udp: Udp {
            parameters:
                @display("p=374.0425,201.4075");
        }
        trafficFlowFilter: TrafficFlowFilter {
            parameters:
                @display("p=881.7925,79.5475");
                ownerType = parent.nodeType;
        }
        gtp_user: GtpUserEth { // no UPF é o gtp do simu5G
            parameters:
                @display("p=525,80");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=750,296;b=1000,5,,,,1");
        }
        at: MessageDispatcher {
            parameters:
                @display("p=750,146;b=1000,5,,,,1");
        }
        //gptp: <default("Gptp5g")> like IApp if hasTimeSynchronization { // protocolo com a adição de IP
        //    gptpNodeType = default("SLAVE_NODE");
        //    slavePort = default("eth0");
        //    masterPorts = default([]);
        //    @display("p=371.952,79.704");
        //}
    connections:

        gtp_user.socketOut --> at.in++;
        gtp_user.socketIn <-- at.out++;

        ethIf.upperLayerOut --> encapIf.lowerLayerIn;
        ethIf.upperLayerIn <-- encapIf.lowerLayerOut;

        trafficFlowFilter.gtpUserGateOut --> gtp_user.trafficFlowFilterGate;

        // at dispatcher to transport layer connections
        at.out++ --> udp.appIn;
        at.in++ <-- udp.appOut;

        at.out++ --> tn.in++;
        at.in++ <-- tn.out++;

        // transport layer to tn dispatcher connections
        udp.ipOut --> tn.in++;
        udp.ipIn <-- tn.out++;

        // tn dispatcher to network layer connections
        tn.out++ --> ipv4.transportIn if hasIpv4;
        tn.in++ <-- ipv4.transportOut if hasIpv4;

        tn.out++ --> ipv6.transportIn if hasIpv6;
        tn.in++ <-- ipv6.transportOut if hasIpv6;

        tn.out++ --> generic.transportIn if hasGn;
        tn.in++ <-- generic.transportOut if hasGn;

        tn.out++ --> nl.in++;
        tn.in++ <-- nl.out++;

        ethIf.phys <--> filterGate;


        //if hasTimeSynchronization {
        //    gptp.socketOut --> at.in++;
        //    at.out++ --> gptp.socketIn;
        //}

        encapIf.upperLayerOut --> trafficFlowFilter.internetFilterGateIn;
        gtp_user.pppGate --> IdealChannel --> encapIf.upperLayerIn;
}
