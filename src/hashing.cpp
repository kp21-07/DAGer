#include "include/hashing.hpp"
#include "include/external/sha1.hpp"

std::string hashing::sha1(std::vector<char> & data)
{
	SHA1 checksum;

	std::string temp(
			data.begin(),
			data.end()
	);
	
	checksum.update(temp);
	
	return checksum.final();
}
