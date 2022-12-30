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
		int	bytes = 0;
		std::string const& body = client.req.body;

		if (client.cgiSentBytes == 0)
		{
			if (pipe(client.clientPipe) == -1)
				return client.error(500, true);
			if (fcntl(client.clientPipe[1], F_SETFL, O_NONBLOCK )  == -1) // only the server-side pipe will be NONBLOCK, CGI uses STDIN as the other side of the pipe!
				return client.error(500, true);
			client.childPid = fork();
			if (client.childPid == -1)
				return client.error(500, true);
			else if (client.childPid == 0)
			{	
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
				client.clientPipe[0] = 0;
				bytes = write(client.clientPipe[1], body.c_str(), body.size());
				if (bytes != -1)
				{
					client.cgiSentBytes += bytes;
					client.req.body.erase(0, bytes);
				}
				else
					client.cgiSentBytes = -1;
				client.state = CGI_PIPING;
				if ((size_t)client.cgiSentBytes == client.req.content_length)
				{
					close(client.clientPipe[1]);
					client.clientPipe[1] = 0;
					client.state = CGI_FINISHED;
				}
			}
		}
		else
		{
			bytes = write(client.clientPipe[1], body.c_str(), body.size());
			if (bytes != -1)
			{
				if (client.cgiSentBytes == -1)
					client.cgiSentBytes = 0;
				client.cgiSentBytes += bytes;
				client.req.body.erase(0, bytes);
			}
			if ((size_t)client.cgiSentBytes < client.req.content_length)
				client.state = CGI_PIPING;
			else
			{
				close(client.clientPipe[1]);
				client.clientPipe[1] = 0;
				client.state = CGI_FINISHED;
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

}
