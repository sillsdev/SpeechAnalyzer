#include "stdafx.h"

#include "SplitFileUtils.h"
#include "SA_View.h"
#include "GlossSegment.h"
#include "ReferenceSegment.h"
#include "MusicPhraseSegment.h"
#include "sa.h"
#include "MainFrm.h"
#include "FileUtils.h"

/**
* Generate a file name for a split file
*
* Since the Ref+space+Gloss string will become the filename, protect the
* Windows Write function from crashing by testing for invalid characters
* in the string (i.e. \ / : * ? “ < > | );
* If you find one I suggest you replace it with a hyphen so the user knows
* something is not correct.
*
* For the Split File feature, after talking to one of the instructors, here
* is a revision to the filename process.
*
* The conditions for building a valid filename are:
* - Ref field can be alpha-numeric including <space>
* - Gloss field can be alpha-numeric including <space>
* - Ref field can be blank
* - Gloss field can be blank
* - Ref field and Gloss field cannot both be blank at the same time
* - Ref field cannot have duplicate data
* - Gloss field cannot have duplicate data unless Ref is different
* - Ref field and Gloss field cannot use invalid characters
*
* For now if an error condition exists, then put up a message that says
* something like “Invalid Filename - Failed to any save data”
* In the future the offending item can be displayed so the user can more
* easily fix the problem.
*
* Convention
* 0 - gloss only
* 1 - ref and gloss
*
* @return returns an empty string if an error has occurred
*/
wstring GenerateWordSplitName(CGlossSegment * g, CSaView * pView, EWordFilenameConvention convention, int index)
{

    wstring gloss(L"");
    wstring ref(L"");
    DWORD dwStart = 0;
    DWORD dwStop = 0;
    int rindex = -1;

    CReferenceSegment * r = (CReferenceSegment *)pView->GetAnnotation(REFERENCE);

    // generate the filename based on the dialog selection
    switch (convention)
    {

    case WFC_GLOSS:   //gloss
        gloss = g->GetSegmentString(index);
        if ((gloss.length()>0) && (gloss[0]=='#'))
        {
            gloss = gloss.substr(1);
        }
        gloss = FilterName(gloss);
        // return empty or not
        return gloss;

    case WFC_REF:   //ref
        dwStart = g->GetOffset(index);
        dwStop = dwStart + g->GetDuration(index);
        // find the ref based on the gloss position, since GLOSS is the iterator
        rindex = FindNearestReferenceIndex(r,dwStart,dwStop);
        if (rindex!=-1)
        {
            ref = pView->GetAnnotation(REFERENCE)->GetSegmentString(rindex);
        }
        ref = FilterName(ref);
        // return empty or not
        return ref;

    default:
    case WFC_REF_GLOSS:     //ref+gloss
        dwStart = g->GetOffset(index);
        dwStop = dwStart + g->GetDuration(index);
        gloss = g->GetSegmentString(index);
        if ((gloss.length()>0) && (gloss[0]=='#'))
        {
            gloss = gloss.substr(1);
        }
        // find the ref based on the gloss position, since GLOSS is the iterator
        rindex = FindNearestReferenceIndex(r,dwStart,dwStop);
        if (rindex!=-1)
        {
            ref = pView->GetAnnotation(REFERENCE)->GetSegmentString(rindex);
        }
        gloss = FilterName(gloss);
        ref = FilterName(ref);
        if (ref.length()==0)
        {
            // if ref is empty, return an empty string to cause an
            // error, regardless of gloss
            return ref;
        }

        // we know we have ref at this point
        if (gloss.length()!=0)
        {
            ref = ref.append(L" ").append(gloss);
            return ref;
        }
        else
        {
            return ref;
        }
    }
}

