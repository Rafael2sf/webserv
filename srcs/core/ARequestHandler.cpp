#include "ARequestHandler.hpp"

namespace HTTP
{

	ARequestHandler::ARequestHandler(void) {

		//Creation of default error pages map
		errorText[400] = "Bad Request";
		errorText[403] = "Forbidden";
		errorText[404] = "Not Found";
		errorText[405] = "Not Allowed";
		errorText[406] = "Not Acceptable";
		errorText[408] = "Request Timeout";
		errorText[411] = "Length Required";
		errorText[413] = "Content Too Large";
		errorText[414] = "URI Too Long";
		errorText[415] = "Unsuported Media Type";
		errorText[501] = "Not Implemented";
	};

	ARequestHandler::ARequestHandler(ARequestHandler const& cpy) { (void)cpy;} ;

	ARequestHandler::~ARequestHandler(void) {};

	ARequestHandler&	ARequestHandler::operator=(ARequestHandler const& rhs) {
		(void)rhs;
		return *this;
	};

	std::string			ARequestHandler::getDate(time_t const& tm_info) {
		
		tm*		current_time;
		char	buffer[DATE_BUF_SIZE];
		std::string	ret;

		memset(buffer, 0, DATE_BUF_SIZE);
		current_time = localtime(&tm_info);
		strftime(buffer, DATE_BUF_SIZE, "%a, %d %b %Y %X %Z",current_time);
		ret = buffer;
		return ret;
	};

	void				ARequestHandler::cgiDealer(Message const& req, int client_fd) {

		int	exit_stat;
		int p[2];
		pipe(p);
		int	pid = fork();
		if (pid == -1)
			DEBUG2("fork failed");
		if (pid == 0) {
			
			close(p[1]);
			CGI	test(req);
			if (dup2(p[0], STDIN_FILENO) == -1)
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);
			}
			close (p[0]);
			if (dup2(client_fd, STDOUT_FILENO) == -1)
			{
				write(2, "error: fatal\n", 13);
				exit(EXIT_FAILURE);
			}
			execve("/usr/bin/python3", test.getArgs(), test.getEnv());
			exit (1);
		}
		else {
			close(p[0]);
			std::string playing = req.getBody();
			size_t wsize = 0;
			size_t b;
			for (b = 0; b + 65000 < playing.size(); b += 65000)
			{
				write(p[1], playing.c_str() + b, 65000);
			}
			wsize = playing.size() - b;
			if (wsize > 0)
				write(p[1], playing.c_str() + b, wsize);
			close(p[1]);
			waitpid(pid, &exit_stat, 0);
		}
	};

	void				ARequestHandler::errorPage(Message const& req, int fd, int code) {
		
		std::string	str;
		Message 	res;
		(void)req;


		res.createMethodVec("HTTP/1.1 " + ftItos(code) + errorText[code]);
		res.add("content-type", "text/html");
		res.add("date", getDate(time(0)));

		res.add("content-length", "12");
		res.setBody("<h1>" + ftItos(code) + "</h1>");
		str = res.responseString();
		send(fd, str.c_str(), str.size(), 0);
		return ;
	}
}