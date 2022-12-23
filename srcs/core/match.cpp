#include "Server.hpp"

namespace HTTP
{
	static int strlwcmp(std::string const& wstr, std::string const& str)
	{
		if (*wstr.begin() != '*')
			return 0;
		size_t len = wstr.find(":");
		if (len == std::string::npos)
			len = wstr.size() - 1;
		if (str.find(wstr.c_str() + 1, 0, len) != std::string::npos)
			return len;
		return 0;
	}

	static int strtwcmp(std::string const& wstr, std::string const& str)
	{
		if (*--wstr.end() != '*')
			return 0;
		if (str.find(wstr.c_str(), 0, wstr.size() - 1) == 0)
			return wstr.size() - 1;
		return 0;
	}

	static bool serverncmp(std::string const& host, std::string const& sname)
	{
		size_t i = 0;

		if (sname.empty() || sname[0] == '*' || *--sname.end() == '*')
			return false;
		while (i < host.size() && i < sname.size()
			&& host[i] == sname[i])
		{
			if (sname[i] == ':' || host[i] == ':')
				break ;
			i++;
		}
		if ((!sname[i] || sname[i] == ':') && (!host[i] || host[i] == ':'))
			return true;
		return (i == host.size() - 1) ? true : false;
	}

	static JSON::Node * matchServerName(Sockets const& so,
		Client const& cli,  std::list<t_sock_info const*> & interest)
	{
		// match exact server_name
		std::string const* req_server_name  = cli.req.getField("host");
		// PART 2 match exact server_name
		for (std::list<t_sock_info>::const_iterator it = so.list.begin();
			it != so.list.end(); it++)
		{
			JSON::Node * server_name = it->config->search(1, "server_name");
			if (server_name)
			{
				for (JSON::Node::iterator name = server_name->begin();
					name != server_name->end(); name++)
				{
					if (serverncmp(*req_server_name,
						name->as<std::string const&>()))
					{
						return it->config;
					}
				}
			}
		}

		t_sock_info const* last_match = 0;
		int match_length = 0;

		// match any leading wildcard
		for (std::list<t_sock_info const*>::const_iterator it = interest.begin();
			it != interest.end(); it++)
		{
			JSON::Node * server_name = (*it)->config->search(1, "server_name");
			if (server_name)
			{
				for (JSON::Node::iterator name = server_name->begin();
					name != server_name->end(); name++)
				{
					int curr_match_len = strlwcmp(name->as<std::string const&>(),
						*req_server_name);
					if (curr_match_len > match_length)
					{
						last_match = *it;
						match_length = curr_match_len;
					}
				}
			}
		}
		if (last_match)
			return last_match->config;

		// match any trailing wildcard
		for (std::list<t_sock_info const*>::const_iterator it = interest.begin();
			it != interest.end(); it++)
		{
			JSON::Node * server_name = (*it)->config->search(1, "server_name");
			if (server_name)
			{
				for (JSON::Node::iterator name = server_name->begin();
					name != server_name->end(); name++)
				{
					int curr_match_len = strtwcmp(name->as<std::string const&>(),
						*req_server_name);
					if (curr_match_len > match_length)
					{
						last_match = *it;
						match_length = curr_match_len;
					}
				}
			}
		}
		if (last_match)
			return last_match->config;

		return (*interest.begin())->config;
	}

	JSON::Node *matchServer(Sockets const& so, Client const &cli)
	{
		std::list<t_sock_info const*> interest;
		bool perfect_match = 0;

		// Match any and equal host:port
		for (std::list<t_sock_info>::const_iterator it = so.list.begin();
			 it != so.list.end(); it++)
		{
			if ((*it).addr.sin_port == cli.ai.sin_port
				&& (((*it).addr.sin_addr.s_addr == cli.ai.sin_addr.s_addr)
					|| (*it).addr.sin_addr.s_addr == 0))
			{
				if (((*it).addr.sin_addr.s_addr) != 0)
					perfect_match = true;
				interest.push_back(&(*it));
			}
		}

		// if equal match ignore any (0.0.0.0)
		if (perfect_match && cli.ai.sin_addr.s_addr != 0)
		{
			std::list<t_sock_info const*>::iterator tmp;
			for (std::list<t_sock_info const*>::iterator it = interest.begin();
				 it != interest.end(); it++)
			{
				if (((*it)->addr.sin_addr.s_addr) == 0)
				{
					tmp = it++;
					interest.erase(tmp);
					it--;
				}
			}
		}

		if (interest.size() == 0)
			return 0;
		else if (interest.size() == 1)
			return (*interest.begin())->config;
		return matchServerName(so, cli, interest);
	}

	JSON::Node *matchLocation(JSON::Node *serv, std::string const &path)
	{
		JSON::Object *	tmp;
		size_t 			last_len =	0;
		JSON::Node *	last_match = 0;
		size_t 			i = 0;

		tmp = dynamic_cast<JSON::Object *>(serv->search(1, "location"));
		if (!tmp)
			return 0;
		
		// match exact
		for (JSON::Node::iterator loc = tmp->begin();
			loc != tmp->end(); loc.skip())
		{
			if (*loc->getProperty().begin() != '=')
				continue ;
			if (path.size() != loc->getProperty().size() - 1)
				continue ;
			if (!loc->getProperty().compare(1, path.size(), path))
				return &*loc;
		}

		// largest match
		for (JSON::Node::iterator loc = tmp->begin();
			loc != tmp->end(); loc.skip())
		{
			if (loc->getProperty()[0] == '*'
				|| loc->getProperty()[0] == '=')
				continue ; // wrong / match =/
			i = path.find(loc->getProperty(), 0);
			if (i != std::string::npos && i == 0)
			{
					i = loc->getProperty().size();
					if (i > last_len && last_len != path.size())
					{
						last_len = i;
						last_match = &*loc;
					}
			}
		}

		// match *
		char const * type = strrchr(path.c_str(), '.');
		if (!type)
			return last_match;
		for (JSON::Node::iterator loc = tmp->begin();
			loc != tmp->end(); loc.skip())
		{
			if (loc->getProperty().size() == strlen(type) + 1
				&& loc->getProperty()[0] == '*')
			{
				if (!loc->getProperty().compare(1, strlen(type), type))
					return &*loc;
			}
		}
		return last_match;
	}
}
