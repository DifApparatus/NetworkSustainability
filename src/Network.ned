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
import inet.applications.udpapp.UdpApp;
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
        @display("bgb=300,300");
        @statistic[dropRate](source=numSent/numDeleted; record=last);
    submodules:
        node1: SensorNode {
            @display("p=150,152;t=1;is=s");
        }
        node2: SensorNode {
            @display("p=100,200;t=2;is=s");
        }
        node3: SensorNode {
            @display("p=168,66;t=3;is=s");
        }
        node4: SensorNode {
            @display("p=170,139;t=4;is=s");
        }
        node5: SensorNode {
            @display("p=204,65;t=5;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=22,27;is=s");
        }
        unitDiskRadioMedium: UnitDiskRadioMedium {
            @display("p=21,75;is=s");
        }
        integratedVisualizer: IntegratedVisualizer {
            @display("p=22,121;is=s");
        }
    connections allowunconnected:
}
