#include <SDKDDKVer.h>
#include "CppUnitTest.h"
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

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace LiftUtils_Test {
	TEST_CLASS(LiftTest) {
public:

	TEST_METHOD( LiftBadFile) {
		try {
			lift document = load(buildSourcePath(L"neverfindit.lift").c_str());
			Assert::IsTrue(false);
		} catch (exception & e) {
		}
	}

	TEST_METHOD( LiftCreateDate) {
		wstring result = Lift::createDate();
		wprintf(L"date=%s\n", result.c_str());
	}

	TEST_METHOD( LiftSimple_01) {
		//lift document = load( buildSourcePath(L"test_samples.lift").c_str());
	};

	TEST_METHOD( LiftSimple_02) {
		//lift_ranges document = load_ranges( buildSourcePath(L"test_samples.lift-ranges").c_str());
	};

	TEST_METHOD( LiftSimple_03) {
		//lift document = load(buildSourcePath(L"test_samples.lift").c_str());
		//store(document, buildResultPath(L"test_samples.lift").c_str());
	};

	TEST_METHOD( LiftSimple_04) {
		lift document = load(buildSourcePath(L"hand_crafted.lift").c_str());
	};

	TEST_METHOD( LiftUUID_01) {
		wstring uuid = Lift::createUUID();
		Assert::IsTrue(uuid.length() > 0);
	}
	};
}
