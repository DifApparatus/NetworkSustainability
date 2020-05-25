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
        std::vector<int> neighbours_Id;
        std::vector<double> distances;
        Ipv4RoutingTable *routingTable;

        double maxDistance; // Maximal distance between nodes
        double optimalDistance; // Optimal distance between nodes
        //double correctingDistance; // Distance when its need to correct work of node
          protected:
            virtual void initialize(int stage) override;
            virtual void processStart() override;
            virtual void sendPacket() override;
            Packet *createPacket(char packetName[]);
            inet::Ptr<CurrentCoordsMessage> createCoordPayload();
            inet::Ptr<NewPathSearchMessage> createNewPathSearchPayload(Ipv4Address addr, double distance);
            virtual void processPacket(Packet *pk) override;
            double distanceFromCoordMessage(Packet *pk);
            double evaluateResilience();
            Ipv4Address wlanAddrByAppId(int appId);
            void updateDistanceInformation(double distance, Ipv4Address addr);
            void updateDistanceInformation(double distance, Ipv4Address destAddr, Ipv4Address gatewayAddr);
        };
}// namespace inet
#endif
