/*
 * BinderTSN5G.cpp
 *
 *  Created on: Apr 4, 2024
 *      Author: sergio
 */

#include "BinderTSN5G.h"

Define_Module(BinderTSN5G);

void BinderTSN5G::setTranslatorSlaves(int gptpDomain, const char* adress){
    TranslatorSlaves[gptpDomain].insert(adress);
}

set<const char*> BinderTSN5G::getTranslatorSlaves(int gptpDomain){
    return TranslatorSlaves[gptpDomain];
}

void BinderTSN5G::setTranslatorMasters(int gptpDomain, const char* adress){
    TranslatorMasters[gptpDomain] = adress;
}

const char* BinderTSN5G::getTranslatorMaster(int gptpDomain){
    return TranslatorMasters[gptpDomain];
}

