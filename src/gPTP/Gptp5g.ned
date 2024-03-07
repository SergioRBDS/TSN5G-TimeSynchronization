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

import inet.linklayer.ieee8021as.Gptp;

simple Gptp5g extends Gptp // gPTP q passa por camada IP
{
    parameters:
        bool TT = default(true);
        string ipNIC;
        string dAddress;
        // portas são necessarias na comunicação IP
        int destPort = default(9000);
        int localPort = default(9000);
        // modificado para esperar o bind (necessario?)
        pdelayInitialOffset = default(0ps);
        @class(Gptp5g);
}
