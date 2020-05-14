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
#include <omnetpp.h>

namespace inet{
    class ResilienceUdpApp : public UdpBasicApp
    {
        double Rnes;
        std::vector<int> neighbours_Id;
        std::vector<int> distances;
        int problemNode;
        int problemDistance;
        UdpSocket broadcastSocket;
        int maxDistance;
        int optimalDistance;
        int correctingDistance;
          protected:
            virtual void initialize(int stage) override;
            virtual void sendPacket() override;
            template<class T> Packet *createPacket(char packetName[], T payload);
            inet::Ptr<CurrentCoordsMessage> createCoordPayload();
            virtual void processPacket(Packet *pk) override;
            void sendBroadcastCoords();
            void sendBroadcastCoordsReply(int destModuleId);
            void sendNewConnectionRequest(L3Address addr);
            int distanceFromCoordMessage(Packet *pk);
        };
}// namespace inet
#endif
