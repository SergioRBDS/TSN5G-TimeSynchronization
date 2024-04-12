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

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import simu5g.common.binder.Binder;
import simu5g.common.carrierAggregation.CarrierAggregation;
import simu5g.world.radio.LteChannelControl;
import inet.networks.base.TsnNetworkBase;

import src.common.binder.BinderTSN5G;

network FiveGNetworkBase extends TsnNetworkBase // modulo para n ter q especificar em cada UseCase e ser mais facil de modificar
{
    parameters:
        macForwardingTableConfigurator.typename = default("");
    submodules:
        channelControl: LteChannelControl {
            @display("p=100,800;is=s");
        }
        binder: BinderTSN5G {
            @display("p=100,900;is=s");
        }
        carrierAggregation: CarrierAggregation {
            @display("p=100,1000;is=s");
        }
}
