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

	char nextToken();
	void consumeWhitespace();
	void encode_utf8(long l, std::string &res);
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
	return j;
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
	std::string res = "";
	while (true) {
		char ch = _parse_string[_i++];
		switch (ch)
		{
		case '"':
			return Json(res);	//??Json(std::move(res))
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
						return Json(Json::NUL, Json::PARSE_INVALID_UNICODE_HEX);
					}
					long codepoint = strtol(hex4.c_str(), nullptr, 16);
					_i += 4;
					if (inRange(codepoint, 0xD800, 0xDBFF)) { // surrogate pair
						if (_parse_string[_i++] != '\\') {
							return Json(Json::NUL, Json::PARSE_INVALID_UNICODE_SURROGATE);
						}
						if (_parse_string[_i++] != 'u') {
							return Json(Json::NUL, Json::PARSE_INVALID_UNICODE_SURROGATE);
						}
						hex4 = _parse_string.substr(_i, 4);
						if (!is_hex4(hex4)) {
							return Json(Json::NUL, Json::PARSE_INVALID_UNICODE_HEX);
						}
						long low = strtol(hex4.c_str(), nullptr, 16);
						_i += 4;
						if (!inRange(low, 0xDC00, 0xDFFF)) {
							return Json(Json::NUL, Json::PARSE_INVALID_UNICODE_SURROGATE);
						}
						codepoint = (((codepoint - 0xD800) << 10) | (low - 0xDC00)) + 0x10000;
					}
					encode_utf8(codepoint, res);
				}
				break;
			default:
				return Json(Json::NUL, Json::PARSE_INVALID_STRING_ESCAPE);
			}
			break;
		case '\0':
			return Json(Json::NUL, Json::PARSE_MISS_QUOTATION_MARK);
		default:
			if ((unsigned char)ch < 0x20) {
				return Json(Json::NUL, Json::PARSE_INVALID_STRING_CHAR);
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

Json::Json(const double _n)
	:_number(_n), _type(Json::NUMBER)
{
}

Json::Json(const std::string & _s)
	:_type(Json::STRING)
{
	new(&_string) std::string(_s);
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

Json Json::parse(const std::string & str)
{
	JsonParser jp(str);
	return jp.parse();
}

void Json::copyUnion(const Json & _j)
{
	switch (_j.type())
	{
	case Json::NUL:			break;
	case Json::BOOLEAN:		_boolean = _j._boolean; break;
	case Json::NUMBER:		_number = _j._number; break;
	case Json::STRING:		new(&_string) std::string(_j._string); break;
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
		break;
	case Json::OBJECT:
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

} // namespace lljson
