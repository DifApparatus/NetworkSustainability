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
#include "inet/physicallayer/contract/packetlevel/ITransmitter.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"

#include <algorithm>
#include <sstream>
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/PatternMatcher.h"
#include "inet/common/Simsignals.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv4/Ipv4Route.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include "inet/networklayer/ipv4/RoutingTableParser.h"
#include "inet/networklayer/common/L3Address_m.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#include "NewPathSearchMessage_m.h"

namespace inet{
    Define_Module(ResilienceUdpApp);

    void ResilienceUdpApp::initialize(int stage){
        UdpBasicApp::initialize(stage);

        Rnes = par("Rnes").doubleValue();
        maxDistance = par("Dmax").doubleValue();
        optimalDistance = par("Dopt").doubleValue();

        cModule *hostNode = getContainingNode(this);// Node that contains this app
        routingTable = dynamic_cast<Ipv4RoutingTable *>(hostNode->getSubmodule("ipv4")->getSubmodule("routingTable"));
    }
    void ResilienceUdpApp::processStart(){
        for (int i = 0; i < routingTable->getNumRoutes(); i++){
           Ipv4Route *route = routingTable->getRoute(i);
           route->setMetric(std::numeric_limits<int>::max());//set infinity to metric
        }
        UdpBasicApp::processStart();
    }
    void ResilienceUdpApp::sendPacket(){
        Packet *packet = createPacket("COORDS");
        packet->insertAtBack(createCoordPayload());
        L3Address destAddr = chooseDestAddr();
        emit(packetSentSignal, packet);
        socket.sendTo(packet, destAddr, destPort);
        numSent++;
    }
    Packet *ResilienceUdpApp::createPacket(char packetName[]){
        std::ostringstream str;
        str << packetName;
        Packet *pk = new Packet(str.str().c_str());
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
            payload->setChunkLength(B(12));
            payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
            return payload;
        }
    inet::Ptr<NewPathSearchMessage> ResilienceUdpApp::createNewPathSearchPayload(Ipv4Address addr, double distance){
        const auto& payload = makeShared<NewPathSearchMessage>();
        payload->setDestAddr(addr);
        payload->setDistance(distance);
        payload->setChunkLength(B(40));
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
            Ipv4Address wlanAddr = wlanAddrByAppId(moduleId);
            updateDistanceInformation(distance, wlanAddr);
            double Reval = evaluateResilience();
            if (Reval < Rnes){//Need for searching new path
                Packet *packet = createPacket("SEARCH_NEW_PATH");
                packet->insertAtBack(createCoordPayload());
                packet->insertAtBack(createNewPathSearchPayload(wlanAddr,distance));
                L3Address destAddr = chooseDestAddr();
                socket.sendTo(packet, destAddr, destPort);
            }
        }
        if ( strcmp(pk->getName(),"SEARCH_NEW_PATH") == 0 ){
            auto data = pk->popAtBack<NewPathSearchMessage>(B(40), Chunk::PeekFlag::PF_ALLOW_SERIALIZATION);
            Ipv4Address addr = data->getDestAddr();
            double destDistance = data->getDistance();//Distance between neighbour node and it's problem node
            double sourceDistance = distanceFromCoordMessage(pk);//Distance to neighbour node

            cModule *hostNode = getContainingNode(this);// Node that contains this app
            Ipv4RoutingTable *routingTable = dynamic_cast<Ipv4RoutingTable *>(hostNode->getSubmodule("ipv4")->getSubmodule("routingTable"));
            for (int i = 0; i < routingTable->getNumRoutes(); i++){
                Ipv4Route *route = routingTable->getRoute(i);
               if (route->getDestination().equals(addr) == true){
                   Ipv4Address gatewayAddr = route->getGateway();
                   if ((gatewayAddr.equals(addr) || gatewayAddr.isUnspecified()) && sourceDistance < destDistance && route->getMetric() < destDistance){
                       Packet *packet = createPacket("SEARCH_NEW_PATH_REPLY");
                       packet->insertAtBack(createCoordPayload());
                       packet->insertAtBack(createNewPathSearchPayload(addr, sourceDistance));
                       Ipv4Address wlanAddr = wlanAddrByAppId(moduleId);
                       socket.sendTo(packet, wlanAddr, destPort);
                   }
                   break;
               }
            }
        }
        if ( strcmp(pk->getName(),"SEARCH_NEW_PATH_REPLY") == 0 ){
            auto data = pk->popAtBack<NewPathSearchMessage>(B(40), Chunk::PeekFlag::PF_ALLOW_SERIALIZATION);
            Ipv4Address problemAddr = data->getDestAddr();
            double newDistance = data->getDistance();
            for (int i = 0; i < routingTable->getNumRoutes(); i++){
                Ipv4Route *route = routingTable->getRoute(i);
               if (route->getDestination().equals(problemAddr) == true || route->getGateway().equals(problemAddr) == true && newDistance < route->getMetric()){
                   Ipv4Address wlanAddr = wlanAddrByAppId(moduleId);
                   route->setGateway(wlanAddr);
                   route->setMetric(newDistance);
               }
            }
        }
    }
    double ResilienceUdpApp::distanceFromCoordMessage(Packet *pk){
        auto data = pk->popAtBack<CurrentCoordsMessage>(B(12));
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
        /*if (distances.size() > 0){
            double averageDistance = 0;
            for (int i = 0; i < distances.size(); i++){
                averageDistance += distances.at(i);
            }
            averageDistance /= distances.size();
            Reval = (maxDistance - averageDistance) / (maxDistance - optimalDistance);
            Reval = (Reval > 1) ? 1 : Reval;
        }*/
        std::vector<Ipv4Address> neighbourAddrs;
        double averageDistance = 0;
        for (int i = 0; i < routingTable->getNumRoutes(); i++){
             Ipv4Route *route = routingTable->getRoute(i);
             Ipv4Address addr = route->getGateway();
             if (addr.isUnspecified()) addr = route->getDestination();
             if (std::find(neighbourAddrs.begin(), neighbourAddrs.end(), addr) == neighbourAddrs.end()){//new neighbour
                 int distance = route->getMetric();
                 if (distance == std::numeric_limits<int>::max()) distance = maxDistance;
                 averageDistance += distance;
                 neighbourAddrs.push_back(addr);
             }
         }
        averageDistance/=neighbourAddrs.size();
        double Reval = (maxDistance - averageDistance) / (maxDistance - optimalDistance);
        Reval = (Reval > 1) ? 1 : Reval;
        return Reval;
    }
