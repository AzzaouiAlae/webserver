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

void Test17()
{
	Tokenizing file("ConfigFilesTESTS/invalid12.conf");
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

void Test18()
{
	Tokenizing file("ConfigFilesTESTS/invalid13.conf");
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

void Test19()
{
	Tokenizing file("ConfigFilesTESTS/invalid14.conf");
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

void Test20()
{
	Tokenizing file("ConfigFilesTESTS/invalid15.conf");
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

void Test21()
{
	Tokenizing file("ConfigFilesTESTS/invalid16.conf");
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

void Test22()
{
	Tokenizing file("ConfigFilesTESTS/invalid17.conf");
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


void Test23()
{
	Tokenizing file("ConfigFilesTESTS/invalid18.conf");
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

void Test24()
{
	Tokenizing file("ConfigFilesTESTS/invalid19.conf");
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

void Test25()
{
	Tokenizing file("ConfigFilesTESTS/invalid20.conf");
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

void Test26()
{
	Tokenizing file("ConfigFilesTESTS/invalid21.conf");
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

void Test27()
{
	Tokenizing file("ConfigFilesTESTS/invalid22.conf");
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

void Test28()
{
	Tokenizing file("ConfigFilesTESTS/invalid23.conf");
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

void Test29()
{
	Tokenizing file("ConfigFilesTESTS/invalid24.conf");
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

void Test30()
{
	Tokenizing file("ConfigFilesTESTS/invalid25.conf");
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

void Test31()
{
	Tokenizing file("ConfigFilesTESTS/invalid26.conf");
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

void Test32()
{
	Tokenizing file("ConfigFilesTESTS/invalid27.conf");
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

void Test33()
{
	Tokenizing file("ConfigFilesTESTS/invalid28.conf");
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

void Test34()
{
	Tokenizing file("ConfigFilesTESTS/invalid29.conf");
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

void Test35()
{
	Tokenizing file("ConfigFilesTESTS/invalid30.conf");
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

void Test36()
{
	Tokenizing file("ConfigFilesTESTS/invalid31.conf");
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

void Test37()
{
	Tokenizing file("ConfigFilesTESTS/invalid32.conf");
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

void Test38()
{
	Tokenizing file("ConfigFilesTESTS/invalid33.conf");
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

void Test39()
{
	Tokenizing file("ConfigFilesTESTS/invalid34.conf");
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

void Test40()
{
	Tokenizing file("ConfigFilesTESTS/invalid35.conf");
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

void Test41()
{
	Tokenizing file("ConfigFilesTESTS/invalid36.conf");
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

void Test42()
{
	Tokenizing file("ConfigFilesTESTS/invalid37.conf");
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

void Test43()
{
	Tokenizing file("ConfigFilesTESTS/invalid38.conf");
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

void Test44()
{
	Tokenizing file("ConfigFilesTESTS/invalid39.conf");
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

void Test45()
{
	Tokenizing file("ConfigFilesTESTS/invalid40.conf");
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

void Test46()
{
	Tokenizing file("ConfigFilesTESTS/invalid41.conf");
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

void Test47()
{
	Tokenizing file("ConfigFilesTESTS/invalid42.conf");
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

void Test48()
{
	Tokenizing file("ConfigFilesTESTS/invalid43.conf");
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

void Test49()
{
	Tokenizing file("ConfigFilesTESTS/invalid44.conf");
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

void Test50()
{
	Tokenizing file("ConfigFilesTESTS/invalid45.conf");
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

void Test51()
{
	Tokenizing file("ConfigFilesTESTS/invalid46.conf");
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

void Test52()
{
	Tokenizing file("ConfigFilesTESTS/invalid47.conf");
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

void Test53()
{
	Tokenizing file("ConfigFilesTESTS/invalid48.conf");
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

void Test54()
{
	Tokenizing file("ConfigFilesTESTS/invalid49.conf");
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

void Test55()
{
	Tokenizing file("ConfigFilesTESTS/invalid50.conf");
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
	RUN_TEST(Test17);
	RUN_TEST(Test18);
	RUN_TEST(Test19);
	RUN_TEST(Test20);
	RUN_TEST(Test21);
	RUN_TEST(Test22);
	RUN_TEST(Test23);
	RUN_TEST(Test24);
	RUN_TEST(Test25);
	RUN_TEST(Test26);
	RUN_TEST(Test27);
	RUN_TEST(Test28);
	RUN_TEST(Test29);
	RUN_TEST(Test30);
	RUN_TEST(Test31);
	RUN_TEST(Test32);
	RUN_TEST(Test33);
	RUN_TEST(Test34);
	RUN_TEST(Test35);
	RUN_TEST(Test36);
	RUN_TEST(Test37);
	RUN_TEST(Test38);
	RUN_TEST(Test39);
	RUN_TEST(Test40);
	RUN_TEST(Test41);
	RUN_TEST(Test42);
	RUN_TEST(Test43);
	RUN_TEST(Test44);
	RUN_TEST(Test45);
	RUN_TEST(Test46);
	RUN_TEST(Test47);
	RUN_TEST(Test48);
	RUN_TEST(Test49);
	RUN_TEST(Test50);
	RUN_TEST(Test51);
	RUN_TEST(Test52);
	RUN_TEST(Test53);
	RUN_TEST(Test54);
	RUN_TEST(Test55);


	return (UnityEnd());
}