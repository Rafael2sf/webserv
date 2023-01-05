#include "Server.hpp"
#include <algorithm>

#define RETURN_EQ(X, Y) if (X == Y) {return Y;}

namespace HTTP
{
	static int count(JSON::Node const& nref, std::string const& s)
	{
		int	i = 0;

		if (nref.type() != JSON::object && nref.type() != JSON::array)
			return 0;
		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip())
		{
			if (it->getProperty() == s)
				i++;
		}
		return i;
	}

	static int isOfType( JSON::Node const& nref, bool unique, int types )
	{
		size_t	count = 0;

		if (!unique && !std::distance(nref.begin(), nref.end()))
		{
			return configError("invalid value type",
				nref.getProperty().c_str());
		}
		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip(), count++)
		{
			if ((unique && count > 0)
				|| ((it->type() & types) == 0))
			{
				return configError("invalid value type",
					 nref.getProperty().c_str());
			}
		}
		return 0;
	}

	static int validateRedirect( JSON::Node const& nref )
	{
		int rcode;
		if (nref.type() != JSON::array)
		{
			return configError("invalid value type",
				nref.getProperty().c_str());
		}

		if (dynamic_cast<JSON::Array const&>(nref).impl.size() != 2)
		{
			return configError("invalid value count",
				 nref.getProperty().c_str());
		}

		if (nref.begin()->type() != JSON::integer
			|| (++nref.begin())->type() != JSON::string
			|| (++nref.begin())->as<std::string const&>().empty())
		{
			return configError("invalid value type",
				nref.getProperty().c_str());
		}

		rcode = nref.begin()->as<int>();
		if ((rcode >= 301 && rcode <= 303) || rcode == 307 || rcode == 308)
			return 0;

		return configError("invalid redirect code",
			itos(rcode, std::dec).c_str());
	}

	static int validateIndex( JSON::Node const& nref )
	{
		if ((nref.type() != JSON::string && nref.type() != JSON::array)
			|| (nref.type() == JSON::array && nref.begin() == nref.end()))
		{
			return configError("invalid value type",
				nref.getProperty().c_str());
		}

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it++)
		{
			if (it->type() != JSON::string)
			{
				return configError("invalid index file",
					nref.getProperty().c_str());
			}
			if (it->as<std::string const&>().empty()
				|| it->as<std::string const&>()[0] == '/')
			{
				return configError("invalid index file",
					it->as<std::string const&>().c_str());
			}
		}
		return 0;
	}

	static int validateLimitExcept( JSON::Node const& nref )
	{
		static char const*methods[] = {
			"CONNECT",
			"DELETE",
			"GET",
			"HEAD",
			"OPTIONS",
			"PATCH",
			"POST",
			"PUT",
			"TRACE",
		};

		if (nref.type() != JSON::array && nref.type() != JSON::string)
		{
			return configError("invalid value type",
				nref.getProperty().c_str());
		}

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it++)
		{
			if (it->type() != JSON::string)
			{
				return configError("invalid value type",
					nref.getProperty().c_str());
			}
			if (std::find(methods, methods + 9, 
				it->as<std::string const&>()) == (methods + 9))
			{
				return configError("no such method",
					it->as<std::string const&>().c_str());
			}
		}
		return 0;
	}

	static int validatePath( JSON::Node const& nref )
	{
		static char const* arr[] = {"root", "index", 
			"cgi", "upload_store", "redirect", 
			"autoindex", "limit_except"};

		if (nref.type() != JSON::object)
		{
			return configError("invalid value type",
				nref.getProperty().c_str());
		}

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip())
		{
			if (count(nref, it->getProperty()) > 1)
			{
				return configError("duplicated directive",
					it->getProperty().c_str());
			}
			char const** s = std::find(arr, arr + 7, it->getProperty());
			switch (s - arr)
			{
				case 0:
					RETURN_EQ(isOfType(*it, true, JSON::string), -1);
					continue ;
				case 1:
					RETURN_EQ(validateIndex(*it), -1);
					continue ;
				case 2:
					RETURN_EQ(isOfType(*it, true, JSON::string), -1);
					continue ;
				case 3:
					RETURN_EQ(isOfType(*it, true, JSON::string), -1);
					continue ;
				case 4:
					RETURN_EQ(validateRedirect(*it), -1);
					continue ;
				case 5:
					RETURN_EQ(isOfType(*it, false, JSON::boolean), -1);
					continue ;
				case 6:
					RETURN_EQ(validateLimitExcept(*it), -1);
					continue ;
				default:
					return configError("unknown directive",
						it->getProperty().c_str());
			}
		}
		return 0;
	}

	static int validateLocationString( std::string const& str )
	{
		std::string::const_iterator it = str.begin();
		
		if (str.empty())
			return -1;
		if (str.size() == 1 && *it != '/')
			return -1;
		if (*it == '*')
		{
			it++;
			if (str.size() <= 2 || *it != '.')
				return -1;
		}
		else
		{
			if (*it == '=')
				it++;
			if (*it != '/')
				return -1;
		}
		for (it++; it != str.end(); it++)
		{
			if (!isgraph(*it) || *it == '*')
				return -1;
		}
		return 0;
	}

	static int validateLocations( JSON::Node const& nref )
	{
		if (nref.type() != JSON::object)
		{
			return configError("invalid value type",
				nref.getProperty().c_str());
		}

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip())
		{
			if (count(nref, it->getProperty()) > 1)
			{
				return configError("duplicated location",
					it->getProperty().c_str());
			}
			if (validateLocationString(it->getProperty()) == -1)
			{
				return configError("invalid location path",
					it->getProperty().c_str());
			}
			if (validatePath(*it) == -1)
				return -1;
		}
		return 0;
	}

	static int validateErrorPage( JSON::Node const& nref )
	{
		int error;
		std::vector<int> used_errors;

		if (nref.type() != JSON::object)
		{
			return configError("invalid value type",
				nref.getProperty().c_str());
		}

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip())
		{
			if (it->getProperty().empty() || it->getProperty().at(0) != '/')
			{
				return configError("invalid error page",
					it->getProperty().c_str());
			}
			if ((it->type() != JSON::integer && it->type() != JSON::array)
				|| (it->type() == JSON::array 
				&& std::distance(it->begin(), it->end()) <= 0))
			{
				return configError("invalid value type",
					it->getProperty().c_str());
			}

			for (JSON::Node::const_iterator it2 = it->begin();
				it2 != it->end(); it2++)
			{
				if (it2->type() != JSON::integer)
				{
					return configError("invalid value type",
						it->getProperty().c_str());
				}
				error = it2->as<int>();
				if (error < 300 || !Server::error.count(error))
				{
					return configError("invalid error code",
						it->getProperty().c_str());
				}
				if (std::find(used_errors.begin(), used_errors.end(),
					 error) != used_errors.end())
				{
					return configError("duplicated error code",
						nref.getProperty().c_str());
				}
				used_errors.push_back(error);
			}
		}
		return 0;
	}

	static int validateServerNameString( std::string const& str )
	{
		bool leading_wcard = false;
		std::string::const_iterator it = str.begin();
		
		if (str.empty())
			return -1;
		if (*it == '*')
		{
			it++;
			if (it == str.end() || it == --str.end() || *it != '.')
				return -1;
			leading_wcard = true;
		}
		for (; it != str.end(); it++)
		{
			if (!isgraph(*it))
				return -1;
			if (*it == '.'
				&& (*(it + 1) == '.'|| *(it - 1) == '.'))
				return -1;
			if (*it == '*' && (leading_wcard || str.size() <= 2
				|| ((it + 1) != str.end() || *(it - 1) != '.')))
				return -1;
		}
		return 0;
	}

	static int validateServerName( JSON::Node const& nref )
	{
		if (nref.type() != JSON::array 
			&& nref.type() != JSON::string)
		{
			return configError("invalid value type",
				nref.getProperty().c_str());
		}
		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it++)
		{
			if (it->type() != JSON::string)
			{
				return configError("invalid value type",
					nref.getProperty().c_str());
			}
			if (validateServerNameString(it->as<std::string const&>()) == -1)
			{
				return configError("invalid server_name string",
					it->as<std::string const&>().c_str());
			}
		}
		return 0;
	}

	static int validateServer( JSON::Node const& nref )
	{
		static char const* arr[] = {"listen", "error_page",
			"client_max_body_size", "location", "server_name"};

		if (nref.type() != JSON::object)
		{
			return configError("invalid value type",
				nref.getProperty().c_str());
		}

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip())
		{
			if (count(nref, it->getProperty()) > 1)
			{
				return configError("duplicated directive",
					it->getProperty().c_str());
			}
			char const** s = std::find(arr, arr + 5, it->getProperty());
			switch (s - arr)
			{
				case 0:
					RETURN_EQ(isOfType(*it, false, JSON::string), -1);
					continue ;
				case 1:
					RETURN_EQ(validateErrorPage(*it), -1);
					continue ;
				case 2:
					RETURN_EQ(isOfType(*it, true, JSON::integer), -1);
					if (it->as<int>() < 0)
					{
						return configError("invalid client_max_body_size",
							itos(it->as<int>(), std::dec).c_str());
					}
					continue ;
				case 3:
					RETURN_EQ(validateLocations(*it),  -1);
					continue ;
				case 4:
					RETURN_EQ(validateServerName(*it), -1);
					continue ;
				default:
					return configError("unknown directive",
						it->getProperty().c_str());
			}
		}
		return 0;
	}

	int validateConfig( JSON::Json const& json )
	{
		for (JSON::Node::iterator it = json.tokens->begin();
			it != json.tokens->end(); it.skip())
		{
			if (it->type() != JSON::object
				&& it->getProperty().empty())
			{
				return configError("invalid value type", "");
			}
			if (it->getProperty() != "server")
			{
				return configError("unknown directive",
					 it->getProperty().c_str());
			}
			else
				RETURN_EQ(validateServer(*it), -1);
		}
		return 0;
	}
}
