#pragma once
#include <string>
#include <vector>
#include <map>

namespace lljson {


class JsonParser;

class Json {
	friend class JsonParser;
public:
	enum Type {
		NUL, BOOLEAN, NUMBER, STRING, ARRAY, OBJECT
	};
	enum State {
		PARSE_OK = 0,
		PARSE_EXPECT_VALUE,
		PARSE_INVALID_VALUE,
		PARSE_ROOT_NOT_SINGULAR
	};

	typedef std::vector<Json> Array;
	typedef std::map<std::string, Json> Object;

	Json(Json::Type _t=Json::Type::NUL, Json::State _s=Json::State::PARSE_OK);
	Json(const bool _b);
	Json(const double _n);
	Json(const std::string &_s);
	Json(const std::vector<Json> &_a);
	Json(const std::map<std::string, Json> _o);

	Json::Type type() const;
	Json::State state() const;

	bool isNull() const;
	bool isBoolean() const;
	bool isNumber() const;
	bool isString() const;
	bool isArray() const;
	bool isObject() const;

	bool& getBoolean() const;
	double& getNumber() const;
	std::string& getString() const;
	Array& getArray() const;
	Object& getObject() const;

	// Array
	Json & operator[](size_t i) const;
	// Object
	Json & operator[](const std::string &str) const;
	
	// Array and Object
	std::size_t size() const;

	static Json parse(const std::string &str);
	std::string stringify();
private:
	Type _type = NUL;
	State _state = PARSE_OK;
	bool _boolean;
	double _number;
	std::string _string;
	Array _array;
	Object _object;
};



} // namespace lljson
