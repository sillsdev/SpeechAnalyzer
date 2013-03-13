#ifndef RECORDSET_H
#define RECORDSET_H

class CRecordset1 : public COleDispatchDriver
{
public:
	CRecordset1() {}		// Calls COleDispatchDriver default constructor
	CRecordset1(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	CRecordset1(const CRecordset1& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

public:

public:
};

#endif
