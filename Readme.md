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

Make sure the projects have the reference project properly, for that, click in the project with the right button, go to properties, the reference project. Simu5G must have INET and our model must have INET and SImu5G

## Usage

After installing all dependencies, the user can run the "omnetpp.ini" file and see how the synchronization works. In the same file, the simulation parameters can be changed such as numerology (u), carrier frequency and synchronization parameters

The netwrork topology can also be changed in the "usecase.ned" itself or creating another ned file and changing the name in the "omnetpp.ini"

Follow the example to have some idea in how changed such parameters an how create your own topology
