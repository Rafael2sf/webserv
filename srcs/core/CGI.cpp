#include "CGI.hpp"

namespace HTTP
{	
	CGI::~CGI( void )
	{
		int i = 0;
		while(env[i] != NULL)
		{
			delete[] env[i++];
		}
		delete[] env;
		
		i = 0;
		while(args[i] != NULL)
		{
			delete[] args[i++];
		}
		delete[] args;
	}

	CGI::CGI( void )
	{}

	CGI::CGI( CGI const& other )
	{
		*this = other;
	}
	
	CGI::CGI(Message const& req)
	{
		env = new char*[9];
		std::vector<std::string>	vec;
		vec.push_back("PATH_INFO=/usr/bin/python3");
		vec.push_back("CONTENT_TYPE=" + req.get_head_val("content-type"));
		vec.push_back("CONTENT_LENGTH=" + req.get_head_val("content-length"));
		vec.push_back("HTTP_USER_AGENT=" + req.get_head_val("user-agent"));
		vec.push_back("SCRIPT_FILENAME=/nfs/homes/rafernan/Desktop/webserv" + req.get_method()[1]);
		vec.push_back("REQUEST_METHOD=" + req.get_method()[0]);
		vec.push_back("SERVER_SOFTWARE=Webserv/0.2");
		
		std::string substr;
		if (req.get_method()[1].find('?') != std::string::npos)
			substr = req.get_method()[1].substr(req.get_method()[1].find('?') + 1);
		else
			substr = "";
		vec.push_back("QUERY_STRING=" + substr);
		for (int i = 0; i < 8; i++) {
			env[i] = new char[vec[i].size() + 1];
			memcpy(env[i], vec[i].c_str(), vec[i].size());
			env[i][vec[i].size()] = '\0';
		}
		env[8] = NULL;

		
		std::string temp = vec[4].substr(vec[4].find_first_of('/'));
		temp.erase(temp.find(".py") + 3);
		args = new char*[3];
		args[0] = new char[strlen("/usr/bin/python3") + 1];
		memcpy(args[0], "/usr/bin/python3", strlen("/usr/bin/python3"));
		args[0][strlen("/usr/bin/python3")] = '\0';
		args[1] = new char[temp.size() + 1];
		memcpy(args[1], temp.c_str(), temp.size());
		args[1][temp.size()] = '\0';
		args[2] = NULL;
	}

	CGI &
	CGI::operator=( CGI const& rhs )
	{
		DEBUG2("called non-implemented function: CGI::operator=( CGI const& rhs )");
		(void) rhs;
		return *this;
	}

	char**	CGI::getEnv(void) {

		return env;
	};

	char**	CGI::getArgs(void) {

		return args;
	};
}