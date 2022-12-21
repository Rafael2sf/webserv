#include "AMethod.hpp"
//#include "Mediator.hpp"
#include "Server.hpp"

namespace HTTP
{
	AMethod::AMethod(void)
	{};

	AMethod::AMethod(AMethod const& cpy)
	{
		(void)cpy;
	};

	AMethod::~AMethod(void)
	{};

	AMethod& AMethod::operator=(AMethod const& rhs)
	{
		(void)rhs;
		return *this;
	};

	void AMethod::cgi(Client & client)
	{
		size_t	bytes = 0;
		std::string const& body = client.req.body;

		if (client.cgiSentBytes == 0)
		{
			if (pipe(client.clientPipe) == -1)
				return client.error(500, true);
			client.childPid = fork();
			if (client.childPid == -1)
				return client.error(500, true);
			else if (client.childPid == 0) {
				
				close(client.clientPipe[1]);
				CGI	test(client);
				while (client.server->getParent() != NULL)
					client.server = client.server->getParent();
				delete client.server;
				if (dup2(client.clientPipe[0], STDIN_FILENO) == -1)
				{
					write(2, "error: fatal\n", 13);
					exit(EXIT_FAILURE);
				}
				close (client.clientPipe[0]);
				if (dup2(client.fd, STDOUT_FILENO) == -1)
				{
					write(2, "error: fatal\n", 13);
					exit(EXIT_FAILURE);
				}
				execve("/usr/bin/python3", test.getArgs(), test.getEnv());
				DEBUG2("Error in execve!");
				exit (EXIT_FAILURE);
			}
			else 
			{
				close(client.clientPipe[0]);
				bytes = write(client.clientPipe[1], body.c_str(), body.size());
				client.cgiSentBytes += bytes;
				client.req.body.clear();
				client.state = CGI_PIPING;
				if (client.cgiSentBytes == client.req.content_length)
				{
					close(client.clientPipe[1]);
					client.state = CGI_FINISHED;
					_wait(client);
				}
			}
		}
		else
		{
			bytes = write(client.clientPipe[1], body.c_str(), body.size());
			client.cgiSentBytes += bytes;
			client.req.body.clear();
			if (client.cgiSentBytes < client.req.content_length)
				client.state = CGI_PIPING;
			else
			{
				close(client.clientPipe[1]);
				client.state = CGI_FINISHED;
				_wait(client);
			}
		}
	};
	
	int	AMethod::_confCheck(Client & client) {

		JSON::Node* var = client.location->search(1, "upload_store");
		if (var == NULL || var->as<std::string const&>() == "")
		{
			client.error(500, true);
			return -1;
		}
		if (client.location->search(1, "cgi") == NULL)
		{
			client.error(500, true);
			return -1;
		}
		return 0;
	};

	void	AMethod::_wait(Client & client) {
	
		int	retVal;

		waitpid(client.childPid, &retVal, 0);
		switch (WEXITSTATUS(retVal))
		{
		case C_SERVER_ERROR:
			client.error(500, true);
			break;
		case C_FORBIDDEN:
			client.error(403, false);
			break;
		case C_FILE_NOT_FOUND:
			client.error(404, false);
			break;
		case C_BAD_REQUEST:
			client.error(400, false);
			break;
		default:
			return;
		}
		
	}
}
