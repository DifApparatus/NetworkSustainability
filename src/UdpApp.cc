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
#include <UdpApp.h>
#include <vector>
#include <algorithm>

#include "omnetpp.h"

namespace inet{

    Define_Module(UdpApp);

    void UdpApp::initialize(int stage)
    {
        UdpBasicBurst::initialize(stage);
        problemNode = -1;
        maxDistance = 200;
        optimalDistance = maxDistance * 0.5;
        correctingDistance = maxDistance * 0.7;
    }
    template<class T>
    Packet *UdpApp::createPacket(char packetName[], T payload)
    {
        std::ostringstream str;
        str << packetName;

        Packet *pk = new Packet(str.str().c_str());
        pk->insertAtBack(payload);
        pk->addPar("sourceId") = getId();
        pk->addPar("msgId") = numSent;

        return pk;
    }
    inet::Ptr<CurrentCoordsMessage> UdpApp::createCoordPayload(){
        cModule *hostNode = getContainingNode(this);// Node that contains this app
        IMobility *mobility = dynamic_cast<IMobility *>(hostNode->getSubmodule("mobility"));
        Coord coords = mobility->getCurrentPosition();

        const auto& payload = makeShared<CurrentCoordsMessage>("CurrentCoordinates");
        payload->setX(coords.x);
        payload->setY(coords.y);
        payload->setZ(0);
        payload->setChunkLength(B(par("messageLength")));
        payload->setSequenceNumber(numSent);
        return payload;
    }

