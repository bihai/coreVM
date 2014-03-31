#include <sneaker/testing/_unittest.h>
#include "../../include/types/errors.h"
#include "../../include/types/native_string.h"


class native_string_unittest : public ::testing::Test {};


TEST_F(native_string_unittest, TestEmptyInitialization)
{
  const corevm::types::native_string str;

  ASSERT_EQ(true, str.empty());
  ASSERT_EQ(0, str.length());
  ASSERT_EQ(0, str.size());
}

TEST_F(native_string_unittest, TestInitializationWithCString)
{
  const char* s = "Hello world!";
  const corevm::types::native_string str(s);

  ASSERT_EQ(false, str.empty());
  ASSERT_EQ(strlen(s), str.length());
  ASSERT_EQ(strlen(s), str.size());

  ASSERT_STREQ(s, str.c_str());
}

TEST_F(native_string_unittest, TestInitialziationWithStdString)
{
  const std::string s("Hello world!");
  const corevm::types::native_string str(s);

  ASSERT_EQ(false, str.empty());
  ASSERT_EQ(s.length(), str.length());
  ASSERT_EQ(s.size(), str.size());

  ASSERT_STREQ(s.c_str(), str.c_str());
}

TEST_F(native_string_unittest, TestCopyConstructor)
{
  const corevm::types::native_string s("Hello world!");
  const corevm::types::native_string str = s;

  ASSERT_EQ(false, str.empty());
  ASSERT_EQ(s.length(), str.length());
  ASSERT_EQ(s.size(), str.size());

  ASSERT_STREQ(s.c_str(), str.c_str());
}

TEST_F(native_string_unittest, TestCopyConstructorOnStdString)
{
  const std::string s("Hello world!");
  const corevm::types::native_string str = s;

  ASSERT_EQ(false, str.empty());
  ASSERT_EQ(s.length(), str.length());
  ASSERT_EQ(s.size(), str.size());

  ASSERT_STREQ(s.c_str(), str.c_str());
}

TEST_F(native_string_unittest, TestCopyConstructorOnCString)
{
  const char* s = "Hello world!";
  const corevm::types::native_string str = s;

  ASSERT_EQ(false, str.empty());
  ASSERT_EQ(strlen(s), str.length());
  ASSERT_EQ(strlen(s), str.size());

  ASSERT_STREQ(s, str.c_str());
}

TEST_F(native_string_unittest, TestCopyConstructorOnIntegerType)
{
  ASSERT_THROW(
    {const corevm::types::native_string str = 123;},
    corevm::types::corevm_native_type_conversion_error
  );
}

TEST_F(native_string_unittest, TestConvertingToIntegerType)
{
  ASSERT_THROW(
    {int i = corevm::types::native_string("123"); i++;},
    corevm::types::corevm_native_type_conversion_error
  );
}

TEST_F(native_string_unittest, TestAssignmentOperator)
{
  const corevm::types::native_string s("Hello world!");
  corevm::types::native_string str;
  str = s;

  ASSERT_EQ(false, str.empty());
  ASSERT_EQ(s.length(), str.length());
  ASSERT_EQ(s.size(), str.size());

  ASSERT_STREQ(s.c_str(), str.c_str());
}

TEST_F(native_string_unittest, TestAssignmentOperatorOnStdString)
{
  const std::string s("Hello world!");
  corevm::types::native_string str;
  str = s;

  ASSERT_EQ(false, str.empty());
  ASSERT_EQ(s.length(), str.length());
  ASSERT_EQ(s.size(), str.size());

  ASSERT_STREQ(s.c_str(), str.c_str());
}

TEST_F(native_string_unittest, TestAssignmentOperatorOnCString)
{
  const char* s ="Hello world!";
  corevm::types::native_string str;
  str = s;

  ASSERT_EQ(false, str.empty());
  ASSERT_EQ(strlen(s), str.length());

  ASSERT_STREQ(s, str.c_str());
}

