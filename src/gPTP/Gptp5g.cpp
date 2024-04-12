/*
 * Gptp5g.cpp
 *
 *  Created on: 6 de fev. de 2024
 *      Author: vboxuser
 */

#include "Gptp5g.h"

#include "inet/linklayer/ieee8021as/GptpPacket_m.h"


#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/linklayer/ppp/PppFrame_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/networklayer/common/NetworkInterface.h"


using namespace inet;

Define_Module(Gptp5g);

simsignal_t Gptp5g::localTimeSignal = cComponent::registerSignal("localTime");
simsignal_t Gptp5g::timeDifferenceSignal = cComponent::registerSignal("timeDifference");
simsignal_t Gptp5g::rateRatioSignal = cComponent::registerSignal("rateRatio");
simsignal_t Gptp5g::peerDelaySignal = cComponent::registerSignal("peerDelay");

// MAC address:
//   01-80-C2-00-00-02 for TimeSync (ieee 802.1as-2020, 13.3.1.2)
//   01-80-C2-00-00-0E for Announce and Signaling messages, for Sync, Follow_Up, Pdelay_Req, Pdelay_Resp, and Pdelay_Resp_Follow_Up messages
const MacAddress Gptp5g::GPTP_MULTICAST_ADDRESS("01:80:C2:00:00:0E");

// EtherType:
//   0x8809 for TimeSync (ieee 802.1as-2020, 13.3.1.2)
//   0x88F7 for Announce and Signaling messages, for Sync, Follow_Up, Pdelay_Req, Pdelay_Resp, and Pdelay_Resp_Follow_Up messages

Gptp5g::~Gptp5g()
{
    cancelAndDeleteClockEvent(selfMsgBindSocket);
}

void Gptp5g::initialize(int stage){

    ClockUserModuleBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL){
        gptpNodeType = static_cast<GptpNodeType>(cEnum::get("GptpNodeType", "inet")->resolve(par("gptpNodeType")));
        translatorType = static_cast<TranslatorType>(cEnum::get("TranslatorType")->resolve(par("translatorType")));


        localPort = par("localPort");
        destPort = par("destPort");
        std::hash<std::string> strHash;
        clockIdentity = strHash(getFullPath());

        binder_ = check_and_cast<BinderTSN5G*>(getSimulation()->getModuleByPath("binder"));
    }
    if (stage == INITSTAGE_LINK_LAYER){
        peerDelay = 0;
        receivedTimeSync = CLOCKTIME_ZERO;
        gmRateRatio = 1.0;
        correctionField = par("correctionField");

        interfaceTable.reference(this, "interfaceTableModule", true);

        const char *TSNPort = par("TSNPort");
        if (*TSNPort) {
            auto nic = CHK(interfaceTable->findInterfaceByName(TSNPort));
            TSNPortId = nic->getInterfaceId();
            nic->subscribe(transmissionEndedSignal, this);
            nic->subscribe(receptionEndedSignal, this);
        }
        else
            throw cRuntimeError("Parameter error: Missing slave port for TSN Translator");

        const char *fiveGPort = par("fiveGPort");
        if (*fiveGPort) {
            auto nic = CHK(interfaceTable->findInterfaceByName(fiveGPort));
            fiveGPortId = nic->getInterfaceId();
            if (fiveGPortId == TSNPortId)
                throw cRuntimeError("Parameter error: the port '%s' specified both master and slave port", fiveGPort);
            nic->subscribe(transmissionEndedSignal, this);
            nic->subscribe(receptionEndedSignal, this);
        }

        auto networkInterfaceTSNPort = interfaceTable->getInterfaceById(TSNPortId);
        if (!networkInterfaceTSNPort->matchesMulticastMacAddress(GPTP_MULTICAST_ADDRESS))
            networkInterfaceTSNPort->addMulticastMacAddress(GPTP_MULTICAST_ADDRESS);
        auto networkInterface5GPort = interfaceTable->getInterfaceById(fiveGPortId);
        if (!networkInterface5GPort->matchesMulticastMacAddress(GPTP_MULTICAST_ADDRESS))
            networkInterface5GPort->addMulticastMacAddress(GPTP_MULTICAST_ADDRESS);


        registerProtocol(Protocol::gptp, gate("socketOut"), gate("socketIn"));

        selfMsgBindSocket = new ClockEvent("bindSocket",106);
        scheduleClockEventAfter(clock->getClockTime(),selfMsgBindSocket);

        auto slaveDomains = check_and_cast<cValueArray *>(par("slaveDomains").objectValue())->asIntVector();
        for(int d : slaveDomains){
            domainNumbers.insert(d);
            binder_->setTranslatorSlaves(d,getParentModule()->getName());
        }
        auto masterDomains = check_and_cast<cValueArray *>(par("masterDomains").objectValue())->asIntVector();
        for (int d : masterDomains){
            domainNumbers.insert(d);
            binder_->setTranslatorMasters(d, getParentModule()->getName());
        }
        cout << domainNumbers.size() << endl;
        WATCH(peerDelay);

    }
}

