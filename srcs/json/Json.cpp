#include "Json.hpp"
#include "Parser.hpp"
#include <climits>
#include <cerrno>

namespace JSON
{
	ParseError::~ParseError() throw()
	{}

	ParseError::ParseError( char const* e )
	: _err(e)
	{}

	char const* ParseError::what( void ) const throw()
	{
		return _err.c_str();
	}

	Json::~Json()
	{
		this->clear();
	}

	Json::Json( void )
	: tokens(0)
	{}

	Json::Json( Json const& other )
	: tokens(0)
	{
		*this = other;
	}

	Json & Json::operator=( Json const& other )
	{
		(void) other;
		return *this;
	}

	bool Json::empty( void ) const
	{
		return !tokens;
	}

	static std::pair<int, std::string>
	  readFile( char const* file )
	{
		std::ifstream	ifs;
		std::string		buffer;

		if (!file)
			return std::make_pair(-1, "");
		ifs.open(file);
		if (!ifs.is_open())
			return std::make_pair(-1, "");
		try
		{
			ifs.seekg (0, std::ios::end);
			if (ifs.tellg() <= 0 || ifs.tellg() == LONG_MAX)
				return std::make_pair(0, "");
			buffer.reserve(ifs.tellg());
			ifs.seekg (0, std::ios::beg);
			if (!ifs.good())
			{
				ifs.close();
				return std::make_pair(-1, "");
			}
			buffer.assign((
				std::istreambuf_iterator<char>(ifs)),
				std::istreambuf_iterator<char>());
		}
		catch (std::exception const& err)
		{
			if (ifs.is_open())
				ifs.close();
			return std::make_pair(-1, "");
		}
		ifs.close();
		return std::make_pair(0, buffer);
	}

	static std::string toStringing( int x )
	{
		std::stringstream	ss;
		std::string			s;

		ss << x;
		ss >> s;
		return s;
	}

	int Json::from( char const* file )
	{
		Parser parser;
		std::pair<int, std::string> f;
		std::pair<size_t, size_t> loc;

		try
		{
			f = readFile(file);
			if (f.first == -1)
			{
				_err = strerror(errno);
				return -1;
			}
			tokens = parser.build(f.second); 
		}
		catch (ParseError const& e)
		{
			tokens = 0;
			loc = parser.errPos();
			_err = toStringing(loc.first) + ":" \
				+ toStringing(loc.second) + ": " + e.what();
			parser.clear();
		}
		catch (std::exception const& e)
		{
			tokens = 0;
			_err = e.what();
			parser.clear();
		}
		return tokens ? 0 : -1;
	}

	std::string const& Json::err( void ) const
	{
		return _err;
	}

	static void rprint(Node const* p, size_t depth)
	{
		size_t i = 0;

		while (i++ != depth)
			std::cerr << " ";
		std::cerr << '\'' << p->getProperty() << "\' : ";
		if (p->type() == object)
		{
			std::cerr << std::endl;
			Object const* tmp = dynamic_cast<Object const*>(p);
			for (std::multiset<Node *>::const_iterator it = tmp->impl.begin();
				it != tmp->impl.end(); it++)
			{
				rprint(*it, depth + 1);
			}
			i = 0;
			// while (i++ != depth)
			// 	std::cout << " ";
			//std::cerr << std::endl;
			return ;
		}
		else if (p->type() == array)
		{
			std::cerr << std::endl;
			Array const* tmp = dynamic_cast<Array const*>(p);
			for (std::vector<Node *>::const_iterator it = tmp->impl.begin();
				it != tmp->impl.end(); it++)
			{
				rprint(*it, depth + 2);
			}
			i = 0;
			// while (i++ != depth)
			// 	std::cout << " ";
			//std::cerr << std::endl;
			return ;
		}
		switch (p->type())
		{
			case string:
				std::cerr << '\"' << p->as<std::string const&>() << '\"';
				break ;
			case boolean:
				std::cerr << '^' << p->as<bool>();
				break ;
			case integer:
				std::cerr << p->as<int>();
				break ;
			case point:
				std::cerr << '!' << p->as<float>();
				break ;
			case null:
				std::cerr << "<null>";
				break ;
			default:
				break ;
		}
		std::cerr << std::endl;
	}

	void Json::cout( void ) const
	{
		rprint(tokens, 0);
	}

	void Json::clear(void)
	{
		if (tokens)
			delete tokens;
		tokens = 0;
		_err.clear();
	}
}
