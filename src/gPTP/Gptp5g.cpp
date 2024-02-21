/*
 * Gptp5g.cpp
 *
 *  Created on: 6 de fev. de 2024
 *      Author: vboxuser
 */

#include "Gptp5g.h"

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/linklayer/ppp/PppFrame_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

using namespace inet;

Define_Module(Gptp5g);

void Gptp5g::initialize(int stage){
    Gptp::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        tt = par("TT");
        localPort = par("localPort");
        destPort = par("destPort");
        domainNumber2 = par("domainNumber");
    }
    if (stage == INITSTAGE_LINK_LAYER){
        interfaceTable2.reference(this, "interfaceTableModule", true);
        dAddress = par("dAddress");
        if(tt){
            auto nic = CHK(interfaceTable2->findInterfaceByName(par("ipNIC")));
            ipPortId = nic->getInterfaceId();
            selfMsgBindSocket = new ClockEvent("bindSocket",106);
            scheduleClockEventAfter(clock->getClockTime(),selfMsgBindSocket);
        }
    }
}

void Gptp5g::handleSelfMessage(cMessage *msg){
    switch(msg->getKind()){
        case 106:
            processStart();
            break;
        default:
            Gptp::handleSelfMessage(msg);
    }
}

void Gptp5g::sendPacketToNIC(Packet *packet, int portId){
    auto networkInterface = interfaceTable2->getInterfaceById(portId);
    EV_INFO << "Sending " << packet << " to output interface = " << networkInterface->getInterfaceName() << ".\n";
    packet->addTag<InterfaceReq>()->setInterfaceId(portId);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::gptp);
    packet->addTag<DispatchProtocolInd>()->setProtocol(&Protocol::gptp);
    auto protocol = networkInterface->getProtocol();
    EV << "!" << ipPortId << " " << portId << endl;
    if (protocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
    if(portId==ipPortId){
        L3Address destAddr;
        L3AddressResolver().tryResolve(dAddress,destAddr);
        emit(packetSentSignal,packet);
        socket.sendTo(packet, destAddr, destPort);
    }else{
        send(packet, "socketOut");
    }
}

void Gptp5g::receiveSignal(cComponent *source, simsignal_t simSignal, cObject *obj, cObject *details)
{
    EV << "recebeu signal" << endl;
    Enter_Method("%s", cComponent::getSignalName(simSignal));

    if (simSignal != receptionEndedSignal && simSignal != transmissionEndedSignal)
        return;

    auto ethernetSignal = check_and_cast<cPacket *>(obj);
    //auto packet = check_and_cast_nullable<Packet *>(ethernetSignal->getEncapsulatedPacket());
    auto packet = check_and_cast<Packet *>(ethernetSignal->getEncapsulatedPacket());
    EV << "recebeu signal" << endl;
    if (!packet)
        EV << "hmmmmm" << endl;
    //    return;

    auto gptp = extractGptpHeader(packet);
    if (!gptp){
        EV << "n eh gptp basico" << endl;
        if(tt){
            EV << "eh para o 5g" << endl;
            auto gptpIp = extractGptpHeaderIp(packet);
            if (gptpIp->getDomainNumber() != domainNumber2)
                return;
            EV << "td certo por aqui" << endl;

            if (simSignal == receptionEndedSignal)
                packet->addTagIfAbsent<GptpIngressTimeInd>()->setArrivalClockTime(clock->getClockTime());
            else if (simSignal == transmissionEndedSignal)
                handleDelayOrSendFollowUp(gptpIp, source);
        }
        else
            return;
    }
    else{
        if (gptp->getDomainNumber() != domainNumber2)
            return;

        if (simSignal == receptionEndedSignal)
            packet->addTagIfAbsent<GptpIngressTimeInd>()->setArrivalClockTime(clock->getClockTime());
        else if (simSignal == transmissionEndedSignal)
            handleDelayOrSendFollowUp(gptp, source);
    }
}


void Gptp5g::processStart(){
    socket.setOutputGate(gate("socketOut"));
    socket.bind(L3Address(),localPort);
}

const GptpBase *Gptp5g::extractGptpHeaderIp(Packet *packet)
{
    b offset;
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (*protocol == Protocol::ethernetPhy){
        const auto& ethPhyHeader = packet->peekAtFront<physicallayer::EthernetPhyHeader>();
        const auto& ethMacHeader = packet->peekDataAt<EthernetMacHeader>(ethPhyHeader->getChunkLength());
        if (ethMacHeader->getTypeOrLength() != ETHERTYPE_IPv4){
            return nullptr;
        }
        offset = ethPhyHeader->getChunkLength() + ethMacHeader->getChunkLength();
    }else if(0==1){

        //if(pppHeader->getProtocol() != 33){
        //    offset = PPP_HEADER_LENGTH;
        //}
    }
    const auto& ipv4Header = packet->peekDataAt<Ipv4Header>(offset);
    const auto& udpHeader = packet->peekDataAt<UdpHeader>(offset+ipv4Header->getChunkLength());

    offset = offset + udpHeader->getChunkLength() + ipv4Header->getChunkLength();

    return packet->peekDataAt<GptpBase>(offset).get();
}
