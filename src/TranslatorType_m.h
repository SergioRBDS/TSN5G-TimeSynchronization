//
// Generated file, do not edit! Created by opp_msgtool 6.0 from src/TranslatorType.msg.
//

#ifndef __TRANSLATORTYPE_M_H
#define __TRANSLATORTYPE_M_H

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wreserved-id-macro"
#endif
#include <omnetpp.h>

// opp_msgtool version check
#define MSGC_VERSION 0x0600
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of opp_msgtool: 'make clean' should help.
#endif

/**
 * Enum generated from <tt>src/TranslatorType.msg:16</tt> by opp_msgtool.
 * <pre>
 * enum TranslatorType
 * {
 *     DS_TT = 91;
 *     NS_TT = 92;
 * }
 * </pre>
 */
enum TranslatorType {
    DS_TT = 91,
    NS_TT = 92
};

inline void doParsimPacking(omnetpp::cCommBuffer *b, const TranslatorType& e) { b->pack(static_cast<int>(e)); }
inline void doParsimUnpacking(omnetpp::cCommBuffer *b, TranslatorType& e) { int n; b->unpack(n); e = static_cast<TranslatorType>(n); }


namespace omnetpp {


}  // namespace omnetpp

#endif // ifndef __TRANSLATORTYPE_M_H

