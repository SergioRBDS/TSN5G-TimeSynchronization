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

import inet.node.tsn.TsnDevice;

import simu5g.nodes.NR.NRUe;
import simu5g.stack.ICellularNic;

// nao usei até agr pq n sei cmo o gptp agiria com a saida sendo por um cellularnic
module UEDSTT extends TsnDevice // é um UE com ethernet ou um TSN com cellularNic ( poderia ver se precisava de tantos modulos como um TsnDevice )
{
    parameters:
        string intHostAddress = default("");
        string nicType = default("NRNicUe");
        string nodeType = "UE";  // DO NOT CHANGE
        int masterId @mutable;
        int macNodeId @mutable = default(0); // TODO: this is not a real parameter
        int macCellId @mutable = default(0); // TODO: this is not a real parameter
        int nrMasterId @mutable = default(0);
        int nrMacNodeId @mutable = default(0); // TODO: this is not a real parameter
        int nrMacCellId @mutable = default(0);
        encap.typename = "EthernetLayer";
        eth[*].typename = "LayeredEthernetInterface";
        *.forwarding = true;
    gates:
        input nrRadioIn @directIn;
    submodules:
        cellularNic: <nicType> like ICellularNic {
            nodeType = parent.nodeType;
            @display("p=1140.36,1026.324");
        }
    connections:
        cellularNic.upperLayerOut --> li.in++;
        cellularNic.upperLayerIn <-- li.out++;

        cellularNic.radioIn <-- radioIn++;
        cellularNic.nrRadioIn <-- nrRadioIn;
}
