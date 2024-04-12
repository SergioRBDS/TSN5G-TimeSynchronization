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

#ifndef SRC_GPTP_GPTPTSN_H_
#define SRC_GPTP_GPTPTSN_H_

#include "inet/clock/contract/ClockTime.h"
#include "inet/clock/common/ClockTime.h"
#include "inet/clock/model/SettableClock.h"
#include "inet/common/INETDefs.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/common/clock/ClockUserModuleBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ethernet/common/Ethernet.h"
#include "inet/linklayer/ethernet/common/EthernetMacHeader_m.h"
#include "inet/physicallayer/wired/ethernet/EthernetPhyHeader_m.h"

using namespace inet;

class GptpTSN : public ClockUserModuleBase, public cListener{
protected:
    //referencia para modulos n conectados
    ModuleRefByPar<IInterfaceTable> interfaceTable;

    // tipo de nó, numero do dominio
    GptpNodeType gptpNodeType;
    int domainNumber;

    //portas
    int slavePortId = -1; // interface ID of slave port
    std::set<int> masterPortIds; // interface IDs of master ports

    // parametros setados no ned acerca de tempos
    clocktime_t syncInterval;
    clocktime_t pdelayInterval;
    clocktime_t pDelayReqProcessingTime;  // processing time between arrived PDelayReq and send of PDelayResp

    // parametros calculados/recebidos para a sincronização
    clocktime_t peerDelay; // delay do link
    clocktime_t correctionField;
    clocktime_t pdelayReqEventEgressTimestamp; // t1
    clocktime_t peerRequestReceiptTimestamp; // t2
    clocktime_t peerResponseOriginTimestamp; //t3
    clocktime_t pdelayRespEventIngressTimestamp; // t4
    clocktime_t peerSentTimeSync; // tempo q mandou sync
    clocktime_t originTimestamp; //origem da proxima sincronização de tempo (GM)
    clocktime_t syncIngressTimestamp; // chegada da sync message
    clocktime_t receivedTimeSync; // chegada da ultima sync message
    clocktime_t newLocalTimeAtTimeSync; // resultado da ultima sincronização
    clocktime_t oldPeerSentTimeSync = -1; // tempo da ultima vez q sync foi mandado (=-1 para indicar q é a primeira sincronização
    clocktime_t oldLocalTimeAtTimeSync; //tempo antigo antes de sincronizar
    double gmRateRatio; // frequency ratio (deveria ser em relação ao GM, é?)
    double receivedRateRatio; // frequency ratio received


    // control variables
    uint16_t sequenceId = 0;
    uint16_t lastSentPdelayReqSequenceId = 0;
    uint16_t lastReceivedGptpSyncSequenceId = 0xffff;
    bool rcvdPdelayResp = false;
    bool rcvdGptpSync = false;

    uint64_t clockIdentity;


    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleSelfMessage(cMessage *msg);

    //send messages functions
    void sendPdelayReq();
    void sendPdelayResp(GptpReqAnswerEvent *req);
    void sendPdelayRespFollowUp(int portId, const GptpPdelayResp* resp);
    void sendSync();
    void sendFollowUp(int portId, const GptpSync *sync, clocktime_t preciseOriginTimestamp);

    //Sendo to NIC
    void sendPacketToNIC(Packet *packet, int portId);

    //process reveived messages
    void processPdelayReq(Packet *packet, const GptpPdelayReq* gptp);
    void processPdelayResp(Packet *packet, const GptpPdelayResp* gptp);
    void processPdelayRespFollowUp(Packet *packet, const GptpPdelayRespFollowUp* gptp);
    void processSync(Packet *packet, const GptpSync* gptp);
    void processFollowUp(Packet *packet, const GptpFollowUp* gptp);
    void synchronize();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
    const GptpBase *extractGptpHeader(Packet *packet);
    void handleDelayOrSendFollowUp(const GptpBase *gptp, omnetpp::cComponent *source);

    //EVENTOS
    ClockEvent* selfMsgSync = nullptr;
    ClockEvent* selfMsgDelayReq = nullptr;
    ClockEvent* requestMsg = nullptr;

    // Statistics information: // TODO remove, and replace with emit() calls
    static simsignal_t localTimeSignal;
    static simsignal_t timeDifferenceSignal;
    static simsignal_t rateRatioSignal;
    static simsignal_t peerDelaySignal;
    static simsignal_t correctionFieldSignal;

    static const MacAddress GPTP_MULTICAST_ADDRESS;
public:
    virtual ~GptpTSN();
};

#endif /* SRC_GPTP_GPTPTSN_H_ */
