/*
 * BinderTSN5G.h
 *
 *  Created on: Apr 4, 2024
 *      Author: sergio
 */

#ifndef SRC_COMMON_BINDERTSN5G_H_
#define SRC_COMMON_BINDERTSN5G_H_

#include <omnetpp.h>
#include "common/binder/Binder.h"

using namespace std;

class BinderTSN5G : public Binder{
public:
    map<int, set<const char*>> TranslatorSlaves;
    map<int, const char*> TranslatorMasters;

    void setTranslatorSlaves(int gptpDomain, const char* adress);
    void setTranslatorMasters(int gptpDomain, const char* adress);
    set<const char*> getTranslatorSlaves(int gptpDomain);
    const char* getTranslatorMaster(int gptpDomain);
};

#endif /* SRC_COMMON_BINDERTSN5G_H_ */
