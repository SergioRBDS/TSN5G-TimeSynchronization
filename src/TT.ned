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

import inet.node.inet.Router;
import inet.node.tsn.TsnSwitch;
import inet.applications.contract.IApp;

module TT extends Router{
    parameters:
        bool hasTimeSynchronization = default(false);
        clock.typename = default(hasTimeSynchronization ? "SettableClock" : ""); // enable explicit local clock model
        hasUdp = default(true);
    submodules:
        gptp: <default("Gptp5g")> like IApp if hasTimeSynchronization {
            gptpNodeType = default("BRIDGE_NODE");
            fiveGPort = default("eth0");
            TSNPort = default("eth1");
            @display("p=700,75");
        }
    connections:
        if hasTimeSynchronization {
            gptp.socketOut --> at.in++;
            at.out++ --> gptp.socketIn;
        }
}