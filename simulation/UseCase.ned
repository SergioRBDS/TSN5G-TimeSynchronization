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

package simulation;

import inet.node.ethernet.Eth10G;
//import inet.node.inet.StandardHost;
//import inet.node.ethernet.EthernetSwitch;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;

//import inet.linklayer.configurator.MacForwardingTableConfigurator;

import simu5g.nodes.NR.NRUe;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.Upf;

import src.FiveGNetworkBase;
import src.NRUeEth;

network UseCase extends FiveGNetworkBase
{
    parameters:
        **.ipv4.arp.typename = "GlobalArp";
        @display("bgb=1449.312,739.536");
    submodules:
        upf: Upf {
            @display("p=706.8,299.08798");
        }
        gNB: gNodeB {
            @display("p=797.568,299.08798");
        }
        ue: NRUeEth {
            @display("p=879.40796,299.08798");
        }
        NSTT: TsnDevice {
            ethernet.typename = "EthernetEncapsulation";
            eth[*].typename = "EthernetInterface";
            forwarding = true;
            @display("p=533.97754,227.03127");
        }
        gPTPAux: TsnDevice {
            ethernet.typename = "EthernetEncapsulation";
            eth[*].typename = "EthernetInterface";
            forwarding = true;
            @display("p=635.6875,430.45126");
        }
        DSTT: TsnDevice {
            ethernet.typename = "EthernetEncapsulation";
            eth[*].typename = "EthernetInterface";
            forwarding = true;
            @display("p=949.344,299.08798");
        }
        receiverTSN: TsnDevice {
            @display("p=1218.672,364.56");
        }
        switch5GC: TsnSwitch {
            @display("p=447.888,364.56");
        }
        switchTSN: TsnSwitch {
            @display("p=1084.752,366.048");
        }
        endHost: TsnDevice {
            @display("p=322.896,367.53598");
        }
    connections:
        switch5GC.ethg++ <--> Eth10G <--> endHost.ethg++;
        gPTPAux.pppg++ <--> Eth10G <--> upf.filterGate;
        NSTT.ethg++ <--> Eth10G <--> gPTPAux.ethg++;
        //NSTT.pppg++ <--> Eth10G <--> upf.filterGate;

        upf.pppg++ <--> Eth10G <--> gNB.ppp;
        ue.ethg++ <--> Eth10G <--> DSTT.ethg++;
        DSTT.ethg++ <--> Eth10G <--> switchTSN.ethg++;
        switchTSN.ethg++ <--> Eth10G <--> receiverTSN.ethg++;
        NSTT.ethg++ <--> Eth10G <--> switch5GC.ethg++;
}
