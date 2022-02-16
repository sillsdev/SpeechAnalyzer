#include "pch.h"
#include "VowelSetVersion.h"

static LPCSTR psz_Version = "Version";

void CVowelSetVersion::WriteProperties(ObjectOStream obs) const {
    obs.WriteBeginMarker(psz_Version, m_szVersion.c_str());
    obs.WriteEndMarker(psz_Version);
}

BOOL CVowelSetVersion::ReadProperties(ObjectIStream& obs) {
    char buffer[1024];
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_Version, buffer, _countof(buffer))) {
        return FALSE;
    }
    m_szVersion = buffer;
    while (!obs.bAtEnd()) {
        if (obs.bEnd(psz_Version)) {
            break;
        }
    }
    return TRUE;
}