TEST_F(native_string_unittest, TestEqualityBetweenEmptyStrings)
{
  corevm::types::native_string str1;
  corevm::types::native_string str2;

  ASSERT_TRUE(str1 == str1);
  ASSERT_TRUE(str2 == str2);

  ASSERT_TRUE(str1 == str2);
}

TEST_F(native_string_unittest, TestEqualityBetweenNonEmptyStrings)
{
  corevm::types::native_string str1("Hello world!");
  corevm::types::native_string str2("Hello");

  ASSERT_FALSE(str1 == str2);
  ASSERT_TRUE(str1 != str2);
}

TEST_F(native_string_unittest, TestEqualityBetweenIdenticalStrings)
{
  corevm::types::native_string str1("Hello world!");
  corevm::types::native_string str2("Hello world!");

  ASSERT_TRUE(str1 == str1);
  ASSERT_TRUE(str2 == str2);

  ASSERT_TRUE(str1 == str2);
}


class native_string_operator_unittest : public native_string_unittest {};


class native_string_unary_operator_unittest : public native_string_operator_unittest {};


TEST_F(native_string_unary_operator_unittest, TestPositiveOperator)
{
  ASSERT_THROW(
    {const corevm::types::native_string str("123"); +str;},
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_unary_operator_unittest, TestNegationOperator)
{
  ASSERT_THROW(
    {const corevm::types::native_string str("123"); -str;},
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_unary_operator_unittest, TestIncrementOperator)
{
  ASSERT_THROW(
    {const corevm::types::native_string str("123"); ++str;},
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_unary_operator_unittest, TestDecrementOperator)
{
  ASSERT_THROW(
    {const corevm::types::native_string str("123"); --str;},
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_unary_operator_unittest, TestLogicalNotOperator)
{
  ASSERT_THROW(
    {const corevm::types::native_string str("123"); !str;},
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_unary_operator_unittest, TestBitwiseNotOperator)
{
  ASSERT_THROW(
    {const corevm::types::native_string str("123"); ~str;},
    corevm::types::corevm_native_type_invalid_operator_error
  );
}


class native_string_binary_operator_unittest : public native_string_operator_unittest {};


TEST_F(native_string_binary_operator_unittest, TestAdditionOperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 + str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_binary_operator_unittest, TestSubtractionOperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 - str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_binary_operator_unittest, TestMultiplicationOperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 * str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_binary_operator_unittest, TestDivisionOperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 / str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_binary_operator_unittest, TestLogicalANDOperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 && str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_binary_operator_unittest, TestLogicalOROperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 || str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_binary_operator_unittest, TestBitwiseANDOperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 & str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_binary_operator_unittest, TestBitwiseOROperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 | str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_binary_operator_unittest, TestBitwiseXOROperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 ^ str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_binary_operator_unittest, TestBitwiseLeftShiftOperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 << str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

TEST_F(native_string_binary_operator_unittest, TestBitwiseRightShiftOperator)
{
  ASSERT_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      str1 >> str2;
    },
    corevm::types::corevm_native_type_invalid_operator_error
  );
}

void f(bool) {}

TEST_F(native_string_binary_operator_unittest, TestEQOperator)
{
  ASSERT_NO_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      f(str1 == str2);
    }
  );
}

TEST_F(native_string_binary_operator_unittest, TestNEQOperator)
{
  ASSERT_NO_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      f(str1 != str2);
    }
  );
}

TEST_F(native_string_binary_operator_unittest, TestGTOperator)
{
  ASSERT_NO_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      f(str1 > str2);
    }
  );
}

TEST_F(native_string_binary_operator_unittest, TestLTOperator)
{
  ASSERT_NO_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      f(str1 < str2);
    }
  );
}

TEST_F(native_string_binary_operator_unittest, TestGTEOperator)
{
  ASSERT_NO_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      f(str1 >= str2);
    }
  );
}

TEST_F(native_string_binary_operator_unittest, TestLTEOperator)
{
  ASSERT_NO_THROW(
    {
      const corevm::types::native_string str1("Hello ");
      const corevm::types::native_string str2("world!");
      f(str1 <= str2);
    }
  );
}