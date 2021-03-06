#include "../CarPublisher/UEPublisherApp.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h"
#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include <fstream>
#include <iostream>

#include "../CarPublisher/packets/PublisherPacket_m.h"
#include "../CarPublisher/packets/PublisherPacket_Types.h"

#include <random>

using namespace inet;
using namespace std;

Define_Module(UEPublisherApp);

UEPublisherApp::UEPublisherApp(){
    selfStart_ = NULL;
}

UEPublisherApp::~UEPublisherApp(){
    cancelAndDelete(selfStart_);
    cancelAndDelete(selfMecAppStart_);
}

void UEPublisherApp::initialize(int stage)
{
    EV << "UEPublisherApp::initialize - stage " << stage << endl;
    cSimpleModule::initialize(stage);
    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    log = par("logger").boolValue();

    //retrieve parameters
    size_ = par("packetSize");
    period_ = par("period");
    localPort_ = par("localPort");
    deviceAppPort_ = par("deviceAppPort");
    sourceSimbolicAddress = (char*)getParentModule()->getFullName();
    deviceSimbolicAppAddress_ = (char*)par("deviceAppAddress").stringValue();
    deviceAppAddress_ = inet::L3AddressResolver().resolve(deviceSimbolicAppAddress_);

    //binding socket
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort_);

    int tos = par("tos");
    if (tos != -1)
        socket.setTos(tos);

    //retrieving car cModule
    ue = this->getParentModule();

    //retrieving mobility module
    cModule *temp = getParentModule()->getSubmodule("mobility");
    if(temp != NULL){
        mobility = check_and_cast<inet::IMobility*>(temp);
    }
    else {
        EV << "UEPublisherApp::initialize - \tWARNING: Mobility module NOT FOUND!" << endl;
        throw cRuntimeError("UEPublisherApp::initialize - \tWARNING: Mobility module NOT FOUND!");
    }

    mecAppName = par("mecAppName").stringValue();

    //initializing the auto-scheduling messages
    selfStart_ = new cMessage("selfStart");
    selfMecAppStart_ = new cMessage("selfMecAppStart");

    //starting UEWarningAlertApp
    simtime_t startTime = par("startTime");
    EV << "UEPublisherApp::initialize - starting sendStartMEWarningAlertApp() in " << startTime << " seconds " << endl;
    scheduleAt(simTime() + startTime, selfStart_);

    //testing
    EV << "UEPublisherApp::initialize - sourceAddress: " << sourceSimbolicAddress << " [" << inet::L3AddressResolver().resolve(sourceSimbolicAddress).str()  <<"]"<< endl;
    EV << "UEPublisherApp::initialize - destAddress: " << deviceSimbolicAppAddress_ << " [" << deviceAppAddress_.str()  <<"]"<< endl;
    EV << "UEPublisherApp::initialize - binding to port: local:" << localPort_ << " , dest:" << deviceAppPort_ << endl;
}

void UEPublisherApp::handleMessage(cMessage *msg)
{
    EV << "UEPublisherApp::handleMessage" << endl;
    // Sender Side
    if (msg->isSelfMessage()) {
        if(!strcmp(msg->getName(), "selfStart")) {
            sendStartMECPublisherApp();
        }

        else if(!strcmp(msg->getName(), "selfSensor")) {
            sendSensorValueToMECApp(msg->getKind());
        } else {
            throw cRuntimeError("UEPublisherApp::handleMessage - \tWARNING: Unrecognized self message");
        }
    }
    // Receiver Side
    else {
        inet::Packet* packet = check_and_cast<inet::Packet*>(msg);

        inet::L3Address ipAdd = packet->getTag<L3AddressInd>()->getSrcAddress();
        int port = packet->getTag<L4PortInd>()->getSrcPort();

        /*
         * From Device app
         * device app usually runs in the UE (loopback), but it could also run in other places
         */
        if(ipAdd == deviceAppAddress_ || ipAdd == inet::L3Address("127.0.0.1")) // dev app
        {
            auto mePkt = packet->peekAtFront<DeviceAppPacket>();

            if (mePkt == 0) {
                throw cRuntimeError("UEPublisherApp::handleMessage - \tFATAL! Error when casting to PublisherPacket");
            }

            if( !strcmp(mePkt->getType(), ACK_START_MECAPP) ) {
                handleAckStartMECPublisherApp(msg);
            } else {
                throw cRuntimeError("UEPublisherApp::handleMessage - \tFATAL! Error, PublisherPacket type %s not recognized", mePkt->getType());
            }
        }
        // From MEC application
        else
        {
            auto mePkt = packet->peekAtFront<PublisherPacket>();
            if (mePkt == 0)
                throw cRuntimeError("UEPublisherApp::handleMessage - \tFATAL! Error when casting to PublisherPacket");

            if(!strcmp(mePkt->getType(), START_NACK)) {
                EV << "UEPublisherApp::handleMessage - MEC app did not started correctly, trying to start again" << endl;

            } else if(!strcmp(mePkt->getType(), START_ACK)) {
                EV << "UEPublisherApp::handleMessage - MEC app started correctly" << endl;
                if(selfMecAppStart_->isScheduled()) {
                    cancelEvent(selfMecAppStart_);
                }
            } else {
                throw cRuntimeError("UEPublisherApp::handleMessage - \tFATAL! Error, WarningAppPacket type %s not recognized", mePkt->getType());
            }
        }
        delete msg;
    }
}

