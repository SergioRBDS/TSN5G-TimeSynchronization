//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/generic/GenericTransmission.h"

namespace inet {

namespace physicallayer {

GenericTransmission::GenericTransmission(const IRadio *transmitter, const Packet *packet, const simtime_t startTime, const simtime_t endTime, const simtime_t preambleDuration, const simtime_t headerDuration, const simtime_t dataDuration, const Coord startPosition, const Coord endPosition, const Quaternion startOrientation, const Quaternion endOrientation, const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel, const ITransmissionSymbolModel *symbolModel, const ITransmissionSampleModel *sampleModel, const ITransmissionAnalogModel *analogModel) :
    TransmissionBase(transmitter, packet, startTime, endTime, preambleDuration, headerDuration, dataDuration, startPosition, endPosition, startOrientation, endOrientation, packetModel, bitModel, symbolModel, sampleModel, analogModel)
{
}

std::ostream& GenericTransmission::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "GenericTransmission";
    return TransmissionBase::printToStream(stream, level);
}

} // namespace physicallayer

} // namespace inet

