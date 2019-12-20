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

#ifndef MYUDPBASICAPP_H_
#define MYUDPBASICAPP_H_

#include "inet/applications/udpapp/UdpBasicApp.h"

namespace inet{

class MyUdpBasicApp : public UdpBasicApp {
    protected:
        void initialize(int stage) override;
        void sendPacket() override;
        void processPacket(Packet *pk) override;
    };
}// namespace inet
#endif /* MYUDPBASICAPP_H_ */