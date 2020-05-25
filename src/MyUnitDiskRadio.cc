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

#include "MyUnitDiskRadio.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"
#include "inet/physicallayer/common/packetlevel/RadioMedium.h"
#include "inet/physicallayer/contract/packetlevel/SignalTag_m.h"

namespace inet{
    Define_Module(MyUnitDiskRadio);

    MyUnitDiskRadio::MyUnitDiskRadio() {
        // TODO Auto-generated constructor stub

    }

    MyUnitDiskRadio::~MyUnitDiskRadio() {
        // TODO Auto-generated destructor stub
    }

    void MyUnitDiskRadio::handleUpperPacket(Packet *packet)
    {
        emit(packetReceivedFromUpperSignal, packet);
        if (isTransmitterMode(radioMode)) {
            if (transmissionTimer->isScheduled())
                return;
            if (separateTransmissionParts)
                startTransmission(packet, physicallayer::IRadioSignal::SIGNAL_PART_PREAMBLE);
            else
                startTransmission(packet, physicallayer::IRadioSignal::SIGNAL_PART_WHOLE);
        }
        else {
            EV_ERROR << "Radio is not in transmitter or transceiver mode, dropping frame." << endl;
            delete packet;
        }
    }
}

