#pragma once

#include <string>

namespace cmd {

void help();
void init();
void hash_obj(const std::string content);
void cat_obj(const std::string hash);

}