/**
* Generate a file name for a split file
*
* Since the Ref+space+Gloss string will become the filename, protect the
* Windows Write function from crashing by testing for invalid characters
* in the string (i.e. \ / : * ? “ < > | );
* If you find one I suggest you replace it with a hyphen so the user knows
* something is not correct.
*
* For the Split File feature, after talking to one of the instructors, here
* is a revision to the filename process.
*
* The conditions for building a valid filename are:
* - Ref field can be alpha-numeric including <space>
* - Gloss field can be alpha-numeric including <space>
* - Ref field can be blank
* - Gloss field can be blank
* - Ref field and Gloss field cannot both be blank at the same time
* - Ref field cannot have duplicate data
* - Gloss field cannot have duplicate data unless Ref is different
* - Ref field and Gloss field cannot use invalid characters
*
* For now if an error condition exists, then put up a message that says
* something like “Invalid Filename - Failed to any save data”
* In the future the offending item can be displayed so the user can more
* easily fix the problem.
*
* Convention
* 0 - gloss only
* 1 - ref and gloss
*
* @return returns an empty string if an error has occurred
*/
bool GeneratePhraseSplitName(EAnnotation type, CMusicPhraseSegment * seg, CSaView * pView, EPhraseFilenameConvention convention, int index, wstring & result)
{

    wstring gloss(L"");
    wstring ref(L"");
    wstring phrase(L"");
    int gindex = -1;
    int rindex = -1;

    DWORD dwStart = seg->GetOffset(index);
    DWORD dwStop = dwStart + seg->GetDuration(index);

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CGlossSegment * g = (CGlossSegment *)pView->GetAnnotation(GLOSS);
    CReferenceSegment * r = (CReferenceSegment *)pView->GetAnnotation(REFERENCE);

    CString szNumber;
    szNumber.Format(L"%d",index);

    CString szPhrase;
    szPhrase.LoadStringW(((type==MUSIC_PL1)?IDS_WINDOW_MUSIC_PL1:IDS_WINDOW_MUSIC_PL2));

    CString szTag;
    szTag = ((type==MUSIC_PL1)?L"PL1":L"PL2");

    // generate the filename based on the dialog selection
    switch (convention)
    {

    case PFC_REF:   //ref
        // find the ref based on the gloss position, since GLOSS is the iterator
        rindex = FindNearestReferenceIndex(r, dwStart, dwStop);
        if (rindex!=-1)
        {
            ref = r->GetSegmentString(rindex);
        }
        result = FilterName(ref);
        if (result.length()==0)
        {
            pApp->ErrorMessage(IDS_SPLIT_NO_REF,(LPCTSTR)szPhrase,(LPCTSTR)szNumber);
            return false;
        }
        result.append(L" ").append(szTag);
        return true;

    case PFC_GLOSS:   //gloss
        gindex = FindNearestGlossIndex(g, dwStart, dwStop);
        if (gindex != -1)
        {
            gloss = g->GetSegmentString(gindex);
            if ((gloss.length()>0) && (gloss[0]=='#'))
            {
                gloss = gloss.substr(1);
            }
        }
        result = FilterName(gloss);
        if (result.length()==0)
        {
            pApp->ErrorMessage(IDS_SPLIT_NO_GLOSS,(LPCTSTR)szPhrase,(LPCTSTR)szNumber);
            return false;
        }
        result.append(L" ").append(szTag);
        return true;

    case PFC_REF_GLOSS:     //ref+gloss
        gindex = FindNearestGlossIndex(g, dwStart, dwStop);
        if (gindex!=-1)
        {
            gloss = g->GetSegmentString(gindex);
            if ((gloss.length()>0) && (gloss[0]=='#'))
            {
                gloss = gloss.substr(1);
            }
        }
        gloss = FilterName(gloss);
        // find the ref based on the gloss position, since GLOSS is the iterator
        rindex = FindNearestReferenceIndex(r,dwStart,dwStop);
        if (rindex!=-1)
        {
            ref = pView->GetAnnotation(REFERENCE)->GetSegmentString(rindex);
        }
        ref = FilterName(ref);
        if ((ref.length()>0) && (gloss.length()>0))
        {
            result = ref.append(L" ").append(gloss).append(L" ").append(szTag);
            return true;
        }
        else if ((ref.length()>0)&&(gloss.length()==0))
        {
            result = ref;
            result.append(L" ").append(szTag);
            return true;
        }
        else if ((ref.length()==0)&&(gloss.length()>0))
        {
            result = gloss;
            result.append(L" ").append(szTag);
            return true;
        }

        // both are empty, complain about reference
        pApp->ErrorMessage(IDS_SPLIT_NO_REF,(LPCTSTR)szPhrase,(LPCTSTR)szNumber);
        return false;

    default:
    case PFC_PHRASE:
        phrase = seg->GetSegmentString(index);
        result = FilterName(phrase);
        if (result.length()>0)
        {
            result.append(L" ").append(szTag);
            return true;
        }
        pApp->ErrorMessage(IDS_SPLIT_NO_PHRASE,(LPCTSTR)szPhrase,(LPCTSTR)szNumber);
        return false;
    }
}

