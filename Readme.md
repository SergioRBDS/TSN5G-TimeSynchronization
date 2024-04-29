# README
⚠️**This repository is in a state of development**

## About time synchronization between TSN and 5G

Integrating Time-Sensitive Networking (TSN) and wireless networks, like the Fifth Generation of mobile networks (5G), affords deterministic communication with great flexibility and mobility, enabling new services to be used in smart industries.

This work aims to provide advances in simulating this integration using the OMNeT++ simulator and developing extended modules to support it.

The time synchronization between TSN and 5G is based on 3GPP Release 17, capable of synchronization in every supported direction using the 5G as a time-aware system.

___
## Requiriments

- OMNeT++ 6.0.2
- INET - https://github.com/inet-framework 
- Simu5g 1.2.2

## Installation

First, make sure to have Omnet++ installed, the instructions can be found at https://github.com/omnetpp/omnetpp.

Download INET, Simu5G, and our model and separate them in a folder to be your workspace. As INET is being continuously updated, if you need you can download it for our branch inet.

Open OMNeT++, select the workspace folder as the workspace of OMNeT++ and import the models (go to files->import, select the folder with the models and select the models to import)

Make sure the projects have the reference project properly, for that, click in the project with the right button, go to properties, the reference project. Simu5G must have INET and our model must have INET and Simu5G

## Usage

After installing all dependencies, the user can run the "simulation/omnetpp.ini" file and see how the synchronization works. In the same file, the simulation parameters can be changed or added such as numerology (u), carrier frequency and synchronization parameters

The netwrork topology can also be changed in the "simulation/UseCase.ned" itself or creating another ned file and changing the name in the "omnetpp.ini"

Follow the example to have some idea in how changed such parameters an how create your own topology

## Examples

```python
# In the simulation/omnetpp.ini
  network=newNetwork # this must be the name of the network you created
  **.numerologyIndex = $(u=1,2,3,4) # Here we are defining a varable named u which the values can be from 1 to 4
  **.carrierFrequency = 2,4GHz
  *.switch*.gptp.masterPorts = ["eth0","eth2"] # the master ports and the slave port must be explicity defined
  *.switch*.gptp.slavePort = "eth1"
```
```python
# For a new network file
package simulation;
import inet.node.ethernet.Eth1G;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;
import inet.node.inet.Router;

import src.FiveGNetworkBase;
import src.NRUeEth;
import src.gNBEth;
import src.UpfEth;
import src.TT;

network UseCase extends FiveGNetworkBase # the file must extend this network, which has esencial submodules to make the simuation works
{
    parameters:
        **.ipv4.arp.typename = "GlobalArp";
        @display("bgb=1569.0674,1097.1675");
    submodules: # the first name is a name of your choice (except for "upf" since Simu5G asks for it be named this way), the second is the module type
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
        PLC2: TsnDevice {
            @display("p=1510.08,426.67624");
        }
        Sensor: TsnDevice {
            @display("p=471.89996,359.82373");
        }
        Actuator2: TsnDevice {
            @display("p=471.89996,359.82373");
        }
    connections:
        PLC.ethg++ <--> Eth1G <--> switch5GC.ethg++;
        PLC2.ethg++ <--> Eth1G <--> switch5GC.ethg++;
        NSTT.ethg++ <--> Eth1G <--> upf.filterGate;
        switch5GC.ethg++ <--> Eth1G <--> NSTT.ethg++;
        upf.ethg++ <--> Eth1G <--> gNB.ethg;
        ue.ethg++ <--> Eth1G <--> DSTT.ethg++;
        DSTT.ethg++ <--> Eth1G <--> switchTSN.ethg++;
        switchTSN.ethg++ <--> Eth1G <--> Sensor.ethg++;
        switchTSN.ethg++ <--> Eth1G <--> Actuator.ethg++;
        switchTSN.ethg++ <--> Eth1G <--> Actuator2.ethg++;
}
```
