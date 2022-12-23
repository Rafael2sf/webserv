#include "webserv.hpp"
#include "Server.hpp"
#include <cstdio>
#include <csignal>
#include "Get.hpp"
#include "Delete.hpp"
#include "Post.hpp"

void handle_ctrl_c( int signum )
{
	HTTP::Server::state = signum;
}

int		main( int argc, char **argv )
{
	HTTP::Server	server;

	if (argc != 2)
		return HTTP::err(EXIT_FAILURE, "Usage: ./webserv 'config_file'");
	signal(SIGINT, handle_ctrl_c);
	signal(SIGPIPE, SIG_IGN);
	if (server.init(argv[1]) == -1)
		return (EXIT_FAILURE);
	server.loop();
	return (EXIT_SUCCESS);
}
