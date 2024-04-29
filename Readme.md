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

Open OMNeT++ and select the workspace folder as the workspace of OMNeT++. Import the models (go to files->import, select the folder with the models and select the models to import)

## Usage

After installing all dependencies, the user can run the omnetpp.ini file and see how the synchronization works.

the simulation parameters can also be changed in that file by changing the 5G parameters of the Simu5G such as numerology (u) and carrier frequency. The synchronization parameters are the same of the gPTP from INET such as the time the synchronization starts, period for recalculate peer delay and the time to send sync messages.

the use cases can be also modified at the usecase.ned file using devices TSN capable
