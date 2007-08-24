/////////////////////////////////////////////////////////////////////////////
// sa_p_lou.h:
// Interface of the CProcessChange class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_CHA_H
#define _SA_P_CHA_H

//###########################################################################
// CProcessChange data processing

class CProcessChange : public CDataProcess
{
// Construction/destruction/creation
public:
  CProcessChange();
  virtual ~CProcessChange();

// Attributes
private:
    CFile*  m_pQualityFile;     // quality data file object
    CFileStatus m_qualityFileStatus; // quality data file status  
    HANDLE  m_hQualityData;       // needed to get m_lpQualityData
    HPSTR   m_lpQualityData;      // pointer to processed quality data
    DWORD  m_dwQualityBufferOffset; // actual quality data buffer offset
    BOOL  m_bQualityBlockBegin; // actual quality block begin flag
    int    m_nQualityMaxValue;   // maximum value of processed quality data


// Operations
public:
  void SetDataInvalid();
  long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);
  void* GetQualityData(DWORD dwOffset, BOOL bBlockBegin = FALSE); // return quality data pointer on given position (offset)
  int GetQualityData(DWORD dwOffset, BOOL*); // return quality data from given position (offset)  
  DWORD GetQualityDataSize() {return m_qualityFileStatus.m_size / 2;} // return processed data size in words (16 bit)
  int GetQualityMaxValue() {return m_nQualityMaxValue;} // return maximum value
private:
  UINT CalculateACF8(CDataProcess*, unsigned char * pSRDdata, BOOL*, UINT nACFinterval, UINT nLagData,
             UINT nMaxLag, DWORD* pdwBlockPos, DWORD dwDataPos, int nLoopCount); // calculate auto correlation coefficient for 8bit data
  DWORD CalculateACF16(CDataProcess*, unsigned char * pSRDdata, BOOL*, UINT nACFinterval, UINT nLagData,
               UINT nMaxLag, DWORD* pdwBlockPos, DWORD dwDataPos, int nLoopCount); // calculate auto correlation coefficient for 16bit data
protected:
  virtual long Exit(int nError); // exit processing on error

};

#endif //_SA_P_CHA_H
