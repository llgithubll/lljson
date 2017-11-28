#include<iostream>
#include<gtest\gtest.h>
#include "lljson.h"

using namespace std;
using namespace lljson;

namespace {

TEST(ConstructorTest, Constructor) {
	Json j;
	EXPECT_TRUE(j.isNull());
	EXPECT_EQ(Json::NUL, j.type());
	EXPECT_EQ(Json::PARSE_OK, j.state());
}

TEST(ParseNullTest, ParseNull) {
	Json j = Json::parse("null");
	EXPECT_EQ(Json::NUL, j.type());
	EXPECT_EQ(Json::PARSE_OK, j.state());
}

TEST(ParseExpectValueTest, ParseExpectValue) {
	Json j = Json::parse("");
	EXPECT_EQ(Json::NUL, j.type());
	EXPECT_EQ(Json::PARSE_EXPECT_VALUE, j.state());
}

TEST(ParseInvalidValueTest, ParseInvalidValue) {
	Json j = Json::parse("nul");
	EXPECT_EQ(Json::NUL, j.type());
	EXPECT_EQ(Json::PARSE_INVALID_VALUE, j.state());
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

