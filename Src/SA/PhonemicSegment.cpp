#include "Stdafx.h"
#include "PhonemicSegment.h"

//###########################################################################
// CPhonemicSegment data processing

CPhonemicSegment::CPhonemicSegment(EAnnotation index, int master) : CDependentSegment(index,master) {
}

CFontTable * CPhonemicSegment::NewFontTable() const {
    return new CFontTableIPA;
};

bool CPhonemicSegment::Filter() {
    bool changed = false;
    for (int i=0; i<m_Text.GetCount(); i++) {
        changed |= Filter(m_Text[i]);
    }
    return changed;
}

bool CPhonemicSegment::Filter(CString & text) {
    TCHAR cIPASpaceReplace = 0xFFFD;        // Box Character
    bool bChanged = false;
    for (int i=0; i<text.GetLength(); i++) {
        if (text[i]==0) {
            break;
        }
        if (text[i] < 0x20) {
            text.SetAt(i, cIPASpaceReplace);
            bChanged = true;
        }
    }
    return bChanged;
}

