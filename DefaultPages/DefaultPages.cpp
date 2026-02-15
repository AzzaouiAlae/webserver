#include "DefaultPages.hpp"

bool DefaultPages::ErrorPages4xx;
bool DefaultPages::ErrorPages5xx;
bool DefaultPages::Index;
bool DefaultPages::AutoIndex;

void DefaultPages::InitErrorPages4xx()
{
	if (ErrorPages4xx)
		return;
	ErrorPages4xx = true;
	StaticFile("400", PAGE_400_S, PAGE_400_E - PAGE_400_S);
	StaticFile("401", PAGE_401_S, PAGE_401_E - PAGE_401_S);
	StaticFile("402", PAGE_402_S, PAGE_402_E - PAGE_402_S);
	StaticFile("403", PAGE_403_S, PAGE_403_E - PAGE_403_S);
	StaticFile("404", PAGE_404_S, PAGE_404_E - PAGE_404_S);
	StaticFile("405", PAGE_405_S, PAGE_405_E - PAGE_405_S);
	StaticFile("408", PAGE_408_S, PAGE_408_E - PAGE_408_S);
	StaticFile("413", PAGE_413_S, PAGE_413_E - PAGE_413_S);
}

void DefaultPages::InitErrorPages5xx()
{
	if (ErrorPages5xx)
		return;
	ErrorPages5xx = true;
	StaticFile("500", PAGE_500_S, PAGE_500_E - PAGE_500_S);
	StaticFile("501", PAGE_501_E, PAGE_501_E - PAGE_501_S);
	StaticFile("502", PAGE_502_S, PAGE_502_E - PAGE_502_S);
	StaticFile("503", PAGE_503_S, PAGE_503_E - PAGE_503_S);
	StaticFile("504", PAGE_504_S, PAGE_504_E - PAGE_504_S);
}

void DefaultPages::InitIndex()
{
	if (Index)
		return;
	Index = true;
	StaticFile("index", PAGE_INDEX_S, PAGE_INDEX_E - PAGE_INDEX_S);
}

void DefaultPages::InitAutoIndex()
{
	if (AutoIndex)
		return;
	AutoIndex = true;
	StaticFile("autoIndex1", PAGE_AUTOINDEX1_S, PAGE_AUTOINDEX1_E - PAGE_AUTOINDEX1_S);
	StaticFile("autoIndex2", PAGE_AUTOINDEX2_S, PAGE_AUTOINDEX2_E - PAGE_AUTOINDEX2_S);
	StaticFile("autoIndex3", PAGE_AUTOINDEX3_S, PAGE_AUTOINDEX3_E - PAGE_AUTOINDEX3_S);
}
void DefaultPages::InitDefaultPages()
{
	InitErrorPages4xx();
	InitErrorPages5xx();
	InitIndex();
	InitAutoIndex();
}