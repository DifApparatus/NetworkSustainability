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

import inet.applications.udpapp.UdpBasicApp;
import inet.networklayer.configurator.ipv4.Ipv4FlatNetworkConfigurator;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.base.ApplicationLayerNodeBase;
import inet.node.inet.AdhocHost;
import inet.node.inet.SensorNode;
import inet.node.inet.WirelessHost;
import inet.visualizer.mobility.MobilityVisualizer;
import inet.physicallayer.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;


//
// TODO auto-generated type
//

simple MySensorNode
{
    gates:
        input in;
        output out;
}

network Network
{
    @display("bgb=600,400");
    types:
        module MySensorNode extends SensorNode
        {
            submodules:

            connections:
        }

    submodules:
        node1: MySensorNode {
            @display("p=253,152;t=1;r=50");
        }
        node2: MySensorNode {
            @display("p=191,223;t=2;r=50");
        }
        node3: MySensorNode {
            @display("p=318,66;t=3;r=50");
        }
        node4: MySensorNode {
            @display("p=344,232;t=4;r=50");
        }
        node5: MySensorNode {
            @display("p=409,138;t=5;r=50");
        }
        radioMedium: Ieee802154NarrowbandScalarRadioMedium {
            @display("p=77,65");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=171,30");
        }
        visualizer: MobilityVisualizer {
            @display("p=477,242");
        }
    connections allowunconnected:
        node2.pppg[0] <--> ned.DatarateChannel <--> node1.pppg[0];
        node1.pppg[1] <--> ned.DatarateChannel <--> node3.pppg[0];
        node1.pppg[2] <--> ned.DatarateChannel <--> node4.pppg[0];
        node4.pppg[1] <--> ned.DatarateChannel <--> node5.pppg[0];
}