void UEPublisherApp::finish()
{

}
/*
 * -----------------------------------------------Sender Side------------------------------------------
 */
void UEPublisherApp::sendStartMECPublisherApp()
{
    inet::Packet* packet = new inet::Packet("PublisherPacketStart");
    auto start = inet::makeShared<DeviceAppStartPacket>();

    //instantiation requirements and info
    start->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    start->setType(START_MECAPP);
    start->setMecAppName(mecAppName.c_str());
    //start->setMecAppProvider("lte.apps.mec.warningAlert_rest.MEWarningAlertApp_rest_External");

    start->setChunkLength(inet::B(2+mecAppName.size()+1));

    packet->insertAtBack(start);

    socket.sendTo(packet, deviceAppAddress_, deviceAppPort_);

    if(log)
    {
        ofstream myfile;
        myfile.open ("uepublisherapp.txt", ios::app);
        if(myfile.is_open())
        {
            myfile <<"["<< NOW << "] UEPublisherApp - UE sent start message to the Device App \n";
            myfile.close();

        }
    }

    //rescheduling
    scheduleAt(simTime() + period_, selfStart_);
}

/*
 * ---------------------------------------------Receiver Side------------------------------------------
 */
void UEPublisherApp::handleAckStartMECPublisherApp(cMessage* msg)
{
    inet::Packet* packet = check_and_cast<inet::Packet*>(msg);
    auto pkt = packet->peekAtFront<DeviceAppStartAckPacket>();

    if(pkt->getResult() == true) {
        mecAppAddress_ = L3AddressResolver().resolve(pkt->getIpAddress());
        mecAppPort_ = pkt->getPort();
        EV << "UEPublisherApp::handleAckStartMECPublisherApp - Received " << pkt->getType() << " type PublisherPacket. mecApp isntance is at: "<< mecAppAddress_<< ":" << mecAppPort_ << endl;
        cancelEvent(selfStart_);
    } else {
        EV << "UEPublisherApp::handleAckStartMECPublisherApp - MEC application cannot be instantiated! Reason: " << pkt->getReason() << endl;
    }

    sendMessageToMECApp();
    //scheduleAt(simTime() + period_, selfMecAppStart_);

}


double getRandomValue(double lower_bound, double upper_bound) {
    static default_random_engine re;

    uniform_real_distribution<double> unif(lower_bound,upper_bound);
    double a_random_double = unif(re);

    return a_random_double;
}

void UEPublisherApp::sendSensorValueToMECApp(int sensorIndex) {
    double start_range = ranges[sensorIndex][0];
    double end_range = ranges[sensorIndex][1];
    double value_to_send = getRandomValue(start_range, end_range);

    inet::Packet* pkt = new inet::Packet("PublisherPacketStart");
    auto alert = inet::makeShared<PublisherPacket>();
    alert->setType(START_PUBLISHING);

    alert->setSensorName(sensorNames[sensorIndex]);
    alert->setUnit(sensorUnits[sensorIndex]);
    alert->setValue(value_to_send);

    alert->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    alert->setChunkLength(inet::B(20));
    pkt->insertAtBack(alert);

    if(log)
    {
        ofstream myfile;
        myfile.open ("uepublisherapp.txt", ios::app);
        if(myfile.is_open())
        {
            myfile <<"["<< NOW << "] (" << sourceSimbolicAddress << ") UEPublisherApp - UE sent sensor data " << sensorNames[sensorIndex] << " " << value_to_send << endl;
            myfile.close();
        }
    }

    socket.sendTo(pkt, mecAppAddress_ , mecAppPort_);
    EV << "UEPublisherApp::sendSensorValueToMECApp() - start Message sent to the MEC app" << endl;
    scheduleAt(simTime() + frequencies[sensorIndex], new cMessage("selfSensor", sensorIndex));
}

void UEPublisherApp::sendMessageToMECApp() {

    EV << "UEPublisherApp::sendMessageToMECApp() - scheduled vector of sending sensors!" << endl;

    for (int i=0; i<sensorCount; i++) {
        scheduleAt(simTime() + getRandomValue(0, 1), new cMessage("selfSensor", i));
    }

}
