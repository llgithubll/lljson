#include<iostream>
#include <map>
#include<gtest\gtest.h>
#include "lljson.h"

using namespace std;
using namespace ll::json;

namespace {

TEST(BasicPropertyTest, Null) {
	Json j;
	EXPECT_TRUE(j.isNull());
	EXPECT_EQ(Json::NUL, j.type());
	EXPECT_EQ(Json::PARSE_OK, j.state());
}

TEST(BasicPropertyTest, Boolean) {
	Json j(true);
	EXPECT_TRUE(j.isBoolean());
	EXPECT_EQ(true, j.getBoolean());
	EXPECT_EQ(Json::PARSE_OK, j.state());

	j = false;
	EXPECT_TRUE(j.isBoolean());
	EXPECT_EQ(false, j.getBoolean());
	EXPECT_EQ(Json::PARSE_OK, j.state());
}


TEST(BasicPropertyTest, Number) {
	Json j(0);	// int number will cast to double
	EXPECT_TRUE(j.isNumber());
	EXPECT_DOUBLE_EQ(0, j.getNumber());
	EXPECT_EQ(Json::PARSE_OK, j.state());
	
	j = 1.7976931348623157e+308;
	EXPECT_DOUBLE_EQ(1.7976931348623157e+308, j.getNumber());
}


TEST(BasicPropertyTest, String) {
	Json j(string("string"));
	EXPECT_EQ(Json::STRING, j.type());
	EXPECT_TRUE(j.isString());
	EXPECT_STREQ("string", j.getString().c_str());
	EXPECT_EQ(Json::PARSE_OK, j.state());

	j = "haha";
	EXPECT_TRUE(j.isString());
	EXPECT_STREQ("haha", j.getString().c_str());
}


TEST(BasicPropertyTest, Array) {
	Json j({ 
		true,									// 0
		1.0,									// 1
		"haha",									// 2
		Json::Array{2.0, "a\\3"},				// 3
		Json::Object{{"key","value"}, {"k",3.0}}// 4
	});
	EXPECT_TRUE(j.isArray());
	auto v = j.getArray();
	EXPECT_TRUE(v[0].isBoolean());
	EXPECT_EQ(true, v[0].getBoolean());
	v[1] = Json(false);
	EXPECT_EQ(false, v[1].getBoolean());
	j.pushbackArrayElement(Json(string("end")));
	EXPECT_EQ(6, j.size());
	EXPECT_STREQ("end", j[5].getString().c_str());
	j.popbackArrayElement();
	EXPECT_EQ(5, j.size());
	j.insertArrayElement(3, Json(3.0));
	EXPECT_EQ(6, j.size());
	EXPECT_DOUBLE_EQ(3.0, j[3].getNumber());
	j.eraseArrayElement(2);
	EXPECT_EQ(5, j.size());
	cout << j << endl;
	EXPECT_TRUE(j[3].isArray());
	EXPECT_EQ(2, j[3].size());
	EXPECT_STREQ("a\\3", j[3][1].getString().c_str());
	EXPECT_TRUE(j[4].isObject());
	EXPECT_EQ(2, j[4].size());
	EXPECT_TRUE(j[4]["key"].isString());
	EXPECT_TRUE(j[4]["k"].isNumber());
	EXPECT_DOUBLE_EQ(3.0, j[4]["k"].getNumber());
	j.clearArray();
	EXPECT_EQ(0, j.size());
}

TEST(BasicPropertyTest, Object) {
	Json j(Json::Object{
		{"0", Json()},
		{"1", false},
		{"2", 3.0},
		{"3", "haha"},
		{"4", Json::Array{5.1, "5.2"}},
		{"5", Json::Object{{"6.1", Json()}}}
	});
	EXPECT_TRUE(j.isObject());
	EXPECT_EQ(6, j.size());
	auto o = j.getObject();
	EXPECT_EQ(false, o["1"].getBoolean());
	EXPECT_TRUE(j["4"].isArray());
	EXPECT_EQ(2, j["4"].size());
	EXPECT_TRUE(j["5"].isObject());
	EXPECT_EQ(1, j["5"].size());
	auto a = j["4"].getArray();
	EXPECT_EQ(2, a.size());
	EXPECT_TRUE(a[1].isString());
	EXPECT_TRUE(j["4"][1].isString());
	EXPECT_STREQ("5.2", j["4"][1].getString().c_str());
	EXPECT_TRUE(j["5"]["6.1"].isNull());

	j["6"] = "hehe";
	auto iter = j.findObjectElement("3");
	EXPECT_STREQ("haha", (iter->second).getString().c_str());
	auto next_iter = j.eraseObjectElement(iter);
	EXPECT_TRUE((next_iter->second).isArray());
	cout << j << endl;
	j.clearObject();
	EXPECT_EQ(0, j.size());
}

TEST(BasicPropertyTest, Copy) {
	Json j(1.0);
	EXPECT_TRUE(j.isNumber());
	j = true;
	EXPECT_TRUE(j.isBoolean());
	EXPECT_EQ(true, j.getBoolean());
	EXPECT_EQ(Json::PARSE_OK, j.state());
	
	j = Json::Array{ Json("haha"), Json(1.0) };
	EXPECT_TRUE(j.isArray());
	EXPECT_STREQ("haha", j[0].getString().c_str());

	j = "hehe";
	EXPECT_TRUE(j.isString());
	EXPECT_STREQ("hehe", j.getString().c_str());

	j = Json::Array{ "1", 1.0, true };
	j = j;
	EXPECT_TRUE(j.isArray());
	EXPECT_STREQ("1", j[0].getString().c_str());

	Json j2 = Json::Array{};
	EXPECT_TRUE(j2.isArray());
	EXPECT_EQ(0, j2.size());
	vector<Json> vj;
	for (int i = 0; i < 1000000; i++) {
		vj.push_back(Json(i));
	}
	j2 = vj;
	EXPECT_EQ(1000000, j2.size());
}


TEST(ParseNullTest, ParseNull) {
	Json j = Json::parse("null");
	EXPECT_TRUE(j.isNull());
	EXPECT_EQ(Json::NUL, j.type());
	EXPECT_EQ(Json::PARSE_OK, j.state());

	j = Json::parse("   null\t\t\n\r");
	EXPECT_TRUE(j.isNull());
	EXPECT_EQ(Json::PARSE_OK, j.state());
}

TEST(ParseBooleanTest, ParseBoolean) {
	Json j = Json::parse("true");
	EXPECT_TRUE(j.isBoolean());
	EXPECT_EQ(true, j.getBoolean());
	EXPECT_EQ(Json::BOOLEAN, j.type());
	EXPECT_EQ(Json::PARSE_OK, j.state());

	j = Json::parse("false");
	EXPECT_TRUE(j.isBoolean());
	EXPECT_EQ(false, j.getBoolean());
	EXPECT_EQ(Json::BOOLEAN, j.type());
	EXPECT_EQ(Json::PARSE_OK, j.state());
}

#define TEST_NUMBER(expect, str)\
	do {\
		Json j = Json::parse(str);\
		EXPECT_EQ(Json::PARSE_OK, j.state());\
		EXPECT_TRUE(j.isNumber());\
		EXPECT_EQ(Json::NUMBER, j.type());\
		EXPECT_DOUBLE_EQ(expect, j.getNumber());\
	} while(0)

TEST(ParseNumberTest, ParseNumber) {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

	TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, str)\
	do {\
		Json j = Json::parse(str);\
		EXPECT_EQ(Json::PARSE_OK, j.state());\
		EXPECT_TRUE(j.isString());\
		EXPECT_STREQ(expect, j.getString().c_str());\
	} while(0)

TEST(ParseStringTest, ParseString) {
	TEST_STRING("", R"("")");
	TEST_STRING("Hello", "\"Hello\"");
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
	TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
	TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}


TEST(ParseArrayTest, ParseArray) {
	Json j = Json::parse(R"([true, 1.0, "ha\\ha", [2.0, 3.0]])");
	EXPECT_EQ(Json::PARSE_OK, j.state());
	EXPECT_EQ(Json::ARRAY, j.type());
	EXPECT_TRUE(j.isArray());
	EXPECT_EQ(4, j.size());
	EXPECT_TRUE(j[3].isArray());
	EXPECT_EQ(2, j[3].size());
	EXPECT_STREQ("ha\\ha", j[2].getString().c_str());
	EXPECT_DOUBLE_EQ(3.0, j[3][1].getNumber());
}

TEST(ParseObjectTest, ParseObject) {
	Json j = Json::parse(R"(
		{
			"n":null,
			"f":false,
			"t":true,
			"i":123,
			"s":"abc",
			"a":[1, 2, 3],
			"o":{"1":1, "2":2, "3":3}
		}
	)");
	EXPECT_EQ(Json::PARSE_OK, j.state());
	EXPECT_TRUE(j.isObject());
	EXPECT_EQ(7, j.size());
	EXPECT_EQ(true, j["t"].getBoolean());
	EXPECT_DOUBLE_EQ(123, j["i"].getNumber());
	EXPECT_STREQ("abc", j["s"].getString().c_str());
	EXPECT_EQ(3, j["a"].size());
	EXPECT_DOUBLE_EQ(2, j["a"][1].getNumber());
	EXPECT_EQ(3, j["o"].size());
	EXPECT_DOUBLE_EQ(3, j["o"]["3"].getNumber());
}


#define TEST_PARSE_ERROR(error, json)\
	do {\
		Json j = Json::parse(json);\
		EXPECT_EQ(Json::NUL, j.type());\
		EXPECT_EQ(error, j.state());\
	} while(0)

TEST(ParseExpectValueTest, ParseExpectValue) {
	TEST_PARSE_ERROR(Json::PARSE_EXPECT_VALUE, "");
	TEST_PARSE_ERROR(Json::PARSE_EXPECT_VALUE, " ");
}

TEST(ParseInvalidValueTest, ParseInvalidValue) {
	TEST_PARSE_ERROR(Json::PARSE_INVALID_VALUE, "nul");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_VALUE, "?");

