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

network UseCase0 extends FiveGNetworkBase
{
    parameters:
        **.ipv4.arp.typename = "GlobalArp";

        @figure[noConnMod](type=rectangle; pos=0,0; size=200,600; lineColor=#808080; cornerRadius=5; fillColor=#ff0000; fillOpacity=0.2);
        @figure[noConnMod.title](type=text; pos=0,0; anchor=ne; text="");
        @figure[TSN](type=rectangle; pos=200,0; size=320,600; lineColor=#808080; cornerRadius=5; fillColor=#00ff00; fillOpacity=0.1);
        @figure[TSN.title](type=text; pos=280,10; anchor=ne; text="TSN domain 1");
        @figure[FiveG](type=rectangle; pos=520,0; size=430,600; lineColor=#808080; cornerRadius=5; fillColor=#0000ff; fillOpacity=0.1);
        @figure[FiveG.title](type=text; pos=550,10; anchor=ne; text="5GS");
        @figure[TSN2](type=rectangle; pos=950,0; size=350,600; lineColor=#808080; cornerRadius=5; fillColor=#000000; fillOpacity=0.2);
        @figure[TSN2.title](type=text; pos=1030,10; anchor=ne; text="TSN domain 2");
        @figure[submodules];
        @display("bgb=1300,600");
    submodules:
        upf: UpfEth {
            @display("p=850,160;is=l");
        }
        gNB: gNBEth {
            @display("p=730,160;is=vl");
        }
        ue: NRUeEth {
            @display("p=600,160");
        }
        DSTT: TT {
            @display("p=520,160;is=l");
        }
        NSTT: TT {
            gptp.translatorType = "NS_TT";
            @display("p=950,160;is=l");
        }
        Actuator: TsnDevice {
            @display("p=250,220;i=misc/drone;is=l");
        }
        switch5GC: TsnSwitch {
            @display("p=1090,160;is=l;bgb=circle,green,30");
        }
        switchTSN: TsnSwitch {
            @display("p=380,160;is=l");
        }
        PLC: TsnDevice {
            @display("p=1220,160;i=device/server;is=l");
        }
        Sensor: TsnDevice {
            @display("p=250,100;i=misc/sensor2;is=l");
        }
    connections:
        PLC.ethg++ <--> Eth10G <--> switch5GC.ethg++;
        NSTT.ethg++ <--> Eth10G <--> upf.filterGate;
        switch5GC.ethg++ <--> Eth10G <--> NSTT.ethg++;
        upf.ethg++ <--> Eth10G <--> gNB.ethg;
        ue.ethg++ <--> Eth10G <--> DSTT.ethg++;
        DSTT.ethg++ <--> Eth10G <--> switchTSN.ethg++;
        switchTSN.ethg++ <--> Eth10G <--> Sensor.ethg++;
        switchTSN.ethg++ <--> Eth10G <--> Actuator.ethg++;

}


network UseCase extends FiveGNetworkBase
{
    parameters:
        **.ipv4.arp.typename = "GlobalArp";
        @figure[noConnMod](type=rectangle; pos=0,0; size=200,600; lineColor=#808080; cornerRadius=5; fillColor=#ff0000; fillOpacity=0.1);
        @figure[noConnMod.title](type=text; pos=0,0; anchor=ne; text="");
        @figure[TSN](type=rectangle; pos=200,0; size=320,300; lineColor=#808080; cornerRadius=5; fillColor=#00ff00; fillOpacity=0.1);
        @figure[TSN.title](type=text; pos=280,10; anchor=ne; text="TSN domain 1");
        @figure[FiveG](type=rectangle; pos=520,0; size=430,600; lineColor=#808080; cornerRadius=5; fillColor=#0000ff; fillOpacity=0.1);
        @figure[FiveG.title](type=text; pos=550,10; anchor=ne; text="5GS");
        @figure[TSN2](type=rectangle; pos=950,0; size=350,600; lineColor=#808080; cornerRadius=5; fillColor=#000000; fillOpacity=0.2);
        @figure[TSN2.title](type=text; pos=1040,10; anchor=ne; text="TSN domain DN");
        @figure[TSN3](type=rectangle; pos=200,300; size=320,300; lineColor=#808080; cornerRadius=5; fillColor=#00ffff; fillOpacity=0.1);
        @figure[TSN3.title](type=text; pos=280,310; anchor=ne; text="TSN domain 2");
        @figure[submodules];
        @display("bgb=1300,600,title=");
    submodules:
        upf: UpfEth {
            @display("p=850,284;is=l");
        }
        gNB: gNBEth {
            @display("p=730,284;is=vl");
        }
        ue: NRUeEth {
            @display("p=600,160");
        }
        DSTT: TT {
            @display("p=520,160;is=l");
        }
        NSTT: TT {
            gptp.translatorType = "NS_TT";
            @display("p=950,284;is=l");
        }
        Actuator: TsnDevice {
            @display("p=250,220;i=misc/drone;is=l");
        }
        switch5GC: TsnSwitch {
            @display("p=1090,284;is=l");
        }
        switchTSN: TsnSwitch {
            @display("p=380,160;is=l");
        }
        PLC: TsnDevice {
            @display("p=1220,284;i=device/server;is=l");
        }
        Sensor: TsnDevice {
            @display("p=250,100;i=misc/sensor2;is=l");
        }
        ue2: NRUeEth {
            @display("p=600,450");
        }
        DSTT2: TT {
            @display("p=520,450;is=l");
        }
        switchTSN2: TsnSwitch {
            @display("p=380,450;is=l");
        }
        Actuator2: TsnDevice {
            @display("p=250,450;i=misc/drone;is=l");
        }
    connections:
        PLC.ethg++ <--> Eth10G <--> switch5GC.ethg++;
        NSTT.ethg++ <--> Eth10G <--> upf.filterGate;
        switch5GC.ethg++ <--> Eth10G <--> NSTT.ethg++;
        upf.ethg++ <--> Eth10G <--> gNB.ethg;
        ue.ethg++ <--> Eth10G <--> DSTT.ethg++;
        DSTT.ethg++ <--> Eth10G <--> switchTSN.ethg++;
        switchTSN.ethg++ <--> Eth10G <--> Sensor.ethg++;
        switchTSN.ethg++ <--> Eth10G <--> Actuator.ethg++;

        ue2.ethg++ <--> Eth10G <--> DSTT2.ethg++;
        DSTT2.ethg++ <--> Eth10G <--> switchTSN2.ethg++;
        switchTSN2.ethg++ <--> Eth10G <--> Actuator2.ethg++;
}
