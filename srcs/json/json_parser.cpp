#include "JsonToken.hpp"

namespace JSON
{
	static bool
	jsonValidValue( t_jparser_info const& info )
	{
		if (!info.property)
		{
			jsonErr(info, "property expected");
			return false;
		}
		if (!info.as_colon)
		{
			jsonErr(info, "colon expected");
			return false;
		}
		return true;
	}

	static std::pair<char *, size_t> 
	jsonScanString( t_jparser_info const& info )
	{
		std::pair<char *, size_t> ret;
		const char * s = info.cursor;
		const char * end = strchr(s + 1, '\"');

		if (!end)
		{
			jsonErr(info, "unexpected end of string");
			return std::make_pair<char *, size_t>(NULL, __SIZE_MAX__);
		}
		if (*(s + 1) == '\"')
		{
			ret.first = new char[1];
			ret.first[0] = 0;
			ret.second = 0;
			return ret;
		}
		ret.second = (end - s - 1);
		ret.first = new char[ret.second + 1];
		memmove(ret.first, s + 1, ret.second);
		ret.first[ret.second] = 0;
		return ret;
	}

	static std::pair<int, size_t> 
	jsonScanInteger( t_jparser_info const& info )
	{
		long value;
		char * endptr = 0;

		if (*(info.cursor) == '-' && !isdigit(*(info.cursor + 1)))
		{
			jsonErr(info, "value expected");
			return std::make_pair(0, __SIZE_MAX__);
		}
		value = strtol(info.cursor, &endptr, 10);
		if (value > 2147483647 || value < -2147483648)
		{
			jsonErr(info, "numeric values must fit in a signed 32 bits integer");
			return std::make_pair(0, __SIZE_MAX__);
		}
		if (*endptr == '.')
		{
			jsonErr(info, "floating point values are not supported");
			return std::make_pair(0, __SIZE_MAX__);
		}
		return std::make_pair(value, endptr - info.cursor);
	}

	static std::pair<bool, size_t> 
	jsonScanBoolean( t_jparser_info const& info )
	{
		if (!strncmp(info.cursor, "false", 5))
			return std::make_pair(false, 5);
		else if (!strncmp(info.cursor, "true", 4))
			return std::make_pair(true, 4);
		jsonErr(info, "value expected");
		return std::make_pair(false, __SIZE_MAX__);
	}

	static std::pair<bool, size_t> 
	jsonScanNull( t_jparser_info const& info )
	{
		if (!strncmp(info.cursor, "null", 4))
			return std::make_pair(true, 4);
		jsonErr(info, "value expected");
		return std::make_pair(false, __SIZE_MAX__);
	}

	static std::pair<JsonToken *, size_t> 
	jsonScanArraySelect( t_jparser_info const& info, char c )
	{
		std::pair<JsonToken *, size_t>	r;
		std::pair<char *, size_t>		s;
		std::pair<bool, size_t>			b;
		std::pair<int, size_t>			i;

		if (c == 't' || c == 'f')
			c = 't';
		switch (c)
		{
			case '\"':
				s = jsonScanString(info);
				if (s.second == __SIZE_MAX__) 
					return std::make_pair((JsonToken *)0, __SIZE_MAX__);
				r.first = new JsonString(s.first);
				r.second = s.second + 2;
				break ;
			case 't':
				b = jsonScanBoolean(info);
				if (s.second == __SIZE_MAX__) 
					return std::make_pair((JsonToken *)0, __SIZE_MAX__);
				r.first = new JsonBoolean(b.first);
				r.second = b.second;
				break ;
			case 'n':
				b = jsonScanNull(info);
				if (s.second == __SIZE_MAX__) 
					return std::make_pair((JsonToken *)0, __SIZE_MAX__);
				r.first = new JsonNull();
				r.second = b.second;
				break ;
			default:
				if (*(info.cursor) == '-'
					|| ( *(info.cursor) >= '0' && *(info.cursor) <= '9'))
				{
					i = jsonScanInteger(info);
					if (i.second == __SIZE_MAX__) 
						return std::make_pair((JsonToken *)0, __SIZE_MAX__);
					r.first = new JsonInteger(i.first);
					r.second = i.second;
					break ;
				}
				jsonErr(info, "value expected");
				return std::make_pair((JsonToken *)0, __SIZE_MAX__);
		}
		return r;
	}

