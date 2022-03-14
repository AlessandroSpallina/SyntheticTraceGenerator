#ifndef __MECPUBLISHERAPP_H_
#define __MECPUBLISHERAPP_H_

#include "omnetpp.h"

#include "../CarPublisher/packets/PublisherPacket_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "nodes/mec/MECPlatform/ServiceRegistry/ServiceRegistry.h"

#include "apps/mec/MecApps/MecAppBase.h"


using namespace std;
using namespace omnetpp;


class MECPublisherApp : public MecAppBase
{

    //UDP socket to communicate with the UeApp
    inet::UdpSocket ueSocket;
    int localUePort;

    inet::L3Address ueAppAddress;
    int ueAppPort;


    int size_;
    std::string subId;

    protected:
        virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
        virtual void initialize(int stage) override;
        virtual void handleMessage(cMessage *msg) override;

        virtual void finish() override;

        virtual void handleServiceMessage() override {};
        virtual void handleMp1Message() override {};

        virtual void handleUeMessage(omnetpp::cMessage *msg) override;

        virtual void handleSelfMessage(cMessage *msg) override {};

        virtual void established(int connId) override {};

    public:
       MECPublisherApp();
       virtual ~MECPublisherApp();

};

#endif
