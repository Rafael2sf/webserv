#include "webserv.hpp"
#include "HTTPServer.hpp"
#include <cstdio>

int		main( void )
{
	ft::HTTPServer	server;

	DEBUG2("starting ...");
	if (server.init() == -1)
		return ft::err(1);
	server.loop();
	return (0);
}
