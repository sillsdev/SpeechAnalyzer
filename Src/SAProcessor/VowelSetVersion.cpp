#include "pch.h"
#include "VowelSetVersion.h"

static LPCSTR psz_Version = "Version";

CVowelSetVersion::CVowelSetVersion(const wstring szVersion) {
    m_szVersion = szVersion;
}

wstring CVowelSetVersion::GetVersion() {
    return m_szVersion;
}

void CVowelSetVersion::SetVersion(const wstring& szVersion) {
    m_szVersion = szVersion;
}

void CVowelSetVersion::WriteProperties(ObjectOStream& obs) const {
    obs.WriteBeginMarker(psz_Version, _to_utf8(m_szVersion).c_str());
    obs.WriteEndMarker(psz_Version);
}

BOOL CVowelSetVersion::ReadProperties(ObjectIStream& obs) {
    char buffer[1024];
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_Version, buffer, _countof(buffer))) {
        return FALSE;
    }
    m_szVersion = _to_wstring(buffer);
    while (!obs.bAtEnd()) {
        if (obs.bEnd(psz_Version)) {
            break;
        }
    }
    return TRUE;
}

