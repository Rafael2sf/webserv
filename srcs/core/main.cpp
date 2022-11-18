#include "webserv.hpp"
#include "HTTPServer.hpp"
#include <cstdio>
#include <csignal>

void handle_ctrl_c( int signum )
{
	(void) signum;
	std::cerr << std::endl;
	DEBUG2("received signal: SIGINT");
	ft::HTTPServer::state = 0;
	DEBUG2("stopping ...");
}

int		main( int argc, char **argv )
{
	ft::HTTPServer	server;

    signal(SIGINT, handle_ctrl_c);
	if (argc != 2)
		return ft::err(EXIT_FAILURE, "Usage: ./webserv 'config_file'");
	DEBUG2("starting ...");
	if (server.init(argv[1]) == -1)
		return (EXIT_FAILURE);
	server.loop();
	return (EXIT_SUCCESS);
}
