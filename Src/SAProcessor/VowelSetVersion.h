#pragma once

class CVowelSetVersion {
public:
    CVowelSetVersion(const wstring szVersion = _T("3.0")) {
        m_szVersion = szVersion;
    };

    void WriteProperties(ObjectOStream& obs) const;
    BOOL ReadProperties(ObjectIStream& obs);

    wstring GetVersion() {
        return m_szVersion;
    };
    void SetVersion(const wstring& szVersion) {
        m_szVersion = szVersion;
    };

private:
    wstring m_szVersion;
};

