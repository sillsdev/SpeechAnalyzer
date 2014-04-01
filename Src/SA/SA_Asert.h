
/////////////////////////////////////////////////////////////////////////////
// Diagnostic support
#ifdef _DEBUG
extern "C"
{
    void AFXAPI SaAssertFailedLine(LPCSTR lpszFileName, int nLine);
}
#define THIS_FILE          __FILE__
#define SA_ASSERT(f)          ((f) ? (void)0 : \
                ::SaAssertFailedLine(THIS_FILE, __LINE__))
#else
#define SA_ASSERT(f)          ((void)(f))
#endif // _DEBUG
