#include "../CarPublisher/MECPublisherApp.h"

#include "apps/mec/DeviceApp/DeviceAppMessages/DeviceAppPacket_Types.h"

#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet_m.h"

#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/common/L4PortTag_m.h"

#include "nodes/mec/utils/httpUtils/httpUtils.h"
#include "nodes/mec/utils/httpUtils/json.hpp"
#include "nodes/mec/MECPlatform/MECServices/packets/HttpResponseMessage/HttpResponseMessage.h"

#include <fstream>

/*
 * MECPublisherApp is a simply mec app that receives asyncronous sensors values,
 * then this values are logged in a file
 */


Define_Module(MECPublisherApp);

using namespace inet;
using namespace omnetpp;

MECPublisherApp::MECPublisherApp(): MecAppBase()
{
    EV << "MECPublisherApp started" << endl;

}
MECPublisherApp::~MECPublisherApp()
{
    EV << "MECPublisherApp ended" << endl;
}


void MECPublisherApp::initialize(int stage)
{
    MecAppBase::initialize(stage);

    // avoid multiple initializations
    if (stage!=inet::INITSTAGE_APPLICATION_LAYER)
        return;

    //retrieving parameters
    size_ = par("packetSize");

    // set Udp Socket
    ueSocket.setOutputGate(gate("socketOut"));

    localUePort = par("localUePort");
    ueSocket.bind(localUePort);

    //testing
    EV << "MECPublisherApp::initialize - Mec application "<< getClassName() << " with mecAppId["<< mecAppId << "] has started!" << endl;

}

void MECPublisherApp::handleMessage(cMessage *msg)
{
//        MecAppBase::handleMessage(msg);
    if (!msg->isSelfMessage())
    {
        if(ueSocket.belongsToSocket(msg))
        {
            handleUeMessage(msg);
            delete msg;
            return;
        }
    }

    //MecAppBase::handleMessage(msg);
    // if self message print something, normally i should not receive one

}

void MECPublisherApp::finish(){
    MecAppBase::finish();
    EV << "MECPublisherApp::finish()" << endl;

    if(gate("socketOut")->isConnected()){

    }
}

void MECPublisherApp::handleUeMessage(omnetpp::cMessage *msg)
{
    // determine its source address/port
    auto pk = check_and_cast<Packet *>(msg);
    ueAppAddress = pk->getTag<L3AddressInd>()->getSrcAddress();
    ueAppPort = pk->getTag<L4PortInd>()->getSrcPort();

    auto mecPk = pk->peekAtFront<PublisherPacket>();

    auto pubPk = dynamicPtrCast<const PublisherPacket>(mecPk);
    if (pubPk == nullptr)
        throw cRuntimeError("MECPublisherApp::handleUeMessage - PublisherPacket is null");

    std::string sensorName = pubPk->getSensorName();
    std::string sensorUnit = pubPk->getUnit();
    double sensorValue = pubPk->getValue();


    EV << "MECPublisherApp::handleUeMessage - Received message" << endl;

    if(par("logger").boolValue())
    {
        ofstream myfile;
        myfile.open ("mecpublisherapp.txt", ios::app);
        if(myfile.is_open())
        {
            myfile <<"["<< NOW << "] MECPublisherApp - Received message from UE, " << sensorName << " " << sensorUnit << " " << sensorValue << endl;
            myfile.close();
        }
    }
}
