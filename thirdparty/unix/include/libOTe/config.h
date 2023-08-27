#pragma once
#include "libOTe/version.h"

#define LIBOTE_VERSION (LIBOTE_VERSION_MAJOR * 10000 + LIBOTE_VERSION_MINOR * 100 + LIBOTE_VERSION_PATCH)

// build the library bit poly mul integration
/* #undef ENABLE_BITPOLYMUL */

// build the library with "simplest" Base OT enabled
/* #undef ENABLE_SIMPLESTOT */

// build the library with the ASM "simplest" Base OT enabled
/* #undef ENABLE_SIMPLESTOT_ASM */

// build the library with POPF Base OT using Ristretto KA enabled
/* #undef ENABLE_MRR */

// build the library with POPF Base OT using Moeller KA enabled
/* #undef ENABLE_MRR_TWIST */

// build the library with Masney Rindal Base OT enabled
/* #undef ENABLE_MR */

// build the library with Masney Rindal Kyber Base OT enabled
/* #undef ENABLE_MR_KYBER */

// build the library with Naor Pinkas Base OT enabled
/* #undef ENABLE_NP */



// build the library with Keller Orse Scholl OT-Ext enabled
/* #undef ENABLE_KOS */

// build the library with IKNP OT-Ext enabled
/* #undef ENABLE_IKNP */

// build the library with Silent OT Extension enabled
/* #undef ENABLE_SILENTOT */



// build the library with KOS Delta-OT-ext enabled
/* #undef ENABLE_DELTA_KOS */

// build the library with IKNP Delta-OT-ext enabled
/* #undef ENABLE_DELTA_IKNP */



// build the library with OOS 1-oo-N OT-Ext enabled
/* #undef ENABLE_OOS */

// build the library with KKRT 1-oo-N OT-Ext enabled
/* #undef ENABLE_KKRT */

// build the library with RR 1-oo-N OT-Ext OT-ext enabled
/* #undef ENABLE_RR */

// build the library with RR approx k-oo-N OT-ext enabled
/* #undef ENABLE_AKN */


#if defined(ENABLE_SIMPLESTOT_ASM) && defined(_MSC_VER)
    #undef ENABLE_SIMPLESTOT_ASM
    #pragma message("ENABLE_SIMPLESTOT_ASM should not be defined on windows.")
#endif
#if defined(ENABLE_MR_KYBER) && defined(_MSC_VER)
    #undef ENABLE_MR_KYBER
    #pragma message("ENABLE_MR_KYBER should not be defined on windows.")
#endif
        
