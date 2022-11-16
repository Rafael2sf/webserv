#include "JsonToken.hpp"

namespace ft
{
	int jsonErr( t_jparser_info const& info, char const* err )
	{
		std::cerr \
			<< "webserv: fatal: " \
			<< info.path \
			<< ":" << info.row \
			<< ":" << info.col \
			<< ": " << err << std::endl;
		return -1;
	}

	char * jsonReadFile( char const* filepath )
	{
		FILE * fd;
		char * mem;
		int size;
		
		fd = fopen(filepath, "r");
		if (fd == NULL)
			return NULL;
		if (fseek(fd, 0, SEEK_END) == 1)
			return NULL;
		size = ftell(fd);
		if (size == -1)
			return NULL;
		mem = new char [size + 1];
		rewind(fd);
		fread(mem, sizeof(char), size, fd);
		if (ferror(fd) != 0)
		{
			delete[] mem;
			return NULL;
		}
		mem[size] = 0;
		fclose(fd);
		return mem;
	}
}
