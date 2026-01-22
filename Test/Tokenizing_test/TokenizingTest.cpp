#include "../Unity-master/src/unity.h"
#include "../../Parsing/Parsing.hpp"
#include "../../Tokenizing/Tokenizing.hpp"

void setUp(void)
{}

void tearDown(void)
{}

void TestConfigFile1()
{
	//arrange
	std::vector<std::string> tab;

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back(":");
    tab.push_back("80");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("server_name");
    tab.push_back("/localhost");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("root");
    tab.push_back("/var/www/wordpress\\\\");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index.php");
    tab.push_back("}");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");

    tab.push_back("{");
    tab.push_back("retun");
    tab.push_back("301");
    tab.push_back("index.html");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("/\\/\\.php$");
    tab.push_back("#error");
    tab.push_back("{");

    tab.push_back("include");
    tab.push_back("fastcgi_params");

    tab.push_back("fastcgi_pass");
    tab.push_back("wp");
    tab.push_back(":");
    tab.push_back(":");
    tab.push_back(":");
    tab.push_back(":");
    tab.push_back("9000");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back(";");
    tab.push_back("fastcgi_index");
    tab.push_back("index.php");
    tab.push_back(";");

    tab.push_back("fastcgi_param");
    tab.push_back("SCRIPT_FILENAME");
    tab.push_back("$document_root$fastcgi_script_name");
    tab.push_back(";");

    tab.push_back("}");
    tab.push_back("}");

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back("443");
    tab.push_back("sslll");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("server_name");
    tab.push_back("localhost@gmail.com");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("root");
    tab.push_back(";");
    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index.php");
    tab.push_back(";");

    tab.push_back("ssl_protocols");
    tab.push_back("TLSv1.2");
    tab.push_back("TLSv1.3");
    tab.push_back("protocol");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("ssl_certificate");
    tab.push_back("/etc/ssl//certs/fullchain.pem");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("ssl_certificate_key");
    tab.push_back("/etc/ssl/private/privkey.pem/././@@");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");

    tab.push_back("Root");
    tab.push_back("/var/www/wordpress");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("#error");

    tab.push_back("index.php");
    tab.push_back("file.html");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("\\.php$");
    tab.push_back("{");

    tab.push_back("include");
    tab.push_back("fastcgi_params");
    tab.push_back(";");

    tab.push_back("fastcgi_pass");
    tab.push_back("unix");
    tab.push_back(":");
    tab.push_back("/var/run/php/php7.4-fpm.sock");
    tab.push_back(";");

    tab.push_back("fastcgi_index");
    tab.push_back("index.php");
    tab.push_back("file.php");
    tab.push_back(";");

    tab.push_back("fastcgi_param");
    tab.push_back("SCRIPT_FILENAME");
    tab.push_back("$document_root$fastcgi_script_name");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("}");

	//act
	Tokenizing ConfigFIle1("./conf/invalide1.conf");
	ConfigFIle1.split_tokens();
	std::vector<std::string> mytab = ConfigFIle1.get_tokens();
	
	//assert
    std::vector<std::string>::iterator exp = tab.begin();
    for (std::vector<std::string>::iterator str = mytab.begin(); str != mytab.end(); ++str)
    {
        // std::cout << *exp << "|" << *str << std::endl;
        TEST_ASSERT_EQUAL_STRING((*exp).c_str(), (*str).c_str());
        exp++;
    }
    
	// TEST_ASSERT_EQUAL_STRING(exp.c_str(), s1.c_str());
}