void Gptp5g::handleSelfMessage(cMessage *msg)
{
    switch(msg->getKind()) {
        case 106:
            socket.setOutputGate(gate("socketOut"));
            socket.bind(L3Address(),localPort);
            break;
        default:
            throw cRuntimeError("Unknown self message (%s)%s, kind=%d", msg->getClassName(), msg->getName(), msg->getKind());
    }
}


void Gptp5g::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    }
    else {
        Packet *packet = check_and_cast<Packet *>(msg);
        auto gptp = packet->peekAtFront<GptpBase>();
        auto gptpMessageType = gptp->getMessageType();

        /*
        int incomingDomainNumber = gptp->getDomainNumber();
        if(auto it = domainNumbers.find(incomingDomainNumber) == domainNumbers.end()){
            EV_ERROR << "Message " << msg->getClassAndFullName() << " arrived with foreign domainNumber " << incomingDomainNumber << ", dropped\n";
            PacketDropDetails details;
            details.setReason(NOT_ADDRESSED_TO_US);
            emit(packetDroppedSignal, packet, &details);
            return;
        }
*/

        switch (gptpMessageType) {
            case GPTPTYPE_SYNC:
                forwardSync(packet,check_and_cast<const GptpSync *>(gptp.get()));
                break;
            case GPTPTYPE_PDELAY_RESP:
                forwardPdelayResp(packet,check_and_cast<const GptpPdelayResp *>(gptp.get()));
                break;
            case GPTPTYPE_FOLLOW_UP:
                forwardFollowUp(packet, check_and_cast<const GptpFollowUp *>(gptp.get()));
                break;
            case GPTPTYPE_PDELAY_RESP_FOLLOW_UP:
                forwardPdelayRespFollowUp(packet, check_and_cast<const GptpPdelayRespFollowUp *>(gptp.get()));
                break;
            case GPTPTYPE_PDELAY_REQ:
                forwardPdelayReq(packet, check_and_cast<const GptpPdelayReq *>(gptp.get()));
                break;
            default:
                throw cRuntimeError("Unknown gPTP packet type: %d", (int)(gptpMessageType));
        }
    }
}



void Gptp5g::sendPacketFromNSTT(Packet *packet, int incomingNicId, int domainNumber){
    int portId;
    if(incomingNicId == TSNPortId){
        portId = fiveGPortId;
    }else{
        portId = TSNPortId;
    }
    auto networkInterface = interfaceTable->getInterfaceById(portId);
    EV_INFO << "Sending " << packet << " to output interface = " << networkInterface->getInterfaceName() << ".\n";
    packet->removeTagIfPresent<InterfaceReq>();
    packet->removeTagIfPresent<PacketProtocolTag>();
    packet->removeTagIfPresent<DispatchProtocolInd>();
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::gptp);
    packet->addTag<DispatchProtocolInd>()->setProtocol(&Protocol::gptp);
    auto protocol = networkInterface->getProtocol();
    if (protocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();



    L3Address destAddr;
    for (const char* address : binder_->getTranslatorSlaves(domainNumber)){
        if(strcmp(address,"NSTT")==0){
            packet->addTag<InterfaceReq>()->setInterfaceId(TSNPortId);
            send(packet->dup(), "socketOut");
        }else{
            packet->addTag<InterfaceReq>()->setInterfaceId(fiveGPortId);
            L3AddressResolver().tryResolve(address,destAddr);
            emit(packetSentSignal,packet);
            socket.sendTo(packet->dup(), destAddr, destPort);
        }
    }delete packet;

}

