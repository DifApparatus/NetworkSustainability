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

simple ResilienceUdpApp extends UdpBasicApp
{
    parameters:
        double Rnes;// Necessary level of resilience (0 <= Rnes <= 1)
        double Dmax;// Maximal distance between nodes
        double Dopt;// Optimal distance between nodes
        double Dcor;// Distance when its need to correct work of node
        volatile double resilienceInterval @unit(s); // should usually be a random value, e.g. exponential(1)
        
    @class(inet::ResilienceUdpApp);
}
