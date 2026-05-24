#pragma once

#include <vector>
#include <string>

namespace obj_store {

std::string write_object(std::vector<char> & data);
std::vector<char> read_object (const std::string& hash);

}
