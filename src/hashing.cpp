#include "dagr.h"
#include "external/sha1.hpp"

string sha1(binary_buffer& data)
{
	SHA1 checksum;

	std::string temp(data.data(), data.size());
	
	checksum.update(temp);
	
	std::string res = checksum.final();
	return string(res.c_str());
}
