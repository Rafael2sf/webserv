#include "Parser.hpp"
#include <limits>
#include <cmath>
#include <cerrno>
// #include <bitset>

namespace JSON
{
	Parser::~Parser()
	{}

	Parser::Parser(void)
	: _row(1), _depth(0), _cur(0), _root(0)
	{}

	Parser::Parser(Parser const &other)
	: _row(1), _depth(0), _cur(0), _root(0)
	{
		*this = other;
	}

	Parser &Parser::operator=(Parser const &other)
	{
		(void)other;
		return *this;
	}

	std::pair<size_t, size_t>
	 Parser::errPos( void ) const
	{
		return std::make_pair(_row, _beg - _lf);
	}

	void Parser::clear(void)
	{
		if (_root)
		{
			while (_root->getParent())
				_root = _root->getParent();
			delete _root;
		}
		if (_cur)
		{
			while (_cur->getParent())
				_cur = _cur->getParent();
			delete _cur;
		}
		reset();
	}

	void Parser::reset(void)
	{
		_cur = 0;
		_root = 0;
		_depth = 0;
		_row = 1;
	}

	Node *Parser::build(std::string const &str)
	{
		_beg = str.begin();
		_end = str.end();
		_lf = _beg;

		_root = new Object();
		if (_beg == _end)
			return _root;
		_ignoreSpaces(false);
		if (*_beg != '{')
		{
			_readValue("", _root);
			dynamic_cast<Object *>(_root)->impl.insert(_cur);
			_cur = 0;
		}
		else
		{
			_readObject(_root);
			if (_depth != 0)
				throw ParseError("expected comma or closing brace");
		}
		_ignoreSpaces(false);
		if (_beg != _end)
			throw ParseError("end of file expected");
		return _root;
	}

	// Make this return new object wihout using prev
	void Parser::_readObject( Node * parent )
	{
		bool comma = false;

		_ignoreSpaces(true);
		if (*_beg != '{')
			throw ParseError("expected open brace");
		_depth++;
		_beg++;
		_ignoreSpaces(false);
		while (_beg != _end)
		{
			if (_depth == 0 && _beg != _end)
				throw ParseError("end of file expected");
			if (*_beg == '}')
			{
				if (comma)
					throw ParseError("trailing comma");
			}
			else
				_readPair(parent);
			_ignoreSpaces(false);
			if (*_beg == ',')
			{
				comma = true;
				_beg++;
			}
			else if (*_beg == '}')
			{
				_depth--;
				_beg++;
				return ;
			}
			else
				throw ParseError("expected comma or closing brace");
			_ignoreSpaces(false);
		}
	}

	static int escapeCharacter( char c)
	{
		switch (c)
		{
			case '\"':
				return '\"';
			case '\\':
				return '\\';
			case '/':
				return '/';
			case 'b':
				return '\b';
			case 'f':
				return '\f';
			case 'r':
				return '\r';
			case 'n':
				return '\n';
			case 't':
				return '\t';
			default:
				break ;
		}
		return -1;
	}

	static void utf8Encode(std::string &s, int utf)
	{
		if (utf <= 0x7F) {
			s += (char) utf;
		}
		else if (utf <= 0x07FF) {
			s += (char) (((utf >> 6) & 0x1F) | 0xC0);
			s += (char) (((utf >> 0) & 0x3F) | 0x80);
		}
		else if (utf <= 0xFFFF) {
			s += (char) (((utf >> 12) & 0x0F) | 0xE0);
			s += (char) (((utf >>  6) & 0x3F) | 0x80);
			s += (char) (((utf >>  0) & 0x3F) | 0x80);
		}
		else if (utf <= 0x10FFFF) {
			s += (char) (((utf >> 18) & 0x07) | 0xF0);
			s += (char) (((utf >> 12) & 0x3F) | 0x80);
			s += (char) (((utf >>  6) & 0x3F) | 0x80);
			s += (char) (((utf >>  0) & 0x3F) | 0x80);
		}
		else { 
			s += (char) 0xEF;
			s += (char) 0xBF;
			s += (char) 0xBD;
		}
	}

