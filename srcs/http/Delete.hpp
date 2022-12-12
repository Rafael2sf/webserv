#pragma once

#include "AMethod.hpp"

namespace HTTP {

	class Delete : public AMethod {
		public:
		virtual ~Delete();
		Delete(void);
		Delete(Delete const& cpy);
		Delete&	operator=(Delete const& rhs);

		/**
		 * @brief Handler of DELETE requests.
		 * @param Client http client class
		*/
		virtual void operator()(Client & client);
		
		private:
	};
}
