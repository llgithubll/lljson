#pragma once
#include <string>
#include <vector>
#include <map>

namespace lljson {

class JsonValue {
public:

};

class Json final {
public:
	enum Type {
		Null, Boolean, Number, String, Array, Object
	};

	typedef std::vector<Json> array;
	typedef std::map<std::string, Json> object;

	Json(const std::string &_json)
	{
	}
	Json(const char * _json)
	{
	}


};

} // namespace lljson
