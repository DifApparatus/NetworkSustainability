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

//
// TODO auto-generated type
//
network Network
{
    @display("bgb=527,393");
    types:
        simple BaseStation
        {
            gates:
                input input_gates[];
                output output_gates[];
        }
        simple Node
        {
            gates:
                input input_gate;
                output output_gate;
        }

    submodules:
        node: Node {
            @display("p=109,193");
        }
        node1: Node {
            @display("p=275,187");
        }
        baseStation: BaseStation {
            @display("p=194,114");
        }
    connections:
        node.output_gate --> baseStation.input_gates++;
        node1.output_gate --> baseStation.input_gates++;
        baseStation.output_gates++ --> node.input_gate;
        baseStation.output_gates++ --> node1.input_gate;
}
