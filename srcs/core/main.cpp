#include "webserv.hpp"
#include "Server.hpp"
#include <cstdio>
#include <csignal>

void handle_ctrl_c( int signum )
{
	(void) signum;
	std::cerr << std::endl;
	DEBUG2("received signal: SIGINT");
	HTTP::Server::state = 0;
	DEBUG2("stopping ...");
}

// static void add_handler(std::string const& str, void *(*HTTP::ARequestHandler::execute f)(void) )
// {

// }

int		main( int argc, char **argv )
{
	HTTP::Server	server;

	signal(SIGINT, handle_ctrl_c);
	signal(SIGPIPE, SIG_IGN);
	if (argc != 2)
		return HTTP::err(EXIT_FAILURE, "Usage: ./webserv 'config_file'");
	DEBUG2("starting ...");
	if (server.init(argv[1]) == -1)
		return (EXIT_FAILURE);
	server.loop();
	return (EXIT_SUCCESS);
}
