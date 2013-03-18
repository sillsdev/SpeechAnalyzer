/////////////////////////////////////////////////////////////////////////////
// undoredo.cpp:
// Implementation of the CUndoRedoDoc class.
// Author: Doug Eberman - mostly borrowed from Keith Rule, from his
//       article about Undo in Windows Tech Journal, May 1997.
// copyright 1997 JAARS Inc. SIL
//
// Revision History
// 1.06.1.2
//         SDM Fixed Memory Leak with Undo/Redo
// 1.06.8
//         SDM Added TRY/CATCH to checkpoint function, function fails with large
//             transcriptions.
/////////////////////////////////////////////////////////////////////////////



#include "stdafx.h"
#include "undoredo.h"

IMPLEMENT_DYNCREATE(CUndoRedoDoc, CDocument)

/***************************************************************************/
// CUndoRedoDoc::CUndoRedoDoc constructor
/***************************************************************************/
CUndoRedoDoc::CUndoRedoDoc(long undolevels, UINT growsize)
    :m_growsize(growsize),
     m_undoLevels(undolevels) {
}

/***************************************************************************/
// CUndoRedoDoc::~CUndoRedoDoc destructor
/***************************************************************************/
CUndoRedoDoc::~CUndoRedoDoc() {
    ClearList(&m_undolist);
    ClearList(&m_redolist);
}

/***************************************************************************/
// CUndoRedoDoc::AddRedo - add a redo entry.
/***************************************************************************/
void CUndoRedoDoc::AddRedo() {
    CMemFile * file = new CMemFile(m_growsize);
    ASSERT(file);
    Store(file);
    AddRedo(file);
}

/***************************************************************************/
// CUndoRedoDoc::Undo - undo a change to the document.
// If the flag bAddRedo is TRUE (default), the state is added to the redo
// list. If the flag bUndo is TRUE (default), the file is really loaded,
// otherwise, just the entry gets removed from the undo list.
/***************************************************************************/
void CUndoRedoDoc::Undo(BOOL bAddRedo, BOOL bUndo) {
    if (CanUndo()) {
        if (bAddRedo) {
            AddRedo();
        }
        CMemFile * file = (CMemFile *)m_undolist.GetHead();
        ASSERT(file);
        if (bUndo) {
            Load(file);
        }
        // SDM delete removed CMemFiles Here this is our last opportunity
        delete file;
        m_undolist.RemoveHead();
    }
}



/***************************************************************************/
// CUndoRedoDoc::Redo - undo an undo
/***************************************************************************/
void CUndoRedoDoc::Redo() {
    if (CanRedo()) {
        CMemFile * file = new CMemFile(m_growsize);
        ASSERT(file);
        Store(file);
        AddUndo(file);
        file = (CMemFile *)m_redolist.GetHead();
        ASSERT(file);
        Load(file);
        // SDM delete removed CMemFiles Here this is our last opportunity
        delete file;
        m_redolist.RemoveHead();
    }
}



/***************************************************************************/
// CUndoRedoDoc::CheckPoint - in order for Undo/Redo to work, the
// user of this class must periodicaly call CheckPoint() to save the
// state of the document at that point.  It is best to call CheckPoint just
// before an operation that changes the document.
/***************************************************************************/
void CUndoRedoDoc::CheckPoint() {
    CMemFile * file = NULL;
    // SDM 1.06.8 (exceptions thrown below may still cause memory leaks)
    try {
        file = new CMemFile(m_growsize);
        ASSERT(file);
        Store(file);
    } catch (...) {
        if (file) {
            delete file;
        }
        return;
    }

    AddUndo(file);
    ClearList(&m_redolist);
}

/***************************************************************************/
/*********************   INTERNAL METHODS **********************************/
/***************************************************************************/
void CUndoRedoDoc::ClearList(CObList * pList) {
    POSITION pos = pList->GetHeadPosition();
    CMemFile * nextFile = NULL;

    while (pos) {
        nextFile = (CMemFile *)pList->GetNext(pos);
        ASSERT(nextFile);
        if (nextFile) {
            delete nextFile;
        }
    }

    pList->RemoveAll();
}



/***************************************************************************/
// CUndoRedoDoc::AddUndo - takes the file which should contain a saved
// document state, and adds it to the undo stream.
/***************************************************************************/
void CUndoRedoDoc::AddUndo(CMemFile * file) {
    if (m_undolist.GetCount() > m_undoLevels) {
        CMemFile * pFile = (CMemFile *)m_undolist.RemoveTail();
        ASSERT(pFile);
        delete  pFile;
    }
    m_undolist.AddHead(file);
}



/***************************************************************************/
// CUndoRedoDoc::AddRedo - takes the file, which should contain a saved
// document state, and adds it to the redo stream.
/***************************************************************************/
void CUndoRedoDoc::AddRedo(CMemFile * file) {
    ASSERT(file);

    if (file) {
        m_redolist.AddHead(file);
    }
};



/***************************************************************************/
// CUndoRedoDoc::Store - stores the current state of the document in the file.
/***************************************************************************/
void CUndoRedoDoc::Store(CMemFile * file) {
    ASSERT(file);

    if (file) {
        file->SeekToBegin();
        CArchive ar(file, CArchive::store);
        SerializeForUndoRedo(ar);
        ar.Close();
    }
}


/***************************************************************************/
// CUndoRedoDoc::Load - restores the document to the state stored in the file
/***************************************************************************/
void CUndoRedoDoc::Load(CMemFile * file) {
    ASSERT(file);

    if (file) {
        file->SeekToBegin();
        CArchive ar(file, CArchive::load);
        SerializeForUndoRedo(ar);
        ar.Close();
    }
}

void CUndoRedoDoc::Serialize(CArchive & /*ar*/) {
}
