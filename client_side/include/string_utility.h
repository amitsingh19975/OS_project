#ifndef STRING_UTILITY_H
#define STRING_UTILITY_H

#include <iostream>
#include <string>
#include <algorithm>

std::string& ltrim(std::string& s){
	auto it = std::find_if(s.begin(), s.end(),
							[](char c) {
								return !std::isspace<char>(c, std::locale::classic()) || c == '\n';
							});
	s.erase(s.begin(), it);
	return s;
}

std::string& rtrim(std::string& s){
	auto it = std::find_if(s.rbegin(), s.rend(),
						[](char c) {
							return !std::isspace<char>(c, std::locale::classic()) || c == '\n';
						});
	s.erase(it.base(), s.end());
	return s;
}

std::string trim(std::string s){
	return ltrim(rtrim(s));
}


#endif // STRING_UTILITY_H
