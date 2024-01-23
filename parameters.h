#ifndef PARAMETERS_H
#define PARAMETERS_H

#if (SECURITY == 64)
    #define NBITS_FIELD     128
    #define NBYTES_FIELD    16
    #define WORDS_FIELD     (NBITS_FIELD/RADIX)
#elif (SECURITY == 96)
    #define NBITS_FIELD     192
    #define NBYTES_FIELD    24
    #define WORDS_FIELD     (NBITS_FIELD/RADIX)
#elif (SECURITY == 128)
    #define NBITS_FIELD     256
    #define NBYTES_FIELD    32
    #define WORDS_FIELD     (NBITS_FIELD/RADIX)
#endif
