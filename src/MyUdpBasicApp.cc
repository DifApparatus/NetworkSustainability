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

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/mobility/contract/IMobility.h"
#include "MyUdpBasicApp.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/applications/udpapp/UdpBasicBurst.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "CurrentCoordsMessage_m.h"
#include "inet/common/packet/chunk/Chunk.h"
#include <string.h>

#include <omnetpp.h>

namespace inet{

    Define_Module(MyUdpBasicApp);

    void MyUdpBasicApp::initialize(int stage)
    {
        UdpBasicBurst::initialize(stage);
        //int distances[];
       // cModule *host = getContainingNode(this);
        //cPar maxDist = host->getAncestorPar("communicationRange");
        //broadcastSocket.bind(Ipv4Address("255.255.255.255"), 1025);
        maxDistance = 150;
        optimalDistance = maxDistance * 0.5;
        correctingDistance = maxDistance * 0.7;
    }
    Packet *MyUdpBasicApp::createPacket()
    {
        cModule *host = getContainingNode(this);
        IMobility *mobility = dynamic_cast<IMobility *>(host->getSubmodule("mobility"));
        Coord coords = mobility->getCurrentPosition();

        char packetName[] = "Coords";
        std::ostringstream str;
        str << packetName;

        Packet *pk = new Packet(str.str().c_str());

        const auto& payload = makeShared<CurrentCoordsMessage>("CurrentCoordinates");
        payload->setX(coords.x);
        payload->setY(coords.y);
        payload->setZ(0);
        payload->setChunkLength(B(par("messageLength")));
        payload->setSequenceNumber(numSent);

        pk->insertAtBack(payload);
        pk->addPar("sourceId") = getId();
        pk->addPar("msgId") = numSent;

        return pk;
    }
    void MyUdpBasicApp::generateBurst(){
        simtime_t now = simTime();
            if (nextPkt < now)
                nextPkt = now;

            double sendInterval = *sendIntervalPar;
            if (sendInterval <= 0.0)
                throw cRuntimeError("The sendInterval parameter must be bigger than 0");
            nextPkt += sendInterval;

            if (activeBurst && nextBurst <= now) {    // new burst
                double burstDuration = *burstDurationPar;
                if (burstDuration < 0.0)
                    throw cRuntimeError("The burstDuration parameter mustn't be smaller than 0");
                double sleepDuration = *sleepDurationPar;

                if (burstDuration == 0.0)
                    activeBurst = false;
                else {
                    if (sleepDuration < 0.0)
                        throw cRuntimeError("The sleepDuration parameter mustn't be smaller than 0");
                    nextSleep = now + burstDuration;
                    nextBurst = nextSleep + sleepDuration;
                }

                if (chooseDestAddrMode == PER_BURST)
                    destAddr = chooseDestAddr();
            }

            if (chooseDestAddrMode == PER_SEND)
                destAddr = chooseDestAddr();

            Packet *payload = createPacket();
            for (int i = 0; i < destAddresses.size(); i++){
                Packet *copy_payload = payload->dup();
                copy_payload->setTimestamp();
                emit(packetSentSignal, copy_payload);
                socket.sendTo(copy_payload, destAddresses[i], destPort);
                numSent++;
            }

            // Next timer
            if (activeBurst && nextPkt >= nextSleep)
                nextPkt = nextBurst;

            if (stopTime >= SIMTIME_ZERO && nextPkt >= stopTime) {
                timerNext->setKind(STOP);
                nextPkt = stopTime;
            }
            scheduleAt(nextPkt, timerNext);
    }
    void MyUdpBasicApp::handleMessageWhenUp(cMessage *msg)
    {
        if (msg->isSelfMessage()) {
            switch (msg->getKind()) {
                case START:
                    processStart();
                    break;

                case SEND:
                    processSend();
                    break;

                case STOP:
                    processStop();
                    break;

                default:
                    throw cRuntimeError("Invalid kind %d in self message", (int)msg->getKind());
            }
        }
        else{
            socket.processMessage(msg);
        }
    }
    void MyUdpBasicApp::socketDataArrived(UdpSocket *socket, Packet *packet)
    {
        // process incoming packet
        processPacket(packet);
    }
    void MyUdpBasicApp::processStart()
    {
        socket.setOutputGate(gate("socketOut"));
        socket.bind(localPort);
        socket.setCallback(this);

        broadcastSocket.setOutputGate(gate("socketOut"));
        broadcastSocket.bind(1025);
        broadcastSocket.setBroadcast(true);
        broadcastSocket.setCallback(this);

        const char *destAddrs = par("destAddresses");
        cStringTokenizer tokenizer(destAddrs);
        const char *token;
        bool excludeLocalDestAddresses = par("excludeLocalDestAddresses");

        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

        while ((token = tokenizer.nextToken()) != nullptr) {
            if (strstr(token, "Broadcast") != nullptr)
                destAddresses.push_back(Ipv4Address::ALLONES_ADDRESS);
            else {
                L3Address addr = L3AddressResolver().resolve(token);
                if (excludeLocalDestAddresses && ift && ift->isLocalAddress(addr))
                    continue;
                destAddresses.push_back(addr);
            }
        }

        nextSleep = simTime();
        nextBurst = simTime();
        nextPkt = simTime();
        activeBurst = false;

        isSource = !destAddresses.empty();

        if (isSource) {
            if (chooseDestAddrMode == ONCE)
                destAddr = chooseDestAddr();

            activeBurst = true;
        }
        timerNext->setKind(SEND);
        processSend();
    }
    void MyUdpBasicApp::processPacket(Packet *pk)
    {
        if (pk->getKind() == UDP_I_ERROR) {
            EV_WARN << "UDP error received\n";
            delete pk;
            return;
        }

        if (pk->hasPar("sourceId") && pk->hasPar("msgId")) {
            // duplicate control
            int moduleId = pk->par("sourceId");
            int msgId = pk->par("msgId");
            auto it = sourceSequence.find(moduleId);
            if (it != sourceSequence.end()) {
                if (it->second >= msgId) {
                    EV_DEBUG << "Out of order packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
                    emit(outOfOrderPkSignal, pk);
                    delete pk;
                    numDuplicated++;
                    return;
                }
                else
                    it->second = msgId;
            }
            else
                sourceSequence[moduleId] = msgId;
        }

        if (delayLimit > 0) {
            if (simTime() - pk->getTimestamp() > delayLimit) {
                EV_DEBUG << "Old packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
                PacketDropDetails details;
                details.setReason(CONGESTION);
                emit(packetDroppedSignal, pk, &details);
                delete pk;
                numDeleted++;
                return;
            }
        }
        if ( strcmp(pk->getName(),"Coords") == 0 ){
                    auto data = pk->popAtBack<CurrentCoordsMessage>();
                    int neighbour_x = data->getX();
                    int neighbour_y = data->getY();

                    cModule *host = getContainingNode(this);
                    IMobility *mobility = dynamic_cast<IMobility *>(host->getSubmodule("mobility"));
                    Coord coords = mobility->getCurrentPosition();
                    int x = coords.getX();
                    int y = coords.getY();
                    int distance = sqrt((neighbour_x - x)*(neighbour_x - x) + (neighbour_y - y)*(neighbour_y - y));

                    if (distance >= correctingDistance){
                        sendBroadcastCoords();
                    }
        }
        if ( strcmp(pk->getName(),"Coords_BROADCAST") == 0 ){
            auto data = pk->popAtBack<CurrentCoordsMessage>();
            int neighbour_x = data->getX();
            int neighbour_y = data->getY();

            cModule *host = getContainingNode(this);
            IMobility *mobility = dynamic_cast<IMobility *>(host->getSubmodule("mobility"));
            Coord coords = mobility->getCurrentPosition();
            int x = coords.getX();
            int y = coords.getY();
            int distance = sqrt((neighbour_x - x)*(neighbour_x - x) + (neighbour_y - y)*(neighbour_y - y));
            EV_INFO <<"??????????????????????????????????????????????????????????????????????????\n";
        }
        numReceived++;
        delete pk;
    }
    void MyUdpBasicApp::sendBroadcastCoords(){
        char broadcast_string[] = "_BROADCAST";
        Packet *pk = createPacket();
        char *prName = strdup(pk->getName());
        pk->setName(strcat(prName,broadcast_string));
        pk->addPar("sourceId") = getId();
        pk->addPar("msgId") = numSent;
        pk->setTimestamp();

        emit(packetSentSignal, pk);
        socket.sendTo(pk, Ipv4Address::ALLONES_ADDRESS, 1025);
        numSent++;
    }
}//namespace inet
