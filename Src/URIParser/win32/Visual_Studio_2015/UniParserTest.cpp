#include <SDKDDKVer.h>
#include "CppUnitTest.h"
#include <conio.h>
#include <string>
#include <uriparser/Uri.h>
#include <memory>
#include <stdio.h>
#include <wchar.h>

using namespace std;

#pragma comment( lib, "uriparser")

extern "C" {
	UriBool uri_TESTING_ONLY_ParseIpSixA(const char * text);
	UriBool uri_TESTING_ONLY_ParseIpFourA(const char * text);
	int uriCompareRangeA(const UriTextRangeA * a, const UriTextRangeA * b);
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UniParserTest {

	TEST_CLASS(UniParserTest) {

		bool testDistinctionHelper(const char * uriText, bool expectedHostSet,
								   bool expectedAbsPath, bool expectedEmptyTailSegment) {
			UriParserStateA state;
			UriUriA uri;
			state.uri = &uri;

			int res = uriParseUriA(&state, uriText);
			if (res != URI_SUCCESS) {
				uriFreeUriMembersA(&uri);
				return false;
			}

			if (expectedHostSet != (uri.hostText.first != NULL)) {
				uriFreeUriMembersA(&uri);
				return false;
			}

			if (expectedAbsPath != (uri.absolutePath == URI_TRUE)) {
				uriFreeUriMembersA(&uri);
				return false;
			}

			if (expectedEmptyTailSegment != ((uri.pathTail != NULL)
				&& (uri.pathTail->text.first == uri.pathTail->text.afterLast))) {
				uriFreeUriMembersA(&uri);
				return false;
			}

			uriFreeUriMembersA(&uri);
			return true;
		}

public:

	TEST_METHOD(testDistinction) {
		/*
		============================================================================
		Rule                                | Example | hostSet | absPath | emptySeg
		------------------------------------|---------|---------|---------|---------
		1) URI = scheme ":" hier-part ...   |         |         |         |
		1) "//" authority path-abempty   | "s://"  | true    |   false |   false
		| "s:///" | true    |   false | true
		| "s://a" | true    |   false | false
		| "s://a/"| true    |   false | true
		2) path-absolute                 | "s:/"   |   false | true    |   false
		3) path-rootless                 | "s:a"   |   false |   false |   false
		| "s:a/"  |   false |   false | true
		4) path-empty                    | "s:"    |   false |   false |   false
		------------------------------------|---------|---------|---------|---------
		2) relative-ref = relative-part ... |         |         |         |
		1) "//" authority path-abempty   | "//"    | true    |   false |   false
		| "///"   | true    |   false | true
		2) path-absolute                 | "/"     |   false | true    |   false
		3) path-noscheme                 | "a"     |   false |   false |   false
		| "a/"    |   false |   false | true
		4) path-empty                    | ""      |   false |   false |   false
		============================================================================
		*/
		Assert::IsTrue(testDistinctionHelper("s://", true, false, false));
		Assert::IsTrue(testDistinctionHelper("s:///", true, false, true));
		Assert::IsTrue(testDistinctionHelper("s://a", true, false, false));
		Assert::IsTrue(testDistinctionHelper("s://a/", true, false, true));
		Assert::IsTrue(testDistinctionHelper("s:/", false, true, false));
		Assert::IsTrue(testDistinctionHelper("s:a", false, false, false));
		Assert::IsTrue(testDistinctionHelper("s:a/", false, false, true));
		Assert::IsTrue(testDistinctionHelper("s:", false, false, false));

		Assert::IsTrue(testDistinctionHelper("//", true, false, false));
		Assert::IsTrue(testDistinctionHelper("///", true, false, true));
		Assert::IsTrue(testDistinctionHelper("/", false, true, false));
		Assert::IsTrue(testDistinctionHelper("a", false, false, false));
		Assert::IsTrue(testDistinctionHelper("a/", false, false, true));
		Assert::IsTrue(testDistinctionHelper("", false, false, false));
	}

	void URI_TEST_IP_FOUR_FAIL(const char * in) {
//		Assert::IsTrue(URI_FALSE == uri_TESTING_ONLY_ParseIpFourA(in));
	}

	void URI_TEST_IP_FOUR_PASS(const char * in) {
//		Assert::IsTrue(URI_TRUE == uri_TESTING_ONLY_ParseIpFourA(in));
	}
	void URI_TEST_IP_SIX_FAIL(const char * in) {
//		Assert::IsTrue(URI_FALSE == uri_TESTING_ONLY_ParseIpSixA(x "]"));
	}
	void URI_TEST_IP_SIX_PASS(const char * in) {
//		Assert::IsTrue(URI_TRUE == uri_TESTING_ONLY_ParseIpSixA(x "]"));
	}

	TEST_METHOD(testIpFour) {
		URI_TEST_IP_FOUR_FAIL("01.0.0.0");
		URI_TEST_IP_FOUR_FAIL("001.0.0.0");
		URI_TEST_IP_FOUR_FAIL("00.0.0.0");
		URI_TEST_IP_FOUR_FAIL("000.0.0.0");
		URI_TEST_IP_FOUR_FAIL("256.0.0.0");
		URI_TEST_IP_FOUR_FAIL("300.0.0.0");
		URI_TEST_IP_FOUR_FAIL("1111.0.0.0");
		URI_TEST_IP_FOUR_FAIL("-1.0.0.0");
		URI_TEST_IP_FOUR_FAIL("0.0.0");
		URI_TEST_IP_FOUR_FAIL("0.0.0.");
		URI_TEST_IP_FOUR_FAIL("0.0.0.0.");
		URI_TEST_IP_FOUR_FAIL("0.0.0.0.0");
		URI_TEST_IP_FOUR_FAIL("0.0..0");
		URI_TEST_IP_FOUR_FAIL(".0.0.0");

		URI_TEST_IP_FOUR_PASS("255.0.0.0");
		URI_TEST_IP_FOUR_PASS("0.0.0.0");
		URI_TEST_IP_FOUR_PASS("1.0.0.0");
		URI_TEST_IP_FOUR_PASS("2.0.0.0");
		URI_TEST_IP_FOUR_PASS("3.0.0.0");
		URI_TEST_IP_FOUR_PASS("30.0.0.0");
	}

	TEST_METHOD(testIpSixPass) {
		// Quad length
		URI_TEST_IP_SIX_PASS("abcd::");

		URI_TEST_IP_SIX_PASS("abcd::1");
		URI_TEST_IP_SIX_PASS("abcd::12");
		URI_TEST_IP_SIX_PASS("abcd::123");
		URI_TEST_IP_SIX_PASS("abcd::1234");

		// Full length
		URI_TEST_IP_SIX_PASS("2001:0db8:0100:f101:0210:a4ff:fee3:9566"); // lower hex
		URI_TEST_IP_SIX_PASS("2001:0DB8:0100:F101:0210:A4FF:FEE3:9566"); // Upper hex
		URI_TEST_IP_SIX_PASS("2001:db8:100:f101:210:a4ff:fee3:9566");
		URI_TEST_IP_SIX_PASS("2001:0db8:100:f101:0:0:0:1");
		URI_TEST_IP_SIX_PASS("1:2:3:4:5:6:255.255.255.255");

		// Legal IPv4
		URI_TEST_IP_SIX_PASS("::1.2.3.4");
		URI_TEST_IP_SIX_PASS("3:4::5:1.2.3.4");
		URI_TEST_IP_SIX_PASS("::ffff:1.2.3.4");
		URI_TEST_IP_SIX_PASS("::0.0.0.0"); // Min IPv4
		URI_TEST_IP_SIX_PASS("::255.255.255.255"); // Max IPv4

												   // Zipper position
		URI_TEST_IP_SIX_PASS("::1:2:3:4:5:6:7");
		URI_TEST_IP_SIX_PASS("1::1:2:3:4:5:6");
		URI_TEST_IP_SIX_PASS("1:2::1:2:3:4:5");
		URI_TEST_IP_SIX_PASS("1:2:3::1:2:3:4");
		URI_TEST_IP_SIX_PASS("1:2:3:4::1:2:3");
		URI_TEST_IP_SIX_PASS("1:2:3:4:5::1:2");
		URI_TEST_IP_SIX_PASS("1:2:3:4:5:6::1");
		URI_TEST_IP_SIX_PASS("1:2:3:4:5:6:7::");

		// Zipper length
		URI_TEST_IP_SIX_PASS("1:1:1::1:1:1:1");
		URI_TEST_IP_SIX_PASS("1:1:1::1:1:1");
		URI_TEST_IP_SIX_PASS("1:1:1::1:1");
		URI_TEST_IP_SIX_PASS("1:1::1:1");
		URI_TEST_IP_SIX_PASS("1:1::1");
		URI_TEST_IP_SIX_PASS("1::1");
		URI_TEST_IP_SIX_PASS("::1"); // == localhost
		URI_TEST_IP_SIX_PASS("::"); // == all addresses

									// A few more variations
		URI_TEST_IP_SIX_PASS("21ff:abcd::1");
		URI_TEST_IP_SIX_PASS("2001:db8:100:f101::1");
		URI_TEST_IP_SIX_PASS("a:b:c::12:1");
		URI_TEST_IP_SIX_PASS("a:b::0:1:2:3");
	}

	TEST_METHOD(testIpSixFail) {
		// 5 char quad
		URI_TEST_IP_SIX_FAIL("::12345");

		// Two zippers
		URI_TEST_IP_SIX_FAIL("abcd::abcd::abcd");

		// Triple-colon zipper
		URI_TEST_IP_SIX_FAIL(":::1234");
		URI_TEST_IP_SIX_FAIL("1234:::1234:1234");
		URI_TEST_IP_SIX_FAIL("1234:1234:::1234");
		URI_TEST_IP_SIX_FAIL("1234:::");

		// No quads, just IPv4
		URI_TEST_IP_SIX_FAIL("1.2.3.4");
		URI_TEST_IP_SIX_FAIL("0001.0002.0003.0004");

		// Five quads
		URI_TEST_IP_SIX_FAIL("0000:0000:0000:0000:0000:1.2.3.4");

		// Seven quads
		URI_TEST_IP_SIX_FAIL("0:0:0:0:0:0:0");
		URI_TEST_IP_SIX_FAIL("0:0:0:0:0:0:0:");
		URI_TEST_IP_SIX_FAIL("0:0:0:0:0:0:0:1.2.3.4");

		// Nine quads (or more)
		URI_TEST_IP_SIX_FAIL("1:2:3:4:5:6:7:8:9");
		URI_TEST_IP_SIX_FAIL("::2:3:4:5:6:7:8:9");
		URI_TEST_IP_SIX_FAIL("1:2:3:4::6:7:8:9");
		URI_TEST_IP_SIX_FAIL("1:2:3:4:5:6:7:8::");

		// Invalid IPv4 part
		URI_TEST_IP_SIX_FAIL("::ffff:001.02.03.004"); // Leading zeros
		URI_TEST_IP_SIX_FAIL("::ffff:1.2.3.1111"); // Four char octet
		URI_TEST_IP_SIX_FAIL("::ffff:1.2.3.256"); // > 255
		URI_TEST_IP_SIX_FAIL("::ffff:311.2.3.4"); // > 155
		URI_TEST_IP_SIX_FAIL("::ffff:1.2.3:4"); // Not a dot
		URI_TEST_IP_SIX_FAIL("::ffff:1.2.3"); // Missing octet
		URI_TEST_IP_SIX_FAIL("::ffff:1.2.3."); // Missing octet
		URI_TEST_IP_SIX_FAIL("::ffff:1.2.3a.4"); // Hex in octet
		URI_TEST_IP_SIX_FAIL("::ffff:1.2.3.4:123"); // Crap input

													// Nonhex
		URI_TEST_IP_SIX_FAIL("g:0:0:0:0:0:0");
	}

	TEST_METHOD(testUri) {
		UriParserStateA stateA;
		UriParserStateW stateW;
		UriUriA uriA;
		UriUriW uriW;

		stateA.uri = &uriA;
		stateW.uri = &uriW;

		// On/off for each
		Assert::IsTrue(0 == uriParseUriA(&stateA, "//user:pass@[::1]:80/segment/index.html?query#frag"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "http://[::1]:80/segment/index.html?query#frag"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "http://user:pass@[::1]/segment/index.html?query#frag"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "http://user:pass@[::1]:80?query#frag"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "http://user:pass@[::1]:80/segment/index.html#frag"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "http://user:pass@[::1]:80/segment/index.html?query"));
		uriFreeUriMembersA(&uriA);

		// Schema, port, one segment
		Assert::IsTrue(0 == uriParseUriA(&stateA, "ftp://host:21/gnu/"));
		uriFreeUriMembersA(&uriA);

		// Relative
		Assert::IsTrue(0 == uriParseUriA(&stateA, "one/two/three"));
		Assert::IsTrue(!uriA.absolutePath);
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "/one/two/three"));
		Assert::IsTrue(uriA.absolutePath);
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "//user:pass@localhost/one/two/three"));
		uriFreeUriMembersA(&uriA);

		// ANSI and Unicode
		Assert::IsTrue(0 == uriParseUriA(&stateA, "http://www.example.com/"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriW(&stateW, L"http://www.example.com/"));
		uriFreeUriMembersW(&uriW);

		// Real life examples
		Assert::IsTrue(0 == uriParseUriA(&stateA, "http://sourceforge.net/projects/uriparser/"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "http://sourceforge.net/project/platformdownload.php?group_id=182840"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "mailto:test@example.com"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "../../"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "/"));
		Assert::IsTrue(uriA.absolutePath);
			uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, ""));
		Assert::IsTrue(!uriA.absolutePath);
			uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 == uriParseUriA(&stateA, "file:///bin/bash"));
		uriFreeUriMembersA(&uriA);

		// Percent encoding
		Assert::IsTrue(0 == uriParseUriA(&stateA, "http://www.example.com/name%20with%20spaces/"));
		uriFreeUriMembersA(&uriA);
		Assert::IsTrue(0 != uriParseUriA(&stateA, "http://www.example.com/name with spaces/"));
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriComponents) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  0              15 01  0      7  01
		const char * const input = "http" "://" "sourceforge.net" "/" "project" "/"
			//		 0                   20 01  0              15
			"platformdownload.php" "?" "group_id=182840";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.scheme.first == input);
		Assert::IsTrue(uriA.scheme.afterLast == input + 4);
		Assert::IsTrue(uriA.userInfo.first == NULL);
		Assert::IsTrue(uriA.userInfo.afterLast == NULL);
		Assert::IsTrue(uriA.hostText.first == input + 4 + 3);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 15);
		Assert::IsTrue(uriA.hostData.ipFuture.first == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.afterLast == NULL);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);

		Assert::IsTrue(uriA.pathHead->text.first == input + 4 + 3 + 15 + 1);
		Assert::IsTrue(uriA.pathHead->text.afterLast == input + 4 + 3 + 15 + 1 + 7);
		Assert::IsTrue(uriA.pathHead->next->text.first == input + 4 + 3 + 15 + 1 + 7 + 1);
		Assert::IsTrue(uriA.pathHead->next->text.afterLast == input + 4 + 3 + 15 + 1 + 7 + 1 + 20);
		Assert::IsTrue(uriA.pathHead->next->next == NULL);
		Assert::IsTrue(uriA.pathTail == uriA.pathHead->next);

		Assert::IsTrue(uriA.query.first == input + 4 + 3 + 15 + 1 + 7 + 1 + 20 + 1);
		Assert::IsTrue(uriA.query.afterLast == input + 4 + 3 + 15 + 1 + 7 + 1 + 20 + 1 + 15);
		Assert::IsTrue(uriA.fragment.first == NULL);
		Assert::IsTrue(uriA.fragment.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriComponents_Bug20070701) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          01  01  01
		const char * const input = "a" ":" "b";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.scheme.first == input);
		Assert::IsTrue(uriA.scheme.afterLast == input + 1);
		Assert::IsTrue(uriA.userInfo.first == NULL);
		Assert::IsTrue(uriA.userInfo.afterLast == NULL);
		Assert::IsTrue(uriA.hostText.first == NULL);
		Assert::IsTrue(uriA.hostText.afterLast == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.first == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.afterLast == NULL);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);

		Assert::IsTrue(uriA.pathHead->text.first == input + 1 + 1);
		Assert::IsTrue(uriA.pathHead->text.afterLast == input + 1 + 1 + 1);
		Assert::IsTrue(uriA.pathHead->next == NULL);
		Assert::IsTrue(uriA.pathTail == uriA.pathHead);

		Assert::IsTrue(uriA.query.first == NULL);
		Assert::IsTrue(uriA.query.afterLast == NULL);
		Assert::IsTrue(uriA.fragment.first == NULL);
		Assert::IsTrue(uriA.fragment.afterLast == NULL);

		Assert::IsTrue(!uriA.absolutePath);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort1) {
		// User info with ":", no port
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  0      7  01  0        9
		const char * const input = "http" "://" "abc:def" "@" "localhost";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.userInfo.first == input + 4 + 3);
		Assert::IsTrue(uriA.userInfo.afterLast == input + 4 + 3 + 7);
		Assert::IsTrue(uriA.hostText.first == input + 4 + 3 + 7 + 1);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 7 + 1 + 9);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort2) {
		// User info with ":", with port
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  0      7  01  0        9
		const char * const input = "http" "://" "abc:def" "@" "localhost"
			//		01   0  3
			":" "123";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.userInfo.first == input + 4 + 3);
		Assert::IsTrue(uriA.userInfo.afterLast == input + 4 + 3 + 7);
		Assert::IsTrue(uriA.hostText.first == input + 4 + 3 + 7 + 1);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 7 + 1 + 9);
		Assert::IsTrue(uriA.portText.first == input + 4 + 3 + 7 + 1 + 9 + 1);
		Assert::IsTrue(uriA.portText.afterLast == input + 4 + 3 + 7 + 1 + 9 + 1 + 3);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort22_Bug1948038) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;

		int res;

		res = uriParseUriA(&stateA, "http://user:21@host/");
		Assert::IsTrue(URI_SUCCESS == res);
		Assert::IsTrue(!memcmp(uriA.userInfo.first, "user:21", 7 * sizeof(char)));
		Assert::IsTrue(uriA.userInfo.afterLast - uriA.userInfo.first == 7);
		Assert::IsTrue(!memcmp(uriA.hostText.first, "host", 4 * sizeof(char)));
		Assert::IsTrue(uriA.hostText.afterLast - uriA.hostText.first == 4);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);

		res = uriParseUriA(&stateA, "http://user:1234@192.168.0.1:1234/foo.com");
		Assert::IsTrue(URI_SUCCESS == res);
		uriFreeUriMembersA(&uriA);

		res = uriParseUriA(&stateA, "http://moo:21@moo:21@moo/");
		Assert::IsTrue(URI_ERROR_SYNTAX == res);
		uriFreeUriMembersA(&uriA);

		res = uriParseUriA(&stateA, "http://moo:21@moo:21@moo:21/");
		Assert::IsTrue(URI_ERROR_SYNTAX == res);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort23_Bug3510198_1) {
		// User info with ":", with port, with escaped chars in password
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;

		int res;
		//                           0   4  0  3  0         10 01  0   4  01
		res = uriParseUriA(&stateA, "http" "://" "user:%2F21" "@" "host" "/");
		Assert::IsTrue(URI_SUCCESS == res);
		Assert::IsTrue(!memcmp(uriA.userInfo.first, "user:%2F21", 10 * sizeof(char)));
		Assert::IsTrue(uriA.userInfo.afterLast - uriA.userInfo.first == 10);
		Assert::IsTrue(!memcmp(uriA.hostText.first, "host", 4 * sizeof(char)));
		Assert::IsTrue(uriA.hostText.afterLast - uriA.hostText.first == 4);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);

	}

	TEST_METHOD(testUriUserInfoHostPort23_Bug3510198_2) {
		// User info with ":", with port, with escaped chars in user name and password
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;

		int res;
		//                           0   4  0  3  0            13 01  0   4  01
		res = uriParseUriA(&stateA, "http" "://" "%2Fuser:%2F21" "@" "host" "/");
		Assert::IsTrue(URI_SUCCESS == res);
		Assert::IsTrue(!memcmp(uriA.userInfo.first, "%2Fuser:%2F21", 13 * sizeof(char)));
		Assert::IsTrue(uriA.userInfo.afterLast - uriA.userInfo.first == 13);
		Assert::IsTrue(!memcmp(uriA.hostText.first, "host", 4 * sizeof(char)));
		Assert::IsTrue(uriA.hostText.afterLast - uriA.hostText.first == 4);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort23_Bug3510198_3) {
		// User info with ":", with port, with escaped chars in password
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;

		int res;
		//                           0   4  0  3  0               16 01  0   4  01
		res = uriParseUriA(&stateA, "http" "://" "user:!$&'()*+,;=" "@" "host" "/");
		Assert::IsTrue(URI_SUCCESS == res);
		Assert::IsTrue(!memcmp(uriA.userInfo.first, "user:!$&'()*+,;=", 16 * sizeof(char)));
		Assert::IsTrue(uriA.userInfo.afterLast - uriA.userInfo.first == 16);
		Assert::IsTrue(!memcmp(uriA.hostText.first, "host", 4 * sizeof(char)));
		Assert::IsTrue(uriA.hostText.afterLast - uriA.hostText.first == 4);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);

	}

	TEST_METHOD(testUriUserInfoHostPort23_Bug3510198_4) {
		// User info with ":", with port, with escaped chars in user name and password
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;

		int res;
		//                           0   4  0  3  0                   20 01  0   4  01
		res = uriParseUriA(&stateA, "http" "://" "!$&'()*+,;=:password" "@" "host" "/");
		Assert::IsTrue(URI_SUCCESS == res);
		Assert::IsTrue(!memcmp(uriA.userInfo.first, "!$&'()*+,;=:password", 20 * sizeof(char)));
		Assert::IsTrue(uriA.userInfo.afterLast - uriA.userInfo.first == 20);
		Assert::IsTrue(!memcmp(uriA.hostText.first, "host", 4 * sizeof(char)));
		Assert::IsTrue(uriA.hostText.afterLast - uriA.hostText.first == 4);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort23_Bug3510198_related_1) {
		// Empty user info
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;

		int res;
		//                           0   4  0  3  01  0   4  01
		res = uriParseUriA(&stateA, "http" "://" "@" "host" "/");
		Assert::IsTrue(URI_SUCCESS == res);
		Assert::IsTrue(uriA.userInfo.afterLast != NULL);
		Assert::IsTrue(uriA.userInfo.first != NULL);
		Assert::IsTrue(uriA.userInfo.afterLast - uriA.userInfo.first == 0);
		Assert::IsTrue(!memcmp(uriA.hostText.first, "host", 4 * sizeof(char)));
		Assert::IsTrue(uriA.hostText.afterLast - uriA.hostText.first == 4);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort23_Bug3510198_related_12) {
		// Empty user info
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;

		int res;
		//                           0   4  0  3  0      7  01
		res = uriParseUriA(&stateA, "http" "://" "%2Fhost" "/");
		Assert::IsTrue(URI_SUCCESS == res);
		Assert::IsTrue(uriA.userInfo.afterLast == NULL);
		Assert::IsTrue(uriA.userInfo.first == NULL);
		Assert::IsTrue(!memcmp(uriA.hostText.first, "%2Fhost", 7 * sizeof(char)));
		Assert::IsTrue(uriA.hostText.afterLast - uriA.hostText.first == 7);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort23_Bug3510198_related_2) {
		// Several colons in userinfo
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;

		int res;
		//                           0   4  0  3  0 2  01  0   4  01
		res = uriParseUriA(&stateA, "http" "://" "::" "@" "host" "/");
		Assert::IsTrue(URI_SUCCESS == res);
		Assert::IsTrue(!memcmp(uriA.userInfo.first, "::", 2 * sizeof(char)));
		Assert::IsTrue(uriA.userInfo.afterLast - uriA.userInfo.first == 2);
		Assert::IsTrue(!memcmp(uriA.hostText.first, "host", 4 * sizeof(char)));
		Assert::IsTrue(uriA.hostText.afterLast - uriA.hostText.first == 4);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort3) {
		// User info without ":", no port
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  0      7  01  0        9
		const char * const input = "http" "://" "abcdefg" "@" "localhost";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.userInfo.first == input + 4 + 3);
		Assert::IsTrue(uriA.userInfo.afterLast == input + 4 + 3 + 7);
		Assert::IsTrue(uriA.hostText.first == input + 4 + 3 + 7 + 1);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 7 + 1 + 9);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort4) {
		// User info without ":", with port
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  0      7  01  0        9
		const char * const input = "http" "://" "abcdefg" "@" "localhost"
			//		01   0  3
			":" "123";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.userInfo.first == input + 4 + 3);
		Assert::IsTrue(uriA.userInfo.afterLast == input + 4 + 3 + 7);
		Assert::IsTrue(uriA.hostText.first == input + 4 + 3 + 7 + 1);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 7 + 1 + 9);
		Assert::IsTrue(uriA.portText.first == input + 4 + 3 + 7 + 1 + 9 + 1);
		Assert::IsTrue(uriA.portText.afterLast == input + 4 + 3 + 7 + 1 + 9 + 1 + 3);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort5) {
		// No user info, no port
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  0        9
		const char * const input = "http" "://" "localhost";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.userInfo.first == NULL);
		Assert::IsTrue(uriA.userInfo.afterLast == NULL);
		Assert::IsTrue(uriA.hostText.first == input + 4 + 3);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 9);
		Assert::IsTrue(uriA.portText.first == NULL);
		Assert::IsTrue(uriA.portText.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriUserInfoHostPort6) {
		// No user info, with port
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  0        9  01  0  3
		const char * const input = "http" "://" "localhost" ":" "123";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.userInfo.first == NULL);
		Assert::IsTrue(uriA.userInfo.afterLast == NULL);
		Assert::IsTrue(uriA.hostText.first == input + 4 + 3);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 9);
		Assert::IsTrue(uriA.portText.first == input + 4 + 3 + 9 + 1);
		Assert::IsTrue(uriA.portText.afterLast == input + 4 + 3 + 9 + 1 + 3);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriHostRegname) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  0          11
		const char * const input = "http" "://" "example.com";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.hostText.first == input + 4 + 3);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 11);
		Assert::IsTrue(uriA.hostData.ip4 == NULL);
		Assert::IsTrue(uriA.hostData.ip6 == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.first == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriHostIpFour1) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  0      7  01  0 2
		const char * const input = "http" "://" "1.2.3.4" ":" "80";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.hostText.first == input + 4 + 3);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 7);
		Assert::IsTrue(uriA.hostData.ip4 != NULL);
		Assert::IsTrue(uriA.hostData.ip6 == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.first == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriHostIpFour2) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  0      7
		const char * const input = "http" "://" "1.2.3.4";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.hostText.first == input + 4 + 3);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 7);
		Assert::IsTrue(uriA.hostData.ip4 != NULL);
		Assert::IsTrue(uriA.hostData.ip6 == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.first == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriHostIpSix1) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  01  45  01  0 2
		const char * const input = "http" "://" "[::1]" ":" "80";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.hostText.first == input + 4 + 3 + 1);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 4);
		Assert::IsTrue(uriA.hostData.ip4 == NULL);
		Assert::IsTrue(uriA.hostData.ip6 != NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.first == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriHostIpSix2) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  01  45
		const char * const input = "http" "://" "[::1]";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.hostText.first == input + 4 + 3 + 1);
		Assert::IsTrue(uriA.hostText.afterLast == input + 4 + 3 + 4);
		Assert::IsTrue(uriA.hostData.ip4 == NULL);
		Assert::IsTrue(uriA.hostData.ip6 != NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.first == NULL);
		Assert::IsTrue(uriA.hostData.ipFuture.afterLast == NULL);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriHostEmpty) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0   4  0  3  01  0  3
		const char * const input = "http" "://" ":" "123";
		const int res = uriParseUriA(&stateA, input);
		Assert::IsTrue(URI_SUCCESS == res);
		Assert::IsTrue(uriA.userInfo.first == NULL);
		Assert::IsTrue(uriA.userInfo.afterLast == NULL);
		Assert::IsTrue(uriA.hostText.first != NULL);
		Assert::IsTrue(uriA.hostText.afterLast != NULL);
		Assert::IsTrue(uriA.hostText.afterLast - uriA.hostText.first == 0);
		Assert::IsTrue(uriA.portText.first == input + 4 + 3 + 1);
		Assert::IsTrue(uriA.portText.afterLast == input + 4 + 3 + 1 + 3);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testUriHostIpFuture) {
		// TODO
	}

	bool testEscapingHelper(const wchar_t * in, const wchar_t * expectedOut,
							bool spaceToPlus = false, bool normalizeBreaks = false) {
		wchar_t * const buffer = new wchar_t[(normalizeBreaks ? 6 : 3)
			* wcslen(in) + 1];
		if (uriEscapeW(in, buffer, spaceToPlus, normalizeBreaks)
			!= buffer + wcslen(expectedOut)) {
			delete[] buffer;
			return false;
		}

		const bool equal = !wcscmp(buffer, expectedOut);
		delete[] buffer;
		return equal;
	}

	TEST_METHOD(testEscaping) {
		const bool SPACE_TO_PLUS = true;
		const bool SPACE_TO_PERCENT = false;
		const bool KEEP_UNMODIFIED = false;
		const bool NORMALIZE = true;

		// '+' to ' '
		Assert::IsTrue(testEscapingHelper(L"abc def", L"abc+def", SPACE_TO_PLUS));
		Assert::IsTrue(testEscapingHelper(L"abc def", L"abc%20def", SPACE_TO_PERCENT));

		// Percent encoding
		Assert::IsTrue(testEscapingHelper(L"\x00", L"\0"));
		Assert::IsTrue(testEscapingHelper(L"\x01", L"%01"));
		Assert::IsTrue(testEscapingHelper(L"\xff", L"%FF"));

		// Linebreak normalization
		Assert::IsTrue(testEscapingHelper(L"\x0d", L"%0D%0A", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"g\x0d", L"g%0D%0A", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"\x0dg", L"%0D%0Ag", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"\x0d", L"%0D", SPACE_TO_PLUS, KEEP_UNMODIFIED));
		Assert::IsTrue(testEscapingHelper(L"g\x0d", L"g%0D", SPACE_TO_PLUS, KEEP_UNMODIFIED));
		Assert::IsTrue(testEscapingHelper(L"\x0dg", L"%0Dg", SPACE_TO_PLUS, KEEP_UNMODIFIED));

		Assert::IsTrue(testEscapingHelper(L"\x0a", L"%0D%0A", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"g\x0a", L"g%0D%0A", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"\x0ag", L"%0D%0Ag", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"\x0a", L"%0A", SPACE_TO_PLUS, KEEP_UNMODIFIED));
		Assert::IsTrue(testEscapingHelper(L"g\x0a", L"g%0A", SPACE_TO_PLUS, KEEP_UNMODIFIED));
		Assert::IsTrue(testEscapingHelper(L"\x0ag", L"%0Ag", SPACE_TO_PLUS, KEEP_UNMODIFIED));

		Assert::IsTrue(testEscapingHelper(L"\x0d\x0a", L"%0D%0A", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"g\x0d\x0a", L"g%0D%0A", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"\x0d\x0ag", L"%0D%0Ag", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"\x0d\x0a", L"%0D%0A", SPACE_TO_PLUS, KEEP_UNMODIFIED));
		Assert::IsTrue(testEscapingHelper(L"g\x0d\x0a", L"g%0D%0A", SPACE_TO_PLUS, KEEP_UNMODIFIED));
		Assert::IsTrue(testEscapingHelper(L"\x0d\x0ag", L"%0D%0Ag", SPACE_TO_PLUS, KEEP_UNMODIFIED));

		Assert::IsTrue(testEscapingHelper(L"\x0a\x0d", L"%0D%0A%0D%0A", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"g\x0a\x0d", L"g%0D%0A%0D%0A", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"\x0a\x0dg", L"%0D%0A%0D%0Ag", SPACE_TO_PLUS, NORMALIZE));
		Assert::IsTrue(testEscapingHelper(L"\x0a\x0d", L"%0A%0D", SPACE_TO_PLUS, KEEP_UNMODIFIED));
		Assert::IsTrue(testEscapingHelper(L"g\x0a\x0d", L"g%0A%0D", SPACE_TO_PLUS, KEEP_UNMODIFIED));
		Assert::IsTrue(testEscapingHelper(L"\x0a\x0dg", L"%0A%0Dg", SPACE_TO_PLUS, KEEP_UNMODIFIED));
	}

	bool testUnescapingHelper(const wchar_t * input, const wchar_t * output,
							  bool plusToSpace = false, UriBreakConversion breakConversion = URI_BR_DONT_TOUCH) {
		wchar_t * working = new wchar_t[URI_STRLEN(input) + 1];
		wcscpy(working, input);
		const wchar_t * newTermZero = uriUnescapeInPlaceExW(working,
															plusToSpace ? URI_TRUE : URI_FALSE, breakConversion);
		const bool success = ((newTermZero == working + wcslen(output))
							  && !wcscmp(working, output));
		delete[] working;
		return success;
	}

	TEST_METHOD(testUnescaping) {
		const bool PLUS_TO_SPACE = true;
		const bool PLUS_DONT_TOUCH = false;


		// Proper
		Assert::IsTrue(testUnescapingHelper(L"abc%20%41BC", L"abc ABC"));
		Assert::IsTrue(testUnescapingHelper(L"%20", L" "));

		// Incomplete
		Assert::IsTrue(testUnescapingHelper(L"%0", L"%0"));

		// Nonhex
		Assert::IsTrue(testUnescapingHelper(L"%0g", L"%0g"));
		Assert::IsTrue(testUnescapingHelper(L"%G0", L"%G0"));

		// No double decoding
		Assert::IsTrue(testUnescapingHelper(L"%2520", L"%20"));

		// Decoding of '+'
		Assert::IsTrue(testUnescapingHelper(L"abc+def", L"abc+def", PLUS_DONT_TOUCH));
		Assert::IsTrue(testUnescapingHelper(L"abc+def", L"abc def", PLUS_TO_SPACE));

		// Line break conversion
		Assert::IsTrue(testUnescapingHelper(L"%0d", L"\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0d", L"\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0d", L"\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0d", L"\x0d", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));

		Assert::IsTrue(testUnescapingHelper(L"%0d%0d", L"\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0d", L"\x0d\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0d", L"\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0d", L"\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));


		Assert::IsTrue(testUnescapingHelper(L"%0a", L"\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0a", L"\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0a", L"\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0a", L"\x0a", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));

		Assert::IsTrue(testUnescapingHelper(L"%0a%0a", L"\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0a", L"\x0d\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0a", L"\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0a", L"\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));


		Assert::IsTrue(testUnescapingHelper(L"%0d%0a", L"\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a", L"\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a", L"\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a", L"\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));

		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0a", L"\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0a", L"\x0d\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0a", L"\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0a", L"\x0d\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));

		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0d", L"\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0d", L"\x0d\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0d", L"\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0d", L"\x0d\x0a\x0d", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));

		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0d%0a", L"\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0d%0a", L"\x0d\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0d%0a", L"\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0d%0a%0d%0a", L"\x0d\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));


		Assert::IsTrue(testUnescapingHelper(L"%0a%0d", L"\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d", L"\x0d\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d", L"\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d", L"\x0a\x0d", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));

		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0a", L"\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0a", L"\x0d\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0a", L"\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0a", L"\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));

		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0d", L"\x0a\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0d", L"\x0d\x0a\x0d\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0d", L"\x0d\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0d", L"\x0a\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));

		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0a%0d", L"\x0a\x0a\x0a", PLUS_DONT_TOUCH, URI_BR_TO_UNIX));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0a%0d", L"\x0d\x0a\x0d\x0a\x0d\x0a", PLUS_DONT_TOUCH, URI_BR_TO_WINDOWS));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0a%0d", L"\x0d\x0d\x0d", PLUS_DONT_TOUCH, URI_BR_TO_MAC));
		Assert::IsTrue(testUnescapingHelper(L"%0a%0d%0a%0d", L"\x0a\x0d\x0a\x0d", PLUS_DONT_TOUCH, URI_BR_DONT_TOUCH));
	}

	bool testAddBaseHelper(const wchar_t * base, const wchar_t * rel, const wchar_t * expectedResult, bool backward_compatibility = false) {
		UriParserStateW stateW;

		// Base
		UriUriW baseUri;
		stateW.uri = &baseUri;
		int res = uriParseUriW(&stateW, base);
		if (res != 0) {
			uriFreeUriMembersW(&baseUri);
			return false;
		}

		// Rel
		UriUriW relUri;
		stateW.uri = &relUri;
		res = uriParseUriW(&stateW, rel);
		if (res != 0) {
			uriFreeUriMembersW(&baseUri);
			uriFreeUriMembersW(&relUri);
			return false;
		}

		// Expected result
		UriUriW expectedUri;
		stateW.uri = &expectedUri;
		res = uriParseUriW(&stateW, expectedResult);
		if (res != 0) {
			uriFreeUriMembersW(&baseUri);
			uriFreeUriMembersW(&relUri);
			uriFreeUriMembersW(&expectedUri);
			return false;
		}

		// Transform
		UriUriW transformedUri;
		if (backward_compatibility) {
			res = uriAddBaseUriExW(&transformedUri, &relUri, &baseUri, URI_RESOLVE_IDENTICAL_SCHEME_COMPAT);
		} else {
			res = uriAddBaseUriW(&transformedUri, &relUri, &baseUri);
		}

		if (res != 0) {
			uriFreeUriMembersW(&baseUri);
			uriFreeUriMembersW(&relUri);
			uriFreeUriMembersW(&expectedUri);
			uriFreeUriMembersW(&transformedUri);
			return false;
		}

		const bool equal = (URI_TRUE == uriEqualsUriW(&transformedUri, &expectedUri));
		if (!equal) {
			wchar_t transformedUriText[1024 * 8];
			wchar_t expectedUriText[1024 * 8];
			uriToStringW(transformedUriText, &transformedUri, 1024 * 8, NULL);
			uriToStringW(expectedUriText, &expectedUri, 1024 * 8, NULL);
#ifdef HAVE_WPRINTF
			wprintf(L"\n\n\nExpected: \"%s\"\nReceived: \"%s\"\n\n\n", expectedUriText, transformedUriText);
#endif
		}

		uriFreeUriMembersW(&baseUri);
		uriFreeUriMembersW(&relUri);
		uriFreeUriMembersW(&expectedUri);
		uriFreeUriMembersW(&transformedUri);
		return equal;
	}

	TEST_METHOD(testTrailingSlash) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		//                          0  3  01
		const char * const input = "abc" "/";
		Assert::IsTrue(0 == uriParseUriA(&stateA, input));

		Assert::IsTrue(uriA.pathHead->text.first == input);
		Assert::IsTrue(uriA.pathHead->text.afterLast == input + 3);
		Assert::IsTrue(uriA.pathHead->next->text.first == uriA.pathHead->next->text.afterLast);
		Assert::IsTrue(uriA.pathHead->next->next == NULL);
		Assert::IsTrue(uriA.pathTail == uriA.pathHead->next);
		uriFreeUriMembersA(&uriA);
	}

	TEST_METHOD(testAddBase) {
		// 5.4.1. Normal Examples
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g:h", L"g:h"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g", L"http://a/b/c/g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"./g", L"http://a/b/c/g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g/", L"http://a/b/c/g/"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"/g", L"http://a/g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"//g", L"http://g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"?y", L"http://a/b/c/d;p?y"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g?y", L"http://a/b/c/g?y"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"#s", L"http://a/b/c/d;p?q#s"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g#s", L"http://a/b/c/g#s"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g?y#s", L"http://a/b/c/g?y#s"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L";x", L"http://a/b/c/;x"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g;x", L"http://a/b/c/g;x"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g;x?y#s", L"http://a/b/c/g;x?y#s"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"", L"http://a/b/c/d;p?q"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L".", L"http://a/b/c/"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"./", L"http://a/b/c/"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"..", L"http://a/b/"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"../", L"http://a/b/"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"../g", L"http://a/b/g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"../..", L"http://a/"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"../../", L"http://a/"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"../../g", L"http://a/g"));

		// 5.4.2. Abnormal Examples
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"../../../g", L"http://a/g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"../../../../g", L"http://a/g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"/./g", L"http://a/g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"/../g", L"http://a/g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g.", L"http://a/b/c/g."));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L".g", L"http://a/b/c/.g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g..", L"http://a/b/c/g.."));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"..g", L"http://a/b/c/..g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"./../g", L"http://a/b/g"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"./g/.", L"http://a/b/c/g/"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g/./h", L"http://a/b/c/g/h"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g/../h", L"http://a/b/c/h"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g;x=1/./y", L"http://a/b/c/g;x=1/y"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g;x=1/../y", L"http://a/b/c/y"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g?y/./x", L"http://a/b/c/g?y/./x"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g?y/../x", L"http://a/b/c/g?y/../x"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g#s/./x", L"http://a/b/c/g#s/./x"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"g#s/../x", L"http://a/b/c/g#s/../x"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"http:g", L"http:g"));

		// Backward compatibility (feature request #4, RFC3986 5.4.2)
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"http:g", L"http:g", false));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"http:g", L"http://a/b/c/g", true));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"http:g?q#f", L"http://a/b/c/g?q#f", true));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"other:g?q#f", L"other:g?q#f", true));

		// Bug related to absolutePath flag set despite presence of host
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"/", L"http://a/"));
		Assert::IsTrue(testAddBaseHelper(L"http://a/b/c/d;p?q", L"/g/", L"http://a/g/"));
	}

	bool testToStringHelper(const wchar_t * text) {
		// Parse
		UriParserStateW state;
		UriUriW uri;
		state.uri = &uri;
		int res = uriParseUriW(&state, text);
		if (res != 0) {
			uriFreeUriMembersW(&uri);
			return false;
		}

		// Back to string, _huge_ limit
		wchar_t shouldbeTheSame[1024 * 8];
		res = uriToStringW(shouldbeTheSame, &uri, 1024 * 8, NULL);
		if (res != 0) {
			uriFreeUriMembersW(&uri);
			return false;
		}

		// Compare
		bool equals = (0 == wcscmp(shouldbeTheSame, text));
		if (!equals) {
#ifdef HAVE_WPRINTF
			wprintf(L"\n\n\nExpected: \"%s\"\nReceived: \"%s\"\n\n\n", text, shouldbeTheSame);
#endif
		}

		// Back to string, _exact_ limit
		const int len = static_cast<int>(wcslen(text));
		int charsWritten;
		res = uriToStringW(shouldbeTheSame, &uri, len + 1, &charsWritten);
		if ((res != 0) || (charsWritten != len + 1)) {
			uriFreeUriMembersW(&uri);
			return false;
		}

		// Back to string, _too small_ limit
		res = uriToStringW(shouldbeTheSame, &uri, len, &charsWritten);
		if ((res == 0) || (charsWritten >= len + 1)) {
			uriFreeUriMembersW(&uri);
			return false;
		}

		uriFreeUriMembersW(&uri);
		return equals;
	}

	TEST_METHOD(testToString) {
		// Scheme
		Assert::IsTrue(testToStringHelper(L"ftp://localhost/"));
		// UserInfo
		Assert::IsTrue(testToStringHelper(L"http://user:pass@localhost/"));
		// IPv4
		Assert::IsTrue(testToStringHelper(L"http://123.0.1.255/"));
		// IPv6
		Assert::IsTrue(testToStringHelper(L"http://[abcd:abcd:abcd:abcd:abcd:abcd:abcd:abcd]/"));
		// IPvFuture
		Assert::IsTrue(testToStringHelper(L"http://[vA.123456]/"));
		// Port
		Assert::IsTrue(testToStringHelper(L"http://example.com:123/"));
		// Path
		Assert::IsTrue(testToStringHelper(L"http://example.com"));
		Assert::IsTrue(testToStringHelper(L"http://example.com/"));
		Assert::IsTrue(testToStringHelper(L"http://example.com/abc/"));
		Assert::IsTrue(testToStringHelper(L"http://example.com/abc/def"));
		Assert::IsTrue(testToStringHelper(L"http://example.com/abc/def/"));
		Assert::IsTrue(testToStringHelper(L"http://example.com//"));
		Assert::IsTrue(testToStringHelper(L"http://example.com/./.."));
		// Query
		Assert::IsTrue(testToStringHelper(L"http://example.com/?abc"));
		// Fragment
		Assert::IsTrue(testToStringHelper(L"http://example.com/#abc"));
		Assert::IsTrue(testToStringHelper(L"http://example.com/?def#abc"));

		// Relative
		Assert::IsTrue(testToStringHelper(L"a"));
		Assert::IsTrue(testToStringHelper(L"a/"));
		Assert::IsTrue(testToStringHelper(L"/a"));
		Assert::IsTrue(testToStringHelper(L"/a/"));
		Assert::IsTrue(testToStringHelper(L"abc"));
		Assert::IsTrue(testToStringHelper(L"abc/"));
		Assert::IsTrue(testToStringHelper(L"/abc"));
		Assert::IsTrue(testToStringHelper(L"/abc/"));
		Assert::IsTrue(testToStringHelper(L"a/def"));
		Assert::IsTrue(testToStringHelper(L"a/def/"));
		Assert::IsTrue(testToStringHelper(L"/a/def"));
		Assert::IsTrue(testToStringHelper(L"/a/def/"));
		Assert::IsTrue(testToStringHelper(L"abc/def"));
		Assert::IsTrue(testToStringHelper(L"abc/def/"));
		Assert::IsTrue(testToStringHelper(L"/abc/def"));
		Assert::IsTrue(testToStringHelper(L"/abc/def/"));
		Assert::IsTrue(testToStringHelper(L"/"));
		Assert::IsTrue(testToStringHelper(L"//a/"));
		Assert::IsTrue(testToStringHelper(L"."));
		Assert::IsTrue(testToStringHelper(L"./"));
		Assert::IsTrue(testToStringHelper(L"/."));
		Assert::IsTrue(testToStringHelper(L"/./"));
		Assert::IsTrue(testToStringHelper(L""));
		Assert::IsTrue(testToStringHelper(L"./abc/def"));
		Assert::IsTrue(testToStringHelper(L"?query"));
		Assert::IsTrue(testToStringHelper(L"#fragment"));
		Assert::IsTrue(testToStringHelper(L"?query#fragment"));

		// Tests for bugs from the past
		Assert::IsTrue(testToStringHelper(L"f:/.//g"));
	}

	TEST_METHOD(testToString_Bug1950126) {
		UriParserStateW state;
		UriUriW uriOne;
		UriUriW uriTwo;
		const wchar_t * const uriOneString = L"http://e.com/";
		const wchar_t * const uriTwoString = L"http://e.com";
		state.uri = &uriOne;
		Assert::IsTrue(URI_SUCCESS == uriParseUriW(&state, uriOneString));
		state.uri = &uriTwo;
		Assert::IsTrue(URI_SUCCESS == uriParseUriW(&state, uriTwoString));
		Assert::IsTrue(URI_FALSE == uriEqualsUriW(&uriOne, &uriTwo));
		uriFreeUriMembersW(&uriOne);
		uriFreeUriMembersW(&uriTwo);

		Assert::IsTrue(testToStringHelper(uriOneString));
		Assert::IsTrue(testToStringHelper(uriTwoString));
	}

	bool testToStringCharsRequiredHelper(const wchar_t * text) {
		// Parse
		UriParserStateW state;
		UriUriW uri;
		state.uri = &uri;
		int res = uriParseUriW(&state, text);
		if (res != 0) {
			uriFreeUriMembersW(&uri);
			return false;
		}

		// Required space?
		int charsRequired;
		if (uriToStringCharsRequiredW(&uri, &charsRequired) != 0) {
			uriFreeUriMembersW(&uri);
			return false;
		}

		// Minimum
		wchar_t * buffer = new wchar_t[charsRequired + 1];
		if (uriToStringW(buffer, &uri, charsRequired + 1, NULL) != 0) {
			uriFreeUriMembersW(&uri);
			delete[] buffer;
			return false;
		}

		// One less than minimum
		if (uriToStringW(buffer, &uri, charsRequired, NULL) == 0) {
			uriFreeUriMembersW(&uri);
			delete[] buffer;
			return false;
		}

		uriFreeUriMembersW(&uri);
		delete[] buffer;
		return true;
	}

	TEST_METHOD(testToStringCharsRequired) {
		Assert::IsTrue(testToStringCharsRequiredHelper(L"http://www.example.com/"));
		Assert::IsTrue(testToStringCharsRequiredHelper(L"http://www.example.com:80/"));
		Assert::IsTrue(testToStringCharsRequiredHelper(L"http://user:pass@www.example.com/"));
		Assert::IsTrue(testToStringCharsRequiredHelper(L"http://www.example.com/index.html"));
		Assert::IsTrue(testToStringCharsRequiredHelper(L"http://www.example.com/?abc"));
		Assert::IsTrue(testToStringCharsRequiredHelper(L"http://www.example.com/#def"));
		Assert::IsTrue(testToStringCharsRequiredHelper(L"http://www.example.com/?abc#def"));
		Assert::IsTrue(testToStringCharsRequiredHelper(L"/test"));
		Assert::IsTrue(testToStringCharsRequiredHelper(L"test"));
	}

	bool testNormalizeMaskHelper(const wchar_t * uriText, unsigned int expectedMask) {
		UriParserStateW state;
		UriUriW uri;
		state.uri = &uri;
		int res = uriParseUriW(&state, uriText);
		if (res != 0) {
			uriFreeUriMembersW(&uri);
			return false;
		}

		const unsigned int maskBefore = uriNormalizeSyntaxMaskRequiredW(&uri);
		if (maskBefore != expectedMask) {
			uriFreeUriMembersW(&uri);
			return false;
		}

		res = uriNormalizeSyntaxW(&uri);
		if (res != 0) {
			uriFreeUriMembersW(&uri);
			return false;
		}

		const unsigned int maskAfter = uriNormalizeSyntaxMaskRequiredW(&uri);
		uriFreeUriMembersW(&uri);

		// Second call should be no problem
		uriFreeUriMembersW(&uri);

		return (maskAfter == URI_NORMALIZED);
	}

	TEST_METHOD(testNormalizeSyntaxMaskRequired) {
		Assert::IsTrue(testNormalizeMaskHelper(L"http://localhost/", URI_NORMALIZED));
		Assert::IsTrue(testNormalizeMaskHelper(L"httP://localhost/", URI_NORMALIZE_SCHEME));
		Assert::IsTrue(testNormalizeMaskHelper(L"http://%0d@localhost/", URI_NORMALIZE_USER_INFO));
		Assert::IsTrue(testNormalizeMaskHelper(L"http://localhosT/", URI_NORMALIZE_HOST));
		Assert::IsTrue(testNormalizeMaskHelper(L"http://localhost/./abc", URI_NORMALIZE_PATH));
		Assert::IsTrue(testNormalizeMaskHelper(L"http://localhost/?AB%43", URI_NORMALIZE_QUERY));
		Assert::IsTrue(testNormalizeMaskHelper(L"http://localhost/#AB%43", URI_NORMALIZE_FRAGMENT));
	}

	bool testNormalizeSyntaxHelper(const wchar_t * uriText, const wchar_t * expectedNormalized,
								   unsigned int mask = static_cast<unsigned int>(-1)) {
		UriParserStateW stateW;
		int res;

		UriUriW testUri;
		stateW.uri = &testUri;
		res = uriParseUriW(&stateW, uriText);
		if (res != 0) {
			uriFreeUriMembersW(&testUri);
			return false;
		}

		// Expected result
		UriUriW expectedUri;
		stateW.uri = &expectedUri;
		res = uriParseUriW(&stateW, expectedNormalized);
		if (res != 0) {
			uriFreeUriMembersW(&testUri);
			uriFreeUriMembersW(&expectedUri);
			return false;
		}

		// First run
		res = uriNormalizeSyntaxExW(&testUri, mask);
		if (res != 0) {
			uriFreeUriMembersW(&testUri);
			uriFreeUriMembersW(&expectedUri);
			return false;
		}

		bool equalAfter = (URI_TRUE == uriEqualsUriW(&testUri, &expectedUri));

		// Second run
		res = uriNormalizeSyntaxExW(&testUri, mask);
		if (res != 0) {
			uriFreeUriMembersW(&testUri);
			uriFreeUriMembersW(&expectedUri);
			return false;
		}

		equalAfter = equalAfter
			&& (URI_TRUE == uriEqualsUriW(&testUri, &expectedUri));

		uriFreeUriMembersW(&testUri);
		uriFreeUriMembersW(&expectedUri);
		return equalAfter;
	}

	TEST_METHOD(testNormalizeSyntax) {
		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"eXAMPLE://a/./b/../b/%63/%7bfoo%7d",
			L"example://a/b/c/%7Bfoo%7D"));

		// Testcase by Adrian Manrique
		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"http://examp%4Ce.com/",
			L"http://example.com/"));

		// Testcase by Adrian Manrique
		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"http://example.com/a/b/%2E%2E/",
			L"http://example.com/a/"));

		// Reported by Adrian Manrique
		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"http://user:pass@SOMEHOST.COM:123",
			L"http://user:pass@somehost.com:123"));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"HTTP://a:b@HOST:123/./1/2/../%41?abc#def",
			L"http://a:b@host:123/1/A?abc#def"));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"../../abc",
			L"../../abc"));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"../../abc/..",
			L"../../"));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"../../abc/../def",
			L"../../def"));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"abc/..",
			L""));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"abc/../",
			L""));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"../../abc/./def",
			L"../../abc/def"));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"./def",
			L"def"));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"def/.",
			L"def/"));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"./abc:def",
			L"./abc:def"));
	}

	TEST_METHOD(testNormalizeSyntaxComponents) {
		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"HTTP://%41@EXAMPLE.ORG/../a?%41#%41",
			L"http://%41@EXAMPLE.ORG/../a?%41#%41",
			URI_NORMALIZE_SCHEME));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"HTTP://%41@EXAMPLE.ORG/../a?%41#%41",
			L"HTTP://A@EXAMPLE.ORG/../a?%41#%41",
			URI_NORMALIZE_USER_INFO));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"HTTP://%41@EXAMPLE.ORG/../a?%41#%41",
			L"HTTP://%41@example.org/../a?%41#%41",
			URI_NORMALIZE_HOST));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"HTTP://%41@EXAMPLE.ORG/../a?%41#%41",
			L"HTTP://%41@EXAMPLE.ORG/a?%41#%41",
			URI_NORMALIZE_PATH));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"HTTP://%41@EXAMPLE.ORG/../a?%41#%41",
			L"HTTP://%41@EXAMPLE.ORG/../a?A#%41",
			URI_NORMALIZE_QUERY));

		Assert::IsTrue(testNormalizeSyntaxHelper(
			L"HTTP://%41@EXAMPLE.ORG/../a?%41#%41",
			L"HTTP://%41@EXAMPLE.ORG/../a?%41#A",
			URI_NORMALIZE_FRAGMENT));
	}

	TEST_METHOD(testNormalizeCrash_Bug20080224) {
		UriParserStateW stateW;
		int res;
		UriUriW testUri;
		stateW.uri = &testUri;

		res = uriParseUriW(&stateW, L"http://example.org/abc//../def");
		Assert::IsTrue(res == 0);

		// First call will make us owner of copied memory
		res = uriNormalizeSyntaxExW(&testUri, URI_NORMALIZE_SCHEME);
		Assert::IsTrue(res == 0);
		res = uriNormalizeSyntaxExW(&testUri, URI_NORMALIZE_HOST);
		Assert::IsTrue(res == 0);

		// Frees empty path segment -> crash
		res = uriNormalizeSyntaxW(&testUri);
		Assert::IsTrue(res == 0);

		uriFreeUriMembersW(&testUri);
	}

	void testFilenameUriConversionHelper(const wchar_t * filename,
										 const wchar_t * uriString, bool forUnix) {
		const int prefixLen = forUnix ? 7 : 8;

		// Filename to URI string
		const size_t uriBufferLen = prefixLen + 3 * wcslen(filename) + 1;
		wchar_t * uriBuffer = new wchar_t[uriBufferLen];
		if (forUnix) {
			uriUnixFilenameToUriStringW(filename, uriBuffer);
		} else {
			uriWindowsFilenameToUriStringW(filename, uriBuffer);
		}
#ifdef HAVE_WPRINTF
		// wprintf(L"1 [%s][%s]\n", uriBuffer, uriString);
#endif
		Assert::IsTrue(!wcscmp(uriBuffer, uriString));
		delete[] uriBuffer;

		// URI string to filename
		const size_t filenameBufferLen = wcslen(uriString) + 1;
		wchar_t * filenameBuffer = new wchar_t[filenameBufferLen];
		if (forUnix) {
			uriUriStringToUnixFilenameW(uriString, filenameBuffer);
		} else {
			uriUriStringToWindowsFilenameW(uriString, filenameBuffer);
		}
#ifdef HAVE_WPRINTF
		// wprintf(L"2 [%s][%s]\n", filenameBuffer, filename);
#endif
		Assert::IsTrue(!wcscmp(filenameBuffer, filename));
		delete[] filenameBuffer;
	}

	TEST_METHOD(testFilenameUriConversion) {
		const bool FOR_UNIX = true;
		const bool FOR_WINDOWS = false;
		testFilenameUriConversionHelper(L"/bin/bash", L"file:///bin/bash", FOR_UNIX);
		testFilenameUriConversionHelper(L"./configure", L"./configure", FOR_UNIX);

		testFilenameUriConversionHelper(L"E:\\Documents and Settings", L"file:///E:/Documents%20and%20Settings", FOR_WINDOWS);
		testFilenameUriConversionHelper(L".\\Readme.txt", L"./Readme.txt", FOR_WINDOWS);

		testFilenameUriConversionHelper(L"index.htm", L"index.htm", FOR_WINDOWS);
		testFilenameUriConversionHelper(L"index.htm", L"index.htm", FOR_UNIX);

		testFilenameUriConversionHelper(L"abc def", L"abc%20def", FOR_WINDOWS);
		testFilenameUriConversionHelper(L"abc def", L"abc%20def", FOR_UNIX);

		testFilenameUriConversionHelper(L"\\\\Server01\\user\\docs\\Letter.txt", L"file://Server01/user/docs/Letter.txt", FOR_WINDOWS);
	}

	TEST_METHOD(testCrash_FreeUriMembers_Bug20080116) {
		// Testcase by Adrian Manrique
		UriParserStateA state;
		UriUriA uri;
		state.uri = &uri;
		uriParseUriA(&state, "http://test/?");
		uriNormalizeSyntaxA(&uri);
		uriFreeUriMembersA(&uri);

		Assert::IsTrue(true);
	}

	TEST_METHOD(testCrash_Report2418192) {
		// Testcase by Harvey Vrsalovic
		helperTestQueryString("http://svcs.cnn.com/weather/wrapper.jsp?&csiID=csi1", 1);
	}

	TEST_METHOD(testPervertedQueryString) {
		helperTestQueryString("http://example.org/?&&=&&&=&&&&==&===&====", 5);
	}

	void helperTestQueryString(char const * uriString, int pairsExpected) {
		UriParserStateA state;
		UriUriA uri;
		state.uri = &uri;
		int res = uriParseUriA(&state, uriString);
		Assert::IsTrue(res == URI_SUCCESS);

		UriQueryListA * queryList = NULL;
		int itemCount = 0;

		res = uriDissectQueryMallocA(&queryList, &itemCount,
									 uri.query.first, uri.query.afterLast);
		Assert::IsTrue(res == URI_SUCCESS);
		Assert::IsTrue(queryList != NULL);
		Assert::IsTrue(itemCount == pairsExpected);
		uriFreeQueryListA(queryList);
		uriFreeUriMembersA(&uri);
	}

	TEST_METHOD(testCrash_MakeOwner_Bug20080207) {
		// Testcase by Adrian Manrique
		UriParserStateA state;
		UriUriA sourceUri;
		state.uri = &sourceUri;
		const char * const sourceUriString = "http://user:pass@somehost.com:80/";
		if (uriParseUriA(&state, sourceUriString) != 0) {
			Assert::IsTrue(false);
		}
		if (uriNormalizeSyntaxA(&sourceUri) != 0) {
			Assert::IsTrue(false);
		}
		uriFreeUriMembersA(&sourceUri);
		Assert::IsTrue(true);
	}

	void testQueryListHelper(const wchar_t * input, int expectedItemCount) {
		int res;

		UriBool spacePlusConversion = URI_TRUE;
		UriBool normalizeBreaks = URI_FALSE;
		UriBreakConversion breakConversion = URI_BR_DONT_TOUCH;

		int itemCount;
		UriQueryListW * queryList;
		res = uriDissectQueryMallocExW(&queryList, &itemCount,
									   input, input + wcslen(input), spacePlusConversion, breakConversion);
		Assert::IsTrue(res == URI_SUCCESS);
		Assert::IsTrue(itemCount == expectedItemCount);
		Assert::IsTrue((queryList == NULL) == (expectedItemCount == 0));

		if (expectedItemCount != 0) {
			// First
			int charsRequired;
			res = uriComposeQueryCharsRequiredExW(queryList, &charsRequired, spacePlusConversion,
												  normalizeBreaks);
			Assert::IsTrue(res == URI_SUCCESS);
			Assert::IsTrue(charsRequired >= (int)wcslen(input));

			wchar_t * recomposed = new wchar_t[charsRequired + 1];
			int charsWritten;
			res = uriComposeQueryExW(recomposed, queryList, charsRequired + 1,
									 &charsWritten, spacePlusConversion, normalizeBreaks);
			Assert::IsTrue(res == URI_SUCCESS);
			Assert::IsTrue(charsWritten <= charsRequired);
			Assert::IsTrue(charsWritten == (int)wcslen(input) + 1);
			Assert::IsTrue(!wcscmp(input, recomposed));
			delete[] recomposed;

			recomposed = NULL;
			res = uriComposeQueryMallocW(&recomposed, queryList);
			Assert::IsTrue(res == URI_SUCCESS);
			Assert::IsTrue(recomposed != NULL);
			Assert::IsTrue(charsWritten == (int)wcslen(input) + 1);
			Assert::IsTrue(!wcscmp(input, recomposed));
			free(recomposed);
		}

		uriFreeQueryListW(queryList);
	}

	TEST_METHOD(testQueryList) {
		testQueryListHelper(L"one=ONE&two=TWO", 2);
		testQueryListHelper(L"one=ONE&two=&three=THREE", 3);
		testQueryListHelper(L"one=ONE&two&three=THREE", 3);
		testQueryListHelper(L"one=ONE", 1);
		testQueryListHelper(L"one", 1);
		testQueryListHelper(L"", 0);
	}

	void testQueryListPairHelper(const char * pair, const char * unescapedKey,
								 const char * unescapedValue, const char * fixed = NULL) {
		int res;
		UriQueryListA * queryList;
		int itemCount;

		res = uriDissectQueryMallocA(&queryList, &itemCount, pair, pair + strlen(pair));
		Assert::IsTrue(res == URI_SUCCESS);
		Assert::IsTrue(queryList != NULL);
		Assert::IsTrue(itemCount == 1);
		Assert::IsTrue(!strcmp(queryList->key, unescapedKey));
		Assert::IsTrue(!strcmp(queryList->value, unescapedValue));

		char * recomposed;
		res = uriComposeQueryMallocA(&recomposed, queryList);
		Assert::IsTrue(res == URI_SUCCESS);
		Assert::IsTrue(recomposed != NULL);
		Assert::IsTrue(!strcmp(recomposed, (fixed != NULL) ? fixed : pair));
		free(recomposed);
		uriFreeQueryListA(queryList);
	}

	TEST_METHOD(testQueryListPair) {
		testQueryListPairHelper("one+two+%26+three=%2B", "one two & three", "+");
		testQueryListPairHelper("one=two=three", "one", "two=three", "one=two%3Dthree");
		testQueryListPairHelper("one=two=three=four", "one", "two=three=four", "one=two%3Dthree%3Dfour");
	}

	TEST_METHOD(testQueryDissection_Bug3590761) {
		int res;
		UriQueryListA * queryList;
		int itemCount;
		const char * const pair = "q=hello&x=&y=";

		res = uriDissectQueryMallocA(&queryList, &itemCount, pair, pair + strlen(pair));
		Assert::IsTrue(res == URI_SUCCESS);
		Assert::IsTrue(queryList != NULL);
		Assert::IsTrue(itemCount == 3);

		Assert::IsTrue(!strcmp(queryList->key, "q"));
		Assert::IsTrue(!strcmp(queryList->value, "hello"));

		Assert::IsTrue(!strcmp(queryList->next->key, "x"));
		Assert::IsTrue(!strcmp(queryList->next->value, ""));

		Assert::IsTrue(!strcmp(queryList->next->next->key, "y"));
		Assert::IsTrue(!strcmp(queryList->next->next->value, ""));

		Assert::IsTrue(!queryList->next->next->next);

		uriFreeQueryListA(queryList);
	}

	TEST_METHOD(testFreeCrash_Bug20080827) {
		char const * const sourceUri = "abc";
		char const * const baseUri = "http://www.example.org/";

		int res;
		UriParserStateA state;
		UriUriA absoluteDest;
		UriUriA relativeSource;
		UriUriA absoluteBase;

		state.uri = &relativeSource;
		res = uriParseUriA(&state, sourceUri);
		Assert::IsTrue(res == URI_SUCCESS);

		state.uri = &absoluteBase;
		res = uriParseUriA(&state, baseUri);
		Assert::IsTrue(res == URI_SUCCESS);

		res = uriRemoveBaseUriA(&absoluteDest, &relativeSource, &absoluteBase, URI_FALSE);
		Assert::IsTrue(res == URI_ERROR_REMOVEBASE_REL_SOURCE);

		uriFreeUriMembersA(&relativeSource);
		uriFreeUriMembersA(&absoluteBase);
		uriFreeUriMembersA(&absoluteDest); // Crashed here
	}

	TEST_METHOD(testParseInvalid_Bug16) {
		UriParserStateA stateA;
		UriUriA uriA;
		stateA.uri = &uriA;
		const char * const input = "A>B";

		const int res = uriParseUriA(&stateA, input);

		Assert::IsTrue(res == URI_ERROR_SYNTAX);
		Assert::IsTrue(stateA.errorPos == input + 1);
		Assert::IsTrue(stateA.errorCode == URI_ERROR_SYNTAX);  /* failed previously */

		uriFreeUriMembersA(&uriA);
	}

	void testEqualsHelper(const char * uri_to_test) {
		UriParserStateA state;
		UriUriA uriOne;
		UriUriA uriTwo;
		state.uri = &uriOne;
		Assert::IsTrue(URI_SUCCESS == uriParseUriA(&state, uri_to_test));
		state.uri = &uriTwo;
		Assert::IsTrue(URI_SUCCESS == uriParseUriA(&state, uri_to_test));
		Assert::IsTrue(URI_TRUE == uriEqualsUriA(&uriOne, &uriTwo));
		uriFreeUriMembersA(&uriOne);
		uriFreeUriMembersA(&uriTwo);
	}

	TEST_METHOD(testEquals) {
		testEqualsHelper("http://host");
		testEqualsHelper("http://host:123");
		testEqualsHelper("http://foo:bar@host:123");
		testEqualsHelper("http://foo:bar@host:123/");
		testEqualsHelper("http://foo:bar@host:123/path");
		testEqualsHelper("http://foo:bar@host:123/path?query");
		testEqualsHelper("http://foo:bar@host:123/path?query#fragment");

		testEqualsHelper("path");
		testEqualsHelper("/path");
		testEqualsHelper("/path/");
		testEqualsHelper("//path/");
		testEqualsHelper("//host");
		testEqualsHelper("//host:123");
	}

	void testCompareRangeHelper(const char * a, const char * b, int expected, bool avoidNullRange = true) {
		UriTextRangeA ra;
		UriTextRangeA rb;

		if (a) {
			ra.first = a;
			ra.afterLast = a + strlen(a);
		} else {
			ra.first = NULL;
			ra.afterLast = NULL;
		}

		if (b) {
			rb.first = b;
			rb.afterLast = b + strlen(b);
		} else {
			rb.first = NULL;
			rb.afterLast = NULL;
		}

		const int received = uriCompareRangeA(
			((a == NULL) && avoidNullRange) ? NULL : &ra,
			((b == NULL) && avoidNullRange) ? NULL : &rb);
		if (received != expected) {
			printf("Comparing <%s> to <%s> yields %d, expected %d.\n",
				   a, b, received, expected);
		}
		Assert::IsTrue(received == expected);
	}

	TEST_METHOD(testRangeComparison) {
		testCompareRangeHelper("", "", 0);
		testCompareRangeHelper("a", "", 1);
		testCompareRangeHelper("", "a", -1);

		testCompareRangeHelper("a", "a", 0);
		testCompareRangeHelper("a", "b", -1);
		testCompareRangeHelper("b", "a", 1);

		testCompareRangeHelper("a", "aa", -1);
		testCompareRangeHelper("aa", "a", 1);

		// Fixed with 0.8.1:
		testCompareRangeHelper(NULL, "a", -1);
		testCompareRangeHelper("a", NULL, 1);
		testCompareRangeHelper(NULL, NULL, 0);

		// Fixed with 0.8.3
		const bool KEEP_NULL_RANGE = false;
		const bool AVOID_NULL_RANGE = true;
		testCompareRangeHelper(NULL, "", -1, AVOID_NULL_RANGE);
		testCompareRangeHelper(NULL, "", -1, KEEP_NULL_RANGE);
		testCompareRangeHelper("", NULL, 1, AVOID_NULL_RANGE);
		testCompareRangeHelper("", NULL, 1, KEEP_NULL_RANGE);
	}

	bool testAddOrRemoveBaseHelper(const char * ref, const char * base,
								   const char * expected, bool add) {
		return testAddOrRemoveBaseHelper(ref,base,expected,add,false);
	}

	bool testAddOrRemoveBaseHelper(const char * ref, const char * base,
									const char * expected) {
		return testAddOrRemoveBaseHelper(ref,base,expected,true,false);
	}

	bool testAddOrRemoveBaseHelper(const char * ref, const char * base,
							const char * expected, bool add, bool domainRootMode) {
		UriParserStateA stateA;

		// Base
		UriUriA baseUri;
		stateA.uri = &baseUri;
		int res = uriParseUriA(&stateA, base);
		if (res != 0) {
			return false;
		}

		// Rel
		UriUriA relUri;
		stateA.uri = &relUri;
		res = uriParseUriA(&stateA, ref);
		if (res != 0) {
			uriFreeUriMembersA(&baseUri);
			return false;
		}

		// Expected result
		UriUriA expectedUri;
		stateA.uri = &expectedUri;
		res = uriParseUriA(&stateA, expected);
		if (res != 0) {
			uriFreeUriMembersA(&baseUri);
			uriFreeUriMembersA(&relUri);
			uriFreeUriMembersA(&expectedUri);
			return false;
		}

		// Transform
		UriUriA transformedUri;
		if (add) {
			res = uriAddBaseUriA(&transformedUri, &relUri, &baseUri);
		} else {
			res = uriRemoveBaseUriA(&transformedUri, &relUri, &baseUri,	domainRootMode ? URI_TRUE : URI_FALSE);
		}
		if (res != 0) {
			uriFreeUriMembersA(&baseUri);
			uriFreeUriMembersA(&relUri);
			uriFreeUriMembersA(&expectedUri);
			uriFreeUriMembersA(&transformedUri);
			return false;
		}

		const bool equal = (URI_TRUE == uriEqualsUriA(&transformedUri, &expectedUri));
		if (!equal) {
			char transformedUriText[1024 * 8];
			char expectedUriText[1024 * 8];
			uriToStringA(transformedUriText, &transformedUri, 1024 * 8, NULL);
			uriToStringA(expectedUriText, &expectedUri, 1024 * 8, NULL);
			printf("\n\n\nExpected: \"%s\"\nReceived: \"%s\"\n\n\n", expectedUriText, transformedUriText);
		}

		uriFreeUriMembersA(&baseUri);
		uriFreeUriMembersA(&relUri);
		uriFreeUriMembersA(&expectedUri);
		uriFreeUriMembersA(&transformedUri);
		return equal;
	}

	TEST_METHOD(absolutize_test_cases) {
		const char * const BASE_URI[] = {
			"http://a/b/c/d;p?q",
			"http://a/b/c/d;p?q=1/2",
			"http://a/b/c/d;p=1/2?q",
			"fred:///s//a/b/c",
			"http:///s//a/b/c" };

		// ref, base, exptected

		// http://lists.w3.org/Archives/Public/uri/2004Feb/0114.html
		Assert::IsTrue(testAddOrRemoveBaseHelper("../c", "foo:a/b", "foo:c"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("foo:.", "foo:a", "foo:"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/foo/../../../bar", "zz:abc", "zz:/bar"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/foo/../bar", "zz:abc", "zz:/bar"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("foo/../../../bar", "zz:abc", "zz:bar"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("foo/../bar", "zz:abc", "zz:bar"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("zz:.", "zz:abc", "zz:"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/.", BASE_URI[0], "http://a/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/.foo", BASE_URI[0], "http://a/.foo"));
		Assert::IsTrue(testAddOrRemoveBaseHelper(".foo", BASE_URI[0], "http://a/b/c/.foo"));

		// http://gbiv.com/protocols/uri/test/rel_examples1.html
		// examples from RFC 2396
		Assert::IsTrue(testAddOrRemoveBaseHelper("g:h", BASE_URI[0], "g:h"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g", BASE_URI[0], "http://a/b/c/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./g", BASE_URI[0], "http://a/b/c/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g/", BASE_URI[0], "http://a/b/c/g/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/g", BASE_URI[0], "http://a/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("//g", BASE_URI[0], "http://g"));

		// changed with RFC 2396bis
		Assert::IsTrue(testAddOrRemoveBaseHelper("?y", BASE_URI[0], "http://a/b/c/d;p?y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g?y", BASE_URI[0], "http://a/b/c/g?y"));

		// changed with RFC 2396bis
		Assert::IsTrue(testAddOrRemoveBaseHelper("#s", BASE_URI[0], "http://a/b/c/d;p?q#s"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g#s", BASE_URI[0], "http://a/b/c/g#s"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g?y#s", BASE_URI[0], "http://a/b/c/g?y#s"));
		Assert::IsTrue(testAddOrRemoveBaseHelper(";x", BASE_URI[0], "http://a/b/c/;x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g;x", BASE_URI[0], "http://a/b/c/g;x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g;x?y#s", BASE_URI[0], "http://a/b/c/g;x?y#s"));

		// changed with RFC 2396bis
		Assert::IsTrue(testAddOrRemoveBaseHelper("", BASE_URI[0], "http://a/b/c/d;p?q"));
		Assert::IsTrue(testAddOrRemoveBaseHelper(".", BASE_URI[0], "http://a/b/c/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./", BASE_URI[0], "http://a/b/c/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("..", BASE_URI[0], "http://a/b/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../", BASE_URI[0], "http://a/b/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../g", BASE_URI[0], "http://a/b/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../..", BASE_URI[0], "http://a/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../", BASE_URI[0], "http://a/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../g", BASE_URI[0], "http://a/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../../g", BASE_URI[0], "http://a/g")); // http://a/../g
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../../../g", BASE_URI[0], "http://a/g")); // http://a/../../g

																							// changed with RFC 2396bis
		Assert::IsTrue(testAddOrRemoveBaseHelper("/./g", BASE_URI[0], "http://a/g"));

		// changed with RFC 2396bis
		Assert::IsTrue(testAddOrRemoveBaseHelper("/../g", BASE_URI[0], "http://a/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g.", BASE_URI[0], "http://a/b/c/g."));
		Assert::IsTrue(testAddOrRemoveBaseHelper(".g", BASE_URI[0], "http://a/b/c/.g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g..", BASE_URI[0], "http://a/b/c/g.."));
		Assert::IsTrue(testAddOrRemoveBaseHelper("..g", BASE_URI[0], "http://a/b/c/..g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./../g", BASE_URI[0], "http://a/b/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./g/.", BASE_URI[0], "http://a/b/c/g/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g/./h", BASE_URI[0], "http://a/b/c/g/h"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g/../h", BASE_URI[0], "http://a/b/c/h"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g;x=1/./y", BASE_URI[0], "http://a/b/c/g;x=1/y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g;x=1/../y", BASE_URI[0], "http://a/b/c/y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g?y/./x", BASE_URI[0], "http://a/b/c/g?y/./x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g?y/../x", BASE_URI[0], "http://a/b/c/g?y/../x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g#s/./x", BASE_URI[0], "http://a/b/c/g#s/./x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g#s/../x", BASE_URI[0], "http://a/b/c/g#s/../x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("http:g", BASE_URI[0], "http:g")); // http://a/b/c/g
		Assert::IsTrue(testAddOrRemoveBaseHelper("http:", BASE_URI[0], "http:")); // BASE_URI[0]

																			   // not sure where this one originated
		Assert::IsTrue(testAddOrRemoveBaseHelper("/a/b/c/./../../g", BASE_URI[0], "http://a/a/g"));

		// http://gbiv.com/protocols/uri/test/rel_examples2.html
		// slashes in base URI's query args
		Assert::IsTrue(testAddOrRemoveBaseHelper("g", BASE_URI[1], "http://a/b/c/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./g", BASE_URI[1], "http://a/b/c/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g/", BASE_URI[1], "http://a/b/c/g/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/g", BASE_URI[1], "http://a/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("//g", BASE_URI[1], "http://g"));

		// changed in RFC 2396bis
		// Assert::IsTrue(testAddOrRemoveBaseHelper("?y", BASE_URI[1], "http://a/b/c/?y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("?y", BASE_URI[1], "http://a/b/c/d;p?y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g?y", BASE_URI[1], "http://a/b/c/g?y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g?y/./x", BASE_URI[1], "http://a/b/c/g?y/./x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g?y/../x", BASE_URI[1], "http://a/b/c/g?y/../x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g#s", BASE_URI[1], "http://a/b/c/g#s"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g#s/./x", BASE_URI[1], "http://a/b/c/g#s/./x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g#s/../x", BASE_URI[1], "http://a/b/c/g#s/../x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./", BASE_URI[1], "http://a/b/c/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../", BASE_URI[1], "http://a/b/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../g", BASE_URI[1], "http://a/b/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../", BASE_URI[1], "http://a/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../g", BASE_URI[1], "http://a/g"));

		// http://gbiv.com/protocols/uri/test/rel_examples3.html
		// slashes in path params
		// all of these changed in RFC 2396bis
		Assert::IsTrue(testAddOrRemoveBaseHelper("g", BASE_URI[2], "http://a/b/c/d;p=1/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./g", BASE_URI[2], "http://a/b/c/d;p=1/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g/", BASE_URI[2], "http://a/b/c/d;p=1/g/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g?y", BASE_URI[2], "http://a/b/c/d;p=1/g?y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper(";x", BASE_URI[2], "http://a/b/c/d;p=1/;x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g;x", BASE_URI[2], "http://a/b/c/d;p=1/g;x"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g;x=1/./y", BASE_URI[2], "http://a/b/c/d;p=1/g;x=1/y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g;x=1/../y", BASE_URI[2], "http://a/b/c/d;p=1/y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./", BASE_URI[2], "http://a/b/c/d;p=1/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../", BASE_URI[2], "http://a/b/c/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../g", BASE_URI[2], "http://a/b/c/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../", BASE_URI[2], "http://a/b/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../g", BASE_URI[2], "http://a/b/g"));

		// http://gbiv.com/protocols/uri/test/rel_examples4.html
		// double and triple slash, unknown scheme
		Assert::IsTrue(testAddOrRemoveBaseHelper("g:h", BASE_URI[3], "g:h"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g", BASE_URI[3], "fred:///s//a/b/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./g", BASE_URI[3], "fred:///s//a/b/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g/", BASE_URI[3], "fred:///s//a/b/g/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/g", BASE_URI[3], "fred:///g")); // may change to fred:///s//a/g
		Assert::IsTrue(testAddOrRemoveBaseHelper("//g", BASE_URI[3], "fred://g")); // may change to fred:///s//g
		Assert::IsTrue(testAddOrRemoveBaseHelper("//g/x", BASE_URI[3], "fred://g/x")); // may change to fred:///s//g/x
		Assert::IsTrue(testAddOrRemoveBaseHelper("///g", BASE_URI[3], "fred:///g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./", BASE_URI[3], "fred:///s//a/b/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../", BASE_URI[3], "fred:///s//a/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../g", BASE_URI[3], "fred:///s//a/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../", BASE_URI[3], "fred:///s//")); // may change to fred:///s//a/../
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../g", BASE_URI[3], "fred:///s//g")); // may change to fred:///s//a/../g
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../../g", BASE_URI[3], "fred:///s/g")); // may change to fred:///s//a/../../g
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../../../g", BASE_URI[3], "fred:///g")); // may change to fred:///s//a/../../../g

																						   // http://gbiv.com/protocols/uri/test/rel_examples5.html
																						   // double and triple slash, well-known scheme
		Assert::IsTrue(testAddOrRemoveBaseHelper("g:h", BASE_URI[4], "g:h"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g", BASE_URI[4], "http:///s//a/b/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./g", BASE_URI[4], "http:///s//a/b/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("g/", BASE_URI[4], "http:///s//a/b/g/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/g", BASE_URI[4], "http:///g")); // may change to http:///s//a/g
		Assert::IsTrue(testAddOrRemoveBaseHelper("//g", BASE_URI[4], "http://g")); // may change to http:///s//g
		Assert::IsTrue(testAddOrRemoveBaseHelper("//g/x", BASE_URI[4], "http://g/x")); // may change to http:///s//g/x
		Assert::IsTrue(testAddOrRemoveBaseHelper("///g", BASE_URI[4], "http:///g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./", BASE_URI[4], "http:///s//a/b/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../", BASE_URI[4], "http:///s//a/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../g", BASE_URI[4], "http:///s//a/g"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../", BASE_URI[4], "http:///s//")); // may change to http:///s//a/../
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../g", BASE_URI[4], "http:///s//g")); // may change to http:///s//a/../g
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../../g", BASE_URI[4], "http:///s/g")); // may change to http:///s//a/../../g
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../../../g", BASE_URI[4], "http:///g")); // may change to http:///s//a/../../../g

																						   // from Dan Connelly's tests in http://www.w3.org/2000/10/swap/uripath.py
		Assert::IsTrue(testAddOrRemoveBaseHelper("bar:abc", "foo:xyz", "bar:abc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../abc", "http://example/x/y/z", "http://example/x/abc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("http://example/x/abc", "http://example2/x/y/z", "http://example/x/abc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../r", "http://ex/x/y/z", "http://ex/x/r"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("q/r", "http://ex/x/y", "http://ex/x/q/r"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("q/r#s", "http://ex/x/y", "http://ex/x/q/r#s"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("q/r#s/t", "http://ex/x/y", "http://ex/x/q/r#s/t"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("ftp://ex/x/q/r", "http://ex/x/y", "ftp://ex/x/q/r"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("", "http://ex/x/y", "http://ex/x/y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("", "http://ex/x/y/", "http://ex/x/y/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("", "http://ex/x/y/pdq", "http://ex/x/y/pdq"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("z/", "http://ex/x/y/", "http://ex/x/y/z/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("#Animal", "file:/swap/test/animal.rdf", "file:/swap/test/animal.rdf#Animal"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../abc", "file:/e/x/y/z", "file:/e/x/abc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/example/x/abc", "file:/example2/x/y/z", "file:/example/x/abc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../r", "file:/ex/x/y/z", "file:/ex/x/r"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/r", "file:/ex/x/y/z", "file:/r"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("q/r", "file:/ex/x/y", "file:/ex/x/q/r"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("q/r#s", "file:/ex/x/y", "file:/ex/x/q/r#s"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("q/r#", "file:/ex/x/y", "file:/ex/x/q/r#"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("q/r#s/t", "file:/ex/x/y", "file:/ex/x/q/r#s/t"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("ftp://ex/x/q/r", "file:/ex/x/y", "ftp://ex/x/q/r"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("", "file:/ex/x/y", "file:/ex/x/y"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("", "file:/ex/x/y/", "file:/ex/x/y/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("", "file:/ex/x/y/pdq", "file:/ex/x/y/pdq"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("z/", "file:/ex/x/y/", "file:/ex/x/y/z/"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("file://meetings.example.com/cal#m1", "file:/devel/WWW/2000/10/swap/test/reluri-1.n3", "file://meetings.example.com/cal#m1"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("file://meetings.example.com/cal#m1", "file:/home/connolly/w3ccvs/WWW/2000/10/swap/test/reluri-1.n3", "file://meetings.example.com/cal#m1"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./#blort", "file:/some/dir/foo", "file:/some/dir/#blort"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./#", "file:/some/dir/foo", "file:/some/dir/#"));

		// Ryan Lee
		Assert::IsTrue(testAddOrRemoveBaseHelper("./", "http://example/x/abc.efg", "http://example/x/"));

		// Graham Klyne's tests
		// http://www.ninebynine.org/Software/HaskellUtils/Network/UriTest.xls
		// 01-31 are from Connelly's cases

		// 32-49
		Assert::IsTrue(testAddOrRemoveBaseHelper("./q:r", "http://ex/x/y", "http://ex/x/q:r"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./p=q:r", "http://ex/x/y", "http://ex/x/p=q:r"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("?pp/rr", "http://ex/x/y?pp/qq", "http://ex/x/y?pp/rr"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("y/z", "http://ex/x/y?pp/qq", "http://ex/x/y/z"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("local/qual@domain.org#frag", "mailto:local", "mailto:local/qual@domain.org#frag"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("more/qual2@domain2.org#frag", "mailto:local/qual1@domain1.org", "mailto:local/more/qual2@domain2.org#frag"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("y?q", "http://ex/x/y?q", "http://ex/x/y?q"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/x/y?q", "http://ex?p", "http://ex/x/y?q"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("c/d", "foo:a/b", "foo:a/c/d"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/c/d", "foo:a/b", "foo:/c/d"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("", "foo:a/b?c#d", "foo:a/b?c"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("b/c", "foo:a", "foo:b/c"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../b/c", "foo:/a/y/z", "foo:/a/b/c"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("./b/c", "foo:a", "foo:b/c"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/./b/c", "foo:a", "foo:/b/c"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../d", "foo://a//b/c", "foo://a/d"));
		Assert::IsTrue(testAddOrRemoveBaseHelper(".", "foo:a", "foo:"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("..", "foo:a", "foo:"));

		// 50-57 (cf. TimBL comments --
		// http://lists.w3.org/Archives/Public/uri/2003Feb/0028.html,
		// http://lists.w3.org/Archives/Public/uri/2003Jan/0008.html)
		Assert::IsTrue(testAddOrRemoveBaseHelper("abc", "http://example/x/y%2Fz", "http://example/x/abc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../../x%2Fabc", "http://example/a/x/y/z", "http://example/a/x%2Fabc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../x%2Fabc", "http://example/a/x/y%2Fz", "http://example/a/x%2Fabc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("abc", "http://example/x%2Fy/z", "http://example/x%2Fy/abc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("q%3Ar", "http://ex/x/y", "http://ex/x/q%3Ar"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/x%2Fabc", "http://example/x/y%2Fz", "http://example/x%2Fabc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/x%2Fabc", "http://example/x/y/z", "http://example/x%2Fabc"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("/x%2Fabc", "http://example/x/y%2Fz", "http://example/x%2Fabc"));

		// 70-77
		Assert::IsTrue(testAddOrRemoveBaseHelper("local2@domain2", "mailto:local1@domain1?query1", "mailto:local2@domain2"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("local2@domain2?query2", "mailto:local1@domain1", "mailto:local2@domain2?query2"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("local2@domain2?query2", "mailto:local1@domain1?query1", "mailto:local2@domain2?query2"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("?query2", "mailto:local@domain?query1", "mailto:local@domain?query2"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("local@domain?query2", "mailto:?query1", "mailto:local@domain?query2"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("?query2", "mailto:local@domain?query1", "mailto:local@domain?query2"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("http://example/a/b?c/../d", "foo:bar", "http://example/a/b?c/../d"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("http://example/a/b#c/../d", "foo:bar", "http://example/a/b#c/../d"));

		// 82-88
		Assert::IsTrue(testAddOrRemoveBaseHelper("http:this", "http://example.org/base/uri", "http:this"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("http:this", "http:base", "http:this"));
		// Whole in the URI spec, see http://lists.w3.org/Archives/Public/uri/2007Aug/0003.html
		// Assert::IsTrue(testAddOrRemoveBaseHelper(".//g", "f:/a", "f://g")); // ORIGINAL
		Assert::IsTrue(testAddOrRemoveBaseHelper(".//g", "f:/a", "f:/.//g")); // FIXED ONE
		Assert::IsTrue(testAddOrRemoveBaseHelper("b/c//d/e", "f://example.org/base/a", "f://example.org/base/b/c//d/e"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("m2@example.ord/c2@example.org", "mid:m@example.ord/c@example.org", "mid:m@example.ord/m2@example.ord/c2@example.org"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("mini1.xml", "file:///C:/DEV/Haskell/lib/HXmlToolbox-3.01/examples/", "file:///C:/DEV/Haskell/lib/HXmlToolbox-3.01/examples/mini1.xml"));
		Assert::IsTrue(testAddOrRemoveBaseHelper("../b/c", "foo:a/y/z", "foo:a/b/c"));
	}



	TEST_METHOD(relativize_test_cases) {
		const bool REMOVE_MODE = false;
		const bool DOMAIN_ROOT_MODE = true;

		// to convert, base, exptected

		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c", "s://ex/a/d", "b/c", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/b/b/c", "s://ex/a/d", "/b/b/c", REMOVE_MODE, DOMAIN_ROOT_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c", "s://ex/a/b/", "c", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://other.ex/a/b/", "s://ex/a/d", "//other.ex/a/b/", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c", "s://other.ex/a/d", "//ex/a/b/c", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("t://ex/a/b/c", "s://ex/a/d", "t://ex/a/b/c", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c", "t://ex/a/d", "s://ex/a/b/c", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a", "s://ex/b/c/d", "/a", REMOVE_MODE, DOMAIN_ROOT_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/b/c/d", "s://ex/a", "b/c/d", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c?h", "s://ex/a/d?w", "b/c?h", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c#h", "s://ex/a/d#w", "b/c#h", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c?h#i", "s://ex/a/d?w#j", "b/c?h#i", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a#i", "s://ex/a", "#i", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a?i", "s://ex/a", "?i", REMOVE_MODE));

		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/", "s://ex/a/b/", "", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b", "s://ex/a/b", "", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/", "s://ex/", "", REMOVE_MODE));

		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c", "s://ex/a/d/c", "../b/c", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c/", "s://ex/a/d/c", "../b/c/", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c/d", "s://ex/a/d/c/d", "../../b/c/d", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/c", "s://ex/d/e/f", "/a/b/c", REMOVE_MODE, DOMAIN_ROOT_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b/", "s://ex/a/c/d/e", "../../b/", REMOVE_MODE));

		// Some tests to ensure that empty path segments don't cause problems.
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/b", "s://ex/a//b/c", "../../b", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a///b", "s://ex/a/", ".///b", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a/", "s://ex/a///b", "../../", REMOVE_MODE));
		Assert::IsTrue(testAddOrRemoveBaseHelper("s://ex/a//b/c", "s://ex/a/b", ".//b/c", REMOVE_MODE));
	}

	int testParseUri(const char * uriText, const char ** expectedErrorPos) {
		UriParserStateA state;
		UriUriA uri;
		state.uri = &uri;
		int res = uriParseUriA(&state, uriText);
		if (expectedErrorPos != NULL) {
			*expectedErrorPos = state.errorPos;
		}
		uriFreeUriMembersA(&uri);
		return res;
	}

	bool testGoodUri(const char * uriText) {
		return (testParseUri(uriText,NULL) == 0);
	}

	bool testBadUri(const char * uriText, int expectedErrorOffset) {
		const char * errorPos = NULL;
		const int ret = testParseUri(uriText, &errorPos);
		return ((ret == URI_ERROR_SYNTAX)
				&& (errorPos != NULL)
				&& (
				(expectedErrorOffset == -1)
				|| (errorPos == (uriText + expectedErrorOffset))
				));
	}

	TEST_METHOD(good_URI_references) {
		Assert::IsTrue(testGoodUri("file:///foo/bar"));
		Assert::IsTrue(testGoodUri("mailto:user@host?subject=blah"));
		Assert::IsTrue(testGoodUri("dav:")); // empty opaque part / rel-path allowed by RFC 2396bis
		Assert::IsTrue(testGoodUri("about:")); // empty opaque part / rel-path allowed by RFC 2396bis

											// the following test cases are from a Perl script by David A. Wheeler
											// at http://www.dwheeler.com/secure-programs/url.pl
		Assert::IsTrue(testGoodUri("http://www.yahoo.com"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com/"));
		Assert::IsTrue(testGoodUri("http://1.2.3.4/"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com/stuff"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com/stuff/"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com/hello%20world/"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com?name=obi"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com?name=obi+wan&status=jedi"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com?onery"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com#bottom"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com/yelp.html#bottom"));
		Assert::IsTrue(testGoodUri("https://www.yahoo.com/"));
		Assert::IsTrue(testGoodUri("ftp://www.yahoo.com/"));
		Assert::IsTrue(testGoodUri("ftp://www.yahoo.com/hello"));
		Assert::IsTrue(testGoodUri("demo.txt"));
		Assert::IsTrue(testGoodUri("demo/hello.txt"));
		Assert::IsTrue(testGoodUri("demo/hello.txt?query=hello#fragment"));
		Assert::IsTrue(testGoodUri("/cgi-bin/query?query=hello#fragment"));
		Assert::IsTrue(testGoodUri("/demo.txt"));
		Assert::IsTrue(testGoodUri("/hello/demo.txt"));
		Assert::IsTrue(testGoodUri("hello/demo.txt"));
		Assert::IsTrue(testGoodUri("/"));
		Assert::IsTrue(testGoodUri(""));
		Assert::IsTrue(testGoodUri("#"));
		Assert::IsTrue(testGoodUri("#here"));

		// Wheeler's script says these are invalid, but they aren't
		Assert::IsTrue(testGoodUri("http://www.yahoo.com?name=%00%01"));
		Assert::IsTrue(testGoodUri("http://www.yaho%6f.com"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com/hello%00world/"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com/hello+world/"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com?name=obi&"));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com?name=obi&type="));
		Assert::IsTrue(testGoodUri("http://www.yahoo.com/yelp.html#"));
		Assert::IsTrue(testGoodUri("//"));

		// the following test cases are from a Haskell program by Graham Klyne
		// at http://www.ninebynine.org/Software/HaskellUtils/Network/URITest.hs
		Assert::IsTrue(testGoodUri("http://example.org/aaa/bbb#ccc"));
		Assert::IsTrue(testGoodUri("mailto:local@domain.org"));
		Assert::IsTrue(testGoodUri("mailto:local@domain.org#frag"));
		Assert::IsTrue(testGoodUri("HTTP://EXAMPLE.ORG/AAA/BBB#CCC"));
		Assert::IsTrue(testGoodUri("//example.org/aaa/bbb#ccc"));
		Assert::IsTrue(testGoodUri("/aaa/bbb#ccc"));
		Assert::IsTrue(testGoodUri("bbb#ccc"));
		Assert::IsTrue(testGoodUri("#ccc"));
		Assert::IsTrue(testGoodUri("#"));
		Assert::IsTrue(testGoodUri("A'C"));

		// escapes
		Assert::IsTrue(testGoodUri("http://example.org/aaa%2fbbb#ccc"));
		Assert::IsTrue(testGoodUri("http://example.org/aaa%2Fbbb#ccc"));
		Assert::IsTrue(testGoodUri("%2F"));
		Assert::IsTrue(testGoodUri("aaa%2Fbbb"));

		// ports
		Assert::IsTrue(testGoodUri("http://example.org:80/aaa/bbb#ccc"));
		Assert::IsTrue(testGoodUri("http://example.org:/aaa/bbb#ccc"));
		Assert::IsTrue(testGoodUri("http://example.org./aaa/bbb#ccc"));
		Assert::IsTrue(testGoodUri("http://example.123./aaa/bbb#ccc"));

		// bare authority
		Assert::IsTrue(testGoodUri("http://example.org"));

		// IPv6 literals (from RFC2732):
		Assert::IsTrue(testGoodUri("http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html"));
		Assert::IsTrue(testGoodUri("http://[1080:0:0:0:8:800:200C:417A]/index.html"));
		Assert::IsTrue(testGoodUri("http://[3ffe:2a00:100:7031::1]"));
		Assert::IsTrue(testGoodUri("http://[1080::8:800:200C:417A]/foo"));
		Assert::IsTrue(testGoodUri("http://[::192.9.5.5]/ipng"));
		Assert::IsTrue(testGoodUri("http://[::FFFF:129.144.52.38]:80/index.html"));
		Assert::IsTrue(testGoodUri("http://[2010:836B:4179::836B:4179]"));
		Assert::IsTrue(testGoodUri("//[2010:836B:4179::836B:4179]"));

		// Random other things that crop up
		Assert::IsTrue(testGoodUri("http://example/Andr&#567;"));
		Assert::IsTrue(testGoodUri("file:///C:/DEV/Haskell/lib/HXmlToolbox-3.01/examples/"));
	}

	TEST_METHOD(bad_URI_references) {
		Assert::IsTrue(testBadUri("beepbeep\x07\x07", 8));
		Assert::IsTrue(testBadUri("\n", 0));
		Assert::IsTrue(testBadUri("::", 0)); // not OK, per Roy Fielding on the W3C uri list on 2004-04-01

										  // the following test cases are from a Perl script by David A. Wheeler
										  // at http://www.dwheeler.com/secure-programs/url.pl
		Assert::IsTrue(testBadUri("http://www yahoo.com", 10));
		Assert::IsTrue(testBadUri("http://www.yahoo.com/hello world/", 26));
		Assert::IsTrue(testBadUri("http://www.yahoo.com/yelp.html#\"", 31));

		// the following test cases are from a Haskell program by Graham Klyne
		// at http://www.ninebynine.org/Software/HaskellUtils/Network/URITest.hs
		Assert::IsTrue(testBadUri("[2010:836B:4179::836B:4179]", 0));
		Assert::IsTrue(testBadUri(" ", 0));
		Assert::IsTrue(testBadUri("%", 1));
		Assert::IsTrue(testBadUri("A%Z", 2));
		Assert::IsTrue(testBadUri("%ZZ", 1));
		Assert::IsTrue(testBadUri("%AZ", 2));
		Assert::IsTrue(testBadUri("A C", 1));
		Assert::IsTrue(testBadUri("A\\'C", 1)); // r"A\'C"
		Assert::IsTrue(testBadUri("A`C", 1));
		Assert::IsTrue(testBadUri("A<C", 1));
		Assert::IsTrue(testBadUri("A>C", 1));
		Assert::IsTrue(testBadUri("A^C", 1));
		Assert::IsTrue(testBadUri("A\\\\C", 1)); // r'A\\C'
		Assert::IsTrue(testBadUri("A{C", 1));
		Assert::IsTrue(testBadUri("A|C", 1));
		Assert::IsTrue(testBadUri("A}C", 1));
		Assert::IsTrue(testBadUri("A[C", 1));
		Assert::IsTrue(testBadUri("A]C", 1));
		Assert::IsTrue(testBadUri("A[**]C", 1));
		Assert::IsTrue(testBadUri("http://[xyz]/", 8));
		Assert::IsTrue(testBadUri("http://]/", 7));
		Assert::IsTrue(testBadUri("http://example.org/[2010:836B:4179::836B:4179]", 19));
		Assert::IsTrue(testBadUri("http://example.org/abc#[2010:836B:4179::836B:4179]", 23));
		Assert::IsTrue(testBadUri("http://example.org/xxx/[qwerty]#a[b]", 23));

		// from a post to the W3C uri list on 2004-02-17
		// breaks at 22 instead of 17 because everything up to that point is a valid userinfo
		Assert::IsTrue(testBadUri("http://w3c.org:80path1/path2", 22));
	}

	bool normalizeAndCompare(const char * uriText,
							 const char * expectedNormalized) {
		UriParserStateA stateA;
		int res;

		UriUriA testUri;
		stateA.uri = &testUri;
		res = uriParseUriA(&stateA, uriText);
		if (res != 0) {
			uriFreeUriMembersA(&testUri);
			return false;
		}

		// Expected result
		UriUriA expectedUri;
		stateA.uri = &expectedUri;
		res = uriParseUriA(&stateA, expectedNormalized);
		if (res != 0) {
			uriFreeUriMembersA(&testUri);
			uriFreeUriMembersA(&expectedUri);
			return false;
		}

		res = uriNormalizeSyntaxA(&testUri);
		if (res != 0) {
			uriFreeUriMembersA(&testUri);
			uriFreeUriMembersA(&expectedUri);
			return false;
		}

		const bool equalAfter = (URI_TRUE == uriEqualsUriA(&testUri, &expectedUri));
		uriFreeUriMembersA(&testUri);
		uriFreeUriMembersA(&expectedUri);
		return equalAfter;
	}

	TEST_METHOD(caseNormalizationTests) {
		Assert::IsTrue(normalizeAndCompare("HTTP://www.EXAMPLE.com/", "http://www.example.com/"));
		Assert::IsTrue(normalizeAndCompare("example://A/b/c/%7bfoo%7d", "example://a/b/c/%7Bfoo%7D"));
	}

	TEST_METHOD(pctEncNormalizationTests) {
		Assert::IsTrue(normalizeAndCompare("http://host/%7Euser/x/y/z", "http://host/~user/x/y/z"));
		Assert::IsTrue(normalizeAndCompare("http://host/%7euser/x/y/z", "http://host/~user/x/y/z"));
	}

	TEST_METHOD(pathSegmentNormalizationTests) {
		Assert::IsTrue(normalizeAndCompare("/a/b/../../c", "/c"));
		// Assert::IsTrue(normalizeAndCompare("a/b/../../c", "a/b/../../c"));
		// Fixed:
		Assert::IsTrue(normalizeAndCompare("a/b/../../c", "c"));
		Assert::IsTrue(normalizeAndCompare("/a/b/././c", "/a/b/c"));
		// Assert::IsTrue(normalizeAndCompare("a/b/././c", "a/b/././c"));
		// Fixed:
		Assert::IsTrue(normalizeAndCompare("a/b/././c", "a/b/c"));
		Assert::IsTrue(normalizeAndCompare("/a/b/../c/././d", "/a/c/d"));
		// Assert::IsTrue(normalizeAndCompare("a/b/../c/././d", "a/b/../c/././d"));
		// Fixed:
		Assert::IsTrue(normalizeAndCompare("a/b/../c/././d", "a/c/d"));
	}
	};
}

