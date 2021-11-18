/**
 * @file ppmd.cpp
 * @authors SkymeFactor (sergei.51351@gmail.com)
 * @brief implementation of ppmd namespace functions and classes
 * 
 * @version 1.01
 * @date 2021-11-17
 * 
 * @copyright MIT 2021
 */
#include <iostream>
#include "ppmd.h"


namespace ppmd {


static Context * MinContext, * MaxContext, * PrevContext;
static Context::STATS* pCStats;
static unsigned char CharMask[256];
static int NumMasked, NumContexts, MaxOrder;
static uint16_t BSVal, BinSumm[128][8], Esc2Summ[256];


void StartModel() {
    Context* Order0 = (Context *) calloc(1, sizeof(Context));

    Order0->Lesser = nullptr;
    Order0->NumStats = 256;
    Order0->SummFreq = 257;
    Order0->Stats = (Context::STATS *) calloc(256, sizeof(Context::STATS));

    for (int i = 0; i < 256; i++) {
        Order0->Stats[i].Symbol = i;
        Order0->Stats[i].Freq = 1;
        Order0->Stats[i].Successor = nullptr;
    }

    auto p = (MaxContext = Order0)->Stats;
    for (int i = NumContexts = 1; ; i++) {
        MaxContext = new Context(p, MaxContext);
        if (i == MaxOrder)
            break;
        MaxContext->NumStats = 1;
        p = MaxContext->oneState();
        p->Symbol = 0;
        p->Freq = 1;
    }

    MinContext = MaxContext->Lesser;
    if ( pCStats )
        return;
    
    for (int i = 0; i < 128; i++) {
        BinSumm[i][0] = BinSumm[i][1] = BinSumm[i][2] = BinSumm[i][3] =
        BinSumm[i][4] = BinSumm[i][5] = BinSumm[i][6] = BinSumm[i][7] =
            ((i+1) << (INT_BITS+PERIOD_BITS))/(i+2);
        Esc2Summ[(i & ~1)+0] = Esc2Summ[(i & ~1)+1] = (i+2) << PERIOD_BITS;
    }

    for (int i = 128; i < 255; i++)
        Esc2Summ[(i & ~1)+0] = Esc2Summ[(i & ~1)+1] = 130 << PERIOD_BITS;
    
    std::fill_n(CharMask, sizeof(CharMask), 0);
    return;
}


void UpdateModel(int symbol) {
    Context::STATS* p, *ps[MAX_ORDER + 1], **pps;
    Context* pc;
    unsigned int InitFreq;

    if ( !MaxContext->NumStats ) {
        if (MinContext->NumStats == 1) {

            InitFreq = pCStats->Freq;
            pc = MinContext;

            while ( (pc = pc->Lesser)->NumStats == 1 ) {
                if (InitFreq < pc->oneState()->Freq)
                    InitFreq = pc->oneState()->Freq;
            }
        } else {
            unsigned int bf = pCStats->Freq, ef = MinContext->SummFreq-pCStats->Freq;
            InitFreq = 1 + ((2*bf < 3*ef) ? (2*bf > ef) : ((bf + (ef >> 1)) / ef));
        }
    }
    for (pc = MaxContext, pps = ps; pc != MinContext; pc = pc->Lesser, *pps++ = p) {
        if (pc->NumStats < 1)
            (p = pc->oneState())->Freq = InitFreq;
        else if (pc->NumStats == 1) {
            Context::STATS tmp = *(pc->oneState());
            if (tmp.Freq > 16)
                tmp.Freq = 16;
            pc->SummFreq = tmp.Freq + 3 + (BSVal < (INTERVAL << PERIOD_BITS) / 3) +
                    (BSVal < (INTERVAL << PERIOD_BITS)/4);
            pc->Stats = (Context::STATS*) calloc(2, sizeof(Context::STATS));
            if ( !pc->Stats )
                StartModel(); // restart model
            pc->Stats[0] = tmp;
            (p = pc->Stats+1)->Freq = 1;
        } else {
            pc->Stats = (Context::STATS*) realloc(pc->Stats,
                    sizeof(Context::STATS) * (pc->NumStats + 1));
            if ( !pc->Stats )
                StartModel(); // restart model
            if (pc->NumStats == 255)
                for ( p = pc->Stats + pc->NumStats, pc->SummFreq = 0; p != pc->Stats; )
                        pc->SummFreq += (--p)->Freq;
            pc->SummFreq += 2;
            (p = pc->Stats + pc->NumStats)->Freq = 1;
        }
        pc->NumStats++;
        p->Symbol = symbol;
    }
    if ( pCStats->Successor )
        MinContext = pc = pCStats->Successor;
    else
        if ((pc = new Context(pCStats,pc)) == nullptr)
            StartModel();
    
    while (--pps != ps) {
        if ((pc = new Context(*pps,pc)) == nullptr)
            StartModel();
    }
    MaxContext = (*pps)->Successor = pc;
    return;
}


Context::Context(STATS* pStats,Context *LesserContext):
    NumStats(0), Lesser(LesserContext)
{
    pStats->Successor = this;
    NumContexts++;
}


void Context::rescale() {
    int i = NumStats - 1, EscFreq;
    STATS* p1, *p = Stats;

    EscFreq = SummFreq - p->Freq;
    if (this != MaxContext) {
        SummFreq = (p->Freq -= (p->Freq >> 1));
        do {
            EscFreq -= (++p)->Freq;
            SummFreq += (p->Freq -= (p->Freq >> 1));
            if (p[0].Freq > p[-1].Freq) {
                STATS tmp = *(p1 = p);

                do p1[0] = p1[-1]; while (--p1 != Stats && tmp.Freq > p1[-1].Freq);
                
                *p1 = tmp;
            }
        } while (--i != 0);
        pCStats = Stats;
    } else {
        SummFreq = (p->Freq >>= 1);
        do {
            EscFreq -= (++p)->Freq;
            SummFreq += (p->Freq >>= 1);
        } while (--i != 0);

        if (p->Freq == 0) {
            do { i++; } while ((--p)->Freq == 0);
            EscFreq += i;
            if ((NumStats -= i) == 1) {
                STATS tmp = *Stats;

                do { tmp.Freq -= (tmp.Freq >> 1); EscFreq >>= 1; } while (EscFreq > 1);
                
                free(Stats);
                *(pCStats=oneState())=tmp;

                return;
            }
        }
    }
    SummFreq += (EscFreq -= (EscFreq >> 1));
    Stats = (Context::STATS *) realloc(Stats,sizeof(STATS) * NumStats);
}


bool Context::encodeBinSymbol(int symbol) {
    STATS* p = oneState();
    uint16_t& bs = BinSumm[p->Freq - 1][(Lesser->NumStats < 8) ? (Lesser->NumStats) : (0)];

    if (p->Symbol == symbol) {
        SubRange.LowCount = 0;
        SubRange.HighCount = bs;
        bs += INTERVAL - GET_MEAN(bs, PERIOD_BITS, 2);
        p->Freq += (p->Freq < 128);
        pCStats = p;

        return true;
    } else {
        CharMask[p->Symbol] = 1;
        NumMasked = 1;
        SubRange.LowCount = bs;
        SubRange.HighCount = INTERVAL << PERIOD_BITS;
        BSVal = (bs -= GET_MEAN(bs, PERIOD_BITS, 2));

        return false;
    }
}


int Context::decodeBinSymbol() {
    int count = ariGetCurrentShiftCount(INT_BITS + PERIOD_BITS);
    STATS* p = oneState();
    uint16_t& bs=BinSumm[p->Freq-1][(Lesser->NumStats < 8) ? (Lesser->NumStats) : (0)];

    if (count < bs) {
        SubRange.LowCount = 0;
        SubRange.HighCount = bs;
        bs += INTERVAL - GET_MEAN(bs, PERIOD_BITS, 2);
        p->Freq += (p->Freq < 128);
        pCStats = p;
        
        return p->Symbol;
    } else {
        CharMask[p->Symbol] = 1;
        NumMasked = 1;
        SubRange.LowCount = bs;
        SubRange.HighCount = INTERVAL << PERIOD_BITS;
        BSVal = (bs -= GET_MEAN(bs, PERIOD_BITS, 2));
        
        return ESCAPE;
    }
}


bool Context::encodeSymbol1(int symbol) {
    SubRange.scale = SummFreq;
    if (Stats->Symbol == symbol) {
        SubRange.LowCount = 0;
        SubRange.HighCount = Stats->Freq;
        pCStats = Stats;
        SummFreq += 2;
        if ((Stats->Freq += 2) > MAX_FREQ)
            rescale();
        return true;
    }
    return innerEncode1(symbol);
}


int Context::decodeSymbol1() {
    SubRange.scale = SummFreq;
    int count = ariGetCurrentCount();
    if (Stats->Freq > count) {
        SubRange.LowCount = 0;
        SubRange.HighCount = Stats->Freq;
        pCStats = Stats;
        SummFreq += 2;
        if ((Stats->Freq += 2) > MAX_FREQ)
            rescale();
        return pCStats->Symbol;
    }
    return innerDecode1(count);
}


bool Context::innerEncode1(int symbol) {
    STATS* p = Stats;
    int i = NumStats - 1;
    SubRange.LowCount = p->Freq;
    do {
        if ((++p)->Symbol == symbol) {
            SubRange.HighCount = SubRange.LowCount + p->Freq;
            UPDATE1(p);
            return true;
        }
        SubRange.LowCount += p->Freq;
    } while (--i != 0);
    CharMask[p->Symbol] = 1;
    
    do { CharMask[(--p)->Symbol] = 1; } while (p != Stats);
    
    SubRange.HighCount = SubRange.scale;
    NumMasked = NumStats;

    return false;
}


int Context::innerDecode1(int count) {
    STATS* p = Stats;
    int i = NumStats - 1;
    SubRange.HighCount = p->Freq;
    do {
        SubRange.HighCount += (++p)->Freq;
        if (SubRange.HighCount > count) {
            SubRange.LowCount = SubRange.HighCount-p->Freq;
            UPDATE1(p);
            
            return p->Symbol;
        }
    } while (--i != 0);
    CharMask[p->Symbol] = 1;
    
    do { CharMask[(--p)->Symbol] = 1; } while (p != Stats);

    SubRange.LowCount = SubRange.HighCount;
    SubRange.HighCount = SubRange.scale;
    NumMasked = NumStats;

    return ESCAPE;
}


bool Context::encodeSymbol2(int symbol) {
    int i = NumStats - NumMasked;
    uint16_t* pes = Esc2Summ + (i & ~1) + (SummFreq < 4 * NumStats);
    uint16_t EscFreq = (NumStats != 256) ? GET_MEAN(*pes, PERIOD_BITS, 1) : 0;
    
    SubRange.scale = EscFreq + (EscFreq == 0);
    *pes -= EscFreq;
    STATS* p = Stats - 1;   // Relevant for multiple contexts on the same level
    SubRange.LowCount = 0;

    do {
        do { p++; } while ( CharMask[p->Symbol] );
        if (p->Symbol == symbol) {
            SubRange.scale += (SubRange.HighCount=SubRange.LowCount+p->Freq);
            if (--i != 0) {
                STATS* p1 = p;
                do {
                    do { p1++; } while ( CharMask[p1->Symbol] );
                    SubRange.scale += p1->Freq;
                } while ( --i );
            }
            UPDATE2(p);
            return true;
        }
        SubRange.LowCount += p->Freq;
    } while ( --i );
    
    for (CharMask[p->Symbol] = 1; p != Stats; CharMask[(--p)->Symbol] = 1);

    SubRange.HighCount = (SubRange.scale += SubRange.LowCount);
    *pes += SubRange.scale;
    NumMasked = NumStats;

    return false;
}

int Context::decodeSymbol2() {
    int count, i = NumStats - NumMasked;
    uint16_t* pes = Esc2Summ + (i & ~1) + (SummFreq < 4 * NumStats);
    uint16_t EscFreq = (NumStats != 256) ? GET_MEAN(*pes, PERIOD_BITS, 1) : 0;
    
    SubRange.scale = EscFreq + (EscFreq == 0);
    *pes -= EscFreq;

    STATS* p = Stats - 1;   // Relevant for multiple contexts on the same level
    do {
        do { p++; } while ( CharMask[p->Symbol] );
        SubRange.scale += p->Freq;
    } while ( --i );

    count = ariGetCurrentCount();
    SubRange.HighCount = 0;
    p = Stats-1;
    i = NumStats-NumMasked;

    do {
        do { p++; } while ( CharMask[p->Symbol] );
        SubRange.HighCount += p->Freq;
        if (SubRange.HighCount > count) {
            SubRange.LowCount = SubRange.HighCount-p->Freq;
            UPDATE2(p);
            return pCStats->Symbol;
        }
    } while ( --i );

    for (CharMask[p->Symbol] = 1;p != Stats;CharMask[(--p)->Symbol] = 1);

    SubRange.LowCount = SubRange.HighCount;
    SubRange.HighCount = SubRange.scale;
    *pes += SubRange.scale;
    NumMasked = NumStats;

    return ESCAPE;
}


void EncodeSequence(int order, FILE* EncodedFile, FILE* DecodedFile) {
    
    ppmd::MaxOrder = order;
	pCStats = nullptr;

    ariInitEncoder(EncodedFile);
    StartModel();
    
    bool SymbolEncoded;

    for (int ns = MinContext->NumStats, i = 0x10000; ; ns = MinContext->NumStats) {
        int c = getc(DecodedFile);
        ariEncoderNormalize(EncodedFile);

        if (ns == 1) {
            SymbolEncoded = MinContext->encodeBinSymbol(c);
            ariShiftEncodeSymbol(INT_BITS + PERIOD_BITS);
        } else {
            SymbolEncoded = MinContext->encodeSymbol1(c);
            ariEncodeSymbol();
        }

        while ( !SymbolEncoded ) {
            ariEncoderNormalize(EncodedFile);
            PrevContext = MinContext;

            do {
                MinContext = MinContext->Lesser;
                if ( !MinContext ){
                    // Finish the encoding process
					ariFlushEncoder(EncodedFile);
					return;
				}
            } while (MinContext->NumStats == NumMasked);

            SymbolEncoded = MinContext->encodeSymbol2(c);
            ariEncodeSymbol();
        }
        
        if (MaxContext == MinContext)
            MinContext = MaxContext = pCStats->Successor;
        else
            UpdateModel(c);
        
        if ( !--i )
            i = 0x20000;
    }
}

void DecodeSequence(int order, FILE* EncodedFile, FILE* DecodedFile) {
    
    ppmd::MaxOrder=order;
	pCStats=nullptr;

	ariInitDecoder(EncodedFile);
    StartModel();

    for (int ns = MinContext->NumStats, i = 0x10000; ;ns = MinContext->NumStats) {
        ariDecoderNormalize(EncodedFile);
        int c = (ns == 1) ? MinContext->decodeBinSymbol() : MinContext->decodeSymbol1();
        ariRemoveSubrange();

        while (c == ESCAPE) {
            ariDecoderNormalize(EncodedFile);
            PrevContext = MinContext;

            do {
                MinContext = MinContext->Lesser;
                if ( !MinContext ) {
                    // Finish the decoding process
                    return;
                }
            } while (MinContext->NumStats == NumMasked);

            c = MinContext->decodeSymbol2();
            ariRemoveSubrange();
        }
        putc(c, DecodedFile);

        if (MaxContext == MinContext)
            MinContext = MaxContext = pCStats->Successor;
        else
            UpdateModel(c);
        
        if ( !--i )
            i=0x20000;
    }
}

} // end of namespace ppmd;
