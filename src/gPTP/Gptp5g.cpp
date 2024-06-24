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


using namespace omnetpp;

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
    cancelAndDeleteClockEvent(selfMsgSync);
    cancelAndDeleteClockEvent(requestMsg);
}

void Gptp5g::initialize(int stage){

    ClockUserModuleBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL){
        translatorType = static_cast<TranslatorType>(cEnum::get("TranslatorType")->resolve(par("translatorType")));

        pDelayReqProcessingTime = par("pDelayReqProcessingTime");

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

        const char *fiveGPort = par("fiveGPort");
        if (*fiveGPort) {
            auto nic = CHK(interfaceTable->findInterfaceByName(fiveGPort));
            fiveGPortId = nic->getInterfaceId();
        }

        const char *str = par("slavePort");
        if (*str) {
            auto nic = CHK(interfaceTable->findInterfaceByName(str));
            slavePortId = nic->getInterfaceId();
            if(slavePortId != fiveGPortId){
                TSNPortIds.insert(slavePortId);
            }
            nic->subscribe(transmissionEndedSignal, this);
            nic->subscribe(receptionEndedSignal, this);
        }
        auto v = check_and_cast<cValueArray *>(par("masterPorts").objectValue())->asStringVector();
        for (const auto& p : v) {
            auto nic = CHK(interfaceTable->findInterfaceByName(p.c_str()));
            int portId = nic->getInterfaceId();
            if(portId != fiveGPortId){
                TSNPortIds.insert(portId);
            }
            masterPortIds.insert(portId);
            if(portId != slavePortId){
                nic->subscribe(transmissionEndedSignal, this);
                nic->subscribe(receptionEndedSignal, this);
            }
        }


        if (slavePortId != -1) {
            auto networkInterface = interfaceTable->getInterfaceById(slavePortId);
            if (!networkInterface->matchesMulticastMacAddress(GPTP_MULTICAST_ADDRESS))
                networkInterface->addMulticastMacAddress(GPTP_MULTICAST_ADDRESS);
        }
        for (auto id: masterPortIds) {
            auto networkInterface = interfaceTable->getInterfaceById(id);
            if (!networkInterface->matchesMulticastMacAddress(GPTP_MULTICAST_ADDRESS))
                networkInterface->addMulticastMacAddress(GPTP_MULTICAST_ADDRESS);
        }


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
        if(TSNPortIds.find(slavePortId) != TSNPortIds.end())
        {
            requestMsg = new ClockEvent("requestToSendSync", GPTP_REQUEST_TO_SEND_SYNC);

            // Schedule Pdelay_Req message is sent by slave port
            // without depending on node type which is grandmaster or bridge
            selfMsgDelayReq = new ClockEvent("selfMsgPdelay", GPTP_SELF_MSG_PDELAY_REQ);
            pdelayInterval = par("pdelayInterval");
            scheduleClockEventAfter(par("pdelayInitialOffset"), selfMsgDelayReq);
        }
        WATCH(peerDelay);
        emit(rateRatioSignal, gmRateRatio);
        emit(localTimeSignal, CLOCKTIME_AS_SIMTIME(CLOCKTIME_ZERO));
        emit(timeDifferenceSignal, CLOCKTIME_AS_SIMTIME(CLOCKTIME_ZERO));
    }

}

