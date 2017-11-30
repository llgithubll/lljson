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
	char nextToken();
	void consumeWhitespace();
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

Json Json::parse(const std::string & str)
{
	JsonParser jp(str);
	return jp.parse();
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

} // namespace lljson
