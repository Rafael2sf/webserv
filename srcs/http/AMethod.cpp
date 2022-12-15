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
		int		p[2];
		std::string const& body = client.req.body;

		pipe(p);
		pid = fork();

		if (pid == 0) {
			
			close(p[1]);
			CGI	test(client.req);
			while (client.config->getParent() != NULL)
				client.config = client.config->getParent();
			delete client.config;
			if (dup2(p[0], STDIN_FILENO) == -1)
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);
			}
			close (p[0]);
			if (dup2(client.fd, STDOUT_FILENO) == -1)
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);
			}
			execve("/usr/bin/python3", test.getArgs(), test.getEnv());
			exit (1);
		}
		close(p[0]);
		while (1)
		{
			if (body.size() - bytes > S_PIPE_LIMIT)
			{
				write(p[1], body.c_str() + bytes, S_PIPE_LIMIT);
				bytes += S_PIPE_LIMIT;
			}
			else
			{
				write(p[1], body.c_str() + bytes, body.size() - bytes);
				bytes += body.size() - bytes;
				//DEBUG2("BYTES SENT TO CGI " << bytes);
				break ;
			}
		}
		close(p[1]);
		wait(0);
	};
}
