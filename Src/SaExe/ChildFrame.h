#ifndef CHILDFRAME_H
#define CHILDFRAME_H

class CChildFrame : public CMDIChildWnd
{
    DECLARE_DYNCREATE(CChildFrame)
public:
    CChildFrame();
    virtual ~CChildFrame();

    virtual void ActivateFrame(int nCmdShow = -1) ;

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext & dc) const;
#endif

protected:

    DECLARE_MESSAGE_MAP()
};

#endif
