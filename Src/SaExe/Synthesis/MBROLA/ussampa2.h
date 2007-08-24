/*****************************************************************************************************\
 
                                   MULTIPLE-BYTE IPA TO SAMPA LOOKUP TABLE
 
---------------------------------------------------------------------------------------------------------
   ASAP     SAMPA                  IPA                 SAMPA         Articulatory Description
IPA Codes*  Codes                 Glyph                String
---------------------------------------------------------------------------------------------------------*/
"\x41\x49 \x41\x49"    , // script a, I                  AI
"\x41\x55 \x61\x55"    , // script a, upsilon            aU
"\x45\xAB \x65\x40"    , // epsilon, schwa               e@
"\x49\xAB \x49\x40"    , // I, schwa                     I@
"\x4E\x60 \x4E"        , // eng, syllabic                N
"\x52\xE2 \x6E"        , // flap box                     n  Error: should be flap tilde
"\x52\xF2 \x6E"        , // flap tilde                   n
"\x55\xAB \x55\x40"    , // upsilon, schwa               U@
"\x62\x7C \x62"        , // b, ¬                         b
"\x64\x5A \x64\x5A"    , // d ezh                        dZ
"\x64\x7C \x64"        , // d, ¬                         d
"\x67\x7C \x67"        , // g, ¬                         g
"\x6B\x7C \x6B"        , // k, ¬                         k
"\x6C\x60 \x6C"        , // l, syllabic                  l
"\x6D\x60 \x6D"        , // m, syllabic                  m
"\x6E\x60 \x6E"        , // n, syllabic                  n
"\x70\x7C \x70"        , // p, ¬                         p
"\x74\x53 \x74\x53"    , // t esh                        tS    affricate
"\x74\x7C \x74"        , // t, ¬                         t
"\x75\x2B \x75"        , // u, cross                     u
"\x75\xB1 \x75"        , // u, box                       u  Error: should be u cross
"\x8D\x49 \x4F\x49"    , // turned c, I                  OI
"\xAB\xA5 \x56"        , // upside down V                V
"\xAB\xA8\x38 \x72\x3D", // schwa upside dn backwd r o   r=
"\xAB\xA8\xA5 \x72\x3D", // schwa upside dn backwd r box r= Error: should be o
"\xCE\xD5 \x72\x3D"      // reversed epsilon-rt hk.      r=
 
 
 
 
 
// * first hex code must not be \x20
