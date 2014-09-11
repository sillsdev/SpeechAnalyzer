#include <SDKDDKVer.h>
#include <gtest/gtest.h>
#include <conio.h>
#include <string>
#include "Test.h"
#include "LiftUtils.h"

using namespace Lift;
using namespace Lift13;

#pragma comment( lib, "XMLUtils")
#pragma comment( lib, "LiftUtils")
#pragma comment( lib, "FileUtils")
#ifdef _DEBUG
#pragma comment( lib, "xerces-c_3D")
#else
#pragma comment( lib, "xerces-c_3")
#endif

TEST(Lift, LiftBadFile) {
	try {
		lift document = load(buildSourcePath(L"neverfindit.lift").c_str());
		ASSERT_TRUE(false);
	} catch (exception & e) {
	}
}

TEST(Lift, LiftCreateDate) {
	wstring result = Lift::createDate();
	wprintf(L"date=%s\n",result.c_str());
}

TEST(Lift, LiftSimple_01) {
    //lift document = load( buildSourcePath(L"test_samples.lift").c_str());
};

TEST(Lift, LiftSimple_02) {
    //lift_ranges document = load_ranges( buildSourcePath(L"test_samples.lift-ranges").c_str());
};

TEST(Lift, LiftSimple_03) {
    //lift document = load(buildSourcePath(L"test_samples.lift").c_str());
    //store(document, buildResultPath(L"test_samples.lift").c_str());
};

TEST(Lift, LiftSimple_04) {
    lift document = load(buildSourcePath(L"hand_crafted.lift").c_str());
};

TEST(Lift, LiftUUID_01) {
	wstring uuid = Lift::createUUID();
	ASSERT_TRUE(uuid.length()>0);
};
