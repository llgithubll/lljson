#include<iostream>
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

TEST(BasicPropertyTest, True) {
	Json j(true);
	EXPECT_TRUE(j.isBoolean());
	EXPECT_EQ(true, j.getBoolean());
	EXPECT_EQ(Json::PARSE_OK, j.state());
}

TEST(BasicPropertyTest, False) {
	Json j(false);
	EXPECT_TRUE(j.isBoolean());
	EXPECT_EQ(false, j.getBoolean());
	EXPECT_EQ(Json::PARSE_OK, j.state());
}

TEST(BasicPropertyTest, Number) {

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

#define TEST_ERROR(error, json)\
	do {\
		Json j = Json::parse(json);\
		EXPECT_EQ(Json::NUL, j.type());\
		EXPECT_EQ(error, j.state());\
	} while(0)\


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




} // namespace

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}



#if 0
{
	// interface design: version 1
	Json();
	Json(bool b);
	Json(double num);
	Json(string &str);
	Json(vector<Json> &v);
	Json(map<string, Json> &m);

	
	j = Json(...);
	
	j.is_null();
	j.is_boolean();
	j.is_number();
	j.is_string();
	j.is_array();
	j.is_object();	
	
	b = j.get_boolean();	// if j is boolean	,b is a true or false
	n = j.get_number();		// if j is number	,n is a double
	s = j.get_string();		// if j is string	,s is a string
	a = j.get_array();		// if j is array	,a is a vector<Json>
	j[0] and a[0];
	o = j.get_object();		// if j is object	,o is a map<string, Json>
	j["key"] and o["key"];

	j = Json::parse(str);
	str = j.stringify();
}
#endif

