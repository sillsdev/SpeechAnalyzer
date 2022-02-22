#pragma once

class CObjectOStream;
class CObjectIStream;

class CVowelSetVersion {
public:
    CVowelSetVersion(const wstring szVersion = _T("3.0"));

    void WriteProperties(CObjectOStream& obs) const;
    BOOL ReadProperties(CObjectIStream& obs);

    wstring GetVersion();
    void SetVersion(const wstring& szVersion);

private:
    wstring m_szVersion;
};

