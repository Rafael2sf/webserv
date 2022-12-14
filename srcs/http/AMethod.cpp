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

	std::string AMethod::getDate(time_t const& tm_info)
	{
		tm*		current_time;
		char	buffer[DATE_BUF_SIZE];
		std::string	ret;

		memset(buffer, 0, DATE_BUF_SIZE);
		current_time = localtime(&tm_info);
		strftime(buffer, DATE_BUF_SIZE, "%a, %d %b %Y %X %Z",current_time);
		ret = buffer;
		return ret;
	};

	void AMethod::cgi(Client & client)
	{
		size_t	bytes = 0;
		int		pid;
		int		p[2];
		std::string const& body = client.req.body;

		DEBUG2("POST");

		pipe(p);
		pid = fork();

		if (pid == 0) {
			
			close(p[1]);
			CGI	test(client.req);
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
		DEBUG2("HI!");
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
				DEBUG2("BYTES SENT TO CGI " << bytes);
				break ;
			}
		}
		close(p[1]);
		wait(0);
	};
}
