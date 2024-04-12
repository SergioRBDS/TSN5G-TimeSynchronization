//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UNITDISKTRANSMISSIONANALOGMODEL_H
#define __INET_UNITDISKTRANSMISSIONANALOGMODEL_H

#include "inet/common/Units.h"
#include "inet/physicallayer/wireless/common/analogmodel/common/SignalAnalogModel.h"

namespace inet {

namespace physicallayer {

using namespace inet::units::values;

class INET_API UnitDiskTransmissionAnalogModel : public SignalAnalogModel, public ITransmissionAnalogModel
{
  protected:
    const m communicationRange;
    const m interferenceRange;
    const m detectionRange;

  public:
    UnitDiskTransmissionAnalogModel(simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration, m communicationRange, m interferenceRange, m detectionRange);

    virtual m getCommunicationRange() const { return communicationRange; }
    virtual m getInterferenceRange() const { return interferenceRange; }
    virtual m getDetectionRange() const { return detectionRange; }
};

} // namespace physicallayer

} // namespace inet

#endif

