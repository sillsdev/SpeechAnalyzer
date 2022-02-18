#pragma once

class CVowelSetVersion {
public:
    CVowelSetVersion(const wstring szVersion = _T("3.0"));

    void WriteProperties(ObjectOStream& obs) const;
    BOOL ReadProperties(ObjectIStream& obs);

    wstring GetVersion();
    void SetVersion(const wstring& szVersion);

private:
    wstring m_szVersion;
};

