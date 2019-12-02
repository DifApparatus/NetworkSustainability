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

package networksustainability;

import inet.networklayer.configurator.ipv4.Ipv4FlatNetworkConfigurator;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.SensorNode;
import inet.node.inet.WirelessHost;
import inet.visualizer.mobility.MobilityVisualizer;
import inet.physicallayer.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;


//
// TODO auto-generated type
//
network Network
{
    @display("bgb=600,400");

    submodules:
        node1: SensorNode {
            @display("p=253,152");
        }
        node2: SensorNode {
            @display("p=191,223");
        }
        node3: SensorNode {
            @display("p=318,66");
        }
        node4: SensorNode {
            @display("p=344,232");
        }
        node5: SensorNode {
            @display("p=409,138");
        }
        radioMedium: Ieee802154NarrowbandScalarRadioMedium;
        configurator: Ipv4NetworkConfigurator;
        visualizer: MobilityVisualizer;
    connections allowunconnected:
        node2.pppg[0] <--> ned.DatarateChannel <--> node1.pppg[0];
        node1.pppg[1] <--> ned.DatarateChannel <--> node3.pppg[0];
        node1.pppg[2] <--> ned.DatarateChannel <--> node4.pppg[0];
        node4.pppg[1] <--> ned.DatarateChannel <--> node5.pppg[0];
        
}
