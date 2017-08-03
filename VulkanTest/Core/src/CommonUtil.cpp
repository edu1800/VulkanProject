#include "../include/CommonUtil.h"

std::string CommonUtil::GetFileExtension(const std::string & str)
{
	return str.substr(str.find_last_of('.') + 1);
}

unsigned CommonUtil::GetHashCode(const char* str)
{
	//prime
	unsigned h = 37;
	while (*str) {
		h = (h * 54059) ^ (str[0] * 76963);
		str++;
	}
	return h; // or return h % C;
}
