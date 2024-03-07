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

import simu5g.nodes.NR.gNodeB;
import inet.linklayer.ethernet.contract.IEthernetLayer;
import inet.linklayer.contract.IEthernetInterface;

module gNBEth extends gNodeB // eh um gNB com ethernet
{
    gates:
        inout ethg @labels(EtherFrame-conn) @allowUnconnected;
    submodules:
        eth: <default("LayeredEthernetInterface")> like IEthernetInterface {
            parameters:
                @display("p=158,519.82,row,150;q=txQueue");
        }
        encap: <default("EthernetLayer")> like IEthernetLayer if typename != "" {
            parameters:
                //registerProtocol = true;
                @display("p=259.12,505.6");
        }
    connections:
        encap.lowerLayerOut --> eth.upperLayerIn;
        encap.lowerLayerIn <-- eth.upperLayerOut;

        encap.upperLayerOut --> nl.in++;
        encap.upperLayerIn <-- nl.out++;

        ethg <--> { @display("m=s"); } <--> eth.phys;
}
