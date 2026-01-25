#include "../Unity-master/src/unity.h"
#include "../../Validation/Validation.hpp"

void setUp(void)
{}

void tearDown(void)
{}

void Test1()
{
	Tokenizing file("ConfigFilesTESTS/valid1.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_PASS();
	}
	catch (const std::exception& e)
	{
	    TEST_FAIL_MESSAGE(e.what());
	}
}

void Test2()
{
	Tokenizing file("ConfigFilesTESTS/valid2.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_PASS();
	}
	catch (const std::exception& e)
	{
	    TEST_FAIL_MESSAGE(e.what());
	}
}

void Test3()
{
	Tokenizing file("ConfigFilesTESTS/valid3.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_PASS();
	}
	catch (const std::exception& e)
	{
	    TEST_FAIL_MESSAGE(e.what());
	}
}

void Test4()
{
	Tokenizing file("ConfigFilesTESTS/valid4.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_PASS();
	}
	catch (const std::exception& e)
	{
	    TEST_FAIL_MESSAGE(e.what());
	}
}

void Test5()
{
	Tokenizing file("ConfigFilesTESTS/valid5.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_PASS();
	}
	catch (const std::exception& e)
	{
	    TEST_FAIL_MESSAGE(e.what());
	}
}

void Test6()
{
	Tokenizing file("ConfigFilesTESTS/invalid1.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

void Test7()
{
	Tokenizing file("ConfigFilesTESTS/invalid2.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

void Test8()
{
	Tokenizing file("ConfigFilesTESTS/invalid3.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

void Test9()
{
	Tokenizing file("ConfigFilesTESTS/invalid4.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

void Test10()
{
	Tokenizing file("ConfigFilesTESTS/invalid5.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

void Test11()
{
	Tokenizing file("ConfigFilesTESTS/invalid6.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

void Test12()
{
	Tokenizing file("ConfigFilesTESTS/invalid7.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

void Test13()
{
	Tokenizing file("ConfigFilesTESTS/invalid8.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

void Test14()
{
	Tokenizing file("ConfigFilesTESTS/invalid9.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

void Test15()
{
	Tokenizing file("ConfigFilesTESTS/invalid10.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

void Test16()
{
	Tokenizing file("ConfigFilesTESTS/invalid11.conf");
    file.split_tokens();
    Validation valid(file.get_tokens());
	try
	{
	    valid.CheckValidation();
	    TEST_FAIL_MESSAGE("It Should FAIL Because of Invalid syntax");
	}
	catch (const std::exception& e)
	{
	    TEST_PASS();
	}
}

int main()
{
	// VALID
	RUN_TEST(Test1);
	RUN_TEST(Test2);
	RUN_TEST(Test3);
	RUN_TEST(Test4);
	RUN_TEST(Test5);

	// INVALID
	RUN_TEST(Test6);
	RUN_TEST(Test7);
	RUN_TEST(Test8);
	RUN_TEST(Test9);
	RUN_TEST(Test10);
	RUN_TEST(Test11);
	RUN_TEST(Test12);
	RUN_TEST(Test13);
	RUN_TEST(Test14);
	RUN_TEST(Test15);
	RUN_TEST(Test16);


	return (UnityEnd());
}