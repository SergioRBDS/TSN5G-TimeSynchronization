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


module NRUeEth extends NRUe{ // eh um NRUe com ethernet e com o nic com outro NRMac para adicionar os dispositivos descritos em intHostAddress
    parameters:
        string intHostAddress = default("");
        *.NRMacType = default("NRMacUe2"); 
        numEthInterfaces = default(1);
        encap.typename = "EthernetEncapsulation";
        eth[*].typename = "EthernetInterface";
        *.forwarding = true;
    gates:
        inout ethg[]  @labels(EtherFrame-conn) @allowUnconnected;
    connections allowunconnected:
        for i=0..sizeof(ethg)-1{
	        ethg[i] <--> { @display("m=s"); } <--> eth[i].phys;
        }
	
}