/**
 * Only for one wlan
 */
Ipv4Address ResilienceUdpApp::wlanAddrByAppId(int appId) {
    cModule *app = getSimulation()->getModule(appId);
    cModule *hostNode = getContainingNode(app);//Node that transmitted this packet
    InterfaceTable *interfaceTable = dynamic_cast<InterfaceTable *>(hostNode->getSubmodule("interfaceTable"));
    InterfaceEntry *wlan = interfaceTable->getInterface(1);//wlan0
    Ipv4Address wlanAddr = wlan->getIpv4Address();//Address of wlan0
    return wlanAddr;
}
/**
 * Coordinates got from neighbour node with neighbourWlanAddr of wlan0
 */
    void ResilienceUdpApp::updateDistanceInformation(double distance, Ipv4Address neighbourWlanAddr){
         for (int i = 0; i < routingTable->getNumRoutes(); i++){
             Ipv4Route *route = routingTable->getRoute(i);
            if (route->getDestination().equals(neighbourWlanAddr) == true){
                if (route->getGateway().isUnspecified())//If nodes are connected directly
                    route->setMetric(distance);
                else if (route->getMetric() > distance){//if nodes are connected undirectly but direct connection is better
                    route->setMetric(distance);
                    route->setGateway(Ipv4Address::UNSPECIFIED_ADDRESS);
                }
            }
            if (route->getGateway().equals(neighbourWlanAddr) == true){//if neighbour node is a gateway
                route->setMetric(distance);//Update distance
            }
         }
    }

void ResilienceUdpApp::updateDistanceInformation(double distance, Ipv4Address destAddr, Ipv4Address gatewayAddr) {
    for (int i = 0; i < routingTable->getNumRoutes(); i++){
        Ipv4Route *route = routingTable->getRoute(i);
       if (route->getDestination().equals(destAddr) == true || route->getGateway().equals(destAddr) == true){
          route->setGateway(gatewayAddr);
          route->setMetric(distance);
       }
    }
}

}// namesapce inet
