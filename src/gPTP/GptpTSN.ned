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

package src.gPTP;

import inet.linklayer.ieee8021as.IGptp;

simple GptpTSN like IGptp
{
   parameters:
        string clockModule = default(""); // relative path of a module that implements IClock; optional
        string interfaceTableModule; // relative path of the interface table module
        string gptpNodeType; // @enum("GptpNodeType"): MASTER_NODE, BRIDGE_NODE, SLAVE_NODE
        int domainNumber = default(0); // specifies the time domain number used in gPTP messages
        string slavePort = default(""); // port for receiving time (empty for MASTER_NODE)
        object masterPorts = default([]); // ports for sending out time (empty for SLAVE_NODE)
        double correctionField @unit(s) = default(0s); // time correction for link delay measurements
        double syncInterval @unit(s) = default(0.125s); // time interval between SYNC messages
        double pdelayInterval @unit(s) = default(1s); // frequency of link delay measurements
        double syncInitialOffset @unit(s) = default(syncInterval); // time of first SYNC message
        double pdelayInitialOffset @unit(s) = default(0s); // time of first link delay measurement
        
        double pDelayReqProcessingTime @unit(s) = default(8us);  // processing time between arrived PDelayReq and send of PDelayResp
        double followUpInterval @unit(s) = default(7us);
        
        @display("i=block/timer");
        @signal[localTime](type=simtime_t);  // as clocktime_t
        @signal[timeDifference](type=simtime_t);
        @signal[correctionField](type=simtime_t);
        @signal[rateRatio](type=double);
        @signal[peerDelay](type=simtime_t);
        @statistic[localTime](record=vector; interpolationmode=linear);
        @statistic[timeDifference](record=vector; interpolationmode=linear);
        @statistic[correctionField](record=vector; interpolationmode=linear);
        @statistic[rateRatio](record=vector; interpolationmode=sample-hold);
        @statistic[peerDelay](record=vector; interpolationmode=sample-hold);
        @selfMessageKinds(inet::GptpSelfMsgKind);
    gates:
        input socketIn;
        output socketOut;
}
