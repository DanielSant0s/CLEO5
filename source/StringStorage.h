#pragma once
#include <deque>
#include <string>

class StringStorage
{
public:
	StringStorage(size_t capacity);

	char* Store(const char* str, size_t len = -1);

private:
	size_t capacity;
	std::deque<std::string> data;
};

