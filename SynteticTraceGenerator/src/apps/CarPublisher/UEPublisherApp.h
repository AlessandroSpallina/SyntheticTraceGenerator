#ifndef __UEPUBLISHERAPP_H_
#define __UEPUBLISHERAPP_H_

#include "../CarPublisher/packets/PublisherPacket_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "common/binder/Binder.h"

//inet mobility
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/EulerAngles.h"
#include "inet/mobility/contract/IMobility.h"



using namespace omnetpp;


class UEPublisherApp: public cSimpleModule
{

    //communication to device app and mec app
    inet::UdpSocket socket;

    int size_;
    simtime_t period_;
    int localPort_;
    int deviceAppPort_;
    inet::L3Address deviceAppAddress_;

    char* sourceSimbolicAddress;            //Ue[x]
    char* deviceSimbolicAppAddress_;              //meHost.virtualisationInfrastructure

    // MEC application endPoint (returned by the device app)
    inet::L3Address mecAppAddress_;
    int mecAppPort_;

    std::string mecAppName;

    // mobility informations
    cModule* ue;
    inet::IMobility *mobility;
    inet::Coord position;

    //scheduling
    cMessage *selfStart_;
    cMessage *selfStop_;

    cMessage *selfMecAppStart_;

    // uses to write in a log a file
    bool log;

    // fake sensors - to put in ue!
    int sensorCount = 3;
    const char *sensorNames[3] = {"engine_temperature", "brake_temperature", "tire_temperature"};
    const char *sensorUnits[3] = {"celsius degree", "celsius degree", "celsius degree"};

    public:
        ~UEPublisherApp();
        UEPublisherApp();

    protected:

        virtual int numInitStages() const { return inet::NUM_INIT_STAGES; }
        void initialize(int stage);
        virtual void handleMessage(cMessage *msg);
        virtual void finish();

        void sendStartMECPublisherApp();
        void sendMessageToMECApp();
        // void sendStopMECPublisherApp();

        void handleAckStartMECPublisherApp(cMessage* msg);
        // void handleInfoMEWarningAlertApp(cMessage* msg);
        // void handleAckStopMECPublisherApp(cMessage* msg);
};


#endif
