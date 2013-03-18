#ifndef ERROR_H

#include "dspTypes.h"
#include "Modules.h"
#include "ErrCodes.h"

#define ERROR_H



//#define Code(errorCode)  (((dspError_t)errorCode<<24)&0xFF000000 | ((dspError_t)MODULE<<16)&0x00FF0000 | ((dspError_t)__LINE__)&0x0000FFFF)
#define Code(errorCode)  (errorCode)

void  Warn(const int32 Message);
void  FlushLog(void);
int32  Warning(void);
void  GetMessage(char * Text, const int32 ErrCode);

#endif
