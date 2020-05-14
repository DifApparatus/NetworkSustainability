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

#include "ResilienceUdpApp.h"

#include "inet/common/packet/printer/PacketPrinter.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "inet/applications/udpapp/UdpBasicApp.h"
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
#include <vector>
#include <algorithm>
#include "omnetpp.h"
#include "inet/networklayer/common/InterfaceTable.h"

namespace inet{
    Define_Module(ResilienceUdpApp);

    void ResilienceUdpApp::initialize(int stage)
    {
        UdpBasicApp::initialize(stage);

        Rnes = par("Rnes").doubleValue();
        problemNode = -1;
        maxDistance = par("Dmax").doubleValue();
        optimalDistance = par("Dopt").doubleValue();
        //correctingDistance = par("Dcor").doubleValue();
    }
    void ResilienceUdpApp::sendPacket()
    {
        Packet *packet = createPacket("COORDS", createCoordPayload());
        L3Address destAddr = chooseDestAddr();
        emit(packetSentSignal, packet);
        socket.sendTo(packet, destAddr, destPort);
        numSent++;
    }
    template<class T>
    Packet *ResilienceUdpApp::createPacket(char packetName[], T payload)
        {
            std::ostringstream str;
            str << packetName;

            Packet *pk = new Packet(str.str().c_str());
            pk->insertAtBack(payload);
            pk->addPar("sourceId") = getId();
            pk->addPar("msgId") = numSent;

            return pk;
        }
        inet::Ptr<CurrentCoordsMessage> ResilienceUdpApp::createCoordPayload(){
            cModule *hostNode = getContainingNode(this);// Node that contains this app
            IMobility *mobility = dynamic_cast<IMobility *>(hostNode->getSubmodule("mobility"));
            Coord coords = mobility->getCurrentPosition();

            const auto& payload = makeShared<CurrentCoordsMessage>();
            payload->setX(coords.x);
            payload->setY(coords.y);
            payload->setZ(0);
            payload->setChunkLength(B(par("messageLength")));
            payload->setSequenceNumber(numSent);
            payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
            return payload;
        }
    void ResilienceUdpApp::processPacket(Packet* pk){
        if (pk->getKind() == UDP_I_ERROR) {
            EV_WARN << "UDP error received\n";
            delete pk;
            return;
        }
        /*double maxDist;
        if (distances.size()>0){
            maxDist = 0;
            for (int i = 0; i < distances.size();i++){
                if (maxDist < distances[i])
                    maxDist = distances[i];
            }
        }
        else maxDist = maxDistance;*/
        int moduleId = pk->par("sourceId");
        int msgId = pk->par("msgId");

        if ( strcmp(pk->getName(),"COORDS") == 0 ){
            double distance = distanceFromCoordMessage(pk);
            if (distance > 1){//Not self packet
                if (std::find(neighbours_Id.begin(), neighbours_Id.end(), moduleId) == neighbours_Id.end()){
                   neighbours_Id.push_back(moduleId);
                   distances.push_back(distance);
                }
                int index = std::distance(neighbours_Id.begin(), std::find(neighbours_Id.begin(), neighbours_Id.end(), moduleId));
                distances.at(index) = distance;
                double Reval = evaluateResilience();
                if (Reval < Rnes) EV << "!!!!!!!!!!!!ALARM!!!!!!!!!!!" << endl;
                else EV << "========================" << Reval << "==================================" << endl;
            }
        }
        if ( strcmp(pk->getName(),"Coords_BROADCAST") == 0 ){
            /*int distance = distanceFromCoordMessage(pk);
            if (std::find(neighbours_Id.begin(), neighbours_Id.end(), moduleId) == neighbours_Id.end()){
                sendBroadcastCoordsReply(moduleId);
            }*/
        }
        if ( strcmp(pk->getName(),"Coords_BROADCAST_REPLY") == 0 ){
        /*    int distance = distanceFromCoordMessage(pk);
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
            }*/
        }
        if ( strcmp(pk->getName(),"NEW_CONNECTION_REQUEST") == 0 ){
           /* if (std::find(neighbours_Id.begin(), neighbours_Id.end(), moduleId) == neighbours_Id.end()){
                cModule *module = getSimulation()->getModule(moduleId);
                cModule *neighbourNode = getContainingNode(module);
                L3Address addr = L3AddressResolver().resolve(neighbourNode->getFullName());

                b dataLength = pk->getDataLength();
                auto data = pk->popAtBack<CurrentCoordsMessage>(dataLength);
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
            }*/
        }
        numReceived++;
    }
    void ResilienceUdpApp::sendBroadcastCoords(){
        Packet *pk = createPacket("COORDS_BROADCAST", createCoordPayload());

        emit(packetSentSignal, pk);
        socket.sendTo(pk, Ipv4Address::ALLONES_ADDRESS, 1025);
        numSent++;
    }
    void ResilienceUdpApp::sendBroadcastCoordsReply(int moduleId){
        Packet *pk = createPacket("Coords_BROADCAST_REPLY", createCoordPayload());

        cModule *module = getSimulation()->getModule(moduleId);
        cModule *neighbourNode = getContainingNode(module);
        L3Address addr = L3AddressResolver().resolve(neighbourNode->getFullName());
        emit(packetSentSignal, pk);
        socket.sendTo(pk, addr, 1024);
        numSent++;
    }
    void ResilienceUdpApp::sendNewConnectionRequest(L3Address addr){
        Packet *pk = createPacket("Coords_NEW_CONNECTION_REQUEST", createCoordPayload());

        emit(packetSentSignal, pk);
        socket.sendTo(pk, addr, 1024);
    }
    double ResilienceUdpApp::distanceFromCoordMessage(Packet *pk){
        b dataLength = pk->getDataLength();
        auto data = pk->popAtBack<CurrentCoordsMessage>(dataLength);
        double neighbour_x = data->getX();
        double neighbour_y = data->getY();

        cModule *host = getContainingNode(this);
        IMobility *mobility = dynamic_cast<IMobility *>(host->getSubmodule("mobility"));
        Coord coords = mobility->getCurrentPosition();
        double x = coords.getX();
        double y = coords.getY();
        double distance = sqrt((neighbour_x - x)*(neighbour_x - x) + (neighbour_y - y)*(neighbour_y - y));
        return distance;
    }
    double ResilienceUdpApp::evaluateResilience(){
        double Reval = 0;
        if (distances.size() > 0){
            double averageDistance = 0;
            for (int i = 0; i < distances.size(); i++){
                averageDistance += distances.at(i);
            }
            averageDistance /= distances.size();
            Reval = (maxDistance - averageDistance) / (maxDistance - optimalDistance);
            Reval = (Reval > 1) ? 1 : Reval;
        }
        return Reval;
    }

}// namesapce inet