void Gptp5g::sendPacketReqFromNSTT(Packet *packet, int incomingNicId, int domainNumber){
    int portId;
    L3Address destAddr;
    const char* address = binder_->getTranslatorMaster(domainNumber);
    if(incomingNicId == TSNPortId){
        portId = fiveGPortId;
    }else{
        if(strcmp(address,"NSTT")==0){
            portId = TSNPortId;
        }else{
            portId = fiveGPortId;
        }
    }
    auto networkInterface = interfaceTable->getInterfaceById(portId);
    EV_INFO << "Sending " << packet << " to output interface = " << networkInterface->getInterfaceName() << ".\n";
    packet->removeTagIfPresent<InterfaceReq>();
    packet->removeTagIfPresent<PacketProtocolTag>();
    packet->removeTagIfPresent<DispatchProtocolInd>();
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::gptp);
    packet->addTag<DispatchProtocolInd>()->setProtocol(&Protocol::gptp);
    auto protocol = networkInterface->getProtocol();
    if (protocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();



    if(strcmp(address,"NSTT")==0){
        packet->addTag<InterfaceReq>()->setInterfaceId(TSNPortId);
        send(packet, "socketOut");
    }else{
        packet->addTag<InterfaceReq>()->setInterfaceId(fiveGPortId);
        L3AddressResolver().tryResolve(address,destAddr);
        emit(packetSentSignal,packet);
        socket.sendTo(packet, destAddr, destPort);
    }

}

void Gptp5g::sendPacketFromDSTT(Packet *packet, int incomingNicId){
    int portId;
    if(incomingNicId == TSNPortId){
        portId = fiveGPortId;
    }else{
        portId = TSNPortId;
    }
    auto networkInterface = interfaceTable->getInterfaceById(portId);
    EV_INFO << "Sending " << packet << " to output interface = " << networkInterface->getInterfaceName() << ".\n";
    packet->removeTagIfPresent<InterfaceReq>();
    packet->removeTagIfPresent<PacketProtocolTag>();
    packet->removeTagIfPresent<DispatchProtocolInd>();
    packet->addTag<InterfaceReq>()->setInterfaceId(portId);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::gptp);
    packet->addTag<DispatchProtocolInd>()->setProtocol(&Protocol::gptp);
    auto protocol = networkInterface->getProtocol();
    if (protocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
    if (portId == fiveGPortId){
        L3Address destAddr;
        L3AddressResolver().tryResolve("NSTT",destAddr);
        emit(packetSentSignal,packet);
        socket.sendTo(packet, destAddr, destPort);
    }else if(portId == TSNPortId){
        send(packet, "socketOut");
    }
}


void Gptp5g::forwardPdelayReq(Packet *packet, const GptpPdelayReq* gptp)
{
    auto incomingNicId = packet->getTag<InterfaceInd>()->getInterfaceId();
    int incomingDomainNumber = gptp->getDomainNumber();
    auto copyPacket = new Packet("GptpPdelayReq");
    copyPacket->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto copyGptp = makeShared<GptpPdelayReq>();
    copyGptp->setDomainNumber(incomingDomainNumber);
    copyGptp->setSourcePortIdentity(gptp->getSourcePortIdentity());
    copyGptp->setSequenceId(gptp->getSequenceId());
    copyGptp->setCorrectionField(CLOCKTIME_ZERO);
    copyPacket->insertAtFront(copyGptp);
    delete packet;
    if (translatorType == DS_TT){
        sendPacketFromDSTT(copyPacket, incomingNicId);
    }
    else if (translatorType == NS_TT){
        sendPacketReqFromNSTT(copyPacket, incomingNicId, incomingDomainNumber);
    }
}