    /** Creates packets to send per intarvals/
     * This differs from UdpBasicBurst::generateBurst by commentaries and
     * that it has group cast. It sends usual messages(coordinates)
     */
    void UdpApp::generateBurst(){
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
            Packet *payload = createPacket("Coords", createCoordPayload());
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
    void UdpApp::processStart()
    {
        broadcastSocket.setOutputGate(gate("socketOut"));
        broadcastSocket.bind(1025);
        broadcastSocket.setBroadcast(true);
        broadcastSocket.setCallback(this);

        UdpBasicBurst::processStart();
    }
    void UdpApp::processPacket(Packet *pk)
    {
        if (pk->getKind() == UDP_I_ERROR) {
            EV_WARN << "UDP error received\n";
            delete pk;
            return;
        }
        double nodeConnectQuality = 1;
        double maxDist;
        if (distances.size()>0){
            maxDist = 0;
            for (int i = 0; i < distances.size();i++){
                if (maxDist < distances[i])
                    maxDist = distances[i];
            }
        }
        else maxDist = maxDistance;
        if (maxDist > optimalDistance) nodeConnectQuality = 1-(maxDist-optimalDistance)/(maxDistance-optimalDistance);
        int moduleId = pk->par("sourceId");
        int msgId = pk->par("msgId");

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

                    if (std::find(neighbours_Id.begin(), neighbours_Id.end(), moduleId) == neighbours_Id.end()){
                       neighbours_Id.push_back(moduleId);
                       distances.push_back(distance);
                    }
                    int index = std::distance(neighbours_Id.begin(), std::find(neighbours_Id.begin(), neighbours_Id.end(), moduleId));
                    distances.at(index) = distance;
                    if (distance >= correctingDistance){
                        if (problemNode == -1) {
                            problemNode = moduleId;
                        }
                        problemDistance = distance;
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
            if (std::find(neighbours_Id.begin(), neighbours_Id.end(), moduleId) == neighbours_Id.end()){
                sendBroadcastCoordsReply(moduleId);
            }
        }
        if ( strcmp(pk->getName(),"Coords_BROADCAST_REPLY") == 0 ){
            auto data = pk->popAtBack<CurrentCoordsMessage>();
            int neighbour_x = data->getX();
            int neighbour_y = data->getY();

            cModule *host = getContainingNode(this);
            IMobility *mobility = dynamic_cast<IMobility *>(host->getSubmodule("mobility"));
            Coord coords = mobility->getCurrentPosition();
            int x = coords.getX();
            int y = coords.getY();
            int distance = sqrt((neighbour_x - x)*(neighbour_x - x) + (neighbour_y - y)*(neighbour_y - y));
            if( problemDistance > distance){
                cModule *del_module = getSimulation()->getModule(problemNode);
                cModule *del_neighbourNode = getContainingNode(del_module);
                L3Address del_addr = L3AddressResolver().resolve(del_neighbourNode->getFullName());
                std::vector<L3Address>::iterator del_pos = std::find(destAddresses.begin(), destAddresses.end(), del_addr);
                int index = std::distance(destAddresses.begin(), del_pos);
                distances.erase(distances.begin() + index);
                destAddresses.erase(del_pos);

                std::vector<int>::iterator pos = std::find(neighbours_Id.begin(), neighbours_Id.end(), problemNode);
                neighbours_Id.erase(pos);

                cModule *module = getSimulation()->getModule(moduleId);
                cModule *neighbourNode = getContainingNode(module);
                L3Address addr = L3AddressResolver().resolve(neighbourNode->getFullName());

                destAddresses.push_back(addr);
                distances.push_back(distance);

                problemDistance = 0;
                problemNode = -1;
                neighbours_Id.push_back(moduleId);

                sendNewConnectionRequest(addr);
            }
        }
        if ( strcmp(pk->getName(),"NEW_CONNECTION_REQUEST") == 0 ){
            if (std::find(neighbours_Id.begin(), neighbours_Id.end(), moduleId) == neighbours_Id.end()){
                cModule *module = getSimulation()->getModule(moduleId);
                cModule *neighbourNode = getContainingNode(module);
                L3Address addr = L3AddressResolver().resolve(neighbourNode->getFullName());

                auto data = pk->popAtBack<CurrentCoordsMessage>();
                int neighbour_x = data->getX();
                int neighbour_y = data->getY();

                cModule *host = getContainingNode(this);
                IMobility *mobility = dynamic_cast<IMobility *>(host->getSubmodule("mobility"));
                Coord coords = mobility->getCurrentPosition();
                int x = coords.getX();
                int y = coords.getY();
                int distance = sqrt((neighbour_x - x)*(neighbour_x - x) + (neighbour_y - y)*(neighbour_y - y));

                destAddresses.push_back(addr);
                distances.push_back(distance);

                neighbours_Id.push_back(moduleId);
            }
        }
        numReceived++;
        delete pk;
    }
    void UdpApp::sendBroadcastCoords(){
        char broadcast_string[] = "_BROADCAST";
        Packet *pk = createPacket("Coords", createCoordPayload());
        char *prName = strdup(pk->getName());
        pk->setName(strcat(prName,broadcast_string));
        pk->addPar("sourceId") = getId();
        pk->addPar("msgId") = numSent;
        pk->setTimestamp();

        emit(packetSentSignal, pk);
        socket.sendTo(pk, Ipv4Address::ALLONES_ADDRESS, 1025);
        numSent++;
    }
    void UdpApp::sendBroadcastCoordsReply(int moduleId){
        char broadcast_string[] = "_BROADCAST_REPLY";
        Packet *pk = createPacket("Coords", createCoordPayload());
        char *prName = strdup(pk->getName());
        pk->setName(strcat(prName,broadcast_string));
        pk->addPar("sourceId") = getId();
        pk->addPar("msgId") = numSent;
        pk->setTimestamp();

        cModule *module = getSimulation()->getModule(moduleId);
        cModule *neighbourNode = getContainingNode(module);
        L3Address addr = L3AddressResolver().resolve(neighbourNode->getFullName());
        emit(packetSentSignal, pk);
        socket.sendTo(pk, addr, 1024);
        numSent++;
    }
    void UdpApp::sendNewConnectionRequest(L3Address addr){
        Packet *pk = createPacket("Coords", createCoordPayload());
        char prName[] = "NEW_CONNECTION_REQUEST";
        pk->setName(prName);
        pk->addPar("sourceId") = getId();
        pk->addPar("msgId") = numSent;
        pk->setTimestamp();

        emit(packetSentSignal, pk);
        socket.sendTo(pk, addr, 1024);
    }
}//namespace inet
