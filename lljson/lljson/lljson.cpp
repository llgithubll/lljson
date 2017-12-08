#include <cassert>
#include <cerrno>
#include <cstdlib>
#include "lljson.h"


#include <iostream>


namespace lljson {

class JsonParser {
public:
	JsonParser(const std::string &_str);
	JsonParser(const char *_str);
	Json parse();
private:
	Json::State _parse_state = Json::PARSE_OK;
	const std::string &_parse_string;
	// index of parse string
	size_t _i = 0;

	Json parseValue();
	Json parseNull();
	Json parseBoolean();
	Json parseNumber();
	Json parseString();
	Json parseArray();
	Json parseObject();

	std::string parseRawString();
	char nextToken();
	void consumeWhitespace();
	void encode_utf8(long l, std::string &res);
};


class JsonStringify {
public:
	JsonStringify(const Json &_j);
	std::string stringify();
private:
	const Json &_json;
	std::string stringifyString(const std::string &_s);

};


//========================aux function=========================================
static inline bool inRange(long x, long lower, long upper) {
	return (lower <= x && x <= upper);
}

//========================JsonParser===========================================
JsonParser::JsonParser(const std::string & _str)
	:_parse_string(_str)
{
}

JsonParser::JsonParser(const char * _str)
	:JsonParser(std::string(_str))
{
}

Json JsonParser::parse()
{
	Json j = parseValue();
	if (j.state() == Json::PARSE_OK) {
		consumeWhitespace();
		if (_i != _parse_string.size()) {
			return Json(Json::NUL, Json::PARSE_ROOT_NOT_SINGULAR);
		}
	}
	return std::move(j);
}

Json JsonParser::parseValue()
{
	char ch = nextToken();
	if (_parse_state != Json::PARSE_OK) return Json(Json::NUL, _parse_state);
	switch (ch)
	{
	case 'n':	return parseNull();
	case 't':	// down to below
	case 'f':	return parseBoolean();
	case '"':	return parseString();
	case '[':	return parseArray();
	case '{':	return parseObject();
	default:	return parseNumber();
	}
}

Json JsonParser::parseNull()
{
	_i--;
	assert(_parse_string[_i] == 'n' && _parse_state == Json::PARSE_OK);
	if (_parse_string.substr(_i, 4) == "null") {
		_i += 4;
		return Json(Json::NUL, Json::PARSE_OK);
	}
	else {
		return Json(Json::NUL, Json::PARSE_INVALID_VALUE);
	}
}

Json JsonParser::parseBoolean()
{
	_i--;
	assert((_parse_string[_i] == 't' || _parse_string[_i] == 'f')
		&& _parse_state == Json::PARSE_OK);
	if (_parse_string.substr(_i, 4) == "true") {
		_i += 4;
		return Json(true);
	}
	else if (_parse_string.substr(_i, 5) == "false") {
		_i += 5;
		return Json(false);
	}
	else {
		return Json(Json::NUL, Json::PARSE_INVALID_VALUE);
	}
}

Json JsonParser::parseNumber()
{

	/*
	number = ["-"] int [frac] [exp]
	int = "0" / digit1-9(digit)+
	frac = "." (digit)+
	exp = ("e" / "E")["-" / "+"] (digit)+
	*/
	_i--;
	size_t idx = _i;
	// ["-"]
	if (_parse_string[idx] == '-') idx++;
	// int
	if (_parse_string[idx] == '0') idx++;
	else {
		if (!inRange(_parse_string[idx], '1', '9')) {
			return Json(Json::NUL, Json::PARSE_INVALID_VALUE);
		}
		for (idx++; inRange(_parse_string[idx], '0', '9'); idx++);
	}
	// [frac]
	if (_parse_string[idx] == '.') {
		idx++;
		if (!inRange(_parse_string[idx], '0', '9')) {
			return Json(Json::NUL, Json::PARSE_INVALID_VALUE);
		}
		for (idx++; inRange(_parse_string[idx], '0', '9'); idx++);
	}
	// [exp]
	if (_parse_string[idx] == 'e' || _parse_string[idx] == 'E') {
		idx++;
		if (_parse_string[idx] == '-' || _parse_string[idx] == '+') idx++;
		if (!inRange(_parse_string[idx], '0', '9')) {
			return Json(Json::NUL, Json::PARSE_INVALID_VALUE);
		}
		for (idx++; inRange(_parse_string[idx], '0', '9'); idx++);
	}

	
	errno = 0;
	double n = strtod(_parse_string.substr(_i).c_str(), NULL);
	if (errno == ERANGE && (n == HUGE_VAL || n == -HUGE_VAL)) {
		return Json(Json::NUL, Json::PARSE_NUMBER_TOO_BIG);
	}
	_i = idx;
	return Json(n);
}


Json JsonParser::parseString()
{
	std::string res = parseRawString();
	if (_parse_state != Json::PARSE_OK) {
		return Json(Json::NUL, _parse_state);
	}
	return Json(res);
}


Json JsonParser::parseArray()
{
	std::vector<Json> arr;
	char ch = nextToken();
	if (ch == ']') return arr;

	while (true) {
		_i--;
		Json elem = parseValue();
		if (elem.state() != Json::PARSE_OK) {
			return elem;
		}
		arr.push_back(elem);

		ch = nextToken();
		if (ch == ',') {
			ch = nextToken();
		}
		else if (ch == ']') {
			return arr;
		}
		else {
			return Json(Json::NUL, Json::PARSE_MISS_COMMA_OR_SQUARE_BRACKET);
		}
	}
}

Json JsonParser::parseObject()
{
	std::map<std::string, Json> object;
	char ch = nextToken();
	if (ch == '}') return object;

	while (true) {
		if (ch != '"') {
			return Json(Json::NUL, Json::PARSE_MISS_KEY);
		}

		std::string key = parseRawString();
		if (_parse_state != Json::PARSE_OK) {
			return Json(Json::NUL, _parse_state);
		}
		
		ch = nextToken();
		if (ch != ':') {
			return Json(Json::NUL, Json::PARSE_MISS_COLON);
		}
		
		auto value = parseValue();
		if (_parse_state != Json::PARSE_OK) {
			return Json(Json::NUL, _parse_state);
		}
		object[std::move(key)] = std::move(value);

		ch = nextToken();
		if (ch == ',') {
			ch = nextToken();
		}
		else if (ch == '}') {
			break;
		}
		else {
			return Json(Json::NUL, Json::PARSE_MISS_COMMA_OR_CURLY_BRACKET);
		}
	}
	return object;
}

std::string JsonParser::parseRawString()
{
	std::string res = "";
	while (true) {
		char ch = _parse_string[_i++];
		switch (ch)
		{
		case '"':
			return res;	//??Json(std::move(res))
		case '\\':
			switch (_parse_string[_i++])
			{
			case '"':	res += '"'; break;
			case '\\':	res += '\\'; break;
			case '/':	res += '/'; break;
			case 'b':	res += '\b'; break;
			case 'f':	res += '\f'; break;
			case 'n':	res += '\n'; break;
			case 'r':	res += '\r'; break;
			case 't':	res += '\t'; break;
			case 'u':	{
					std::string hex4 = _parse_string.substr(_i, 4);
					auto is_hex4 = [&](const std::string &hex) {
						for (size_t j = 0; j < 4; j++) {
							if (!inRange(hex[j], 'a', 'f')
								&& !inRange(hex[j], 'A', 'F')
								&& !inRange(hex[j], '0', '9')) {
								return false;
							}
						}
						return true;
					};
					
					if (!is_hex4(hex4)) {
						_parse_state = Json::PARSE_INVALID_UNICODE_HEX;
						return "";
					}
					long codepoint = strtol(hex4.c_str(), nullptr, 16);
					_i += 4;
					if (inRange(codepoint, 0xD800, 0xDBFF)) { // surrogate pair
						if (_parse_string[_i++] != '\\') {
							_parse_state = Json::PARSE_INVALID_UNICODE_SURROGATE;
							return "";
						}
						if (_parse_string[_i++] != 'u') {
							_parse_state = Json::PARSE_INVALID_UNICODE_SURROGATE;
							return "";
						}
						hex4 = _parse_string.substr(_i, 4);
						if (!is_hex4(hex4)) {
							_parse_state = Json::PARSE_INVALID_UNICODE_HEX;
							return "";
						}
						long low = strtol(hex4.c_str(), nullptr, 16);
						_i += 4;
						if (!inRange(low, 0xDC00, 0xDFFF)) {
							_parse_state = Json::PARSE_INVALID_UNICODE_SURROGATE;
							return "";
						}
						codepoint = (((codepoint - 0xD800) << 10) | (low - 0xDC00)) + 0x10000;
					}
					encode_utf8(codepoint, res);
				}
				break;
			default:
				_parse_state = Json::PARSE_INVALID_STRING_ESCAPE;
				return "";
			}
			break;
		case '\0':
			_parse_state = Json::PARSE_MISS_QUOTATION_MARK;
			return "";
		default:
			if ((unsigned char)ch < 0x20) {
				_parse_state = Json::PARSE_INVALID_STRING_CHAR;
				return "";
			}
			res += ch;
			break;
		}
	}
}

char JsonParser::nextToken()
{
	consumeWhitespace();
	if (_parse_state != Json::PARSE_OK) return 0;
	if (_i == _parse_string.size()) {
		_parse_state = Json::PARSE_EXPECT_VALUE;
		return 0;
	}
	return _parse_string[_i++];
}

void JsonParser::consumeWhitespace()
{
	while (_parse_string[_i] == ' '
		|| _parse_string[_i] == '\r'
		|| _parse_string[_i] == '\t'
		|| _parse_string[_i] == '\n')
		_i++;
}

void JsonParser::encode_utf8(long l, std::string & res)
{
	if (l < 0) return;
	if (l < 0x80) {
		res += static_cast<char>(l & 0xFF);
	}
	else if (l < 0x800) {
		res += static_cast<char>(0xC0 | ((l >> 6) & 0xFF));
		res += static_cast<char>(0x80 | (l        & 0x3F));
	}
	else if (l < 0x10000) {
		res += static_cast<char>(0xE0 | ((l >> 12) & 0xFF));
		res += static_cast<char>(0x80 | ((l >>  6) & 0x3F));
		res += static_cast<char>(0x80 | (l         & 0x3F));
	}
	else {
		assert(l <= 0x10FFFF);
		res += static_cast<char>(0xF0 | ((l >> 18) & 0xFF));
		res += static_cast<char>(0x80 | ((l >> 12) & 0x3F));
		res += static_cast<char>(0x80 | ((l >>  6) & 0x3F));
		res += static_cast<char>(0x80 | (l         & 0x3F));
	}
}


//========================Json=================================================
Json::Json(Json::Type _t, Json::State _s)
	:_type(_t), _state(_s)
{
}

Json::Json(const bool _b)
	:_boolean(_b), _type(Json::BOOLEAN)
{
}

Json::Json(const int _n)
	:_number(static_cast<double>(_n)), _type(Json::NUMBER)
{
}

Json::Json(const double _n)
	:_number(_n), _type(Json::NUMBER)
{
}

Json::Json(const std::string & _s)
	:_type(Json::STRING)
{
	new(&_string) std::string(_s);
}

Json::Json(const char * _c)
	:_type(Json::STRING)
{
	new(&_string) std::string(_c);
}

Json::Json(const std::vector<Json>& _a)
	:_type(Json::ARRAY)
{
	new(&_array) std::vector<Json>(_a);
}

Json::Json(const std::map<std::string, Json> &_o)
	:_type(Json::OBJECT)
{
	new(&_object) std::map<std::string, Json>(_o);
}

Json::Json(const Json & _j)
	:_type(_j.type()), _state(_j.state())
{
	copyUnion(_j);
}

Json & Json::operator=(const Json & _j)
{
	destroyUnion();
	copyUnion(_j);
	_state = _j.state();
	_type = _j.type();
	return *this;
}

Json & Json::operator=(bool _b)
{
	destroyUnion();
	_boolean = _b;
	_type = Json::BOOLEAN;
	return *this;
}

Json & Json::operator=(int _n)
{
	destroyUnion();
	_number = static_cast<double>(_n);
	_type = Json::NUMBER;
	return *this;
}

Json & Json::operator=(double _n)
{
	destroyUnion();
	_number = _n;
	_type = Json::NUMBER;
	return *this;
}

Json & Json::operator=(const std::string & _s)
{
	destroyUnion();
	new(&_string) std::string(_s);
	_type = Json::STRING;
	return *this;
}

Json & Json::operator=(const char * _c)
{
	destroyUnion();
	new(&_string) std::string(_c);
	_type = Json::STRING;
	return *this;
}

Json & Json::operator=(const std::vector<Json>& _a)
{
	destroyUnion();
	new(&_array) std::vector<Json>(_a);
	_type = Json::ARRAY;
	return *this;
}

Json & Json::operator=(const std::map<std::string, Json> &_o)
{
	destroyUnion();
	new(&_object) std::map<std::string, Json>(_o);
	_type = Json::OBJECT;
	return *this;
}


Json::~Json()
{
	destroyUnion();
}

Json::Type Json::type() const
{
	return _type;
}

Json::State Json::state() const
{
	return _state;
}

bool Json::isNull() const
{
	return _type == NUL;
}

bool Json::isBoolean() const
{
	return _type == BOOLEAN;
}

bool Json::isNumber() const
{
	return _type == NUMBER;
}

bool Json::isString() const
{
	return _type == STRING;
}

bool Json::isArray() const
{
	return _type == ARRAY;
}

bool Json::isObject() const
{
	return _type == OBJECT;
}

std::size_t Json::size() const
{
	assert(_type == ARRAY || _type == OBJECT);
	return (_type == ARRAY) ? _array.size() : _object.size();
}

Json Json::parse(const std::string & str)
{
	JsonParser jp(str);
	return jp.parse();
}

std::string Json::stringify(const Json & j)
{
	JsonStringify js(j);
	return js.stringify();
}


void Json::copyUnion(const Json & _j)
{
	switch (_j.type())
	{
	case Json::NUL:			break;
	case Json::BOOLEAN:		_boolean = _j._boolean; break;
	case Json::NUMBER:		_number = _j._number; break;
	case Json::STRING:		new(&_string) std::string(_j._string); break;
	case Json::ARRAY:		new(&_array) std::vector<Json>(_j._array); break;
	case Json::OBJECT:		new(&_object) std::map<std::string, Json>(_j._object); break;
	default:
		break;
	}
}

void Json::destroyUnion()
{
	switch (_type)
	{
	case Json::NUL:		
		break;
	case Json::BOOLEAN:	
		break;
	case Json::NUMBER:	
		break;
	case Json::STRING:	
		using std::string; // can't straight use _string.~std::string();
		_string.~string();
		break;
	case Json::ARRAY:
		using std::vector;
		_array.~vector();
		break;
	case Json::OBJECT:
		using std::map;
		_object.~map();
		break;
	default:
		break;
	}
}



bool Json::getBoolean() const
{
	assert(this->isBoolean());
	return _boolean;
}

double Json::getNumber() const
{
	assert(this->isNumber());
	return _number;
}

const std::string & Json::getString() const
{
	assert(this->isString());
	return _string;
}

const Json::Array & Json::getArray() const
{
	assert(this->isArray());
	return _array;
}

const Json::Object & Json::getObject() const
{
	assert(this->isObject());
	return _object;
}

const Json & Json::operator[](size_t i) const
{
	assert(_type == ARRAY && i < _array.size());
	return _array[i];
}

const Json & Json::operator[](const std::string & str) const
{
	assert(_type == OBJECT);
	//auto iter = _object.find(str);
	//return (iter == _object.end() ? Json() : iter->second); // this may return temp variable Json()..., add move later??
	//return _object[str]; // the same as above ? can not use
	return _object.at(str); // simple use at, later add exception handler??
}

//========================JsonStringify========================================
JsonStringify::JsonStringify(const Json & _j)
	:_json(_j)
{
}

std::string JsonStringify::stringify()
{
	switch (_json.type())
	{
		case Json::NUL:
			return "null";
		case Json::BOOLEAN:
			return (_json.getBoolean() == true ? "true" : "false");
		case Json::NUMBER: {
				char buf[32];
				snprintf(buf, sizeof buf, "%.17g", _json.getNumber());
				return buf;
			}
		case Json::STRING:
			return stringifyString(_json.getString());
		case Json::ARRAY: {
				std::string res;
				res += '[';
				for (int i = 0; i < _json.size(); i++) {
					if (i != 0) { res += ','; }
					res += JsonStringify(_json[i]).stringify();
				}
				res += ']';
				return std::move(res);
			}
		case Json::OBJECT: {
				bool first = true;
				std::string res;
				res += '{';
				const auto &object = _json.getObject();
				for (const auto &kv : object) {
					if (!first) { res += ','; }
					res += stringifyString(kv.first);
					res += ':';
					res += JsonStringify(kv.second).stringify();
					first = false;
				}
				res += '}';
				return std::move(res);
			}
		default:
			break;
	}
}


std::string JsonStringify::stringifyString(const std::string & _s)
{
	std::string res;
	res += '"';
	for (int i = 0; i < _s.size(); i++) {
		const char ch = _s[i];
		switch (ch)
		{
			case '"':	res += R"(\")"; break;
			case '\\':	res += R"(\\)";	break;
			case '\b':	res += R"(\b)";	break;
			case '\f':	res += R"(\f)";	break;
			case '\n':	res += R"(\n)";	break;
			case '\r':	res += R"(\r)";	break;
			case '\t':	res += R"(\t)";	break;
			default:
				if (static_cast<uint8_t>(ch) < 0x20) {
					char buf[8];
					snprintf(buf, sizeof(buf), "\\u%04X", ch);
					res += buf;
				}
				else {
					res += ch;
				}
				break;
		}
	}
	res += '"';
	return std::move(res);
}


} // namespace lljson