	/* invalid number */
	TEST_PARSE_ERROR(Json::PARSE_INVALID_VALUE, "+0");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_VALUE, "+1");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
	TEST_PARSE_ERROR(Json::PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
	TEST_PARSE_ERROR(Json::PARSE_INVALID_VALUE, "INF");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_VALUE, "inf");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_VALUE, "NAN");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_VALUE, "nan");
}

TEST(ParseRootNotSingularTest, ParseRootNotSingular) {
	TEST_PARSE_ERROR(Json::PARSE_ROOT_NOT_SINGULAR, "null x");

	/* invalid number */
	TEST_PARSE_ERROR(Json::PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
	TEST_PARSE_ERROR(Json::PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_PARSE_ERROR(Json::PARSE_ROOT_NOT_SINGULAR, "0x123");
}

TEST(ParseNumberTooBigTest, ParseNumberTooBig) {
	TEST_PARSE_ERROR(Json::PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_PARSE_ERROR(Json::PARSE_NUMBER_TOO_BIG, "-1e309");
}

TEST(ParseMissingQuotationMarkTest, ParseMissingQuotationMark) {
	TEST_PARSE_ERROR(Json::PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_PARSE_ERROR(Json::PARSE_MISS_QUOTATION_MARK, "\"abc");
}

TEST(ParseInvalidStringEscapeTest, ParseInvalidStringEscape) {
	TEST_PARSE_ERROR(Json::PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

TEST(ParseInvalidStringCharTest, ParseInvalidStringChar) {
	TEST_PARSE_ERROR(Json::PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

TEST(ParseInvalidUnicodeHexTest, ParseInvalidUnicodeHex) {
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

TEST(ParseInvalidUnicodeSurrogateTest, ParseInvalidUnicodeSurrogate) {
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_PARSE_ERROR(Json::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

TEST(ParseMissCommaOrSquareBracketTest, ParseMissCommaOrSquareBracket) {
	TEST_PARSE_ERROR(Json::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
	TEST_PARSE_ERROR(Json::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
	TEST_PARSE_ERROR(Json::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
	TEST_PARSE_ERROR(Json::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

TEST(ParseMissKeyTest, ParseMissKey) {
	TEST_PARSE_ERROR(Json::PARSE_MISS_KEY, "{:1,");
	TEST_PARSE_ERROR(Json::PARSE_MISS_KEY, "{1:1,");
	TEST_PARSE_ERROR(Json::PARSE_MISS_KEY, "{true:1,");
	TEST_PARSE_ERROR(Json::PARSE_MISS_KEY, "{false:1,");
	TEST_PARSE_ERROR(Json::PARSE_MISS_KEY, "{null:1,");
	TEST_PARSE_ERROR(Json::PARSE_MISS_KEY, "{[]:1,");
	TEST_PARSE_ERROR(Json::PARSE_MISS_KEY, "{{}:1,");
	TEST_PARSE_ERROR(Json::PARSE_MISS_KEY, "{\"a\":1,");
}

TEST(ParseMissColonTest, ParseMissColon) {
	TEST_PARSE_ERROR(Json::PARSE_MISS_COLON, "{\"a\"}");
	TEST_PARSE_ERROR(Json::PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

TEST(ParseMissCommaOrCurlyBracketTest, ParseMissCommaOrCurlyBracket) {
	TEST_PARSE_ERROR(Json::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
	TEST_PARSE_ERROR(Json::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
	TEST_PARSE_ERROR(Json::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
	TEST_PARSE_ERROR(Json::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

TEST(StringifyTest, Object) {
	Json j(Json::Object{
		{ "0", Json() },
		{ "1", false },
		{ "3", "haha" },
		{ "2", 3.0 },
		{ "4", Json::Array{ 5.1, "5.2" } },
		{ "5", Json::Object{ { "6.1", Json() } } }
	});
	string s = Json::stringify(j);
	char buf[128];
	snprintf(buf, sizeof buf, R"({"0":null,"1":false,"2":%.17g,"3":"haha","4":[%.17g,"5.2"],"5":{"6.1":null}})", 3.0, 5.1);
	EXPECT_STREQ(buf, s.c_str());
}

#define TEST_ROUNDTRIP(str)\
	do {\
		Json j = Json::parse(str);\
		EXPECT_EQ(Json::PARSE_OK, j.state());\
		string s = Json::stringify(j);\
		EXPECT_STREQ(str, s.c_str());\
	} while(0)

TEST(StringifyTest, Roundtrip) {
	TEST_ROUNDTRIP("null");
	TEST_ROUNDTRIP("false");
	TEST_ROUNDTRIP("true");

	TEST_ROUNDTRIP("0");
	TEST_ROUNDTRIP("-0");
	TEST_ROUNDTRIP("1");
	TEST_ROUNDTRIP("-1");
	TEST_ROUNDTRIP("1.5");
	TEST_ROUNDTRIP("-1.5");
	TEST_ROUNDTRIP("3.25");
	TEST_ROUNDTRIP("1e+20");
	TEST_ROUNDTRIP("1.234e+20");
	TEST_ROUNDTRIP("1.234e-20");

	TEST_ROUNDTRIP("1.0000000000000002"); /* the smallest number > 1 */
	TEST_ROUNDTRIP("4.9406564584124654e-324"); /* minimum denormal */
	TEST_ROUNDTRIP("-4.9406564584124654e-324");
	TEST_ROUNDTRIP("2.2250738585072009e-308");  /* Max subnormal double */
	TEST_ROUNDTRIP("-2.2250738585072009e-308");
	TEST_ROUNDTRIP("2.2250738585072014e-308");  /* Min normal positive double */
	TEST_ROUNDTRIP("-2.2250738585072014e-308");
	TEST_ROUNDTRIP("1.7976931348623157e+308");  /* Max double */
	TEST_ROUNDTRIP("-1.7976931348623157e+308");

	TEST_ROUNDTRIP("\"\"");
	TEST_ROUNDTRIP("\"Hello\"");
	TEST_ROUNDTRIP("\"Hello\\nWorld\"");
	TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
	TEST_ROUNDTRIP("\"Hello\\u0000World\"");

	TEST_ROUNDTRIP("[]");
	TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");

	TEST_ROUNDTRIP("{}");
	TEST_ROUNDTRIP("{\"a\":[1,2,3],\"f\":false,\"i\":123,\"n\":null,\"o\":{\"1\":1,\"2\":2,\"3\":3},\"s\":\"abc\",\"t\":true}");
}

#define TEST_EQUAL(json1, json2, equality)\
	do {\
		Json j1 = Json::parse(json1);\
		Json j2 = Json::parse(json2);\
		EXPECT_EQ(Json::PARSE_OK, j1.state());\
		EXPECT_EQ(Json::PARSE_OK, j2.state());\
		EXPECT_EQ(equality, j1 == j2);\
		EXPECT_EQ(equality, !(j1 != j2));\
	} while(0)

TEST(EqualTest, Equal) {
	TEST_EQUAL("true", "false", 0);
	TEST_EQUAL("true", "true", 1);
	TEST_EQUAL("false", "false", 1);
	TEST_EQUAL("null", "null", 1);
	TEST_EQUAL("null", "0", 0);
	TEST_EQUAL("123", "123", 1);
	TEST_EQUAL("123", "456", 0);
	TEST_EQUAL("\"abc\"", "\"abc\"", 1);
	TEST_EQUAL("\"abc\"", "\"abcd\"", 0);
	TEST_EQUAL("[]", "[]", 1);
	TEST_EQUAL("[]", "null", 0);
	TEST_EQUAL("[1,2,3]", "[1,2,3]", 1);
	TEST_EQUAL("[1,2,3]", "[1,2,3,4]", 0);
	TEST_EQUAL("[[]]", "[[]]", 1);
	TEST_EQUAL("{}", "{}", 1);
	TEST_EQUAL("{}", "null", 0);
	TEST_EQUAL("{}", "[]", 0);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2}", 1);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"b\":2,\"a\":1}", 1);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":3}", 0);
	TEST_EQUAL("{\"a\":1,\"b\":2}", "{\"a\":1,\"b\":2,\"c\":3}", 0);
	TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":{}}}}", 1);
	TEST_EQUAL("{\"a\":{\"b\":{\"c\":{}}}}", "{\"a\":{\"b\":{\"c\":[]}}}", 0);
}

} // namespace

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

