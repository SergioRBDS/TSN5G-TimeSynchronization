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

import inet.networks.base.TsnNetworkBase;
//import inet.linklayer.configurator.MacForwardingTableConfigurator;

import ned.IdealChannel;
import simu5g.nodes.NR.NRUe;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.Upf;

import src.FiveGNetworkBase;
import src.NRUeEth;
import src.gNBEth;
import src.UpfEth;
import src.TT;

network wiredCase extends TsnNetworkBase
{
    parameters:
        int numIntDevices = default(0);
        **.ipv4.arp.typename = "GlobalArp";
        @display("bgb=1569.2401,739.2138");
    submodules:
        DSTT: TT {
            @display("p=509.25873,220.21999");
        }
        NSTT: TT {
            @display("p=1384.2399,220.21999");
        }
        intDevice[numIntDevices]: TsnSwitch {
        }
    connections:
        DSTT.ethg++ <--> Eth10G <--> NSTT.ethg++ if numIntDevices == 0;

        DSTT.ethg++ <--> Eth10G <--> intDevice[0].ethg++ if numIntDevices > 0;
        NSTT.ethg++ <--> Eth10G <--> intDevice[numIntDevices-1].ethg++ if numIntDevices > 0;
        for i=0..numIntDevices-2 {
            intDevice[i].ethg++ <--> Eth10G <--> intDevice[i+1].ethg++;
        }
}
