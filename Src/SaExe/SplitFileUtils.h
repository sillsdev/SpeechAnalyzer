#ifndef SPLITFILEUTILS_H
#define SPLITFILEUTILS_H

#include "AppDefs.h"

#include <string>
using std::wstring;

class CSaView;
class CGlossSegment;
class CMusicPhraseSegment;
class CReferenceSegment;

// split file feature methods
wstring GenerateWordSplitName( CGlossSegment * g, CSaView* pView, EWordFilenameConvention convention, int index);
bool GeneratePhraseSplitName( Annotations type, CMusicPhraseSegment * s, CSaView* pView, EPhraseFilenameConvention convention, int index, wstring & result);
wstring FilterName( wstring text);
bool ValidateWordFilenames( EWordFilenameConvention convention, LPCTSTR path, BOOL skipEmptyGloss);
bool ValidatePhraseFilenames( Annotations & type, EPhraseFilenameConvention & convention, wstring & path);
bool ExportWordSegments( int & count, EWordFilenameConvention convention, LPCTSTR glossPath, BOOL skipEmptyGloss);
int ComposeWordSegmentFilename( CGlossSegment * g, int index, EWordFilenameConvention convention, LPCTSTR path, BOOL skipEmptyGloss, LPTSTR szBuffer, size_t size);
int ExportWordSegment( int & count, CGlossSegment * g, int index, EWordFilenameConvention convention, LPCTSTR glossPath, BOOL skipEmptyGloss);
bool ExportPhraseSegments( Annotations type, int & count, EPhraseFilenameConvention & convention, wstring & phrasePath);
int FindNearestGlossIndex( class CGlossSegment * g, DWORD dwStart, DWORD dwStop);
int FindNearestReferenceIndex( class CReferenceSegment * r, DWORD dwStart, DWORD dwStop);

#endif
