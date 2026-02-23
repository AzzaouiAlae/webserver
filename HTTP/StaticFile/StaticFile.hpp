#pragma once
#include "../Headers.hpp"

class StaticFile {
	static map<string, StaticFile> files;
	const char* data;
	int size;
public:
	StaticFile(const StaticFile &obj);
	StaticFile();
	StaticFile(const string& name, const char* data, int size);
	static StaticFile *GetFileByName(const string& name);
	const char* GetData();
	int GetSize();
};

extern "C" {
    extern const char _binary_HTTP_DefaultPages_Pages_400_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_400_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_401_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_401_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_402_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_402_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_403_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_403_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_404_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_404_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_405_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_405_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_408_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_408_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_409_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_409_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_413_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_413_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_500_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_500_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_501_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_501_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_502_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_502_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_503_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_503_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_504_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_504_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_autoindex1_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_autoindex1_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_autoindex2_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_autoindex2_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_autoindex3_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_autoindex3_htm_end[];

    extern const char _binary_HTTP_DefaultPages_Pages_index_htm_start[];
    extern const char _binary_HTTP_DefaultPages_Pages_index_htm_end[];
}

#define PAGE_400_S _binary_HTTP_DefaultPages_Pages_400_htm_start
#define PAGE_400_E _binary_HTTP_DefaultPages_Pages_400_htm_end

#define PAGE_401_S _binary_HTTP_DefaultPages_Pages_401_htm_start
#define PAGE_401_E _binary_HTTP_DefaultPages_Pages_401_htm_end

#define PAGE_402_S _binary_HTTP_DefaultPages_Pages_402_htm_start
#define PAGE_402_E _binary_HTTP_DefaultPages_Pages_402_htm_end

#define PAGE_403_S _binary_HTTP_DefaultPages_Pages_403_htm_start
#define PAGE_403_E _binary_HTTP_DefaultPages_Pages_403_htm_end

#define PAGE_404_S _binary_HTTP_DefaultPages_Pages_404_htm_start
#define PAGE_404_E _binary_HTTP_DefaultPages_Pages_404_htm_end

#define PAGE_405_S _binary_HTTP_DefaultPages_Pages_405_htm_start
#define PAGE_405_E _binary_HTTP_DefaultPages_Pages_405_htm_end

#define PAGE_408_S _binary_HTTP_DefaultPages_Pages_408_htm_start
#define PAGE_408_E _binary_HTTP_DefaultPages_Pages_408_htm_end

#define PAGE_409_S _binary_HTTP_DefaultPages_Pages_409_htm_start
#define PAGE_409_E _binary_HTTP_DefaultPages_Pages_409_htm_end

#define PAGE_413_S _binary_HTTP_DefaultPages_Pages_413_htm_start
#define PAGE_413_E _binary_HTTP_DefaultPages_Pages_413_htm_end

#define PAGE_500_S _binary_HTTP_DefaultPages_Pages_500_htm_start
#define PAGE_500_E _binary_HTTP_DefaultPages_Pages_500_htm_end

#define PAGE_501_S _binary_HTTP_DefaultPages_Pages_501_htm_start
#define PAGE_501_E _binary_HTTP_DefaultPages_Pages_501_htm_end

#define PAGE_502_S _binary_HTTP_DefaultPages_Pages_502_htm_start
#define PAGE_502_E _binary_HTTP_DefaultPages_Pages_502_htm_end

#define PAGE_503_S _binary_HTTP_DefaultPages_Pages_503_htm_start
#define PAGE_503_E _binary_HTTP_DefaultPages_Pages_503_htm_end

#define PAGE_504_S _binary_HTTP_DefaultPages_Pages_504_htm_start
#define PAGE_504_E _binary_HTTP_DefaultPages_Pages_504_htm_end

#define PAGE_AUTOINDEX1_S _binary_HTTP_DefaultPages_Pages_autoindex1_htm_start
#define PAGE_AUTOINDEX1_E _binary_HTTP_DefaultPages_Pages_autoindex1_htm_end

#define PAGE_AUTOINDEX2_S _binary_HTTP_DefaultPages_Pages_autoindex2_htm_start
#define PAGE_AUTOINDEX2_E _binary_HTTP_DefaultPages_Pages_autoindex2_htm_end

#define PAGE_AUTOINDEX3_S _binary_HTTP_DefaultPages_Pages_autoindex3_htm_start
#define PAGE_AUTOINDEX3_E _binary_HTTP_DefaultPages_Pages_autoindex3_htm_end

#define PAGE_INDEX_S _binary_HTTP_DefaultPages_Pages_index_htm_start
#define PAGE_INDEX_E _binary_HTTP_DefaultPages_Pages_index_htm_end
