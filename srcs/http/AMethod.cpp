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
		int		pid;
		std::string const& body = client.req.body;

		if (client.cgiSentBytes == 0)
		{
			pipe(client.clientPipe);
			pid = fork();

			if (pid == 0) {
				
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
				exit (1);
			}
			close(client.clientPipe[0]);
			bytes = write(client.clientPipe[1], body.c_str(), body.size());
			client.cgiSentBytes += bytes;
			client.req.body.clear();
			client.state = CGI_PIPING;
			if (client.cgiSentBytes == client.req.content_length)
			{
				if (client.cgiSentBytes == 0)	//Scripts can be sent without a body, putting this var as 1 is just for a check in server::_update()
					client.cgiSentBytes = 1;
				close(client.clientPipe[1]);
				client.state = SENDING;
			}
		}
		else
		{
			if (client.cgiSentBytes < client.req.content_length)
			{
				bytes = write(client.clientPipe[1], body.c_str(), body.size());
				client.cgiSentBytes += bytes;
				client.req.body.clear();
				client.state = CGI_PIPING;
			}
			else
			{
				close(client.clientPipe[1]);
				client.state = SENDING;
			}
		}
	};
}
