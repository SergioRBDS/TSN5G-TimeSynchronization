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

package src;

import simu5g.nodes.NR.NRUe;

module NRUeEth extends NRUe{
    parameters:
        string intHostAddress = default("");
        nicType = default("NRNicUe2");
        numEthInterfaces = default(1);
        encap.typename = "EthernetLayer";
        eth[*].typename = "LayeredEthernetInterface";
        *.forwarding = true;
    gates:
        inout ethg[numEthInterfaces]  @labels(EtherFrame-conn) @allowUnconnected;
    connections allowunconnected:
        for i=0..sizeof(ethg)-1{
	        ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
        }
	
}