/**
* Given a start/stop range, find the nearest reference segment.
* This will be the one that overlaps, or starts with
* the same start, or the next one before the end of the
* reference segment
*
* returns -1 on error
*/
int FindNearestReferenceIndex(CReferenceSegment * seg, DWORD dwStart, DWORD dwStop)
{

    TRACE("trying to find ref segment for %d %d\n",dwStart,dwStop);
    for (int j=0; j<seg->GetOffsetSize(); j++)
    {
        DWORD dwRStart = seg->GetOffset(j);
        DWORD dwRStop = dwRStart+seg->GetDuration(j);
        TRACE("comparing start=%d end=%d\n",dwRStart,dwRStop);
        if (dwStart==dwRStart)
        {
            TRACE("found identical start\n");
            return j;
        }
        else if ((dwRStart<dwStart)&&(dwStart<=dwRStop))
        {
            TRACE("ref overlaps segment\n");
            return j;
        }
        else if ((dwStart<dwRStart)&&(dwRStop<=dwStop))
        {
            TRACE("ref within segment\n");
            return j;
        }
    }
    TRACE("no match found!\n");
    return -1;
}

/**
* Given a start/stop range, find the nearest gloss segment.
* This will be the one that overlaps, or starts with
* the same offset, or the next one before the end of the
* gloss segment
*
* returns -1 on error
*/
int FindNearestGlossIndex(CGlossSegment * seg, DWORD dwStart, DWORD dwStop)
{

    TRACE("trying to find gloss segment for %d %d\n",dwStart,dwStop);
    for (int j=0; j<seg->GetOffsetSize(); j++)
    {
        DWORD dwGStart = seg->GetOffset(j);
        DWORD dwGStop = dwGStart+seg->GetDuration(j);
        TRACE("comparing start=%d end=%d\n",dwGStart,dwGStop);
        if (dwStart==dwGStart)
        {
            TRACE("found identical start\n");
            return j;
        }
        else if ((dwGStart<dwStart)&&(dwStart<=dwGStop))
        {
            TRACE("gloss overlaps segment\n");
            return j;
        }
        else if ((dwStart<dwGStart)&&(dwGStop<=dwStop))
        {
            TRACE("gloss within segment\n");
            return j;
        }
    }
    TRACE("no match found!\n");
    return -1;
}

/**
* Given a start/stop range, find the nearest phrase segment.
* priority:
* segment starts within range
* segment contained in range
* segment overlaps range
* segment ends in range
*
* returns -1 on error
*/
int FindNearestPhraseIndex(CMusicPhraseSegment * seg, DWORD dwStart, DWORD dwStop)
{

    TRACE("trying to find phrase segment for %d %d\n",dwStart,dwStop);
    for (int j=0; j<seg->GetOffsetSize(); j++)
    {
        DWORD dwGStart = seg->GetOffset(j);
        DWORD dwGStop = dwGStart+seg->GetDuration(j);
        TRACE("comparing start=%d end=%d\n",dwGStart,dwGStop);
        if ((dwStart<=dwGStart)&&(dwGStart<=dwStop))
        {
            TRACE("phrase starts in segment\n");
            return j;
        }
    }
    for (int j=0; j<seg->GetOffsetSize(); j++)
    {
        DWORD dwGStart = seg->GetOffset(j);
        DWORD dwGStop = dwGStart+seg->GetDuration(j);
        TRACE("comparing start=%d end=%d\n",dwGStart,dwGStop);
        if ((dwStart<=dwGStart)&&(dwGStop<=dwStop))
        {
            TRACE("phrase within segment\n");
            return j;
        }
    }
    for (int j=0; j<seg->GetOffsetSize(); j++)
    {
        DWORD dwGStart = seg->GetOffset(j);
        DWORD dwGStop = dwGStart+seg->GetDuration(j);
        TRACE("comparing start=%d end=%d\n",dwGStart,dwGStop);
        if ((dwGStart<dwStart)&&(dwStop<dwGStop))
        {
            TRACE("phrase overlaps segment\n");
            return j;
        }
    }
    for (int j=0; j<seg->GetOffsetSize(); j++)
    {
        DWORD dwGStart = seg->GetOffset(j);
        DWORD dwGStop = dwGStart+seg->GetDuration(j);
        TRACE("comparing start=%d end=%d\n",dwGStart,dwGStop);
        if ((dwStart<dwGStop)&&(dwGStop<=dwStop))
        {
            TRACE("phrase ends in segment\n");
            return j;
        }
    }
    return -1;
}

