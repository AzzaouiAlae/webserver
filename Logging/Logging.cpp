#include "Logging.hpp"

Logging *Logging::current;

void Logging::PrintLogToScreen(const string &log)
{
	cout << log << endl;
	this->log << log << endl;
}

Logging::Logging(string filename)
{
	current = this;
	log.open(filename.c_str());
}