	static void escapeUnicode(std::string & str,
		std::string::const_iterator & beg,
		std::string::const_iterator const& end)
	{
		int digits = 0;
		int value = 0;

		beg++;
		while (beg != end && (isdigit(*beg) 
			|| (tolower(*beg) >= 'a' && tolower(*beg) <= 'f')))
		{
			value *= 16;
			if (isdigit(*beg))
				value += (*beg - '0');
			else
				value += (tolower(*beg) - 'a' + 10);
			beg++;
			digits++;
			if (digits == 4)
				break ;
		}
		if (beg == end)
			throw ParseError("unexpected end of string");
		if (digits != 4)
			throw ParseError("invalide unicode sequence in string");
		beg--;
		utf8Encode(str, value);
	}

	static std::string readString(std::string::const_iterator & beg,
		std::string::const_iterator const& end)
	{
		std::string str;
		std::string::const_iterator it = beg + 1;

		while (it != end)
		{
			if (*it == '\"')
				break ;
			else if (*it == '\n')
				throw ParseError("unexpected end of string");
			else if (*it == '\\')
			{
				it++;
				if (it == end)
					throw ParseError("unexpected end of string");
				if (*it == 'u')
					escapeUnicode(str, it, end);
				else if (escapeCharacter(*it) == -1)
					throw ParseError("invalid escape character in string");
				else
					str += escapeCharacter(*it);
			}
			else
				str += *it;
			it++;
		}
		if (it == end)
			throw ParseError("unexpected end of string");
		beg = ++it;
		return str;
	}

	void
	jsonReadString(std::string::const_iterator &beg,
		std::string::const_iterator const &end, Node *&dst)
	{
		dst = new String(readString(beg, end));
	}

	void
	jsonReadFloat(std::string::const_iterator &beg,
		std::string::const_iterator const &end, Node *&dst)
	{
		double n = 0;
		char *endptr = 0;

		(void)end;
		n = strtof(&*beg, &endptr);
		if (!endptr || !*endptr)
			throw ParseError("unexpected end of file");
		if (*endptr == 'e' || *endptr == 'E')
			throw ParseError("unexpected end of number");
		if (n > std::numeric_limits<float>::max() 
			|| n < -std::numeric_limits<float>::max())
			throw ParseError("floating-point values must fit in a signed 32 bits float");
		beg += (endptr - &*beg);
		dst = new Point(n);
	}

	static int analyzeInteger(char const* ptr, char** dot, char** exp, char** endptr)
	{
		*dot = 0;
		*exp = 0;
		if (*ptr == '-')
			ptr++;
		if (!isdigit(*ptr))
			return -1;
		if (*ptr == '0' && *(ptr + 1) == '0')
		{
			*endptr = const_cast<char *>(ptr + 1);
			return 0;
		}
		while (*ptr)
		{
			if (*ptr == '.')
			{
				if (*dot || !isdigit(*(ptr + 1)))
					return -1;
				*dot = const_cast<char *>(ptr);
			}
			else if (*ptr == 'e' || *ptr == 'E')
			{
				if (*(ptr + 1) == '-' || *(ptr + 1) == '+')
					ptr++;
				if (*exp || !isdigit(*(ptr + 1)))
					return -1;
				*exp = const_cast<char *>(ptr);
			}
			else if (!isdigit(*ptr))
				break ;
			ptr++;
		}
		*endptr = const_cast<char *>(ptr);
		return 0;
	}

