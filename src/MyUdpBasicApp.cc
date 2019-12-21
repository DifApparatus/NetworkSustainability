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
    }
    void MyUdpBasicApp::sendPacket()
       {
           cModule *host = getContainingNode(this);
           IMobility *mobility = dynamic_cast<IMobility *>(host->getSubmodule("mobility"));
           Coord coords = mobility->getCurrentPosition();

           std::ostringstream str;
           //str << packetName;
           Packet *packet = new Packet(str.str().c_str());
           //if(dontFragment)
               packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

           const auto& payload = makeShared<CurrentCoordsMessage>("CurrentCoordinates");
           payload->setX(coords.x);
           payload->setY(coords.y);
           payload->setZ(0);
           payload->setChunkLength(B(par("messageLength")));
           payload->setSequenceNumber(numSent);
           payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

           packet->insertAtBack(payload);
           //L3Address destAddr = chooseDestAddr();
           /*for (int i=0;i<destAddresses.size();i++){
               Packet *copy_packet = packet->dup();
               emit(packetSentSignal, copy_packet);
               socket.sendTo(copy_packet, destAddresses[i], destPort);
               EV_INFO << "==========================================================" << destAddresses[i] <<endl;
               numSent++;
           }*/

           /*L3Address *destAddr = new L3Address();
           Ipv4Address *ipv4 = new Ipv4Address(255,255,255,255);
           destAddr->set(*ipv4);
           Packet *copy_packet = packet->dup();
           emit(packetSentSignal, copy_packet);
           socket.sendTo(copy_packet, *destAddr, destPort);*/
           delete packet;
       }
    //HORRIBLE method, but...
    void MyUdpBasicApp::sendPacket(char* broadcast_string){
        /*cModule *host = getContainingNode(this);
                   IMobility *mobility = dynamic_cast<IMobility *>(host->getSubmodule("mobility"));
                   Coord coords = mobility->getCurrentPosition();

                   std::ostringstream str;
                   str << std::strcat(broadcast_string, packetName);
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

                  /* L3Address *destAddr = new L3Address();
                   Ipv4Address *ipv4 = new Ipv4Address(255,255,255,255);*/
                   //L3Address destAddr = chooseDestAddr();
                   //destAddr->set(*ipv4);
                   //emit(packetSentSignal, packet);
                   //socket.sendTo(packet, *destAddr, destPort);
                   //EV_INFO << "+++++++++++++++++++++++++++++++" << packet->getName() << "+++++++++++++++++++++++++++++++++++++\n";
                   //EV_INFO << "============================================================================\n";*/

    }
    void MyUdpBasicApp::processPacket(Packet *pk)
    {
        emit(packetReceivedSignal, pk);
        //EV << "+++++++++++++++++++++++++++++++" << pk->getName() << "+++++++++++++++++++++++++++++++++++++\n";
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
            if (distance){
                /*char broadcast_string[] = "_Broadcast";
                Packet* pk1 = pk->dup();
                L3Address destAddr = chooseDestAddr();
                emit(packetSentSignal, pk1);
                socket.sendTo(pk1, destAddr, destPort);*/
                //this->sendPacket(broadcast_string);
            }
            numReceived++;
        }
        if (strcmp(pk->getName(),"BROADCAST_Coords") == 0 ){
           EV_INFO << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" <<endl;
        }
        delete pk;
    }
}//namespace inet
