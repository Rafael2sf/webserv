#include "webserv.hpp"
#include "Server.hpp"
#include <cstdio>
#include <csignal>
#include "Get.hpp"
#include "Delete.hpp"
#include "Post.hpp"

void handle_ctrl_c( int signum )
{
	(void) signum;
	HTTP::Server::state = 0;
}

int		main( int argc, char **argv )
{
	HTTP::Server	server;

	signal(SIGINT, handle_ctrl_c);
	signal(SIGPIPE, SIG_IGN);

	if (argc != 2)
		return HTTP::err(EXIT_FAILURE, "Usage: ./webserv 'config_file'");
	if (server.init(argv[1]) == -1)
		return (EXIT_FAILURE);

	server.add_handler("GET", new HTTP::Get());
	server.add_handler("POST", new HTTP::Post());
	server.add_handler("DELETE", new HTTP::Delete());

	server.loop();
	return (EXIT_SUCCESS);
}
