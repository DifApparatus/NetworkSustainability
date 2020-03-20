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

#ifndef UDPAPP_H_
#define UDPAPP_H_

#include "inet/applications/udpapp/UdpBasicBurst.h"
#include <vector>

namespace inet{

class UdpApp : public UdpBasicBurst {
    std::vector<int> neighbours_Id;
    std::vector<int> distances;
    int problemNode;
    int problemDistance;
    UdpSocket broadcastSocket;
    int maxDistance;
    int optimalDistance;
    int correctingDistance;
    protected:
        void initialize(int stage) override;
        template<class T> Packet *createPacket(char packetName[], T payload);
        inet::Ptr<CurrentCoordsMessage> createCoordPayload();
        virtual void generateBurst() override;
        virtual void processPacket(Packet *pk) override;
        void processResiliencePacket(Packet *pk);
        virtual void processStart() override;
        void sendBroadcastCoords();
        void sendBroadcastCoordsReply(int destModuleId);
        void sendNewConnectionRequest(L3Address addr);
    };
}// namespace inet
#endif /* UDPAPP_H_ */
