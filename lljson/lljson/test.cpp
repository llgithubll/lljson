#include<iostream>
#include<gtest\gtest.h>

namespace {

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
	a[i];
	o = j.get_object();		// if j is object	,o is a map<string, Json>
	o["key"];

	j = lljson::parse(str);
	str = lljson::stringify(j);
}
#endif

