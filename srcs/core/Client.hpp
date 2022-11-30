#pragma once

#include "webserv.hpp"
#include "Message.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

namespace HTTP
{
	class Client
	{
		public:
			Message		req;
			Message		res;
			int			fd;
			struct \
			sockaddr_in	ai;
			double		timestamp;
			JSON::Json*	conf;
	};
}
