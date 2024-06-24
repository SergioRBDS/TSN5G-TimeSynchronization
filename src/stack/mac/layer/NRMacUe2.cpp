/*
 * NRMacUe2.cpp
 *
 *  Created on: 30 de jan. de 2024
 *      Author: vboxuser
 */

#include "NRMacUe2.h"

#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include <cstring>

Define_Module(NRMacUe2);

using namespace inet;
using namespace omnetpp;

NRMacUe2::NRMacUe2() : NRMacUe(){
}

NRMacUe2::~NRMacUe2() {
}

void NRMacUe2::initialize(int stage)
{
    LteMacUeD2D::initialize(stage);
    if (stage == inet::INITSTAGE_NETWORK_LAYER)
    {
        if (strcmp(getFullName(),"nrMac") == 0)
            nodeId_ = getAncestorPar("nrMacNodeId");
        else
            nodeId_ = getAncestorPar("macNodeId");
        const char* intHostAddress = getAncestorPar("intHostAddress").stringValue();
        if (strcmp(intHostAddress, "") != 0)
        {
            // register the address of the external host to enable forwarding
            cStringTokenizer tokenizer(intHostAddress);
            const char *interface2Connect= "eth0"; // interface name
            const char *token;
            while ((token = tokenizer.nextToken()) != nullptr) {
                EV << "Address: " << token << endl;
                cModule *intHostModule = getModuleByPath(token);
                if (!intHostModule) {
                    EV << "Dispositivo não encontrado: " << intHostAddress << endl;
                    return;
                }
                else
                    EV << intHostModule->getFullName() << endl;

                IInterfaceTable *intHostInterfaceTable = dynamic_cast<IInterfaceTable *>(intHostModule->getSubmodule("interfaceTable"));
                if (!intHostInterfaceTable) {
                    EV << "Tabela de interface não encontrada para o dispositivo: " << intHostAddress << endl;
                    return;
                }
                else
                    EV << intHostInterfaceTable->getFullPath() << endl;

                NetworkInterface *intHostIndInterface = intHostInterfaceTable->findInterfaceByName(interface2Connect);
                if (!intHostIndInterface) {
                    EV << "Interface IPv4 com o nome '" << interface2Connect << "' não encontrada para o módulo externo: " << intHostAddress << endl;
                    return;
                }
                else
                    EV << "ok" << endl;

                auto ipv4intHost = intHostIndInterface->getProtocolData<Ipv4InterfaceData>();
                if(ipv4intHost == nullptr)
                    throw new cRuntimeError("no Ipv4 interface data - cannot bind node %i", nodeId_);
                else
                    EV << ipv4intHost->getIPAddress() << endl;
                binder_->setMacNodeId(ipv4intHost->getIPAddress(), nodeId_);
                // register the address of the external host to enable forwarding
            }


        }
    }
}

