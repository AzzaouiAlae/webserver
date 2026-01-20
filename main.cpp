#include "AbstractSyntaxTree/AST.hpp"

void print(std::string value, int lvl)
{
	for(int i = 0; i < lvl; i++)
		std::cout << "\t";
	std::cout << value << "\n";
}

void test()
{
	AST<std::string> ASTroot("ASTroot");


	AST<std::string> srv1("Srv1");
	AST<std::string> srv2("Srv2");
	srv1.SetNext(srv2);
	AST<std::string> srv3("Srv3");
	srv3.SetNext(srv2);

	

	AST<std::string> listen("listen");
	listen.AddArgument("80");

	AST<std::string> root("root");
	root.AddArgument("/app");

	AST<std::string> index("index");
	index.AddArgument("index.php");
	index.AddArgument("index.html");


	AST<std::string> server_name("server_name");
	server_name.AddArgument("nginx");

	AST<std::string> location("location");
	location.AddArgument("/");

	AST<std::string> retur("return");
	retur.AddArgument("200");
	retur.AddArgument("index.html");
	location.AddChild(retur);

	srv1.AddChild(listen);
	srv1.AddChild(root);
	srv1.AddChild(index);
	srv1.AddChild(server_name);
	srv1.AddChild(location);


	ASTroot.AddChild(srv1);
	ASTroot.AddChild(srv2);
	ASTroot.AddChild(srv3);

	ASTroot.PreorderTraversal(print);
}

int main()
{
	// test();
	// std::vector<st>
}

/*

	server 
	{
		listen 
		0.0.....0.0
		:
		80
		-
		;
        root /app;
        index index.php index.html;
        server_name nginx;

        location / {
			return 200 index.html;
        }
	}
*/


/*
map<string, f()>

listent
root

*/