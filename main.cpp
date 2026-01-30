#include "Headers.hpp"

int main(int argc, char *argv[])
{
	string filename;

	if (argc < 2)
		filename = DEFAULT_CONF;
	else
		filename = argv[1];
	try {
		Tokenizing token(filename);
		token.split_tokens();
		Validation valid(token.get_tokens());
		valid.CheckValidation();
	} catch (const std::exception& e) {
		cerr << e.what() << "\n";
		return 1;
	}
}
