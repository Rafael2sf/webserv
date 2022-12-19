#include "Server.hpp"
#include <algorithm>

#define RETURN_EQ(X, Y) if (X == Y) {return Y;}

namespace HTTP
{
	static int countEq(JSON::Node const& nref, std::string const& s)
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

	static int vOf( JSON::Node const& nref, bool unique, int types )
	{
		size_t	count = 0;

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip(), count++)
		{
			if ((unique && count > 0)
				|| ((it->type() & types) == 0))
			{
				return err(-1, "invalid argument type",
					 nref.getProperty().c_str());
			}
		}
		return 0;
	}

	static int vRedirect( JSON::Node const& nref )
	{
		int rcode;
		if (nref.type() != JSON::array)
		{
			return err(-1, "config: invalid type",
				nref.getProperty().c_str());
		}

		if (dynamic_cast<JSON::Array const&>(nref).impl.size() != 2)
		{
			return err(-1, "config: invalid argument type",
				 nref.getProperty().c_str());
		}

		if (nref.begin()->type() != JSON::integer
			|| (++nref.begin())->type() != JSON::string)
		{
			return err(-1, "config: invalid argument type",
				 nref.getProperty().c_str());
		}

		rcode = nref.begin()->as<int>();
		if ((rcode >= 301 && rcode <= 303) || rcode == 307 || rcode == 308)
			return 0;
		return err(-1, "config: invalid redirect code", "-");
	}

	static int vPath( JSON::Node const& nref )
	{
		static char const* arr[] = {"root", "index",
			"cgi", "upload_store", "redirect", "autoindex"};

		if (nref.type() != JSON::object)
		{
			return err(-1, "config: invalid type",
				nref.getProperty().c_str());
		}

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip())
		{
			if (countEq(nref, it->getProperty()) > 1)
			{
				return err(-1, "config: duplicate directive",
					it->getProperty().c_str());
			}
			char const** s = std::find(arr, arr + 5, it->getProperty());
			switch (s - arr)
			{
				case 0:
					RETURN_EQ(vOf(*it, true, JSON::string), -1);
					continue ;
				case 1:
					RETURN_EQ(vOf(*it, false, JSON::string), -1);
					continue ;
				case 2:
					RETURN_EQ(vOf(*it, true, JSON::string), -1);
					continue ;
				case 3:
					RETURN_EQ(vOf(*it, true, JSON::string), -1);
					continue ;
				case 4:
					RETURN_EQ(vRedirect(*it), -1);
					continue ;
				case 5:
					RETURN_EQ(vOf(*it, false, JSON::boolean), -1);
					continue ;
				default:
					return err(-1, "config: unknown directive",
						it->getProperty().c_str());
			}
		}
		return 0;
	}

	static int vLocation( JSON::Node const& nref )
	{
		if (nref.type() != JSON::object)
		{
			return err(-1, "config: invalid type",
				nref.getProperty().c_str());
		}

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip())
		{
			if (countEq(nref, it->getProperty()) > 1)
			{
				return err(-1, "config: duplicate directive",
					it->getProperty().c_str());
			}
			if (vPath(*it) == -1)
				return -1;
		}
		return 0;
	}

	static int vErrorPage( JSON::Node const& nref )
	{
		int error;
		std::vector<int> used_errors;

		if (nref.type() != JSON::object)
		{
			return err(-1, "config: invalid type",
				nref.getProperty().c_str());
		}

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip())
		{
			if (it->type() != JSON::integer && it->type() != JSON::array)
			{
				return err(-1, "config: invalid type",
					nref.getProperty().c_str());
			}

			for (JSON::Node::const_iterator it2 = it->begin();
				it2 != it->end(); it2++)
			{
				if (it2->type() != JSON::integer)
				{
					return err(-1, "config: invalid type",
						it2->getProperty().c_str());
				}
				error = it2->as<int>();
				if (error != 400
					&& error != 403
					&& error != 404
					&& error != 405
					&& error != 406
					&& error != 408
					&& error != 411
					&& error != 413
					&& error != 414
					&& error != 415
					&& error != 500
					&& error != 501
					&& error != 505)
				{
					return err(-1, "config: invalid error code",
						nref.getProperty().c_str());
				}
				if (std::find(used_errors.begin(), used_errors.end(),
					 error) != used_errors.end())
				{
					return err(-1, "config: duplicate error code",
						nref.getProperty().c_str());
				}
				used_errors.push_back(error);
			}
		}
		return 0;
	}

	static int vServer( JSON::Node const& nref )
	{
		static char const* arr[] = {"listen", "error_page",
			"client_max_body_size", "location", "server_name"};

		if (nref.type() != JSON::object)
		{
			return err(-1, "config: invalid type",
				nref.getProperty().c_str());
		}

		for (JSON::Node::const_iterator it = nref.begin();
			it != nref.end(); it.skip())
		{
			if (countEq(nref, it->getProperty()) > 1)
			{
				return err(-1, "config: duplicate directive",
					it->getProperty().c_str());
			}
			char const** s = std::find(arr, arr + 4, it->getProperty());
			switch (s - arr)
			{
				case 0:
					RETURN_EQ(vOf(*it, false, JSON::string), -1);
					continue ;
				case 1:
					RETURN_EQ(vErrorPage(*it), -1);
					continue ;
				case 2:
					RETURN_EQ(vOf(*it, true, JSON::integer), -1);
					continue ;
				case 3:
					RETURN_EQ(vLocation(*it),  -1);
					continue ;
				case 4:
					RETURN_EQ(vOf(*it, false, JSON::string), -1);
					continue ;
				default:
					return err(-1, "config: unknown directive",
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
			if (it->getProperty() != "server")
			{
				return err(-1, "config: unknown directive",
					 it->getProperty().c_str());
			}
			else
				RETURN_EQ(vServer(*it), -1);
		}
		return 0;
	}
}
