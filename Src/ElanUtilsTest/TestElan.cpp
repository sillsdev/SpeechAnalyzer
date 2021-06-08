#include <SDKDDKVer.h>
#include "CppUnitTest.h"
#include <conio.h>
#include <string>
#include "Test.h"
#include "ElanUtils.h"

using namespace Elan;

#pragma comment( lib, "XMLUtils")
#pragma comment( lib, "ElanUtils")

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

