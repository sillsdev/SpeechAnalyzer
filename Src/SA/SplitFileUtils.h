#ifndef SPLITFILEUTILS_H
#define SPLITFILEUTILS_H

#include "AppDefs.h"

using std::wstring;

class CSaView;
class CGlossSegment;
class CMusicPhraseSegment;
class CReferenceSegment;

// split file feature methods
wstring FilterName(LPCTSTR text);

wstring GenerateWordSplitName(CGlossSegment * g, CSaView * pView, EWordFilenameConvention convention, int index, LPCTSTR prefix, LPCTSTR suffix);
bool ValidateWordFilenames(EWordFilenameConvention convention, BOOL skipEmptyGloss, LPCTSTR prefix, LPCTSTR suffix);
int ComposeWordSegmentFilename(CGlossSegment * seg, int index, EWordFilenameConvention convention, LPCTSTR path, wstring & out, LPCTSTR prefix, LPCTSTR suffix);
bool ExportWordSegments(EWordFilenameConvention convention, LPCTSTR path, BOOL skipEmptyGloss, int & dataCount, int & wavCount, LPCTSTR prefix, LPCTSTR suffix);
int ExportWordSegment(CGlossSegment * seg, int index, LPCTSTR filename, BOOL skipEmptyGloss, int & dataCount, int & wavCount);

bool GeneratePhraseSplitName(EAnnotation type, CMusicPhraseSegment * s, CSaView * pView, EPhraseFilenameConvention convention, int index, wstring & result, LPCTSTR prefix, LPCTSTR suffix);
bool ValidatePhraseFilenames(EAnnotation type, EPhraseFilenameConvention convention, LPCTSTR prefix, LPCTSTR suffix);
int ComposePhraseSegmentFilename(EAnnotation type, CMusicPhraseSegment * seg, int index, EPhraseFilenameConvention convention, LPCTSTR path, wstring & out, LPCTSTR prefix, LPCTSTR suffix);
bool ExportPhraseSegments(EAnnotation type, EPhraseFilenameConvention convention, wstring & path, int & dataCount, int & wavCount, LPCTSTR prefix, LPCTSTR suffix);
int ExportPhraseSegment(CMusicPhraseSegment * seg, int index, wstring & filename, int & dataCount, int & wavCount);

int FindNearestPhraseIndex(CMusicPhraseSegment * seg, DWORD dwStart, DWORD dwStop);
int FindNearestGlossIndex(CGlossSegment * seg, DWORD dwStart, DWORD dwStop);
int FindNearestReferenceIndex(CReferenceSegment * seg, DWORD dwStart, DWORD dwStop);

#endif
