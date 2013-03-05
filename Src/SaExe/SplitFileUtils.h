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
wstring FilterName( wstring text);

wstring GenerateWordSplitName( CGlossSegment * g, CSaView* pView, EWordFilenameConvention convention, int index);
bool ValidateWordFilenames( EWordFilenameConvention convention, BOOL skipEmptyGloss);
int ComposeWordSegmentFilename( CGlossSegment * seg, int index, EWordFilenameConvention convention, LPCTSTR path, wstring & out);
bool ExportWordSegments( int & count, EWordFilenameConvention convention, LPCTSTR path, BOOL skipEmptyGloss);
int ExportWordSegment( int & count, CGlossSegment * seg, int index, LPCTSTR filename, BOOL skipEmptyGloss);

bool GeneratePhraseSplitName( Annotations type, CMusicPhraseSegment * s, CSaView* pView, EPhraseFilenameConvention convention, int index, wstring & result);
bool ValidatePhraseFilenames( Annotations type, EPhraseFilenameConvention convention);
int ComposePhraseSegmentFilename( Annotations type, CMusicPhraseSegment * seg, int index, EPhraseFilenameConvention convention, LPCTSTR path, wstring & out);
bool ExportPhraseSegments( Annotations type, int & count, EPhraseFilenameConvention convention, wstring & path);
int ExportPhraseSegment( int & count, CMusicPhraseSegment * seg, int index, wstring & filename);

int FindNearestPhraseIndex( CMusicPhraseSegment * seg, DWORD dwStart, DWORD dwStop);
int FindNearestGlossIndex( CGlossSegment * seg, DWORD dwStart, DWORD dwStop);
int FindNearestReferenceIndex( CReferenceSegment * seg, DWORD dwStart, DWORD dwStop);

#endif
