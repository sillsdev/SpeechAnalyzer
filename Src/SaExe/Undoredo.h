/////////////////////////////////////////////////////////////////////////////
// undoredo.h
// Interface of the CUndoRedoDoc class
// Author: Doug Eberman - mostly borrowed from Keith Rule, from his
//       article about Undo in Windows Tech Journal, May 1997.
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _UNREDO_H
#define _UNREDO_H


// Undo/Redo for MFC by Keith Rule

class CUndoRedoDoc : public CDocument
{
    DECLARE_DYNCREATE(CUndoRedoDoc)
public:
    CUndoRedoDoc(long undolevels = 100, UINT growsize = 32768);
    ~CUndoRedoDoc();

    // properties
    BOOL CanUndo() const
    {
        return (m_undolist.GetCount() > 0);
    };
    BOOL CanRedo() const
    {
        return (m_redolist.GetCount() > 0);
    };

    // key public methods
    void Undo(BOOL bAddRedo = TRUE, BOOL bUndo = TRUE);
    void Redo();
    void CheckPoint();

    // the user of this class should inherit from it and
    // define this method in their class.  It should act
    // like Serialize() (or call it if Serialize is defined)
    virtual void SerializeForUndoRedo(CArchive & /*ar*/)
    {
        ASSERT(FALSE);
    };
    virtual void Serialize(CArchive & ar);  // overridden for document i/o

private:  // internal methods
    void AddUndo(CMemFile *);
    void AddRedo();
    void AddRedo(CMemFile * pFile);
    void Load(CMemFile *);
    void Store(CMemFile *);
    static void ClearList(CObList * pList);

private:  // internal data
    CObList m_undolist;
    CObList m_redolist;
    UINT    m_growsize;
    long    m_undoLevels;

    //public:    // test methods
    //    void TestUndo();
};


#endif //_UNREDO_H
