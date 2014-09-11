#include <SDKDDKVer.h>
#include <gtest/gtest.h>
#include <conio.h>
#include <string>
#include "Test.h"
#include "ElanUtils.h"

using namespace Elan;

#pragma comment( lib, "XMLUtils")
#pragma comment( lib, "ElanUtils")
#ifdef _DEBUG
#pragma comment( lib, "xerces-c_3D")
#else
#pragma comment( lib, "xerces-c_3")
#endif

TEST(Elan, ElanSimple_01) {
    CAnnotationDocument document = LoadDocument(buildSourcePath(L"elan-example3.eaf").c_str());
    ASSERT_TRUE(document.version.compare(L"2.3")==0);
    ASSERT_TRUE(document.timeOrder.timeSlots.size()==190);
};

