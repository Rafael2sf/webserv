#pragma once

#include <iostream>

/*
	To enable debug code provide
	DEBUG_MODE on compilation
*/
#ifdef DEBUG_MODE
# define DEBUG(X) X
# define DEBUG2(X) std::cerr << "[DEBUG] " <<  X << std::endl
#else
# define DEBUG(X)
# define DEBUG2(X)
#endif

namespace HTTP
{
	/*
		variables and defaults
		to server configuration
	*/
	enum
	{
		S_DEFAULT_HOST = 0,
		S_DEFAULT_PORT = 8000,
		S_MAX_CLIENT = 42,
		S_EPOLL_TIMEOUT = 5,
		S_CONN_TIMEOUT = 30,
		S_PIPE_LIMIT = 64000,
		S_BUFFER_SIZE = 8192,
	};

	/* core/Sockets */

	typedef struct s_sock_info	t_sock_info;

	/* utils */

	int	err( int ret );
	int	err( int ret, char const*str );
	int	err( int ret, char const*err, char const*str );
	std::string getDate(time_t const& tm_info);
}

namespace JSON
{
	/* config/json */
	class Json;
	class Node;
}
