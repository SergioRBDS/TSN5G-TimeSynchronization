/*
 * NRMacUe2.h
 *
 *  Created on: 30 de jan. de 2024
 *      Author: vboxuser
 */

#ifndef SRC_STACK_MAC_LAYER_NRMACUE2_H_
#define SRC_STACK_MAC_LAYER_NRMACUE2_H_

#include "stack/mac/layer/NRMacUe.h"

// basicamente adiciona os dispositivos descritos em intHostAddress no no NRUeEth
class NRMacUe2 : public NRMacUe{
    protected:
        virtual void initialize(int stage) override;
public:
    NRMacUe2();
    virtual ~NRMacUe2();
};

#endif /* SRC_STACK_MAC_LAYER_NRMACUE2_H_ */
