//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskSnir.h"

#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskNoise.h"
#include "inet/physicallayer/wireless/common/analogmodel/unitdisk/UnitDiskReceptionAnalogModel.h"

namespace inet {

namespace physicallayer {

UnitDiskSnir::UnitDiskSnir(const IReception *reception, const INoise *noise) :
    SnirBase(reception, noise)
{
    auto unitDiskReception = check_and_cast<const UnitDiskReceptionAnalogModel *>(reception->getAnalogModel());
    auto power = unitDiskReception->getPower();
    auto unitDiskNoise = dynamic_cast<const UnitDiskNoise *>(noise);
    auto isInterfering = unitDiskNoise != nullptr && unitDiskNoise->isInterfering();
    isInfinite = (power == UnitDiskReceptionAnalogModel::POWER_RECEIVABLE) && !isInterfering;
}

std::ostream& UnitDiskSnir::printToStream(std::ostream& stream, int level, int evFlags) const
{
    stream << "UnitDiskSnir";
    if (level <= PRINT_LEVEL_DEBUG)
         stream << EV_FIELD(isInfinite);
    return stream;
}

} // namespace physicallayer

} // namespace inet

