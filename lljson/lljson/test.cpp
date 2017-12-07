#include<iostream>
#include <map>
#include<gtest\gtest.h>
#include "lljson.h"

using namespace std;
using namespace lljson;

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
	Json j(0.0);
	EXPECT_TRUE(j.isNumber());
	EXPECT_DOUBLE_EQ(0.0, j.getNumber());
	EXPECT_EQ(Json::PARSE_OK, j.state());
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
	const Json j({ 
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
	EXPECT_TRUE(j[3].isArray());
	EXPECT_EQ(2, j[3].size());
	EXPECT_STREQ("a\\3", j[3][1].getString().c_str());
	EXPECT_TRUE(j[4].isObject());
	EXPECT_EQ(2, j[4].size());
	EXPECT_TRUE(j[4]["key"].isString());
	EXPECT_TRUE(j[4]["k"].isNumber());
	EXPECT_DOUBLE_EQ(3.0, j[4]["k"].getNumber());
}

TEST(BasicPropertyTest, Object) {
	Json j(Json::Object{
		{"0", Json()},
		{"1", false},
		{"2", 3.0},
		{"3", "haha"},
		{"4", Json::Array{Json(5.1), Json("5.2")}},
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
	EXPECT_TRUE(j.isArray());
	EXPECT_STREQ("1", j[0].getString().c_str());
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

#define TEST_NUMBER(expect, json)\
	do {\
		Json j = Json::parse(json);\
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

#define TEST_STRING(expect, json)\
	do {\
		Json j = Json::parse(json);\
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


#define TEST_ERROR(error, json)\
	do {\
		Json j = Json::parse(json);\
		EXPECT_EQ(Json::NUL, j.type());\
		EXPECT_EQ(error, j.state());\
	} while(0)

TEST(ParseExpectValueTest, ParseExpectValue) {
	TEST_ERROR(Json::PARSE_EXPECT_VALUE, "");
	TEST_ERROR(Json::PARSE_EXPECT_VALUE, " ");
}

TEST(ParseInvalidValueTest, ParseInvalidValue) {
	TEST_ERROR(Json::PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(Json::PARSE_INVALID_VALUE, "?");

	/* invalid number */
	TEST_ERROR(Json::PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(Json::PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(Json::PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
	TEST_ERROR(Json::PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
	TEST_ERROR(Json::PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(Json::PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(Json::PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(Json::PARSE_INVALID_VALUE, "nan");
}

TEST(ParseRootNotSingularTest, ParseRootNotSingular) {
	TEST_ERROR(Json::PARSE_ROOT_NOT_SINGULAR, "null x");

	/* invalid number */
	TEST_ERROR(Json::PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
	TEST_ERROR(Json::PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(Json::PARSE_ROOT_NOT_SINGULAR, "0x123");
}

TEST(ParseNumberTooBigTest, ParseNumberTooBig) {
	TEST_ERROR(Json::PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(Json::PARSE_NUMBER_TOO_BIG, "-1e309");
}

TEST(ParseMissingQuotationMarkTest, ParseMissingQuotationMark) {
	TEST_ERROR(Json::PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_ERROR(Json::PARSE_MISS_QUOTATION_MARK, "\"abc");
}

TEST(ParseInvalidStringEscapeTest, ParseInvalidStringEscape) {
	TEST_ERROR(Json::PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(Json::PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(Json::PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(Json::PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

TEST(ParseInvalidStringCharTest, ParseInvalidStringChar) {
	TEST_ERROR(Json::PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(Json::PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

TEST(ParseInvalidUnicodeHexTest, ParseInvalidUnicodeHex) {
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

TEST(ParseInvalidUnicodeSurrogateTest, ParseInvalidUnicodeSurrogate) {
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(Json::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

TEST(ParseMissCommaOrSquareBracketTest, ParseMissCommaOrSquareBracket) {
	TEST_ERROR(Json::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
	TEST_ERROR(Json::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
	TEST_ERROR(Json::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
	TEST_ERROR(Json::PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

TEST(ParseMissKeyTest, ParseMissKey) {
	TEST_ERROR(Json::PARSE_MISS_KEY, "{:1,");
	TEST_ERROR(Json::PARSE_MISS_KEY, "{1:1,");
	TEST_ERROR(Json::PARSE_MISS_KEY, "{true:1,");
	TEST_ERROR(Json::PARSE_MISS_KEY, "{false:1,");
	TEST_ERROR(Json::PARSE_MISS_KEY, "{null:1,");
	TEST_ERROR(Json::PARSE_MISS_KEY, "{[]:1,");
	TEST_ERROR(Json::PARSE_MISS_KEY, "{{}:1,");
	TEST_ERROR(Json::PARSE_MISS_KEY, "{\"a\":1,");
}

TEST(ParseMissColonTest, ParseMissColon) {
	TEST_ERROR(Json::PARSE_MISS_COLON, "{\"a\"}");
	TEST_ERROR(Json::PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

TEST(ParseMissCommaOrCurlyBracketTest, ParseMissCommaOrCurlyBracket) {
	TEST_ERROR(Json::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
	TEST_ERROR(Json::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
	TEST_ERROR(Json::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
	TEST_ERROR(Json::PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

} // namespace

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

