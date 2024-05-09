/*
 * GtpUserGnb.h
 *
 *  Created on: May 9, 2024
 *      Author: pregui
 */

#ifndef SRC_CORENETWORK_GTP_GTPUSERGNB_H_
#define SRC_CORENETWORK_GTP_GTPUSERGNB_H_

#include <map>
#include <omnetpp.h>
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "corenetwork/gtp/GtpUserMsg_m.h"
#include <inet/common/ModuleAccess.h>
#include <inet/networklayer/common/NetworkInterface.h>
#include <inet/networklayer/common/L3AddressResolver.h>
#include "common/binder/Binder.h"
#include <inet/linklayer/common/InterfaceTag_m.h>


class GtpUserGnb : public omnetpp::cSimpleModule
{
    inet::UdpSocket socket_;
    int localPort_;

    // reference to the LTE Binder module
    Binder* binder_;

    // the GTP protocol Port
    unsigned int tunnelPeerPort_;

    // IP address of the gateway to the Internet
    inet::L3Address gwAddress_;

    // specifies the type of the node that contains this filter (it can be ENB or PGW)
    CoreNodeType ownerType_;

    CoreNodeType selectOwnerType(const char * type);

    // if this module is on BS, this variable includes the ID of the BS
    MacNodeId myMacNodeID;

    inet::NetworkInterface* ie_;

  protected:

    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(omnetpp::cMessage *msg) override;

    // receive and IP Datagram from the traffic filter, encapsulates it in a GTP-U packet than forwards it to the proper next hop
    void handleFromTrafficFlowFilter(inet::Packet * datagram);

    // receive a GTP-U packet from Udp, reads the TEID and decides whether performing label switching or removal
    void handleFromUdp(inet::Packet * gtpMsg);

    // detect outgoing interface name (CellularNic)
    inet::NetworkInterface *detectInterface();
};

#endif
