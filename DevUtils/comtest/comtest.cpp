// comtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#import <mscorlib.tlb> raw_interfaces_only
#import "dbutils.tlb" no_namespace named_guids


int _tmain(int argc, _TCHAR* argv[])
{
   ISaWaveDocumentReader *cpr = NULL;
   ISaWaveDocumentWriter *cpw = NULL;

   int retval = 1;

   // Initialize COM and create an instance of the InterfaceImplementation class:
   CoInitialize(NULL);

   HRESULT hr = CoCreateInstance(CLSID_SaWaveDocumentReader,
               NULL, CLSCTX_INPROC_SERVER,
               DIID_ISaWaveDocumentReader, reinterpret_cast<void**>(&cpr));

	if (FAILED(hr))
	{
		printf("Couldn't create the instance!... 0x%x\n", hr);
	}
	else
	{
		VARIANT_BOOL ret = cpr->Initialize("C:\\junk\\comtest\\wave1.wav");
		
		hr = CoCreateInstance(CLSID_SaWaveDocumentWriter,
               NULL, CLSCTX_INPROC_SERVER,
               DIID_ISaWaveDocumentWriter, reinterpret_cast<void**>(&cpw));

		if (FAILED(hr))
		{
			printf("Couldn't create the instance!... 0x%x\n", hr);
		}

		_bstr_t md5 = cpr->GetMD5HashCode();
		int dcs = cpr->DataChunkSize;
		
		_bstr_t _name = cpr->GetSpeakerName();
		wchar_t *name = _name;
		
		_bstr_t _dialect = cpr->GetDialect();
		wchar_t *dialect = _dialect;
		
		_bstr_t _country = cpr->GetCountry();
		wchar_t *country = _country;


		wprintf(_T("Name: %s, Dialect: %s, Country: %s\n\n"), name, dialect, country);

		long offset = NULL;
		long length = NULL;
		BSTR *annotation = new BSTR();
		
		while (cpr->ReadSegment(0, &offset, &length, annotation))
			wprintf(_T("Offset: %d, Length: %d, Annotation: %s\n"), offset, length, *annotation);

		
		ret = cpw->Initialize("C:\\junk\\comtest\\wave1.wav", md5);
		cpw->PutSpeakerName("David Olson");
		cpw->PutCountry("USA");
		cpw->PutDialect("English");
		cpw->DataChunkSize = 1234567;
		cpw->AddSegment(0, 555, 111, "sadie");
		cpw->AddSegment(0, 777, 222, "cameron");
		cpw->AddSegment(0, 888, 333, "dawson");
		cpw->AddSegment(0, 999, 444, "bear");
		cpw->Commit();

		cpw->Close();	
		cpw->Release();
		cpw = NULL;

		cpr->Close();
		cpr->Release();
		cpr = NULL;
	}

	CoUninitialize();
	return 0;
}

