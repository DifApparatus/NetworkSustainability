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
#include "inet/applications/udpapp/UdpBasicApp.h"
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

#include <omnetpp.h>

namespace inet{

    Define_Module(MyUdpBasicApp);

    void MyUdpBasicApp::initialize(int stage)
    {
        UdpBasicApp::initialize(stage);
    }
    void MyUdpBasicApp::sendPacket()
       {
           cModule *host = getContainingNode(this);
           IMobility *mobility = dynamic_cast<IMobility *>(host->getSubmodule("mobility"));
           Coord coords = mobility->getCurrentPosition();

           std::ostringstream str;
           str << packetName;
           Packet *packet = new Packet(str.str().c_str());
           if(dontFragment)
               packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

           const auto& payload = makeShared<CurrentCoordsMessage>("CurrentCoordinates");
           payload->setX(coords.x);
           payload->setY(coords.y);
           payload->setZ(0);
           payload->setChunkLength(B(par("messageLength")));
           payload->setSequenceNumber(numSent);
           payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

           packet->insertAtBack(payload);

           //PacketPrinter printer; // turns packets into human readable strings
           //printer.printPacket(std::cout, packet); // print to standard output

           L3Address destAddr = chooseDestAddr();

           emit(packetSentSignal, packet);
           socket.sendTo(packet, destAddr, destPort);
           numSent++;
       }
    void MyUdpBasicApp::processPacket(Packet *pk)
    {
        emit(packetReceivedSignal, pk);
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
            EV_INFO << "=================================================================================================" << distance << endl;
            EV_INFO << "DISTANCE TO NEIGHBOUR EQUALS " << distance << endl;
        }
        EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
        delete pk;
        numReceived++;
    }
}//namespace inet
