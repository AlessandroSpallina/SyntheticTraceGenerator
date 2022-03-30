//
//                  Simu5G
//
// Authors: Giovanni Nardini, Giovanni Stea, Antonio Virdis (University of Pisa)
//
// This file is part of a software released under the license included in file
// "license.pdf". Please read LICENSE and README files before using it.
// The above files and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_m.h"
#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"

#include "./packets/WarningAlertPacket_m.h"
#include "./packets/WarningAlertPacket_Types.h"
#include "./packets/PingPongPacket_m.h"
#include "./packets/PingPongPacket_Types.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/chunk/BytesChunk.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include <fstream>
#include "UEPingPongApp.h"

#include "spdlog/spdlog.h"  // logging library
#include "spdlog/sinks/basic_file_sink.h"
#include <ctime>
#include <fmt/format.h>

#include <iostream>
#include <iomanip>
#include <random>
#include <string>
#include <map>

using namespace inet;
using namespace std;

Define_Module(UEPingPongApp);

float get_next_time_packet(int lambda=1) {

    std::random_device rd;
    std::mt19937 gen(rd());

    std::exponential_distribution<> d(lambda);

    return d(gen);
}

int get_packet_dimension() {
    static std::default_random_engine generator;
    static std::normal_distribution<double> distribution(1024.0,100.0);

    return int(distribution(generator));
}


UEPingPongApp::UEPingPongApp(){
    selfStart_ = NULL;
    selfStop_ = NULL;
}

UEPingPongApp::~UEPingPongApp(){
    cancelAndDelete(selfStart_);
    cancelAndDelete(selfStop_);
    cancelAndDelete(selfMecAppStart_);

}

void UEPingPongApp::initialize(int stage)
{
    EV << "UEPingPongApp::initialize - stage " << stage << endl;
    cSimpleModule::initialize(stage);
    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    static int counter = 0;
    log = par("logger").boolValue();
    myLogger = spdlog::basic_logger_mt(fmt::format("ueLogger{}", counter++), fmt::format("logs/uepingpongapp_{}.txt", std::time(nullptr)));

    numCars = par("numCars");
    myLogger->info(fmt::format("@ Lambda={}", numCars));


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
        myLogger->info("UEPingPongApp::initialize - \tWARNING: Mobility module NOT FOUND!");
        EV << "UEPingPongApp::initialize - \tWARNING: Mobility module NOT FOUND!" << endl;
        throw cRuntimeError("UEPingPongApp::initialize - \tWARNING: Mobility module NOT FOUND!");
    }

    mecAppName = par("mecAppName").stringValue();

    //initializing the auto-scheduling messages
    selfStart_ = new cMessage("selfStart");
    selfStop_ = new cMessage("selfStop");
    selfMecAppStart_ = new cMessage("selfMecAppStart");
    selfSendPing_ = new cMessage("selfSendPing");

    //starting UEWarningAlertApp
    simtime_t startTime = par("startTime");
    EV << "UEPingPongApp::initialize - starting sendStartMEWarningAlertApp() in " << startTime << " seconds " << endl;
    scheduleAt(simTime() + startTime, selfStart_);

    //testing
    EV << "UEPingPongApp::initialize - sourceAddress: " << sourceSimbolicAddress << " [" << inet::L3AddressResolver().resolve(sourceSimbolicAddress).str()  <<"]"<< endl;
    EV << "UEPingPongApp::initialize - destAddress: " << deviceSimbolicAppAddress_ << " [" << deviceAppAddress_.str()  <<"]"<< endl;
    EV << "UEPingPongApp::initialize - binding to port: local:" << localPort_ << " , dest:" << deviceAppPort_ << endl;
}