/**
* walk through the gloss and reference transcriptions to see if everything
* is in order
*/
bool ValidateWordFilenames(EWordFilenameConvention convention, BOOL skipEmptyGloss)
{

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *) AfxGetMainWnd())->GetCurrSaView()->GetDocument();
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    CString szGloss;
    szGloss.LoadStringW(IDS_WINDOW_GLOSS);

    CGlossSegment * g = (CGlossSegment *)pView->GetAnnotation(GLOSS);

    // loop through the gloss segments
    int nLoop = g->GetOffsetSize();
    for (int i=0; i<nLoop; i++)
    {
        CString szNumber;
        szNumber.Format(_T("%d"), i);

        // check for empty gloss strings
        wstring gloss = g->GetSegmentString(i);
        if (gloss.length()==0)
        {
            if (skipEmptyGloss)
            {
                continue;
            }
        }
        else if ((gloss.length()==1)&&(gloss[0]=='#'))
        {
            if (skipEmptyGloss)
            {
                continue;
            }
        }

        /**
        * if we are ignoring blank gloss fields, then we have already
        * bypassed this section.
        * if not, then we have already errored on an empty gloss
        * so, for this following check, it can only fail if the reference is empty
        */
        // can we piece the name together?
        wstring splitname = GenerateWordSplitName(g, pView, convention, i);
        if (splitname.length()==0)
        {
            pApp->ErrorMessage(IDS_SPLIT_NO_REF,(LPCTSTR)szGloss,(LPCTSTR)szNumber);
            return false;
        }
    }
    return true;
}

/**
* walk through the gloss and reference transcriptions to see if everything
* is in order
*/
bool ValidatePhraseFilenames(EAnnotation type, EPhraseFilenameConvention convention)
{

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *) AfxGetMainWnd())->GetCurrSaView()->GetDocument();
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    CMusicPhraseSegment * s = (CMusicPhraseSegment *)pView->GetAnnotation(type);

    // depending on the convention
    // loop through the annotation segments
    int lastOffset = -1;
    int nLoop = s->GetOffsetSize();
    for (int i=0; i<nLoop; i++)
    {
        int offset = s->GetOffset(i);
        if (offset==lastOffset)
        {
            continue;
        }

        // we are on a new segment
        lastOffset = offset;

        // we are at the start of a segment
        wstring splitname;
        if (!GeneratePhraseSplitName(type, s, pView, convention, i, splitname))
        {
            // function will generate error message
            return false;
        }
    }
    return true;
}

/**
* export words for the given file
*/
bool ExportWordSegments(EWordFilenameConvention convention, LPCTSTR path, BOOL skipEmptyGloss, int & dataCount, int & wavCount)
{

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *) AfxGetMainWnd())->GetCurrSaView()->GetDocument();
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    // loop through the gloss segments
    CGlossSegment * g = (CGlossSegment *)pView->GetAnnotation(GLOSS);
    int nLoop = g->GetOffsetSize();
    for (int i=0; i<nLoop; i++)
    {
        wstring dest;
        int result = ComposeWordSegmentFilename(g, i, convention, path, dest);
        if (result==1)
        {
            break;
        }
        result = ExportWordSegment(g, i, dest.c_str(), skipEmptyGloss, dataCount, wavCount);
        if (result<0)
        {
            return false;
        }
    }
    return true;
}

/**
* export a single word segment file
* return 0 on no errors
* return 1 on no more data
*/
int ComposeWordSegmentFilename(CGlossSegment * seg, int index, EWordFilenameConvention convention, LPCTSTR path, wstring & out)
{

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *) AfxGetMainWnd())->GetCurrSaView()->GetDocument();
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    // can we piece the name together?
    wstring name = GenerateWordSplitName(seg, pView, convention, index);
    if (name.length()==0)
    {
        return 1;
    }

    out = L"";
    out.append(path);
    AppendDirSep(out);
    out.append(name);
    out.append(L".wav");

    return 0;
}

