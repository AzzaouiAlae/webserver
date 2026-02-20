#include "../Unity-master/src/unity.h"
#include "../../Headers.hpp"

void setUp(void)
{}

void tearDown(void)
{}

void TestRootPath()
{
	try {
		Tokenizing token("Test/PathTests/ConfigFiles/TestRootPath.conf");
		token.split_tokens();
		Validation valid(token.get_tokens());
		valid.CheckValidation();

	} catch (const exception& e) {
		TEST_FAIL_MESSAGE(e.what());
        return;
	}
	Path path(Singleton::GetASTroot().GetChildren()[0], "/");
	TEST_ASSERT_EQUAL_STRING("/usr/bin/index.html", path.CreatePath().c_str());
}

void TestLocationPath()
{
	try {
		Tokenizing token("Test/PathTests/ConfigFiles/TestLocationPath.conf");
		token.split_tokens();
		Validation valid(token.get_tokens());
		valid.CheckValidation();

	} catch (const exception& e) {
		TEST_FAIL_MESSAGE(e.what());
        return;
	}
	Path path(Singleton::GetASTroot().GetChildren()[0], "/site1");
	TEST_ASSERT_EQUAL_STRING("/home/usr1/site1/site1.html", path.CreatePath().c_str());
}

void TestLocationRootPath()
{
	try {
		Tokenizing token("Test/PathTests/ConfigFiles/TestLocationRootPath.conf");
		token.split_tokens();
		Validation valid(token.get_tokens());
		valid.CheckValidation();

	} catch (const exception& e) {
		TEST_FAIL_MESSAGE(e.what());
        return;
	}
	Path path(Singleton::GetASTroot().GetChildren()[0], "/");
	TEST_ASSERT_EQUAL_STRING("/home/usr/site.html", path.CreatePath().c_str());
}

void TestLocationRootPath()
{
	try {
		Tokenizing token("Test/PathTests/ConfigFiles/TestLocationRootPath.conf");
		token.split_tokens();
		Validation valid(token.get_tokens());
		valid.CheckValidation();

	} catch (const exception& e) {
		TEST_FAIL_MESSAGE(e.what());
        return;
	}
	Path path(Singleton::GetASTroot().GetChildren()[0], "/");
	TEST_ASSERT_EQUAL_STRING("/home/usr/site.html", path.CreatePath().c_str());
}

void TestLocationSubDir()
{
    try {
        Tokenizing token("Test/PathTests/ConfigFiles/TestLocationRootPath.conf");
        token.split_tokens();
        Validation valid(token.get_tokens());
        valid.CheckValidation();

    } catch (const exception& e) {
        TEST_FAIL_MESSAGE(e.what());
        return;
    }
    Path path(Singleton::GetASTroot().GetChildren()[0], "/site2/subdir/index.html");
    TEST_ASSERT_EQUAL_STRING("/home/usr2/site2/subdir/index.html", path.CreatePath().c_str());
}

void TestNonExistionPath()
{
    try {
        Tokenizing token("Test/PathTests/ConfigFiles/TestLocationRootPath.conf");
        token.split_tokens();
        Validation valid(token.get_tokens());
        valid.CheckValidation();

    } catch (const exception& e) {
        TEST_FAIL_MESSAGE(e.what());
        return;
    }
    Path path(Singleton::GetASTroot().GetChildren()[0], "/nonexistion/path/index.html");
    TEST_ASSERT_EQUAL_STRING("/usr/bin/nonexistion/path/index.html", path.CreatePath().c_str());
}

void TestNonExistionPathWithLocationRootPath()
{
    try {
        Tokenizing token("Test/PathTests/ConfigFiles/TestLocationRootPath.conf");
        token.split_tokens();
        Validation valid(token.get_tokens());
        valid.CheckValidation();

    } catch (const exception& e) {
        TEST_FAIL_MESSAGE(e.what());
        return;
    }
    Path path(Singleton::GetASTroot().GetChildren()[0], "/nonexistion/path/index.html");
    TEST_ASSERT_EQUAL_STRING("/home/usr/nonexistion/path/index.html", path.CreatePath().c_str());
}

int main()
{
    RUN_TEST(TestRootPath);
    RUN_TEST(TestLocationPath);
    RUN_TEST(TestLocationRootPath);
    RUN_TEST(TestLocationSubDir);
    RUN_TEST(TestNonExistionPath);
    RUN_TEST(TestNonExistionPathWithLocationRootPath);
    // RUN_TEST();
    // RUN_TEST();


}