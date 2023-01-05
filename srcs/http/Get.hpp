#pragma once

#include "AMethod.hpp"
#include "Json.hpp"

namespace HTTP
{
	class Get : public AMethod
	{
		public:
			virtual ~Get();
			Get( void );
			Get&	operator=(Get const& rhs);

			/**
			 * @brief Handler of DELETE requests.
			 * @param Client http client class
			*/
			void response(Client & client);
	};
}