void UEPingPongApp::handleMessage(cMessage *msg)
{
    myLogger->info(fmt::format("handleMessage: received {}; isSelf? {}", msg->getName(), msg->isSelfMessage()));

    EV << "UEPingPongApp::handleMessage" << endl;
    // Sender Side
    if (msg->isSelfMessage())
    {
        if(!strcmp(msg->getName(), "selfStart"))   sendStartMEWarningAlertApp();

        else if(!strcmp(msg->getName(), "selfStop"))    sendStopMEWarningAlertApp();

        else if(!strcmp(msg->getName(), "selfMecAppStart"))
        {
            sendMessageToMECApp();
            scheduleAt(simTime() + period_, selfMecAppStart_);
        }
        else if(!strcmp(msg->getName(), "selfSendPing")) {
            sendPingPacket();
            scheduleAt(simTime() + get_next_time_packet(numCars), selfSendPing_);
        }

        else    throw cRuntimeError("UEPingPongApp::handleMessage - \tWARNING: Unrecognized self message");
    }
    // Receiver Side
    else{
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

            if (mePkt == 0)
                throw cRuntimeError("UEPingPongApp::handleMessage - \tFATAL! Error when casting to DeviceAppPacket");

            if( !strcmp(mePkt->getType(), ACK_START_MECAPP) )    handleAckStartMEWarningAlertApp(msg);

            else if(!strcmp(mePkt->getType(), ACK_STOP_MECAPP))  handleAckStopMEWarningAlertApp(msg);

            else
            {
                throw cRuntimeError("UEPingPongApp::handleMessage - \tFATAL! Error, DeviceAppPacket type %s not recognized", mePkt->getType());
            }
        }
        // From MEC application
        else
        {
            auto mePkt = packet->peekAtFront<WarningAppPacket>();
            if (mePkt == 0)
                throw cRuntimeError("UEPingPongApp::handleMessage - \tFATAL! Error when casting to WarningAppPacket");

            if(!strcmp(mePkt->getType(), WARNING_ALERT))      handleInfoMEWarningAlertApp(msg);
            else if(!strcmp(mePkt->getType(), START_NACK))
            {
                EV << "UEPingPongApp::handleMessage - MEC app did not started correctly, trying to start again" << endl;
            }
            else if(!strcmp(mePkt->getType(), START_ACK))
            {
                EV << "UEPingPongApp::handleMessage - MEC app started correctly" << endl;
                if(selfMecAppStart_->isScheduled())
                {
                    cancelEvent(selfMecAppStart_);
                }
            }
            else
            {
                throw cRuntimeError("UEWarningAlertApp::handleMessage - \tFATAL! Error, WarningAppPacket type %s not recognized", mePkt->getType());
            }
        }
        delete msg;
    }
}

void UEPingPongApp::finish()
{

}
/*
 * -----------------------------------------------Sender Side------------------------------------------
 */
void UEPingPongApp::sendStartMEWarningAlertApp()
{
    inet::Packet* packet = new inet::Packet("WarningAlertPacketStart");
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
        myfile.open ("example.txt", ios::app);
        if(myfile.is_open())
        {
            myfile <<"["<< NOW << "] UEPingPongApp - UE sent start message to the Device App \n";
            myfile.close();

        }
    }

    //rescheduling
    scheduleAt(simTime() + period_, selfStart_);
}
void UEPingPongApp::sendStopMEWarningAlertApp()
{
    EV << "UEPingPongApp::sendStopMEWarningAlertApp - Sending " << STOP_MEAPP <<" type WarningAlertPacket\n";

    inet::Packet* packet = new inet::Packet("DeviceAppStopPacket");
    auto stop = inet::makeShared<DeviceAppStopPacket>();

    //termination requirements and info
    stop->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    stop->setType(STOP_MECAPP);

    stop->setChunkLength(inet::B(size_));

    packet->insertAtBack(stop);
    socket.sendTo(packet, deviceAppAddress_, deviceAppPort_);

    if(log)
    {
        ofstream myfile;
        myfile.open ("example.txt", ios::app);
        if(myfile.is_open())
        {
            myfile <<"["<< NOW << "] UEWarningAlertApp - UE sent stop message to the Device App \n";
            myfile.close();
        }
    }

    //rescheduling
    if(selfStop_->isScheduled())
        cancelEvent(selfStop_);
    scheduleAt(simTime() + period_, selfStop_);
}

/*
 * ---------------------------------------------Receiver Side------------------------------------------
 */
void UEPingPongApp::handleAckStartMEWarningAlertApp(cMessage* msg)
{
    inet::Packet* packet = check_and_cast<inet::Packet*>(msg);
    auto pkt = packet->peekAtFront<DeviceAppStartAckPacket>();

    if(pkt->getResult() == true)
    {
        mecAppAddress_ = L3AddressResolver().resolve(pkt->getIpAddress());
        mecAppPort_ = pkt->getPort();
        EV << "UEPingPongApp::handleAckStartMEWarningAlertApp - Received " << pkt->getType() << " type WarningAlertPacket. mecApp isntance is at: "<< mecAppAddress_<< ":" << mecAppPort_ << endl;
        cancelEvent(selfStart_);
        //scheduling sendStopMEWarningAlertApp()
        if(!selfStop_->isScheduled()){
            simtime_t  stopTime = par("stopTime");
            scheduleAt(simTime() + stopTime, selfStop_);
            EV << "UEPingPongApp::handleAckStartMEWarningAlertApp - Starting sendStopMEWarningAlertApp() in " << stopTime << " seconds " << endl;
        }
    }
    else
    {
        EV << "UEPingPongApp::handleAckStartMEWarningAlertApp - MEC application cannot be instantiated! Reason: " << pkt->getReason() << endl;
    }

    sendMessageToMECApp();
    scheduleAt(simTime() + period_, selfMecAppStart_);

}

