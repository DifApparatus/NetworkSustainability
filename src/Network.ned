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

import inet.applications.contract.IApp;
import inet.applications.pingapp.PingApp;
import inet.applications.tcpapp.TcpBasicClientApp;
import inet.applications.udpapp.UdpBasicApp;
import inet.applications.udpapp.UdpBasicBurst;
import inet.linklayer.configurator.L2NetworkConfigurator;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.configurator.ipv4.Ipv4FlatNetworkConfigurator;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.base.ApplicationLayerNodeBase;
import inet.node.inet.AdhocHost;
import inet.node.inet.SensorNode;
import inet.node.inet.WirelessHost;
import inet.physicallayer.unitdisk.UnitDiskRadioMedium;
import inet.transportlayer.contract.IUdp;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.mobility.MobilityVisualizer;
import inet.physicallayer.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import inet.visualizer.physicallayer.PhysicalLinkVisualizer;


//
// TODO auto-generated type
//
network Network
{   
    parameters:
            @display("bgb=600,400");
            @statistic[dropRate](source=numSent/numDeleted; record=last);
    submodules:
        node1: SensorNode {
            @display("p=253,152;t=1;");
        }
        node2: SensorNode {
            @display("p=191,223;t=2;");
        }
        node3: SensorNode {
            @display("p=318,66;t=3;");
        }
        node4: SensorNode {
            @display("p=344,232;t=4;");
        }
        node5: SensorNode {
            @display("p=409,138;t=5;");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=51,27");
        }
        unitDiskRadioMedium: UnitDiskRadioMedium {
            @display("p=48,82");
        }
        integratedVisualizer: IntegratedVisualizer {
            @display("p=51,145");
        }
    connections allowunconnected:
}
