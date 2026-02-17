#pragma once
#include "../AMethod/AMethod.hpp"

class GET : public AMethod
{
	// ──── Auto-index state (GET-specific) ────
	stringstream filesList;
	string filesListStr;
	int sendListFiles;
	int targetToSend;
	int sent;

	// ──── File serving ────
	void OpenFile(const string &path);
	void PrepareFileResponse();
	void ServeFile();

	// ──── Static index (when root is empty and path is "/") ────
	void GetStaticIndex();

	// ──── Directory listing: building ────
	string FormatDirectoryEntry(const string &name, const struct stat &st, const string &requestPath);
	void ListFiles(const string &path);
	int  CalculateAutoIndexSize();
	void CreateListFilesResponse();

	// ──── Directory listing: sending (state machine) ────
	void SendChunk(const char *data, int dataSize);
	void SendListFilesStr(const string &str);
	void SendAutoIndex(StaticFile *f);
	void AdvanceListFilesState();
	void SendListFilesResponse();

	// ──── GET dispatch ────
	void GetMethod();

public:
	GET();
	~GET();

	// ──── Implements the pure virtual from AMethod ────
	bool HandleResponse();
};