void UEPingPongApp::sendMessageToMECApp(){

    // send star monitoring message to the MEC application

    inet::Packet* pkt = new inet::Packet("WarningAlertPacketStart");
    auto alert = inet::makeShared<WarningStartPacket>();
    alert->setType(START_WARNING);
    alert->setCenterPositionX(par("positionX").doubleValue());
    alert->setCenterPositionY(par("positionY").doubleValue());
    alert->setRadius(par("radius").doubleValue());
    alert->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    alert->setChunkLength(inet::B(20));
    pkt->insertAtBack(alert);

    if(log)
    {
        ofstream myfile;
        myfile.open ("example.txt", ios::app);
        if(myfile.is_open())
        {
            myfile <<"["<< NOW << "] UEPingPongApp - UE sent start subscription message to the MEC application \n";
            myfile.close();
        }
    }

    socket.sendTo(pkt, mecAppAddress_ , mecAppPort_);
    EV << "UEPingPongApp::sendMessageToMECApp() - start Message sent to the MEC app" << endl;



    // sending Ping to MEC application
    inet::Packet* pkt2 = new inet::Packet("PingPongPacket");
    auto alert2 = inet::makeShared<PingPongPacket>();
    alert2->setType(START_PINGPONG);
    alert2->setData("ping");
    alert2->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    alert2->setChunkLength(inet::B(29));
    pkt2->insertAtBack(alert2);
    socket.sendTo(pkt2, mecAppAddress_, mecAppPort_);
    myLogger->info("@ Sent ping to the MEC");

    scheduleAt(simTime() + get_next_time_packet(numCars), selfSendPing_);

}

void UEPingPongApp::sendPingPacket() {
    // sending Ping to MEC application
    inet::Packet* pkt2 = new inet::Packet("PingPongPacket");
    auto alert2 = inet::makeShared<PingPongPacket>();
    alert2->setType(START_PINGPONG);
    alert2->setData("ping");
    alert2->addTagIfAbsent<inet::CreationTimeTag>()->setCreationTime(simTime());
    alert2->setChunkLength(inet::B(get_packet_dimension()));
    pkt2->insertAtBack(alert2);
    socket.sendTo(pkt2, mecAppAddress_, mecAppPort_);
    myLogger->info("@ Sent ping to the MEC");
}

void UEPingPongApp::handleInfoMEWarningAlertApp(cMessage* msg)
{
    inet::Packet* packet = check_and_cast<inet::Packet*>(msg);
    auto pkt = packet->peekAtFront<WarningAlertPacket>();

    EV << "UEPingPongApp::handleInfoMEWarningrAlertApp - Received " << pkt->getType() << " type WarningAlertPacket"<< endl;

    //updating runtime color of the car icon background
    if(pkt->getDanger())
    {
        if(log)
        {
            ofstream myfile;
            myfile.open ("example.txt", ios::app);
            if(myfile.is_open())
            {
                myfile <<"["<< NOW << "] UEPingPongApp - UE received danger alert TRUE from MEC application \n";
                myfile.close();
            }
        }

        EV << "UEPingPongApp::handleInfoMEWarningrAlertApp - Warning Alert Detected: DANGER!" << endl;
        ue->getDisplayString().setTagArg("i",1, "red");
    }
    else{
        if(log)
        {
            ofstream myfile;
            myfile.open ("example.txt", ios::app);
            if(myfile.is_open())
            {
                myfile <<"["<< NOW << "] UEPingPongApp - UE received danger alert FALSE from MEC application \n";
                myfile.close();
            }
        }

        EV << "UEPingPongApp::handleInfoMEWarningrAlertApp - Warning Alert Detected: NO DANGER!" << endl;
        ue->getDisplayString().setTagArg("i",1, "green");
    }
}
void UEPingPongApp::handleAckStopMEWarningAlertApp(cMessage* msg)
{

    inet::Packet* packet = check_and_cast<inet::Packet*>(msg);
    auto pkt = packet->peekAtFront<DeviceAppStopAckPacket>();

    EV << "UEPingPongApp::handleAckStopMEWarningAlertApp - Received " << pkt->getType() << " type WarningAlertPacket with result: "<< pkt->getResult() << endl;
    if(pkt->getResult() == false)
        EV << "Reason: "<< pkt->getReason() << endl;
    //updating runtime color of the car icon background
    ue->getDisplayString().setTagArg("i",1, "white");

    cancelEvent(selfStop_);
}
