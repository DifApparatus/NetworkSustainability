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

#ifndef __NETWORKSUSTAINABILITY_MYUDPAPP_H_
#define __NETWORKSUSTAINABILITY_MYUDPAPP_H_

#include <inet/applications/udpapp/UdpBasicApp.h>
#include "CurrentCoordsMessage_m.h"
#include "NewPathSearchMessage_m.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
#include <omnetpp.h>

namespace inet{
    class ResilienceUdpApp : public UdpBasicApp
    {
        double Rnes;
        cMessage *resMsg = nullptr;
        std::vector<int> distUpdateTimes;
        Ipv4RoutingTable *routingTable;

        double maxDistance; // Maximal distance between nodes
        double optimalDistance; // Optimal distance between nodes
          protected:
            virtual void initialize(int stage) override;
            virtual void handleMessage(cMessage *msg) override;
            virtual void processStart() override;
            virtual void sendPacket() override;
            Packet *createPacket(char packetName[]);
            inet::Ptr<CurrentCoordsMessage> createCoordPayload();
            inet::Ptr<NewPathSearchMessage> createNewPathSearchPayload(Ipv4Address destAddr, double distance);
            virtual void processPacket(Packet *pk) override;
            double distanceFromCoordMessage(Packet *pk);
            double evaluateResilience();
            Ipv4Address wlanAddrByAppId(int appId);
            void updateDistanceInformation(double distance, Ipv4Address addr);
            void updateDistanceInformation(double distance, Ipv4Address destAddr, Ipv4Address gatewayAddr);
            int chooseRouteNumberToImprove();
        };
}// namespace inet
#endif
