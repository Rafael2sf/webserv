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

void child_handler( int signum )
{
	pid_t pid;
    int stat;
	enum
	{
		C_OK = 0,
		C_SERVER_ERROR = 1,
		C_FILE_NOT_FOUND = 2,
		C_FORBIDDEN = 3,
		C_BAD_REQUEST = 4,
	};

	(void)signum;
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    {
		int	exitVal = 0;

		switch (WEXITSTATUS(stat))
		{
		case C_OK:
			exitVal = 0;
			break;
		case C_FORBIDDEN:
			exitVal = 403;
			break;
		case C_FILE_NOT_FOUND:
			exitVal = 404;
			break;
		case C_BAD_REQUEST:
			exitVal = 400;
			break;
		default:
			exitVal = 500;
		}
		HTTP::Server::childProcInfo[pid] = exitVal;
    }
}

int		main( int argc, char **argv )
{
	HTTP::Server	server;

	if (argc != 2)
		return HTTP::err(EXIT_FAILURE, "Usage: ./webserv 'config_file'");
	signal(SIGINT, handle_ctrl_c);
	signal(SIGPIPE, SIG_IGN);
  	signal(SIGCHLD, child_handler);
	if (server.init(argv[1]) == -1)
		return (EXIT_FAILURE);
	server.loop();
	server.clear();
	return (EXIT_SUCCESS);
}