void TestConfigFile2()
{
    // arrange
    std::vector<std::string> tab;

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back("1..1.1.1");
    tab.push_back(":");
    tab.push_back("80");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("server_name");
    tab.push_back("localhost,,,");
    tab.push_back("test.com");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("location");
    tab.push_back("///");
    tab.push_back("#error");
    tab.push_back("{");

    tab.push_back("root");
    tab.push_back("/path/");
    tab.push_back(";");
    
    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("\\.php$");
    tab.push_back("{");

    tab.push_back("INCLUDE");
    tab.push_back("fastcgi_params");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("fastcgi_pass");
    tab.push_back("wp");
    tab.push_back(":");
    tab.push_back("9000");
    tab.push_back(":");
    tab.push_back("44");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("fastcgi_index");
    tab.push_back("$index.php");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("fastcgi_param");
    tab.push_back("SCRIPT_FILENAME");
    tab.push_back("$document_root$fastcgi_script_name$@");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("}");
    tab.push_back("}");

    tab.push_back("server1");
    tab.push_back("{");
    tab.push_back("#error");

    tab.push_back("listen");
    tab.push_back("one");
    tab.push_back("ssl");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("server_Name");
    tab.push_back("_localhost");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("Sslprotocol");
    tab.push_back("TLSv1.2");
    tab.push_back("TLSv1.3");
    tab.push_back("TLSv234.667");
    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("ssl_certificate");
    tab.push_back("/etc/ssl/certs/fullchain.pem");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");
    tab.push_back("root");
    tab.push_back("/var/www/wordpress");
    tab.push_back(";");
    tab.push_back("}");
    tab.push_back("#error");

    tab.push_back("root");
    tab.push_back("/var/www/");
    tab.push_back(";");
    tab.push_back("test");
    tab.push_back("#error");

    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index.php");
    tab.push_back("#error");

    tab.push_back(";");
    tab.push_back("#error");

    tab.push_back("}");

    // act
    Tokenizing ConfigFIle2("./conf/invalide2.conf");
    ConfigFIle2.split_tokens();
    std::vector<std::string> mytab = ConfigFIle2.get_tokens();

    // assert
    std::vector<std::string>::iterator exp = tab.begin();
    for (std::vector<std::string>::iterator str = mytab.begin(); str != mytab.end(); ++str)
    {
        TEST_ASSERT_EQUAL_STRING((*exp).c_str(), (*str).c_str());
        exp++;
    }
}


