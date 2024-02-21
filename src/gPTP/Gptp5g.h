/*
 * Gptp5g.h
 *
 *  Created on: 6 de fev. de 2024
 *      Author: vboxuser
 */

#ifndef SRC_GPTP_GPTP5G_H_
#define SRC_GPTP_GPTP5G_H_

#include "inet/linklayer/ieee8021as/Gptp.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"

using namespace inet;

class Gptp5g : public Gptp{
    public:
        void sendPacketToNIC(Packet *packet, int portId) override;
    protected:
        bool tt;
        int destPort;
        int localPort;
        int ipPortId;
        int domainNumber2;
        const char* dAddress;
        ClockEvent* selfMsgBindSocket = nullptr;
        UdpSocket socket;
        ModuleRefByPar<IInterfaceTable> interfaceTable2;

        virtual void initialize(int stage) override;
        virtual void handleSelfMessage(cMessage *msg) override;
        virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;

        virtual void processStart();
        virtual const GptpBase *extractGptpHeaderIp(Packet *packet);
};
#endif /* SRC_GPTP_GPTP5G_H_ */
