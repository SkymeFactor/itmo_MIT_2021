/**
 * @file ppmd.h
 * @authors SkymeFactor (sergei.51351@gmail.com)
 * @brief declaration of ppmd namespace functions and classes
 * 
 * @version 1.01
 * @date 2021-11-17
 * 
 * @copyright MIT 2021
 */
#pragma once
#include <vector>
#include <iostream>
#include "arithmetic_coder.h"

namespace ppmd {


#define GET_MEAN(SUMM,SHIFT,ROUND) ((SUMM+(1 << (SHIFT-ROUND))) >> (SHIFT))


const int MAX_FREQ=62, ALFA=16, ESCAPE=256;
const int INT_BITS=7, INTERVAL=1 << INT_BITS, PERIOD_BITS=7;
const int MAX_ORDER = 8;

// Extremely important to align Context in memory
// Otherwise pointers logic will break
// Possibly have to fix
#pragma pack(1)
class Context {
public:
    uint16_t NumStats, SummFreq;

    struct STATS {
        unsigned char Symbol, Freq;
        Context* Successor;
    }  *Stats;

    Context* Lesser;
public:
    inline explicit Context(STATS* pStats, Context *LesserContext);
    inline bool encodeBinSymbol(int symbol);// MaxOrder:
    inline bool encodeSymbol1(int symbol);  //  ABCD    context
    bool innerEncode1(int symbol);          //   BCD    lesser
    bool encodeSymbol2(int symbol);         //   BCDE   successor
    inline int decodeBinSymbol();           // other orders:
    inline int decodeSymbol1();             //   BCD    context
    int innerDecode1(int  count);           //    CD    lesser
    int decodeSymbol2();                    //   BCDE   successor
    void rescale();

    // retutns pointer to current context stats (won't work without memory aligning)
    STATS* oneState() const { return  (STATS*) (((unsigned char*)this)+sizeof(uint16_t)); }

#define UPDATE1(p) {                                                            \
        SummFreq += 2;                                                          \
        p->Freq += 2;                                                           \
        if (p[0].Freq > p[-1].Freq) {                                           \
            STATS tmp = p[0];                                                   \
            do p[0] = p[-1]; while (--p != Stats && tmp.Freq > p[-1].Freq);     \
            *p = tmp;                                                           \
            if ((pCStats = p)->Freq > MAX_FREQ)                                 \
                rescale();                                                      \
        } else                                                                  \
            pCStats=p;                                                          \
    }

#define UPDATE2(p) {                                                            \
        SummFreq += 2;                                                          \
        p->Freq += 2;                                                           \
        if ((pCStats = p)->Freq > MAX_FREQ)                                     \
            rescale();                                                          \
        if (NumMasked == 1)                                                     \
            CharMask[PrevContext->oneState()->Symbol] = 0;                      \
        else                                                                    \
            if (NumMasked < 32) {                                               \
                i = NumMasked - 1;                                              \
                CharMask[(p = PrevContext->Stats)->Symbol] = 0;                 \
                do CharMask[(++p)->Symbol] = 0; while (--i != 0);               \
            } else                                                              \
                std::fill_n(CharMask, sizeof(CharMask), 0);                     \
    }

};
#pragma pack()


void EncodeSequence(int order, FILE* EncodedFile, FILE* DecodedFile);
void DecodeSequence(int order, FILE* EncodedFile, FILE* DecodedFile);


} // end of namespace ppmd;
