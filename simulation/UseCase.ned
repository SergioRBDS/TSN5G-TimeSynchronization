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

import inet.node.ethernet.Eth1G;
import inet.node.ethernet.Eth10G;
//import inet.node.inet.StandardHost;
//import inet.node.ethernet.EthernetSwitch;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;
import inet.node.inet.Router;

//import inet.linklayer.configurator.MacForwardingTableConfigurator;

import simu5g.nodes.NR.NRUe;
import simu5g.nodes.NR.gNodeB;
import simu5g.nodes.Upf;

import src.FiveGNetworkBase;
import src.NRUeEth;
import src.gNBEth;
import src.UpfEth;
import src.TT;

network UseCase extends FiveGNetworkBase
{
    parameters:
        **.ipv4.arp.typename = "GlobalArp";
        @display("bgb=1569.0674,1097.1675");
    submodules:
        upf: UpfEth {
            @display("p=1112.8975,426.67624");
        }
        gNB: gNBEth {
            @display("p=985.0912,426.67624");
        }
        ue: NRUeEth {
            @display("p=827.7912,426.67624");
        }
        DSTT: TT {
            @display("p=742.074,424.986");
        }
        NSTT: TT {
            gptp.translatorType = "NS_TT";
            @display("p=1197.888,424.986");
        }
        Actuator: TsnDevice {
            @display("p=471.89996,487.62997");
        }
        switch5GC: TsnSwitch {
            @display("p=1368.5099,424.71");
        }
        switchTSN: TsnSwitch {
            @display("p=607.5712,424.71");
        }
        PLC: TsnDevice {
            @display("p=1510.08,426.67624");
        }
        Sensor: TsnDevice {
            @display("p=471.89996,359.82373");
        }
    connections:
        PLC.ethg++ <--> Eth1G <--> switch5GC.ethg++;
        NSTT.ethg++ <--> Eth1G <--> upf.filterGate;
        switch5GC.ethg++ <--> Eth1G <--> NSTT.ethg++;
        upf.ethg++ <--> Eth1G <--> gNB.ethg;
        ue.ethg++ <--> Eth1G <--> DSTT.ethg++;
        DSTT.ethg++ <--> Eth1G <--> switchTSN.ethg++;
        switchTSN.ethg++ <--> Eth1G <--> Sensor.ethg++;
        switchTSN.ethg++ <--> Eth1G <--> Actuator.ethg++;

}


network UseCase2 extends FiveGNetworkBase
{
    parameters:
        **.ipv4.arp.typename = "GlobalArp";
        @display("bgb=1569.0674,1097.1675");
    submodules:
        upf: UpfEth {
            @display("p=1112.8975,426.67624");
        }
        gNB: gNBEth {
            @display("p=985.0912,426.67624");
        }
        ue: NRUeEth {
            @display("p=827.7912,426.67624");
        }
        DSTT: TT {
            @display("p=742.074,424.986");
        }
        NSTT: TT {
            gptp.translatorType = "NS_TT";
            @display("p=1197.888,424.986");
        }
        Actuator: TsnDevice {
            @display("p=471.89996,487.62997");
        }
        switch5GC: TsnSwitch {
            @display("p=1368.5099,424.71");
        }
        switchTSN: TsnSwitch {
            @display("p=607.5712,424.71");
        }
        PLC: TsnDevice {
            @display("p=1510.08,426.67624");
        }
        Sensor: TsnDevice {
            @display("p=471.89996,359.82373");
        }
        ue2: NRUeEth {
            @display("p=827.7912,626.67624");
        }
        DSTT2: TT {
            @display("p=742.074,624.986");
        }
        switchTSN2: TsnSwitch {
            @display("p=607.5712,624.71");
        }
		Actuator2: TsnDevice {
            @display("p=471.89996,687.62997");
        }
        connections:
        PLC.ethg++ <--> Eth1G <--> switch5GC.ethg++;
        NSTT.ethg++ <--> Eth1G <--> upf.filterGate;
        switch5GC.ethg++ <--> Eth1G <--> NSTT.ethg++;
        upf.ethg++ <--> Eth1G <--> gNB.ethg;
        ue.ethg++ <--> Eth1G <--> DSTT.ethg++;
        DSTT.ethg++ <--> Eth1G <--> switchTSN.ethg++;
        switchTSN.ethg++ <--> Eth1G <--> Sensor.ethg++;
        switchTSN.ethg++ <--> Eth1G <--> Actuator.ethg++;

        ue2.ethg++ <--> Eth1G <--> DSTT2.ethg++;
        DSTT2.ethg++ <--> Eth1G <--> switchTSN2.ethg++;
        switchTSN2.ethg++ <--> Eth1G <--> Actuator2.ethg++;
}
