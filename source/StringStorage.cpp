#include "stdafx.h"
#include "StringStorage.h"

StringStorage::StringStorage(size_t capacity) :
	capacity(capacity)
{
}

char* StringStorage::Store(const char* str, size_t len)
{
	if(len != -1)
		data.emplace_back(str, str + len);
	else
		data.emplace_back(str);

	while(data.size() > capacity)
	{
		data.pop_front();
	}

	return data.back().data();
}
