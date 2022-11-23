#include "Json.hpp"

namespace ft
{

	static int jsonParseLoop( t_jparser_info & info );
	static int jsonParseToken( t_jparser_info & info );
	static void jsonInsertToken( Json & x ,t_jparser_info & info );

	int Json::parse( char const* filepath )
	{
		int				state;
		t_jparser_info	info;

		memset(&info, 0, sizeof(info));
		info.row = 1;
		info.col = 1;
		info.path = const_cast<char *>(filepath);
		if (!(info.bytes = jsonReadFile(filepath)))
			return ft::err(-1);
		info.cursor = info.bytes;
		while (*(info.cursor))
		{
			if (!(state = jsonParseLoop(info)))
				break ;
			if (state == -1)
			{
				if (info.property)
					delete[] info.property;
				delete[] info.bytes;
				return -1;
			}
			else if (info.token)
				jsonInsertToken(*this, info);
			info.cursor++;
			info.col++;
		}
		delete[] info.bytes;
		return info.depth != 0 ? jsonErr(info, "missing closing bracket(s)") : 0;
	}

	static int jsonParseLoop( t_jparser_info & info )
	{
		while (*(info.cursor) && isspace(*(info.cursor)))
		{
			if (*(info.cursor) == '\n')
			{
				info.row++;
				info.col = 1;
			}
			else
				info.col++;
			info.cursor++;
		}
		if (!*(info.cursor))
			return 0;
		if (info.depth == 0)
		{
			if (info.prev)
				return jsonErr(info, "end of file expected");
			else if (*(info.cursor) != '{')
				return jsonErr(info, "json object expected");
			info.depth++;
			return 1;
		}
		return jsonParseToken(info) < 0 ? -1 : 1;
	}

	static int jsonParseToken( t_jparser_info & info )
	{
		switch (*(info.cursor))
		{
			case '{':
				return jsonParseOpenBracket(info);
			case '}':
				return jsonParseCloseBracket(info);
			case '\"':
				return jsonParseString(info);
			case ':':
				return jsonParseColon(info);
			case ',':
				return jsonParseComma(info);
			case 't':
				return jsonParseBoolean(info);
			case 'f':
				return jsonParseBoolean(info);
			case 'n':
				return jsonParseNull(info);
			case '[':
				return jsonParseArray(info);
			default:
				if (*(info.cursor) == '-'
					|| ( *(info.cursor) >= '0' && *(info.cursor) <= '9'))
					return jsonParseInteger(info);
				if (info.property)
					return jsonErr(info, "value expected");
				return jsonErr(info, "property keys must be doublequoted");
		}
		return 0;
	}

	static void jsonInsertToken( Json & json ,t_jparser_info & info )
	{
		if (info.depth == 1)
		{
			json.tokens.push_back(info.token);
			info.prev = info.token;
		}
		else
		{
			dynamic_cast<JsonObject *>(info.prev)->data.push_back(info.token);
			info.token->setParent(info.prev);
		}
		if (dynamic_cast<JsonObject *>(info.token) != NULL)
		{
			info.depth++;
			info.prev = info.token;
		}
		info.token->setProperty(info.property);
		info.as_colon = 0;
		info.as_comma = 0;
		info.property = 0;
		info.token = 0;
	}
}
