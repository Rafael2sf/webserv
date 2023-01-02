#include "Post.hpp"

namespace HTTP {

	Post::Post(void)
	: AMethod()
	{}

	Post::~Post(void)
	{}

	Post::Post(Post const& cpy)
	: AMethod()
	{
		(void)cpy;
	};

	Post&	Post::operator=(Post const& rhs)
	{
		(void)rhs;
		return *this;
	};

	void	Post::response(Client & client)
	{
		if (_confCheck(client) == -1)
			return;
		cgi(client);
	};
}
