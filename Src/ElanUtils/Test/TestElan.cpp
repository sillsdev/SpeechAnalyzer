#include <SDKDDKVer.h>
#include "CppUnitTest.h"
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

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace ElanUtils_Test {
	TEST_CLASS(ElanTest) {
public:

	TEST_METHOD( ElanSimple_01) {
		CAnnotationDocument document = LoadDocument(buildSourcePath(L"elan-example3.eaf").c_str());
		Assert::IsTrue(document.version.compare(L"2.3") == 0);
		Assert::IsTrue(document.timeOrder.timeSlots.size() == 190);
	}
	};
}