	static std::pair<std::vector<JsonToken *>, size_t> 
	jsonScanArray( t_jparser_info & info )
	{
		std::vector<JsonToken *> 		ret;
		std::pair<JsonToken *, size_t>	token;
		char * begin = info.cursor;
		bool has_comma = false;

		info.col++;info.cursor++;
		while (*(info.cursor))
		{
			if (isspace(*(info.cursor)))
			{
				info.col++;info.cursor++;
				continue ;
			}
			else if (*(info.cursor) == ',')
			{
				if (has_comma || ret.empty())
				{
					jsonErr(info, "value expected");
					return std::make_pair(ret, __SIZE_MAX__);
				}
				has_comma = true;
				info.col++;info.cursor++;
			}
			else if (*(info.cursor) == ']')
			{
				if (has_comma)
				{
					jsonErr(info, "trailing comma");
					return std::make_pair(ret, __SIZE_MAX__);
				}
				break ;
			}
			else
			{
				token = jsonScanArraySelect(info, *(info.cursor));
				if (token.second == __SIZE_MAX__)
					return std::make_pair(ret, __SIZE_MAX__);
				if (!has_comma && !ret.empty())
				{
					jsonErr(info, "expected comma");
					return std::make_pair(ret, __SIZE_MAX__);
				}
				has_comma = false;
				info.col += token.second;
				info.cursor += token.second;
				ret.push_back(token.first);
			}
		}
		if (*(info.cursor) != ']')
			return std::make_pair(ret, __SIZE_MAX__);
		return std::make_pair(ret, info.cursor - begin);
	}

	int jsonParseOpenBracket( t_jparser_info & info )
	{
		if (!jsonValidValue(info))
			return -1;
		info.token = new JsonObject();
		return 0;
	}

	int jsonParseCloseBracket( t_jparser_info & info )
	{
		if (info.property)
		{
			if (!info.as_colon)
				return jsonErr(info, "colon expected");
			return jsonErr(info, "value expected");
		}
		if (info.as_comma)
			return jsonErr(info, "trailing comma");
		if (info.prev && info.prev->getParent())
			info.prev = info.prev->getParent();
		info.depth--;
		return 0;
	}

	int jsonParseString( t_jparser_info & info )
	{
		std::pair<char *, size_t> o;

		if (info.property)
		{
			if (!info.as_colon)
				return jsonErr(info, "colon expected");
			o = jsonScanString(info);
			if (o.second == __SIZE_MAX__)
				return (-1);
			info.token = new JsonString(o.first);
			info.cursor += o.second + 1;
			info.col += o.second + 1;
		}
		else
		{
			if (info.prev && !info.as_comma && (info.depth == 1
				|| !dynamic_cast<JsonObject *>(info.prev)->data.empty()))
				return jsonErr(info, "expected comma");
			o = jsonScanString(info);
			if (o.second == __SIZE_MAX__)
				return (-1);
			info.property = o.first;
			info.as_comma = false;
			info.cursor += o.second + 1;
			info.col += o.second + 1;
		}
		return 0;
	}

	int jsonParseColon( t_jparser_info & info )
	{
		if (info.as_colon)
			return jsonErr(info, "value expected");
		if (!info.property)
			return jsonErr(info, "property expected");
		info.as_colon = !info.as_colon;
		return 0;
	}

	int jsonParseComma( t_jparser_info & info )
	{
		if (info.property)
			return jsonErr(info, "colon expected");
		if (info.as_colon)
			return jsonErr(info, "value expected");
		if (info.as_comma || !info.prev
			|| (info.depth != 1 
			&& static_cast<JsonObject *>(info.prev)->data.empty()))
		{ return jsonErr(info, "property expected"); }
		info.as_comma = true;
		return 0;
	}

	int jsonParseInteger( t_jparser_info & info )
	{
		std::pair<int, size_t> o;

		if (!jsonValidValue(info))
			return -1;
		o = jsonScanInteger(info);
		if (o.second == __SIZE_MAX__)
			return -1;
		info.col += o.second - 1;
		info.cursor += o.second - 1;
		info.token = new JsonInteger(o.first);
		return 0;
	}

	int jsonParseBoolean( t_jparser_info & info )
	{
		std::pair<bool, size_t> o;

		if (!jsonValidValue(info))
			return -1;
		o = jsonScanBoolean(info);
		if (o.second == __SIZE_MAX__)
			return -1;
		info.col += o.second - 1;
		info.cursor += o.second - 1;
		info.token = new JsonBoolean(o.first);
		return 0;
	}

	int jsonParseNull( t_jparser_info & info )
	{
		std::pair<bool, size_t> o;

		if (!jsonValidValue(info))
			return -1;
		o = jsonScanNull(info);
		if (o.second == __SIZE_MAX__)
			return -1;
		info.col += o.second - 1;
		info.cursor += o.second - 1;
		info.token = new JsonNull();
		return 0;
	}

	int jsonParseArray( t_jparser_info & info )
	{
		std::pair<std::vector<JsonToken *>, size_t> o;

		if (!jsonValidValue(info))
			return -1;
		o = jsonScanArray(info);
		if (o.second == __SIZE_MAX__)
		{
			for (std::vector<JsonToken *>::const_iterator it = o.first.begin();
				it != o.first.end(); it++)
				delete *it;
			return -1;
		}
		info.token = new JsonArray(o.first);
		return 0;
	}
}
