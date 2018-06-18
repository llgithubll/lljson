#pragma once
#include <string>
#include <vector>
#include <map>
#include <type_traits>

namespace ll {

namespace json {

class JsonParser;

class Json {
	friend class JsonParser;
	friend class JsonStringify;
	friend bool operator==(const Json &lhs, const Json &rhs);
	friend bool operator!=(const Json &lhs, const Json &rhs);
	friend std::ostream & operator<<(std::ostream &out, const Json &j);
public:
	enum Type {
		NUL, BOOLEAN, NUMBER, STRING, ARRAY, OBJECT
	};
	enum State {
		PARSE_OK = 0,
		PARSE_EXPECT_VALUE,
		PARSE_INVALID_VALUE,
		PARSE_NUMBER_TOO_BIG,
		PARSE_ROOT_NOT_SINGULAR,
		PARSE_INVALID_STRING_CHAR,
		PARSE_MISS_QUOTATION_MARK,
		PARSE_INVALID_STRING_ESCAPE,
		PARSE_INVALID_UNICODE_SURROGATE,
		PARSE_INVALID_UNICODE_HEX,
		PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
		PARSE_MISS_KEY,
		PARSE_MISS_COLON,
		PARSE_MISS_COMMA_OR_CURLY_BRACKET
	};

	typedef std::vector<Json> Array;
	typedef std::map<std::string, Json> Object;
	typedef Object::iterator ObjectIterator;
	typedef Object::const_iterator ConstObjectIterator;

	Json(Json::Type _t=Json::Type::NUL, Json::State _s=Json::State::PARSE_OK);
	Json(const bool _b);
	Json(const int _n);	// receive int value and cast to double
	Json(const double _n);
	Json(const std::string &_s);
	Json(const char *_c);
	Json(const std::vector<Json> &_a);
	Json(const std::map<std::string, Json> &_o);
	Json(const Json &_j);

	Json &operator=(const Json &_j);
	Json &operator=(bool _b);
	Json &operator=(int _n);	// receive int value and cast to double
	Json &operator=(double _n);
	Json &operator=(const std::string &_s);
	Json &operator=(const char *_c);
	Json &operator=(const std::vector<Json> &_a);
	Json &operator=(const std::map<std::string, Json> &_o);

	~Json();

	Json::Type type() const;
	Json::State state() const;

	bool isNull() const;
	bool isBoolean() const;
	bool isNumber() const;
	bool isString() const;
	bool isArray() const;
	bool isObject() const;

	bool getBoolean() const;
	double getNumber() const;
	const std::string& getString() const;
	const Array& getArray() const;
	const Object& getObject() const;

	// =================Array==================
	const Json & operator[](size_t i) const;
	Json & operator[](size_t i);
	void pushbackArrayElement(const Json &e);
	void popbackArrayElement();
	// Insert e to array before i, return new element index
	size_t insertArrayElement(size_t i, const Json &e);
	// Erase element at i in array, return next element index
	size_t eraseArrayElement(size_t i);
	// Clear all elements in array
	void clearArray();

	// =================Object=================
	const Json & operator[](const std::string &str) const;
	Json & operator[](const std::string &str);
	ObjectIterator findObjectElement(const std::string &str);
	ConstObjectIterator findObjectElement(const std::string &str) const;
	ObjectIterator eraseObjectElement(ObjectIterator pos);
	ObjectIterator eraseObjectElement(ConstObjectIterator pos);
	void clearObject();

	// Array and Object
	std::size_t size() const;

	static Json parse(const std::string &str);
	// note: stringify will make Json::Object sorted as lexicographical order
	static std::string stringify(const Json &j);
private:
	Type _type = NUL;
	State _state = PARSE_OK;
	union {
		bool _boolean;
		double _number;
		std::string _string;
		Array _array;
		Object _object;
	};

	void copyUnion(const Json &_j);
	void destroyUnion();
};



} // namespace json
} // namespace ll
