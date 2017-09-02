#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <gtest/gtest.h>

#include <plustache/template.hpp>
#include <plustache/plustache_types.hpp>

// The fixture for testing class Foo.
class ChangeDelimiterTest : public ::testing::Test
{
 protected:
    std::string result_string;
    std::string result_file;
    std::string template_string;
    std::string result_notfound;
    PlustacheTypes::ObjectType ctx;
    std::string file;

    ChangeDelimiterTest()
    {
    }

    virtual ~ChangeDelimiterTest()
    {
    }

    virtual void SetUp()
    {
        template_string = "Hi I am {{name}}.\n";
        template_string += "{{=<\% \%>=}}";
        template_string += "I like <\%pet\%>.";

        file = "multiple.mustache";

        std::ofstream myfile;
        myfile.open (file.c_str());
        myfile << template_string;
        myfile.close();

        ctx["name"] = "Daniel";
        ctx["pet"] = "turtles";
        try {
          std::cout << "starting" << std::endl;
          Plustache::template_t t;
          std::cout << "start2" << std::endl;
          result_string = t.render(template_string, ctx);
          std::cout << "A" << std::endl;
          Plustache::template_t t2;
          std::cout << "B" << std::endl;
          result_file = t2.render(file, ctx);
          std::cout << "end" << std::endl;
        } catch (std::regex_error re) {
          std::cout << re.what() << std::endl;
          std::cout << "??" << std::endl;
          regex_debug(re.code());
        }
    }

    virtual void TearDown()
    {
        remove(file.c_str());
    }

};

// Tests that a simple mustache tag is replaced
TEST_F(ChangeDelimiterTest, TestChangeDelimiterFromString)
{
    std::string expected = "Hi I am Daniel.\n";
           expected += "I like turtles.";
    EXPECT_EQ(expected, result_string);
}

TEST_F(ChangeDelimiterTest, TestChangeDelimiterFromFile)
{
    std::string expected = "Hi I am Daniel.\n";
           expected += "I like turtles.";
    EXPECT_EQ(expected, result_file);
}
