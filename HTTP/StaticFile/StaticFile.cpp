#include "StaticFile.hpp"

map<string, StaticFile> StaticFile::files;

StaticFile::StaticFile()
{
	size = 0;
	data = NULL;
}

StaticFile::StaticFile(const StaticFile &obj)
{
	data = obj.data;
	size = obj.size;
}

StaticFile::StaticFile(const string& name, const char* data, int size): data(data), size(size)
{
	string s = name;
	files[s] = *this;
}

StaticFile *StaticFile::GetFileByName(const string& name) 
{
	map<string, StaticFile>::iterator it = files.find(name);
	if (it == files.end())
		return NULL;
	return &(it->second);
}

const char* StaticFile::GetData()
{
	return data;
}

int StaticFile::GetSize()
{
	return size;
}
