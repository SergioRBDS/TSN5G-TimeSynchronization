/*
 * Gptp5g.h
 *
 *  Created on: 6 de fev. de 2024
 *      Author: vboxuser
 */

#ifndef SRC_GPTP_GPTP5G_H_
#define SRC_GPTP_GPTP5G_H_

#include "inet/clock/contract/ClockTime.h"
#include "inet/clock/common/ClockTime.h"
#include "inet/clock/model/SettableClock.h"
#include "inet/common/INETDefs.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/clock/ClockUserModuleBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"
#include "src/TranslatorType_m.h"
#include "src/common/binder/BinderTSN5G.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"


#include<cstring>


using namespace inet;

class BinderTSN5G;

class Gptp5g : public ClockUserModuleBase, public cListener{
    //parameters:
    ModuleRefByPar<IInterfaceTable> interfaceTable;

    GptpNodeType gptpNodeType;
    TranslatorType translatorType;
    set<int> domainNumbers;
    set<int> slaveDomains;
    set<int> masterDomains;
    clocktime_t correctionField;
    uint64_t clockIdentity = 0;

    double gmRateRatio = 1.0;
    double receivedRateRatio = 1.0;

    clocktime_t originTimestamp; // last outgoing timestamp

    clocktime_t receivedTimeSync;

    clocktime_t syncInterval;
    clocktime_t pdelayInterval;

    uint16_t sequenceId = 0;
    /* Slave port - Variables is used for Peer Delay Measurement */
    uint16_t lastSentPdelayReqSequenceId = 0;
    clocktime_t peerDelay;
    clocktime_t peerRequestReceiptTimestamp;  // pdelayReqIngressTimestamp from peer (received in GptpPdelayResp)
    clocktime_t peerResponseOriginTimestamp; // pdelayRespEgressTimestamp from peer (received in GptpPdelayRespFollowUp)
    clocktime_t pdelayRespEventIngressTimestamp;  // receiving time of last GptpPdelayResp
    clocktime_t pdelayReqEventEgressTimestamp;   // sending time of last GptpPdelayReq
    clocktime_t pDelayReqProcessingTime;  // processing time between arrived PDelayReq and send of PDelayResp

    clocktime_t pdelayReqEventIngressTimestamp; // time req arrive TT


    clocktime_t sentTimeSyncSync;

    /* Slave port - Variables is used for Rate Ratio. All times are drifted based on constant drift */
    // clocktime_t sentTimeSync;
    clocktime_t newLocalTimeAtTimeSync;
    clocktime_t oldLocalTimeAtTimeSync;
    clocktime_t peerSentTimeSync;  // sending time of last received GptpSync
    clocktime_t oldPeerSentTimeSync = -1;  // sending time of previous received GptpSync
    clocktime_t syncIngressTimestamp;  // receiving time of last incoming GptpSync


    uint16_t lastReceivedGptpSyncSequenceId = 0xffff;


    // Statistics information: // TODO remove, and replace with emit() calls
    static simsignal_t localTimeSignal;
    static simsignal_t timeDifferenceSignal;
    static simsignal_t rateRatioSignal;
    static simsignal_t peerDelaySignal;

    public:
        static const MacAddress GPTP_MULTICAST_ADDRESS;

    public:
        virtual ~Gptp5g();

    protected:
        virtual int numInitStages() const override { return NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *msg) override;

        virtual void handleSelfMessage(cMessage *msg);
        void handleDelayOrSendFollowUp(const GptpBase *gptp, omnetpp::cComponent *source);
        const GptpBase *extractGptpHeader(Packet *packet);

        void sendPacketFromDSTT(Packet *packet, int incomingNicId);
        void sendPacketFromNSTT(Packet *packet, int incomingNicId, int domainNumber);
        void sendPacketReqFromNSTT(Packet *packet, int incomingNicId, int domainNumber);

        void forwardSync(Packet *packet, const GptpSync* gptp);
        void forwardFollowUp(Packet *packet, const GptpFollowUp* gptp);
        void forwardPdelayReq(Packet *packet, const GptpPdelayReq* gptp);
        void forwardPdelayResp(Packet *packet, const GptpPdelayResp* gptp);
        void forwardPdelayRespFollowUp(Packet *packet, const GptpPdelayRespFollowUp* gptp);

        virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;

        clocktime_t reqMsgIngressTimestamp;
        clocktime_t reqMsgEgressTimestamp;

        clocktime_t respMsgIngressTimestamp;
        clocktime_t respMsgEgressTimestamp;

        clocktime_t syncMsgIngressTimestamp;
        clocktime_t syncMsgEgressTimestamp;

        bool rcvdPdelayReqForTSN = false;
        bool rcvdPdelayRespForTSN = false;
        bool rcvdGptpSyncForTSN = false;

        int destPort;
        int localPort;

        int fiveGPortId;
        int TSNPortId;

        ClockEvent* selfMsgBindSocket = nullptr;

        UdpSocket socket;

        BinderTSN5G *binder_;
};
#endif /* SRC_GPTP_GPTP5G_H_ */
