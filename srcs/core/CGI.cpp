#include "CGI.hpp"

namespace HTTP
{	
	CGI::~CGI( void )
	{
		int i = 0;
		while(env[i] != NULL)
			delete[] env[i++];
		delete[] env;
		
		i = 0;
		while(args[i] != NULL)
			delete[] args[i++];
		delete[] args;
	}

	CGI::CGI( void )
	{}

	CGI::CGI( CGI const& other )
	{
		*this = other;
	}

	CGI::CGI(Client const& client)
	{
		env = new char*[9];
		std::vector<std::string>	vec;
		JSON::Node *	 			var = client.location->search(1, "cgi");
		std::string 				path = var->as<std::string const&>();

		vec.reserve(13);
		vec.push_back("PATH_INFO=" + path);
		
		if (*--path.end() == '/')
			path.erase(--path.end());
		std::string					filePath = path + client.req.getMethod()[1];

		if (client.req.getField("content-type"))
			vec.push_back("CONTENT_TYPE=" + *client.req.getField("content-type"));
		else
			vec.push_back("CONTENT_TYPE=");
		if (client.req.getField("content-length"))
			vec.push_back("CONTENT_LENGTH=" + *client.req.getField("content-length"));
		else
			vec.push_back("CONTENT_LENGTH=");
		
		var = client.location->search(1, "upload_store");
		path = var->as<std::string const&>();
		vec.push_back("DOCUMENT_ROOT=" + path);
		vec.push_back("SCRIPT_FILENAME=" + client.req.getMethod()[1]);
		vec.push_back("REQUEST_METHOD=" + client.req.getMethod()[0]);
		vec.push_back("SERVER_SOFTWARE=Webserv/1.0");
		vec.push_back("SERVER_NAME=" + *client.req.getField("host"));
		vec.push_back("SERVER_PROTOCOL=" + client.req.getMethod()[2]);
		vec.push_back("QUERY_STRING=" + client.req.getMethod()[3]);
		
		if (client.req.getField("user-agent"))
			vec.push_back("HTTP_USER_AGENT=" + *client.req.getField("user-agent"));
		else
			vec.push_back("HTTP_USER_AGENT=");
		if (client.req.getField("accept"))
			vec.push_back("HTTP_ACCEPT=" + *client.req.getField("accept"));
		else
			vec.push_back("HTTP_ACCEPT=");
		if (client.req.getField("connection"))
			vec.push_back("HTTP_CONNECTION=" + *client.req.getField("connection"));
		else if (!client.req.getField("connection") && client.req.getMethod()[2] == "HTTP/1.0")
			vec.push_back("HTTP_CONNECTION=close");
		else
			vec.push_back("HTTP_CONNECTION=keep-alive");
		
		int	vec_size = vec.size();
		env = new char*[vec_size + 1];
		for (int i = 0; i < vec_size; i++) {
			env[i] = new char[vec[i].size() + 1];
			memcpy(env[i], vec[i].c_str(), vec[i].size());
			env[i][vec[i].size()] = '\0';
		}
		env[vec_size] = NULL;
		args = new char*[3];
		args[0] = new char[strlen("/usr/bin/python3") + 1];
		memcpy(args[0], "/usr/bin/python3", strlen("/usr/bin/python3"));
		args[0][strlen("/usr/bin/python3")] = '\0';
		args[1] = new char[filePath.size() + 1];
		memcpy(args[1], filePath.c_str(), filePath.size());
		args[1][filePath.size()] = '\0';
		args[2] = NULL;
	}

	CGI &
	CGI::operator=( CGI const& rhs )
	{
		DEBUG2("DO NOT CALL THIS COPY OPERATOR");
		(void) rhs;
		return *this;
	}

	char**	CGI::getEnv(void)
	{
		return env;
	};

	char**	CGI::getArgs(void)
	{
		return args;
	};
}