/**
* export a single word segment file
* returns -1 on error
* return 0 on continue
*/
int ExportWordSegment(CGlossSegment * seg, int index, LPCTSTR filename, BOOL skipEmptyGloss, int & dataCount, int & wavCount)
{

    TRACE("exporting gloss segment\n");

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *) AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    BOOL bSuccess = FALSE;
    DWORD dwStart = seg->GetOffset(index);
    DWORD dwStop = dwStart + seg->GetDuration(index);
    TRACE("dwStart=%d dwStop=%d\n",dwStart,dwStop);

    wstring gloss = seg->GetSegmentString(index);
    if (gloss.length()==0)
    {
        if (skipEmptyGloss)
        {
            return 0;
        }
    }
    else if ((gloss.length()==1)&&(gloss[0]=='#'))
    {
        if (skipEmptyGloss)
        {
            return 0;
        }
    }

    // copy the audio portion
    bSuccess = pDoc->CopySectionToNewWavFile(dwStart,dwStop-dwStart, filename, FALSE);
    if (!bSuccess)
    {
        // be sure to delete the file
        RemoveFile(filename);
        CSaApp * pApp = (CSaApp *)AfxGetApp();
        pApp->ErrorMessage(IDS_SPLIT_NO_WRITE,filename);
        return -1;
    }

    dataCount++;
    wavCount++;
    return 0;
}

/**
* export words for the given file
*/
bool ExportPhraseSegments(EAnnotation type, EPhraseFilenameConvention convention, wstring & path, int & dataCount, int & wavCount)
{

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *) AfxGetMainWnd())->GetCurrSaView()->GetDocument();
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    if (!ValidatePhraseFilenames(type, convention))
    {
        return false;
    }

    // loop through the PL1 segments
    CMusicPhraseSegment * seg = (CMusicPhraseSegment *)pView->GetAnnotation(type);
    int nLoop = seg->GetOffsetSize();
    for (int i=0; i<nLoop; i++)
    {
        wstring filename;
        int result = ComposePhraseSegmentFilename(type, seg, i, convention, path.c_str(), filename);
        if (result==1)
        {
            break;
        }
        result = ExportPhraseSegment(seg, i, filename, dataCount, wavCount);
        if (result<0)
        {
            return false;
        }
    }
    return true;
}

/**
* export a single phrase segment file
* return 0 on no errors
* return 1 on no more data
*/
int ComposePhraseSegmentFilename(EAnnotation type, CMusicPhraseSegment * seg, int index, EPhraseFilenameConvention convention, LPCTSTR path, wstring & out)
{

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *) AfxGetMainWnd())->GetCurrSaView()->GetDocument();
    POSITION pos = pDoc->GetFirstViewPosition();
    CSaView * pView = (CSaView *)pDoc->GetNextView(pos);

    // can we piece the name together?
    wstring name;
    if (!GeneratePhraseSplitName(type, seg, pView, convention, index, name))
    {
        return 1;
    }

    // copy the audio portion
    out = L"";
    out.append(path);
    AppendDirSep(out);
    out.append(name);
    out.append(L".wav");

    return 0;
}

/**
* export words for the given file
*/
int ExportPhraseSegment(CMusicPhraseSegment * seg, int index, wstring & filename, int & dataCount, int & wavCount)
{

    TRACE("exporting phrase segment\n");

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *) AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    DWORD dwStart = seg->GetOffset(index);
    DWORD dwStop = dwStart + seg->GetDuration(index);
    TRACE("dwStart=%d dwStop=%d\n",dwStart,dwStop);

    BOOL bSuccess = pDoc->CopySectionToNewWavFile(dwStart,dwStop-dwStart,filename.c_str(),FALSE);
    if (!bSuccess)
    {
        // be sure to delete the file
        RemoveFile(filename.c_str());
        CSaApp * pApp = (CSaApp *)AfxGetApp();
        pApp->ErrorMessage(IDS_SPLIT_NO_WRITE,filename.c_str());
        return -1;
    }

    dataCount++;
    wavCount++;
    return 0;
}

/**
* filters a segment name so that it is usable in a filename
*/
wstring FilterName(wstring text)
{
    wstring result;

    for (size_t i=0; i<text.length(); i++)
    {
        wchar_t c = text[i];
        if (c==0)
        {
            break;
        }
        switch (c)
        {
        case '\\':
        case '/':
        case ':':
        case '*':
        case '?':
        case '“':
        case '<':
        case '>':
        case '|':
        case '^':
            c = '-';
            //ignore
            break;
        default:
            break;
        }
        result = result + c;
    }

    if (result.length()>0)
    {
        if (result[0]=='#')
        {
            result = result.substr(1);
        }
    }

    CString temp = result.c_str();
    result = temp.Trim();
    return result;
}