void TestConfigFile3()
{
    // arrange
    std::vector<std::string> tab;

    tab.push_back("root");
    tab.push_back("/path/target");
    tab.push_back(";");
    
    tab.push_back("inrdex");
    tab.push_back("file.html");
    tab.push_back(";");

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back("009888888888888888698");
    tab.push_back(";");

    tab.push_back("server_name");
    tab.push_back("++-localhost,");
    tab.push_back("$test.com");
    tab.push_back("43.ma");
    tab.push_back("/5464/");
    tab.push_back(";");

    tab.push_back("root");
    tab.push_back("path");
    tab.push_back("||||||||||||||||||||||||||||||||||||||||||||||||||f0");
    tab.push_back(";");

    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index");
    tab.push_back("511.html");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");
    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");
    tab.push_back("}");

    tab.push_back("LOCATION");
    tab.push_back("/");
    tab.push_back("{");
    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");
    tab.push_back("retun");
    tab.push_back("index.html");
    tab.push_back(";"); 
    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("\\.php");
    tab.push_back("{");

    tab.push_back("include");
    tab.push_back("fastcgi_params");
    tab.push_back(";");

    tab.push_back("fastcgi_pass");
    tab.push_back("CGIname3000");
    tab.push_back(":");
    tab.push_back("9000");
    tab.push_back(";");

    tab.push_back("fastcgi_index");
    tab.push_back("index.php");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");

    tab.push_back("fastcgi_param");
    tab.push_back("SCRIPT_FILENAME");
    tab.push_back("$$arg$$");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("root");
    tab.push_back("$arggffr");

    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");
    tab.push_back("}");

    tab.push_back(";");

    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");

    tab.push_back("Server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back("1.1.1.1");
    tab.push_back(":");
    tab.push_back("840");
    tab.push_back("ssl");
    tab.push_back(";");
    tab.push_back("test");
    tab.push_back(";");
    tab.push_back(";");

    tab.push_back("server-name");
    tab.push_back("server_name,");
    tab.push_back("192.168.1.1");
    tab.push_back(":");
    tab.push_back("8080,");
    tab.push_back("location,,,,");
    tab.push_back("ssl");
    tab.push_back(";");

    tab.push_back("root");
    tab.push_back("/path/");
    tab.push_back(";");

    tab.push_back("index");
    tab.push_back("location");
    tab.push_back("index.html");
    tab.push_back("index");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");

    tab.push_back("location");
    tab.push_back("/home/");
    tab.push_back("{");

    tab.push_back("location");
    tab.push_back("/home/path");
    tab.push_back("{");
    tab.push_back("}");
    tab.push_back("}");

    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("\\.php$");
    tab.push_back("{");

    tab.push_back("root");
    tab.push_back(";");
    
    tab.push_back("include");
    tab.push_back("fastcgi_params");
    tab.push_back(";");

    tab.push_back("fastcgi_pass");
    tab.push_back("wp");
    tab.push_back(":");
    tab.push_back("9000");
    tab.push_back(";");

    tab.push_back("fastcgi_index");
    tab.push_back("index.php");
    tab.push_back(";");

    tab.push_back("fastcgi_param");
    tab.push_back("SCRIPT_FILENAME");
    tab.push_back("$document_root$");
    tab.push_back("fastcgi_script_name");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("}");

    tab.push_back("{");
    tab.push_back("{");
    tab.push_back("{");
    tab.push_back("}");

    tab.push_back("error");

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("server_name");
    tab.push_back("localhost,");
    tab.push_back("test.com");
    tab.push_back(";");

    tab.push_back("root");
    tab.push_back("/path/");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/home");
    tab.push_back("{");
    tab.push_back(";");
    tab.push_back("}");
    tab.push_back("}");

    tab.push_back("}");
    tab.push_back("}");
    tab.push_back("}");

    // act
    Tokenizing ConfigFIle3("./conf/invalide3.conf");
    ConfigFIle3.split_tokens();
    std::vector<std::string> mytab = ConfigFIle3.get_tokens();

    // assert
    std::vector<std::string>::iterator exp = tab.begin();
	
    for (std::vector<std::string>::iterator str = mytab.begin(); str != mytab.end(); ++str)
    {
		
        TEST_ASSERT_EQUAL_STRING((*exp).c_str(), (*str).c_str());
        exp++;
		
    }
}

void TestConfigFileValide1()
{
    // arrange
    std::vector<std::string> tab;

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back("80");
    tab.push_back(";");

    tab.push_back("server_name");
    tab.push_back("localhost");
    tab.push_back(";");

    tab.push_back("root");
    tab.push_back("/var/www/wordpress");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");

    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index.php");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");

    tab.push_back("retun");
    tab.push_back("301");
    tab.push_back("index.html");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");
    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("\\.php$");
    tab.push_back("{");

    tab.push_back("include");
    tab.push_back("fastcgi_params");
    tab.push_back(";");

    tab.push_back("fastcgi_pass");
    tab.push_back("wp");
    tab.push_back(":");
    tab.push_back("9000");
    tab.push_back(";");

    tab.push_back("fastcgi_index");
    tab.push_back("index.php");
    tab.push_back(";");

    tab.push_back("fastcgi_param");
    tab.push_back("SCRIPT_FILENAME");
    tab.push_back("$document_root$fastcgi_script_name");
    tab.push_back(";");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("}");

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back("443");
    tab.push_back("ssl");
    tab.push_back(";");

    tab.push_back("server_name");
    tab.push_back("localhost");
    tab.push_back(";");

    tab.push_back("root");
    tab.push_back("/var/www/");
    tab.push_back(";");

    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index.php");
    tab.push_back(";");

    tab.push_back("ssl_protocols");
    tab.push_back("TLSv1.2");
    tab.push_back("TLSv1.3");
    tab.push_back(";");

    tab.push_back("ssl_certificate");
    tab.push_back("/etc/ssl/certs/fullchain.pem");
    tab.push_back(";");

    tab.push_back("ssl_certificate_key");
    tab.push_back("/etc/ssl/private/privkey.pem");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");

    tab.push_back("root");
    tab.push_back("/var/www/wordpress");
    tab.push_back(";");

    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index.php");
    tab.push_back(";");
    
    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("\\.php$");
    tab.push_back("{");

    tab.push_back("include");
    tab.push_back("fastcgi_params");
    tab.push_back(";");

    tab.push_back("fastcgi_pass");
    tab.push_back("unix");
    tab.push_back(":");
    tab.push_back("/var/run/php/php7.4-fpm.sock");
    tab.push_back(";");

    tab.push_back("fastcgi_index");
    tab.push_back("index.php");
    tab.push_back("file.php");
    tab.push_back(";");

    tab.push_back("fastcgi_param");
    tab.push_back("SCRIPT_FILENAME");
    tab.push_back("$document_root$fastcgi_script_name");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("}");

    // act
    Tokenizing ConfigFIleValide1("./conf/valide1.conf");
    ConfigFIleValide1.split_tokens();
    std::vector<std::string> mytab = ConfigFIleValide1.get_tokens();

    // assert
    std::vector<std::string>::iterator exp = tab.begin();
    int i = 0;
	for (std::vector<std::string>::iterator str = mytab.begin(); str != mytab.end(); ++str)
    {
		if ((*exp) != (*str))
		{
			std::cerr << i << " ";
		}
        TEST_ASSERT_EQUAL_STRING((*exp).c_str(), (*str).c_str());
		i++;
        exp++;
    }
}

