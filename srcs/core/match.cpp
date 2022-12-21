#include "Server.hpp"

namespace HTTP
{
	// static int strlwcmp(std::string const& wstr, std::string const& str)
	// {
	// 	size_t index = 0;

	// 	if (*wstr.begin() != '*')
	// 		return 0;
	// 	index = str.find(str, 1);
	// 	if (index == std::string::npos
	// 		|| index == wstr.size() - str.size() - 1)
	// 		return 0;
	// 	return index;
	// }

	// static int strtwcmp(std::string const& wstr, std::string const& str)
	// {
	// 	size_t index = 0;

	// 	if (*--wstr.end() != '*')
	// 		return 0;
	// 	while (index < wstr.size() - 1 && index < str.size()
	// 		&& wstr[index] == str[index])
	// 		index++;
	// 	return index;
	// }

	JSON::Node *matchServer(Sockets const& so, Client const &cli)
	{
		DEBUG2("MATCHING");
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

		// if equal match ignore any
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

		// Just printing
		std::cerr << std::endl <<  "| MATCH LIST |" << std::endl;
		for (std::list<t_sock_info const*>::const_iterator
		 it = interest.begin(); it != interest.end(); it++)
		{
			unsigned int port = ntohl((*it)->addr.sin_addr.s_addr);
			DEBUG(std::cerr \
				<< ((port & 0xff000000) >> 24) << '.' \
				<< ((port & 0x00ff0000) >> 16) << '.' \
				<< ((port & 0x0000ff00) >> 8) << '.' \
				<< (port & 0x000000ff) << ':'
				<< htons((*it)->addr.sin_port) << std::endl);
		}

		if (interest.size() == 0)
			return 0;
		else if (interest.size() == 1)
			return (*interest.begin())->config;

		DEBUG2("Matching exact server");
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
					if (*req_server_name == name->as<std::string const&>())
					{
						DEBUG2("FOUND EXACT MATCH");
						return it->config;
					}
				}
			}
		}

		// t_sock_info const* last_match = 0;
		// int match_count = 0;
		// int match_length = 0;

		// DEBUG2("Matching leading wildcard");
		// for (std::list<t_sock_info>::const_iterator it = so.list.begin();
		// 	it != so.list.end(); it++)
		// {
		// 	JSON::Node * server_name = it->config->search(1, "server_name");
		// 	if (server_name)
		// 	{
		// 		for (JSON::Node::iterator name = server_name->begin();
		// 			name != server_name->end(); name++)
		// 		{
		// 			int curr_match_len = strlwcmp(
		// 				name->as<std::string const&>(), *req_server_name);
		// 			if (curr_match_len > match_length)
		// 			{
		// 				last_match = &*it;
		// 				match_count++;
		// 				match_length = curr_match_len;
		// 			}
		// 		}
		// 	}
		// }
		// if (match_count == 1)
		// 	return last_match->config;

		// DEBUG2("Matching trailing wildcard");
		// for (std::list<t_sock_info>::const_iterator it = so.list.begin();
		// 	it != so.list.end(); it++)
		// {
		// 	JSON::Node * server_name = it->config->search(1, "server_name");
		// 	if (server_name)
		// 	{
		// 		for (JSON::Node::iterator name = server_name->begin();
		// 			name != server_name->end(); name++)
		// 		{
		// 			int curr_match_len = strtwcmp(
		// 				name->as<std::string const&>(), *req_server_name);
		// 			if (curr_match_len > match_length)
		// 			{
		// 				last_match = &*it;
		// 				match_count++;
		// 				match_length = curr_match_len;
		// 			}
		// 		}
		// 	}
		// }
		// if (match_count == 1)
		// 	return last_match->config;
		return (*interest.begin())->config;
	}

	JSON::Node *matchLocation(JSON::Node *serv, std::string const &path)
	{
		JSON::Object *tmp;
		size_t last_len = 0;
		JSON::Node *last_match = 0;
		size_t i = 0;

		tmp = dynamic_cast<JSON::Object *>(serv->search(1, "location"));
		if (!tmp)
			return 0;
		for (JSON::Node::iterator loc = tmp->begin();
			loc != tmp->end(); loc.skip())
		{
			if (path.size() == 1)
				i = path.find_first_of(*path.c_str());
			else
				i = path.find(loc->getProperty());
			if (i != std::string::npos && i == 0)
			{
				i = loc->getProperty().size();
				if (path.size() == i)
					return &*loc;
				if (i > last_len)
				{
					last_len = i;
					last_match = &*loc;
				}
			}
		}
		return last_match;
	}
}
