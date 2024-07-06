/*
 * Spam.hpp
 *
 *  Created on: Jan 21, 2017
 *      Author: nullifiedcat
 */

#pragma once
#include <string>
#include <vector>

class CatCommand;

namespace hacks::spam
{
extern const std::vector<std::string> builtin_default;
extern const std::vector<std::string> builtin_lennyfaces;
//extern const std::vector<std::string> builtin_blanks;
extern const std::vector<std::string> savetf2;
extern const std::vector<std::string> builtin_nonecore;
extern const std::vector<std::string> builtin_lmaobox;
//extern const std::vector<std::string> builtin_lithium;
extern const std::vector<std::string> randomnn;
bool isActive();
void init();
} // namespace hacks::spam
