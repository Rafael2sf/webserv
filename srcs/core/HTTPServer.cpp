#include <cstdlib>
#include "webserv.hpp"
#include "HTTPServer.hpp"
#include "HTTPReq.hpp"

namespace ft
{
	HTTPServer::~HTTPServer( void )
	{}

	HTTPServer::HTTPServer( void )
	{}

	HTTPServer::HTTPServer( HTTPServer const& other )
	{
		*this = other;
	}

	HTTPServer &
	HTTPServer::operator=( HTTPServer const& rhs )
	{
		DEBUG2("called non-implemented function: HTTPServer::operator=( HTTPServer const& rhs )");
		(void) rhs;
		return *this;
	}

	int
	HTTPServer::init( void )
	{
		if (!socks.insert(8090))
			return -1;

		if (!socks.insert(4000))
			return -1;

		for (std::list<ft::t_sock_info>::iterator it = socks.list.begin();
			it != socks.list.end(); it++)
		{DEBUG2("listening on port " << it->port);}
		epoll.init(socks);
		return 0;
	}

	int
	HTTPServer::init( char const* filepath )
	{
		DEBUG2("called non-implemented function: HTTPServer::init( char* filepath )");
		(void) filepath;
		return (-1);
	}
	void
	HTTPServer::loop( void )
	{
		Mediator	med;
		
		if (socks.list.empty())
			exit(err(1, "logic error", "no sockets available"));
		while (1)
		{
			check_times();
			int ev_count = epoll.wait();
			DEBUG2("[" << ev_count << "] ready events");
			if (ev_count == 0)
				continue;
			int ev_socket;
			for (int i = 0; i < ev_count; i++)
			{
				ev_socket = epoll.events[i].data.fd;
				if (socks.find(ev_socket))
				{
					if((ev_socket = accept(ev_socket, NULL, 0)) == -1)
						err(-1, "accept()");
					if (epoll.insert(ev_socket) == -1)
						err(-1, "insert()");
					time_map.insert(std::make_pair(ev_socket, time(NULL)));
				}
				else {
					try
					{
						time_map.at(ev_socket) = time(NULL);
						ft_handle(i, med);
					}
					catch(const std::exception& e)
					{
						DEBUG2(e.what() << ev_socket);
					}

				}
			}
		}
	}

	void	HTTPServer::ft_handle(int i, Mediator & med) {

		static char buffer[30000] = {0};

		int valread = recv(epoll.events[i].data.fd, buffer, 30000, 0);
		if (valread == -1)
			DEBUG2("client disconnect");
		HTTPReq	request(buffer, valread);
		//request.print_map();
		med.method_choice(request, epoll.events[i].data.fd);
		//std::cout << hello << std::endl;
		//DEBUG2(buffer);
		DEBUG2("message sent");
		if (request.get_head_val("Connection") == "close"
				|| valread == 0)
		{
			DEBUG2(epoll.events[i].data.fd << " was erased!!!!!");
			if (epoll.erase(epoll.events[i].data.fd) == -1)
				DEBUG2("ERROR IN ERASE!!!!!!");
			time_map.erase(epoll.events[i].data.fd);
		}
	}

	void	HTTPServer::check_times(void) {
		
		double seconds = time(NULL);
		
		for (std::map<int, double>::iterator it = time_map.begin();
			it != time_map.end(); it++)
		{
			if (seconds - it->second >= 10) {
				DEBUG2(it->first << " was erased by timeout!!!!!");	
				if (epoll.erase(it->first) == -1)
					DEBUG2("ERROR IN ERASE!!!!!!");
				time_map.erase(it->first);
				it = time_map.begin();
				if (it == time_map.end())
					break;
			}
		}
	}
}
