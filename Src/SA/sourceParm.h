#ifndef _SA_PARM_H__

#include "SaString.h"

#define _SA_PARM_H__

typedef struct SSourceParm {
    CSaString     szLanguage;         // language name
    CSaString     szEthnoID;          // ethnologue ID
    CSaString     szDialect;          // dialect
    CSaString     szRegion;           // region
    CSaString     szCountry;          // country
    CSaString     szFamily;           // family
    CSaString     szSpeaker;          // speaker name
    CSaString     szReference;        // notebook reference
    CSaString     szFreeTranslation;  // free translation
    CSaString     szDescription;          // file description
    CSaString     szTranscriber;       // transcriber
    int         nGender;            // speaker gender
} SourceParm;                     // RIFF file header source parameters

#endif