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
    int sensorCount = 6;
    const char *sensorNames[6] = {"engine_temperature", "brake_temperature", "tire_temperature", "speed", "front_distance", "back_distance"};
    const char *sensorUnits[6] = {"°C", "°C", "°C", "km/h", "m", "m"};
    const double ranges[6][2] = {{0, 90}, {0, 90}, {0, 90}, {0, 250}, {0, 1000}, {0, 1000}};
    const double frequencies[6] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1}; // in milliseconds
    double previousValues[6];

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

        void handleAckStartMECPublisherApp(cMessage* msg);

        //sensor stuff
        void sendSensorValueToMECApp(int sensorIndex);
};


#endif
