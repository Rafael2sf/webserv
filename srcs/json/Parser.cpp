#include "Parser.hpp"

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

	static int escape( char c)
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

	static std::string readString(std::string::const_iterator & beg,
		std::string::const_iterator const& end)
	{
		std::string str;
		std::string::const_iterator it = beg + 1;
		size_t size = 0;

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
					throw ParseError("unicode escape is not supported");
				if (escape(*it) == -1)
					throw ParseError("invalid escape character in string");
			}
			it++;
			size++;
		}
		if (size != 0)
			str.resize(size);
		beg += 1;
		for (size_t i = 0; i < size; i++)
		{
			if (*beg == '\\')
				str[i] = escape(*(++beg));
			else
				str[i] = *beg;
			beg++;
		}
		beg++;
		return str;
	}

	void
	jsonReadString(std::string::const_iterator &beg,
				   std::string::const_iterator const &end, Node *&dst)
	{
		dst = new String(readString(beg, end));
	}

	void
	jsonReadInteger(std::string::const_iterator &beg,
					std::string::const_iterator const &end, Node *&dst)
	{
		(void)end;
		long value;
		char *endptr = 0;

		if (*beg == '-' && !isdigit(*(beg + 1)))
			throw ParseError("value expected");
		value = strtol(&*beg, &endptr, 10);
		if (value > 2147483647 || value < -2147483648)
			throw ParseError("numeric values must fit in a signed 32 bits integer");
		beg += (endptr - &*beg);
		if (*beg == '.')
			throw ParseError("floating point values are not supported");
		if (*beg == 'e' || *beg == 'E')
			throw ParseError("exponent numeric values are not supported");
		dst = new Integer(value);
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