void TestConfigFileValide2()
{
    // arrange
    std::vector<std::string> tab;

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back("1.1.1.1");
    tab.push_back(":");
    tab.push_back("80");
    tab.push_back(";");

    tab.push_back("server_name");
    tab.push_back("localhost,");
    tab.push_back("test.com");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");

    tab.push_back("root");
    tab.push_back("/path/");
    tab.push_back(";");

    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("\\.php$");
    tab.push_back("{");

    tab.push_back("include");
    tab.push_back("fastcgi_params");
    tab.push_back(";");

    tab.push_back("fastcgi_pass");
    tab.push_back("wp");
    tab.push_back(":");
    tab.push_back("9000");
    tab.push_back(";");

    tab.push_back("fastcgi_index");
    tab.push_back("index.php");
    tab.push_back(";");

    tab.push_back("fastcgi_param");
    tab.push_back("SCRIPT_FILENAME");
    tab.push_back("$document_root$");
    tab.push_back("fastcgi_script_name");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("}");

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back("443");
    tab.push_back("ssl");
    tab.push_back(";");

    tab.push_back("server_name");
    tab.push_back("localhost");
    tab.push_back(";");

    tab.push_back("ssl_protocols");
    tab.push_back("TLSv1.2");
    tab.push_back("TLSv1.3");
    tab.push_back("protocol");
    tab.push_back(";");

    tab.push_back("ssl_certificate");
    tab.push_back("/etc/ssl/certs/fullchain.pem");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");
    tab.push_back("root");
    tab.push_back("/var/www/wordpress");
    tab.push_back(";");
    tab.push_back("}");

    tab.push_back("root");
    tab.push_back("/var/www/");
    tab.push_back(";");

    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index.php");
    tab.push_back(";");

    tab.push_back("}");

    // act
    Tokenizing ConfigFIleValide2("./conf/valide2.conf");
    ConfigFIleValide2.split_tokens();
    std::vector<std::string> mytab = ConfigFIleValide2.get_tokens();

    // assert
    std::vector<std::string>::iterator exp = tab.begin();
    for (std::vector<std::string>::iterator str = mytab.begin(); str != mytab.end(); ++str)
    {
        TEST_ASSERT_EQUAL_STRING((*exp).c_str(), (*str).c_str());
        exp++;
    }
}