void Gptp5g::forwardPdelayResp(Packet *packet, const GptpPdelayResp* gptp)
{
    auto incomingNicId = packet->getTag<InterfaceInd>()->getInterfaceId();
    int incomingDomainNumber = gptp->getDomainNumber();
    auto copyPacket = new Packet("GptpPdelayResp");
    copyPacket->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto copyGptp = makeShared<GptpPdelayResp>();
    copyGptp->setDomainNumber(incomingDomainNumber);
    copyGptp->setSequenceId(gptp->getSequenceId());
    copyGptp->setRequestingPortIdentity(gptp->getRequestingPortIdentity());
    delete packet;
    if (incomingNicId == TSNPortId){
        copyGptp->setRequestReceiptTimestamp(gptp->getRequestReceiptTimestamp()-reqMsgEgressTimestamp);// manda o tempo de egresso do req modificando o timestap (gptp do inet n usa correctionField no resp)
    }
    else{
        if (translatorType == DS_TT){
            copyGptp->setRequestReceiptTimestamp(gptp->getRequestReceiptTimestamp()+reqMsgIngressTimestamp);// manda o tempo de ingresso do req modificando o timestap (gptp do inet n usa correctionField no resp)
        }
        else if (translatorType == NS_TT){
            if(domainNumbers.find(incomingDomainNumber) != domainNumbers.end()){
                copyGptp->setRequestReceiptTimestamp(gptp->getRequestReceiptTimestamp()+reqMsgIngressTimestamp);// manda o tempo de ingresso do req modificando o timestap (gptp do inet n usa correctionField no resp)
            }else{
                copyGptp->setRequestReceiptTimestamp(gptp->getRequestReceiptTimestamp());// manda o tempo de ingresso do req modificando o timestap (gptp do inet n usa correctionField no resp)
            }
        }

    }
    copyPacket->insertAtFront(copyGptp);
    if (translatorType == DS_TT){
        sendPacketFromDSTT(copyPacket, incomingNicId);
    }
    else if (translatorType == NS_TT){
        sendPacketFromNSTT(copyPacket, incomingNicId, incomingDomainNumber);
    }
}

void Gptp5g::forwardPdelayRespFollowUp(Packet *packet, const GptpPdelayRespFollowUp* gptp)
{
    auto incomingNicId = packet->getTag<InterfaceInd>()->getInterfaceId();
    int incomingDomainNumber = gptp->getDomainNumber();
    auto copyPacket = new Packet("GptpPdelayRespFollowUp");
    copyPacket->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto copyGptp = makeShared<GptpPdelayRespFollowUp>();
    copyGptp->setDomainNumber(incomingDomainNumber);
    copyGptp->setRequestingPortIdentity(gptp->getRequestingPortIdentity());
    copyGptp->setSequenceId(gptp->getSequenceId());
    delete packet;
    if (incomingNicId == TSNPortId){
        rcvdPdelayRespForTSN = false;
        copyGptp->setResponseOriginTimestamp(gptp->getResponseOriginTimestamp()-respMsgIngressTimestamp);
    }
    else{
        if (translatorType == DS_TT){
            copyGptp->setResponseOriginTimestamp(gptp->getResponseOriginTimestamp()+respMsgEgressTimestamp);
        }
        else if (translatorType == NS_TT){
            if(domainNumbers.find(incomingDomainNumber) != domainNumbers.end()){
                copyGptp->setResponseOriginTimestamp(gptp->getResponseOriginTimestamp()+respMsgEgressTimestamp);
            }
            else{
                copyGptp->setResponseOriginTimestamp(gptp->getResponseOriginTimestamp());
            }
        }
    }
    copyPacket->insertAtFront(copyGptp);
    if (translatorType == DS_TT){
        sendPacketFromDSTT(copyPacket, incomingNicId);
    }
    else if (translatorType == NS_TT){
        sendPacketFromNSTT(copyPacket, incomingNicId, incomingDomainNumber);
    }

}



void Gptp5g::forwardSync(Packet *packet, const GptpSync* gptp)
{
    auto incomingNicId = packet->getTag<InterfaceInd>()->getInterfaceId();
    int incomingDomainNumber = gptp->getDomainNumber();
    auto copyPacket = new Packet("GptpSync");
    copyPacket->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto copyGptp = makeShared<GptpSync>();
    copyGptp->setDomainNumber(incomingDomainNumber);
    copyGptp->setSequenceId(gptp->getSequenceId());

    copyPacket->insertAtFront(copyGptp);
    delete packet;
    if (translatorType == DS_TT){
        sendPacketFromDSTT(copyPacket, incomingNicId);
    }
    else if (translatorType == NS_TT){
        sendPacketFromNSTT(copyPacket, incomingNicId, incomingDomainNumber);
    }
}

