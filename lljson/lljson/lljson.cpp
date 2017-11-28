#include <cassert>
#include "lljson.h"

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

	char nextToken();
	void consumeWhitespace();
};


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
	return parseValue();
}

Json JsonParser::parseValue()
{
	char ch = nextToken();
	if (_parse_state != Json::PARSE_OK) return Json(Json::NUL, _parse_state);
	switch (ch)
	{
	case 'n':	return parseNull();
	default:	return Json(Json::NUL, Json::PARSE_INVALID_VALUE);
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



Json::Json(Json::Type _t, Json::State _s)
	:_type(_t), _state(_s)
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

Json Json::parse(const std::string & str)
{
	JsonParser jp(str);
	return jp.parse();
}



} // namespace lljson
