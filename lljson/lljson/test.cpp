#include<iostream>
#include<gtest\gtest.h>

namespace {

} // namespace

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

{
	// use like this
	Json json();						// Json json("[0, true]"); also support
	auto type = json.get_type();		// Json::type type = json.get_type();


	json.parse(R"({"0":[0, true, "haha"], "1":false})");
	auto s = json.stringify();			// std::string s = json.stringify();

	auto obj = json;					// Json obj = json;
	auto o = obj.get_object();			// std::map<string, Json> o = obj.get_object();
	auto arr = o["0"];					// Json arr = o["0"];
	auto a = arr.get_array();			// std::vector<Json> a = arr.get_object();
	auto num = a[0];					// Json num = a[0];
	auto n = num.get_number();			// double n = num.get_number();	
	auto boolean = a[1];				// Json boolean = a[1];
	auto b = boolean.get_boolean();		// bool b = boolean.get_boolean();
	auto str = a[2];					// Json str = a[2];
	auto s = str.get_string();			// std::string s = str.get_string();

	auto &o_ref = obj.get_object();		// std::map<string, Json> &o_ref = obj.get_object();
	o_ref["2"] = Json(R"({"3":4.0})");	// add new Json element into Json object
	auto s_new = obj.stringify();		// s_new = std::string(R"({"0":[0, true, "haha"], "1":false, "2":{"3":4.0}})");

}