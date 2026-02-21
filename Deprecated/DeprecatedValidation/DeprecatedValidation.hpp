#pragma once
#include "../../Headers.hpp"

// ─────────────────────────────────────────────────────────────────
//  Validation
//
//  Single responsibility: walk the completed AST produced by
//  DeprecatedParsing::BuildAST() and enforce all semantic rules.
//
//  Rules enforced:
//    • Required directives are present  (listen, server_name)
//    • No duplicated directives within the same block
//    • Values are within legal ranges   (port, error codes, …)
//    • Scope is respected               (some directives only in
//      server blocks, others only in location blocks)
//    • Conflict constraints             (cgi_pass ↔ delete_files,
//      cgi_pass ↔ client_body_in_file_only)
//
//  What it does NOT do:
//    • Build or modify the AST
//    • Call DeprecatedParsing::Add*
//    • Walk raw tokens
// ─────────────────────────────────────────────────────────────────

class DeprecatedValidation
{
public:
	// Takes the root of the already-built AST.
	explicit DeprecatedValidation(AST<string> &astRoot);

	// Entry point — throws Error on the first rule violation found.
	void Validate();

private:
	AST<string> &_root;

	// ── per-block tracking structs ────────────────────────────
	// Using a plain struct with named booleans prevents the silent
	// map-insertion bug that happens with map<string,bool> and typos.

	struct ServerSeen
	{
		bool listen;
		bool serverName;
		bool root;
		bool index;
		bool autoindex;
		bool returnDir;
		bool clientMaxBodySize;
		bool allowMethods;
		bool errorPage;

		ServerSeen() : listen(false), serverName(false), root(false),
					   index(false), autoindex(false), returnDir(false),
					   clientMaxBodySize(false), allowMethods(false),
					   errorPage(false) {}
	};

	struct LocationSeen
	{
		bool root;
		bool index;
		bool autoindex;
		bool returnDir;
		bool clientMaxBodySize;
		bool allowMethods;
		bool cgiPass;
		bool bodyInFile;
		bool deleteFiles;

		LocationSeen() : root(false), index(false), autoindex(false),
						 returnDir(false), clientMaxBodySize(false),
						 allowMethods(false), cgiPass(false),
						 bodyInFile(false), deleteFiles(false) {}
	};

	// ── top-level validators ──────────────────────────────────
	void validateServer(AST<string> &serverNode);
	void validateLocation(AST<string> &locationNode,
						  LocationSeen &seen);

	// ── directive validators (server scope) ──────────────────
	void validateListen(AST<string> &node, ServerSeen &seen);
	void validateServerName(AST<string> &node, ServerSeen &seen);
	void validateRoot(AST<string> &node, bool &seen);
	void validateIndex(AST<string> &node, bool &seen);
	void validateAutoindex(AST<string> &node, bool &seen);
	void validateReturn(AST<string> &node, bool &seen);
	void validateClientMaxBody(AST<string> &node, bool &seen);
	void validateAllowMethods(AST<string> &node, bool &seen);
	void validateErrorPage(AST<string> &node, bool &seen);

	// ── directive validators (location-only scope) ────────────
	void validateCgiPass(AST<string> &node, LocationSeen &seen);
	void validateBodyInFile(AST<string> &node, LocationSeen &seen);
	void validateDeleteFiles(AST<string> &node, LocationSeen &seen);

	// ── value helpers ─────────────────────────────────────────
	static long parseNumber(const string &s);
	static bool isByteSizeUnit(char c);
	static bool isValidMethod(const string &m);
	static void checkArgCount(AST<string> &node,
							  size_t min, size_t max,
							  const string &name);
	static void requireArgs(AST<string> &node,
							const string &name);
};