void Gptp5g::handleSelfMessage(cMessage *msg)
{
    switch(msg->getKind()) {
        case GPTP_SELF_REQ_ANSWER_KIND:
            // masterport:
            sendPdelayResp(check_and_cast<GptpReqAnswerEvent*>(msg));
            delete msg;
            break;

        case GPTP_SELF_MSG_PDELAY_REQ:
        // slaveport:
            sendPdelayReq(); //TODO on slaveports only
            scheduleClockEventAfter(pdelayInterval, selfMsgDelayReq);
            break;
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


void Gptp5g::sendPdelayReq()
{
    auto packet = new Packet("GptpPdelayReq");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpPdelayReq>();
    gptp->setCorrectionField(CLOCKTIME_ZERO);
    //save and send IDs
    PortIdentity portId;
    portId.clockIdentity = clockIdentity;
    portId.portNumber = slavePortId;
    gptp->setSourcePortIdentity(portId);
    lastSentPdelayReqSequenceId = sequenceId++;
    gptp->setSequenceId(lastSentPdelayReqSequenceId);
    packet->insertAtFront(gptp);
    pdelayReqEventEgressTimestamp = clock->getClockTime();
    rcvdPdelayResp = false;
    sendPacketToNIC(packet, slavePortId);
}
void Gptp5g::sendPdelayResp(GptpReqAnswerEvent* req)
{
    int portId = req->getPortId();
    auto packet = new Packet("GptpPdelayResp");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpPdelayResp>();
    gptp->setRequestingPortIdentity(req->getSourcePortIdentity());
    gptp->setSequenceId(req->getSequenceId());
    gptp->setRequestReceiptTimestamp(req->getIngressTimestamp());
    packet->insertAtFront(gptp);
    sendPacketToNIC(packet, portId);
    // The sendPdelayRespFollowUp(portId) called by receiveSignal(), when GptpPdelayResp sent
}

void Gptp5g::sendPdelayRespFollowUp(int portId, const GptpPdelayResp* resp)
{
    auto packet = new Packet("GptpPdelayRespFollowUp");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpPdelayRespFollowUp>();
    auto now = clock->getClockTime();
    gptp->setResponseOriginTimestamp(now);
    gptp->setRequestingPortIdentity(resp->getRequestingPortIdentity());
    gptp->setSequenceId(resp->getSequenceId());
    packet->insertAtFront(gptp);
    sendPacketToNIC(packet, portId);
}

void Gptp5g::sendPacketToNIC(Packet *packet, int portId)
{
    auto networkInterface = interfaceTable->getInterfaceById(portId);
    EV_INFO << "Sending " << packet << " to output interface = " << networkInterface->getInterfaceName() << ".\n";
    packet->addTag<InterfaceReq>()->setInterfaceId(portId);
    packet->addTag<PacketProtocolTag>()->setProtocol(&Protocol::gptp);
    packet->addTag<DispatchProtocolInd>()->setProtocol(&Protocol::gptp);
    auto protocol = networkInterface->getProtocol();
    if (protocol != nullptr)
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    else
        packet->removeTagIfPresent<DispatchProtocolReq>();
    send(packet, "socketOut");
}


void Gptp5g::sendPacketFromNSTT(Packet *packet, int portId, int domainNumber){

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
            for(auto TSNPortId:TSNPortIds){
                packet->addTag<InterfaceReq>()->setInterfaceId(TSNPortId);
                send(packet->dup(), "socketOut");
            }
        }else{
            packet->addTag<InterfaceReq>()->setInterfaceId(fiveGPortId);
            L3AddressResolver().tryResolve(address,destAddr);
            emit(packetSentSignal,packet);
            socket.sendTo(packet->dup(), destAddr, destPort);
        }
    }delete packet;

}

void Gptp5g::sendPacketFromDSTT(Packet *packet, int portId){

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
    }else{
        send(packet, "socketOut");
    }
}


void Gptp5g::forwardPdelayReq(Packet *packet, const GptpPdelayReq* gptp)
{
    auto resp = new GptpReqAnswerEvent("selfMsgPdelayResp", GPTP_SELF_REQ_ANSWER_KIND);
    resp->setPortId(packet->getTag<InterfaceInd>()->getInterfaceId());
    resp->setIngressTimestamp(packet->getTag<GptpIngressTimeInd>()->getArrivalClockTime());
    resp->setSourcePortIdentity(gptp->getSourcePortIdentity());
    resp->setSequenceId(gptp->getSequenceId());

    scheduleClockEventAfter(pDelayReqProcessingTime, resp);
}


void Gptp5g::forwardPdelayResp(Packet *packet, const GptpPdelayResp* gptp)
{
    // verify IDs
    if (gptp->getRequestingPortIdentity().clockIdentity != clockIdentity || gptp->getRequestingPortIdentity().portNumber != slavePortId) {
        EV_WARN << "GptpPdelayResp arrived with invalid PortIdentity, dropped";
        return;
    }
    if (gptp->getSequenceId() != lastSentPdelayReqSequenceId) {
        EV_WARN << "GptpPdelayResp arrived with invalid sequence ID, dropped";
        return;
    }

    rcvdPdelayResp = true;
    pdelayRespEventIngressTimestamp = packet->getTag<GptpIngressTimeInd>()->getArrivalClockTime();
    peerRequestReceiptTimestamp = gptp->getRequestReceiptTimestamp();
    peerResponseOriginTimestamp = CLOCKTIME_ZERO;
}

