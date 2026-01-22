#include "../Unity-master/src/unity.h"
#include "../../Validation/Validation.hpp"

void setUp(void)
{}

void tearDown(void)
{}

void Test1()
{
	std::string exp = "Hello";
	std::string s1 = "HeLlo";
	TEST_ASSERT_EQUAL_STRING(exp.c_str(), s1.c_str());
}

void Test2()
{
	TEST_PASS();
}

int main()
{
	RUN_TEST(Test1);
	RUN_TEST(Test2);


	return (UnityEnd());
}