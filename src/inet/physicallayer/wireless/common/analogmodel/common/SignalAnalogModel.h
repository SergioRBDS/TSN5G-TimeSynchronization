//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SIGNALANALOGMODEL_H
#define __INET_SIGNALANALOGMODEL_H

#include "inet/physicallayer/wireless/common/contract/bitlevel/ISignalAnalogModel.h"

namespace inet {

namespace physicallayer {

class INET_API SignalAnalogModel : public virtual ISignalAnalogModel
{
  protected:
    const simtime_t preambleDuration;
    const simtime_t headerDuration;
    const simtime_t dataDuration;

  public:
    SignalAnalogModel(simtime_t preambleDuration, simtime_t headerDuration, simtime_t dataDuration);

    virtual std::ostream& printToStream(std::ostream& stream, int level, int evFlags = 0) const override;

    virtual const simtime_t getPreambleDuration() const override { return preambleDuration; }
    virtual const simtime_t getHeaderDuration() const override { return headerDuration; }
    virtual const simtime_t getDataDuration() const override { return dataDuration; }
    virtual const simtime_t getDuration() const override { return preambleDuration + headerDuration + dataDuration; }
};

} // namespace physicallayer

} // namespace inet

#endif

