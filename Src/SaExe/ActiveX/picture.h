#ifndef PICTURE_H
#define PICTURE_H

class CPicture : public COleDispatchDriver
{
public:
	CPicture() {}		// Calls COleDispatchDriver default constructor
	CPicture(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CPicture(const CPicture& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

public:
	long GetHandle();
	long GetHPal();
	void SetHPal(long);
	short GetType();
	long GetWidth();
	long GetHeight();
};

#endif