	void
	jsonReadInteger(std::string::const_iterator &beg,
					std::string::const_iterator const &end, Node *&dst)
	{
		double value = 0;
		char * arr[3];

		(void) end;
		errno = 0;
		if (analyzeInteger(&*beg, &arr[0], &arr[1], &arr[2]) == -1)
			throw ParseError("unexpected end of number");
		if ((*beg == '0' && *(beg + 1)  != '.')
			|| (*beg == '-' && *(beg + 1) == '0' && *(beg + 2) != '.'))
			value = 0;
		else
			value = strtod(&*beg, &arr[2]);
		if (!arr[2])
			throw ParseError("unexpected end of number");
		beg += (arr[2] - &*beg);
		if (!arr[0])
		{
			if (errno == EDOM || errno == ERANGE
				|| value > 2147483647 || value < -2147483648)
				throw ParseError("integer values must fit in a signed 32 bits integer");	
			dst = new Integer(value);
		}
		else
		{
			if (errno == EDOM || errno == ERANGE
				|| value > std::numeric_limits<float>::max() 
				|| value < -std::numeric_limits<float>::max())
				throw ParseError("floating-point values must fit in a signed 32 bits float");
			dst = new Point(value);
		}
	}

	void
	jsonReadUnique(std::string::const_iterator &beg,
				   std::string::const_iterator const &end, Node *&dst)
	{
		(void)end;
		if (*beg == 'n')
		{
			dst = new Null;
			beg += 4;
		}
		else if (*beg == 't')
		{
			dst = new Boolean(true);
			beg += 4;
		}
		else
		{
			dst = new Boolean(false);
			beg += 5;
		}
	}

	void Parser::_readPair( Node * parent )
	{
		std::string property;

		if (*_beg != '\"')
			throw ParseError("property keys must be doublequoted");
		property = readString(_beg, _end);
		_ignoreSpaces(true);
		if (*_beg == ':')
			_beg++;
		else
			throw ParseError("colon expected");
		_ignoreSpaces(true);
		_readValue(property, parent);
		if (_cur)
		{
			dynamic_cast<Object *>(parent)->impl.insert(_cur);
			_cur = 0;
		}
	}

	void
	Parser::_readArray(void)
	{
		Array *tmp = new Array();

		_beg++;
		while (1)
		{
			try
			{
				_ignoreSpaces(true);
				if (*_beg == ']')
				{
					if (tmp->impl.size() != 0)
						throw ParseError("trailing comma");
					_beg++;
					break;
				}
				_readValue("", tmp);
				if (_cur && _cur->type() != object)
					tmp->impl.push_back(_cur);
				_cur = 0;
				_ignoreSpaces(true);
				if (*_beg == ']')
				{
					_beg++;
					break;
				}
				else if (*_beg != ',')
					throw ParseError("comma expected");
				_beg++;
			}
			catch (std::exception const &e)
			{
				delete tmp;
				if (_cur)
					delete _cur;
				_cur = 0;
				throw ParseError(e.what());
			}
		}
		_cur = tmp;
	}

	void Parser::_readValue(std::string const& property, Node * parent)
	{
		Node * tmp;

		switch (*_beg)
		{
			case '\"':
				jsonReadString(_beg, _end, _cur);
				break;
			case '{':
				tmp = new Object();
				tmp->setParent(parent);
				if (parent->type() == array)
					dynamic_cast<Array *>(parent)->impl.push_back(tmp);
				else
					dynamic_cast<Object *>(parent)->impl.insert(tmp);
				_cur = 0;
				_readObject(tmp);
				tmp->setProperty(property);
				return ;
			case '[':
				_readArray();
				break;
			default:
				if (*_beg == '-' || (*_beg >= '0' && *_beg <= '9'))
					jsonReadInteger(_beg, _end, _cur);
				else if (!strncmp(&*_beg, "true", 4) 
					|| !strncmp(&*_beg, "false", 5) || !strncmp(&*_beg, "null", 4))
					jsonReadUnique(_beg, _end, _cur);
				else
					throw ParseError("value expected");
		}
		_cur->setProperty(property);
		_cur->setParent(parent);
	}

	void Parser::_ignoreSpaces(bool throw_except)
	{
		while (_beg != _end && _isSpace(*_beg))
		{
			if (*_beg == '\n')
			{
				_lf = _beg;
				_row++;
			}
			_beg++;
		}
		if (throw_except && _beg == _end)
			throw ParseError("unexpected end of file");
	}

	int Parser::_isSpace(char c) const
	{
		return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
	}
}
