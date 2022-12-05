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
		S_DEFAULT_PORT = 8000,
		S_MAX_CLIENT = 32,
	};

	/* core/Sockets */

	typedef struct s_sock_info	t_sock_info;

	/* error/err.cpp */

	int	err( int ret );
	int	err( int ret, char const*str );
	int	err( int ret, char const*err, char const*str );

}

namespace JSON
{
	/* config/json */
	class Json;
	class Node;
}