void Gptp5g::forwardPdelayRespFollowUp(Packet *packet, const GptpPdelayRespFollowUp* gptp)
{
    if (!rcvdPdelayResp) {
        EV_WARN << "GptpPdelayRespFollowUp arrived without GptpPdelayResp, dropped";
        return;
    }
    // verify IDs
    if (gptp->getRequestingPortIdentity().clockIdentity != clockIdentity || gptp->getRequestingPortIdentity().portNumber != slavePortId) {
        EV_WARN << "GptpPdelayRespFollowUp arrived with invalid PortIdentity, dropped";
        return;
    }
    if (gptp->getSequenceId() != lastSentPdelayReqSequenceId) {
        EV_WARN << "GptpPdelayRespFollowUp arrived with invalid sequence ID, dropped";
        return;
    }

    peerResponseOriginTimestamp = gptp->getResponseOriginTimestamp();

    // computePropTime():
    if(oldPeerResponseOriginTimestamp==-1){
        gmRateRatio = 1;
    }
    else{
        gmRateRatio = (oldPeerResponseOriginTimestamp-peerResponseOriginTimestamp)/(oldPdelayRespEventIngressTimestamp-pdelayRespEventIngressTimestamp);
    }


    peerDelay = (gmRateRatio * (pdelayRespEventIngressTimestamp - pdelayReqEventEgressTimestamp) - (peerResponseOriginTimestamp - peerRequestReceiptTimestamp)) / 2.0;

    EV_INFO << "RATE RATIO                       - " << gmRateRatio << endl;
    EV_INFO << "pdelayReqEventEgressTimestamp    - " << pdelayReqEventEgressTimestamp << endl;
    EV_INFO << "peerResponseOriginTimestamp      - " << peerResponseOriginTimestamp << endl;
    EV_INFO << "pdelayRespEventIngressTimestamp  - " << pdelayRespEventIngressTimestamp << endl;
    EV_INFO << "peerRequestReceiptTimestamp      - " << peerRequestReceiptTimestamp << endl;
    EV_INFO << "PEER DELAY                       - " << peerDelay << endl;

    oldPdelayRespEventIngressTimestamp = pdelayRespEventIngressTimestamp;
    oldPeerResponseOriginTimestamp = peerResponseOriginTimestamp;

    emit(peerDelaySignal, CLOCKTIME_AS_SIMTIME(peerDelay));

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
        for(auto portId: masterPortIds){
            sendPacketFromDSTT(copyPacket->dup(), portId);
        }
    }
    else if (translatorType == NS_TT){

        for(auto portId: masterPortIds){
            sendPacketFromNSTT(copyPacket->dup(), portId, incomingDomainNumber);
        }
    }delete copyPacket;
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
    delete packet;
    if (incomingNicId != fiveGPortId){
        copyGptp->getFollowUpInformationTLVForUpdate().setRateRatio(gmRateRatio);
        rcvdGptpSyncForTSN = false;
        copyGptp->setPreciseOriginTimestamp(gptp->getPreciseOriginTimestamp()+gptp->getCorrectionField()+peerDelay);
        copyGptp->setCorrectionField(syncMsgIngressTimestamp);
    }
    else{
        copyGptp->getFollowUpInformationTLVForUpdate().setRateRatio(gptp->getFollowUpInformationTLV().getRateRatio());
        copyGptp->setPreciseOriginTimestamp(gptp->getPreciseOriginTimestamp());
        if (translatorType == DS_TT){
            copyGptp->setCorrectionField((syncMsgEgressTimestamp-gptp->getCorrectionField())*gptp->getFollowUpInformationTLV().getRateRatio());
        }
        else if (translatorType == NS_TT){
            if(domainNumbers.find(incomingDomainNumber) != domainNumbers.end()){
                copyGptp->setCorrectionField((syncMsgEgressTimestamp-gptp->getCorrectionField())*gptp->getFollowUpInformationTLV().getRateRatio());
            }
            else{
                copyGptp->setCorrectionField(gptp->getCorrectionField());
            }
        }
    }
    copyPacket->insertAtFront(copyGptp);
    if (translatorType == DS_TT){
        for(auto portId: masterPortIds){
            sendPacketFromDSTT(copyPacket->dup(), portId);
        }
    }
    else if (translatorType == NS_TT){
        for(auto portId: masterPortIds){
            sendPacketFromNSTT(copyPacket, portId, incomingDomainNumber);
        }

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
        packet->addTagIfAbsent<GptpIngressTimeInd>()->setArrivalClockTime(clock->getClockTime());
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
    int portId = getContainingNicModule(check_and_cast<cModule*>(source))->getInterfaceId();
    switch (gptp->getMessageType()) {
    case GPTPTYPE_PDELAY_REQ:
        reqMsgEgressTimestamp = clock->getClockTime();
        pdelayReqEventEgressTimestamp = clock->getClockTime();
        break;
    case GPTPTYPE_PDELAY_RESP: {
        auto gptpResp = check_and_cast<const GptpPdelayResp*>(gptp);
        sendPdelayRespFollowUp(portId, gptpResp);
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


