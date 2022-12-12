#pragma once

#include "AMethod.hpp"

namespace HTTP {

	class Post : public AMethod
	{
		public:
		virtual ~Post();
		Post(void);
		Post(Post const& cpy);
		Post & operator=(Post const& rhs);

		/**
		 * @brief Handler of POST requests.
		 * @param client http client class
		*/
		void operator()(Client & client);
		
		private:
	};
}