void TestConfigFileValide3()
{
    // arrange
    std::vector<std::string> tab;

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back("8080");
    tab.push_back(";");

    tab.push_back("server_name");
    tab.push_back("localhost,");
    tab.push_back("test.com");
    tab.push_back("43.ma");
    tab.push_back("5464");
    tab.push_back(";");

    tab.push_back("root");
    tab.push_back("/path/");
    tab.push_back(";");

    tab.push_back("index");
    tab.push_back("index.html");
    tab.push_back("index");
    tab.push_back("511.html");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");
    tab.push_back("retun");
    tab.push_back("301");
    tab.push_back("index.html");
    tab.push_back(";");
    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("\\.php$");
    tab.push_back("{");

    tab.push_back("include");
    tab.push_back("fastcgi_params");
    tab.push_back(";");

    tab.push_back("fastcgi_pass");
    tab.push_back("CGIname3000");
    tab.push_back(":");
    tab.push_back("9000");
    tab.push_back(";");

    tab.push_back("fastcgi_index");
    tab.push_back("index.php");
    tab.push_back(";");

    tab.push_back("fastcgi_param");
    tab.push_back("SCRIPT_FILENAME");
    tab.push_back("$$arg$$");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("root");
    tab.push_back("$arg");

    tab.push_back("}");

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("listen");
    tab.push_back("1.1.1.1");
    tab.push_back(":");
    tab.push_back("840");
    tab.push_back("ssl");
    tab.push_back(";");

    tab.push_back("server_name");
    tab.push_back("server_name,");
    tab.push_back("listen,");
    tab.push_back("location");
    tab.push_back(";");

    tab.push_back("root");
    tab.push_back("/path/");
    tab.push_back(";");

    tab.push_back("index");
    tab.push_back("location");
    tab.push_back("index.html");
    tab.push_back("index");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/");
    tab.push_back("{");
    tab.push_back("}");

    tab.push_back("location");
    tab.push_back("\\.php$");
    tab.push_back("{");

    tab.push_back("root");
    tab.push_back(";");

    tab.push_back("include");
    tab.push_back("fastcgi_params");
    tab.push_back(";");

    tab.push_back("fastcgi_pass");
    tab.push_back("wp");
    tab.push_back(":");
    tab.push_back("9000");
    tab.push_back(";");

    tab.push_back("fastcgi_index");
    tab.push_back("index.php");
    tab.push_back(";");

    tab.push_back("fastcgi_param");
    tab.push_back("SCRIPT_FILENAME");
    tab.push_back("$document_root$");
    tab.push_back("fastcgi_script_name");
    tab.push_back(";");

    tab.push_back("}");

    tab.push_back("}");

    tab.push_back("server");
    tab.push_back("{");

    tab.push_back("server_name");
    tab.push_back("localhost,");
    tab.push_back("test.com");
    tab.push_back(";");

    tab.push_back("root");
    tab.push_back("/path/");
    tab.push_back(";");

    tab.push_back("location");
    tab.push_back("/home");
    tab.push_back("{");
    tab.push_back("}");
    
    tab.push_back("}");

    // act
    Tokenizing ConfigFIleValide3("./conf/valide3.conf");
    ConfigFIleValide3.split_tokens();
    std::vector<std::string> mytab = ConfigFIleValide3.get_tokens();

    // assert
    std::vector<std::string>::iterator exp = tab.begin();
    for (std::vector<std::string>::iterator str = mytab.begin(); str != mytab.end(); ++str)
    {
        TEST_ASSERT_EQUAL_STRING((*exp).c_str(), (*str).c_str());
        exp++;
    }
}



int main()
{
	RUN_TEST(TestConfigFile1);
	RUN_TEST(TestConfigFile2);
    RUN_TEST(TestConfigFile3);
    RUN_TEST(TestConfigFileValide1);
    RUN_TEST(TestConfigFileValide2);
    RUN_TEST(TestConfigFileValide3);
    

	return (UnityEnd());
}