void Gptp5g::forwardFollowUp(Packet *packet, const GptpFollowUp* gptp)
{
    auto incomingNicId = packet->getTag<InterfaceInd>()->getInterfaceId();
    int incomingDomainNumber = gptp->getDomainNumber();
    auto copyPacket = new Packet("GptpFollowUp");
    copyPacket->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto copyGptp = makeShared<GptpFollowUp>();
    copyGptp->setDomainNumber(incomingDomainNumber);
    copyGptp->setSequenceId(gptp->getSequenceId());
    copyGptp->getFollowUpInformationTLVForUpdate().setRateRatio(gmRateRatio);
    delete packet;
    if (incomingNicId == TSNPortId){
        rcvdGptpSyncForTSN = false;
        copyGptp->setPreciseOriginTimestamp(gptp->getPreciseOriginTimestamp()+gptp->getCorrectionField());
        copyGptp->setCorrectionField(syncMsgIngressTimestamp);
    }
    else{
        copyGptp->setPreciseOriginTimestamp(gptp->getPreciseOriginTimestamp());
        if (translatorType == DS_TT){
            copyGptp->setCorrectionField(syncMsgEgressTimestamp-gptp->getCorrectionField());
        }
        else if (translatorType == NS_TT){
            if(domainNumbers.find(incomingDomainNumber) != domainNumbers.end()){
                copyGptp->setCorrectionField(syncMsgEgressTimestamp-gptp->getCorrectionField());
            }
            else{
                copyGptp->setCorrectionField(gptp->getCorrectionField());
            }
        }
    }
    copyPacket->insertAtFront(copyGptp);
    if (translatorType == DS_TT){
        sendPacketFromDSTT(copyPacket, incomingNicId);
    }
    else if (translatorType == NS_TT){
        sendPacketFromNSTT(copyPacket, incomingNicId, incomingDomainNumber);
    }
}





const GptpBase *Gptp5g::extractGptpHeader(Packet *packet)
{
    b offset;
    auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    if (*protocol != Protocol::ethernetPhy)
        return nullptr;

    const auto& ethPhyHeader = packet->peekAtFront<physicallayer::EthernetPhyHeader>();
    const auto& ethMacHeader = packet->peekDataAt<EthernetMacHeader>(ethPhyHeader->getChunkLength());

    offset = ethPhyHeader->getChunkLength() + ethMacHeader->getChunkLength();

    int etherType = ethMacHeader->getTypeOrLength();
    if (ethMacHeader->getTypeOrLength() == ETHERTYPE_IPv4){
        const auto& ipv4Header = packet->peekDataAt<Ipv4Header>(offset);
        const auto& udpHeader = packet->peekDataAt<UdpHeader>(offset+ipv4Header->getChunkLength());
        offset = offset + udpHeader->getChunkLength() + ipv4Header->getChunkLength();
    }else if (ethMacHeader->getTypeOrLength() != ETHERTYPE_GPTP){
        return nullptr;
    }
    return packet->peekDataAt<GptpBase>(offset).get();
}

void Gptp5g::receiveSignal(cComponent *source, simsignal_t simSignal, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(simSignal));

    if (simSignal != receptionEndedSignal && simSignal != transmissionEndedSignal)
        return;

    auto ethernetSignal = check_and_cast<cPacket *>(obj);
    auto packet = check_and_cast_nullable<Packet *>(ethernetSignal->getEncapsulatedPacket());
    if (!packet)
        return;

    auto gptp = extractGptpHeader(packet);
    if (!gptp)
        return;

    if(auto it = domainNumbers.find(gptp->getDomainNumber()) == domainNumbers.end())
        return;
    if (simSignal == receptionEndedSignal){
        switch (gptp->getMessageType()) {
            case GPTPTYPE_PDELAY_REQ:
                reqMsgIngressTimestamp = clock->getClockTime();
                break;
            case GPTPTYPE_PDELAY_RESP: {
                respMsgIngressTimestamp = clock->getClockTime();
                break;
            }
            case GPTPTYPE_SYNC: {
                syncMsgIngressTimestamp = clock->getClockTime();
                break;
            }
            default:
                break;
        }
    }
    else if (simSignal == transmissionEndedSignal){
        handleDelayOrSendFollowUp(gptp, source);
    }
}

void Gptp5g::handleDelayOrSendFollowUp(const GptpBase *gptp, cComponent *source)
{

    switch (gptp->getMessageType()) {
    case GPTPTYPE_PDELAY_REQ:
        reqMsgEgressTimestamp = clock->getClockTime();
        break;
    case GPTPTYPE_PDELAY_RESP: {
        respMsgEgressTimestamp = clock->getClockTime();
        break;
    }
    case GPTPTYPE_SYNC: {
        syncMsgEgressTimestamp = clock->getClockTime();
        break;
    }
    default:
        break;
    }
}


