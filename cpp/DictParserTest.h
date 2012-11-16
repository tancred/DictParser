#ifndef DICTPARSERTEST_H
#define DICTPARSERTEST_H

#include <cxxtest/TestSuite.h>
#include "DictParser.h"

#define MY_ASSERT_THROW(expression, ExceptionType, expected_reason) \
do { \
    bool didThrow_ = false; \
    try { \
        expression; \
    } catch (const ExceptionType & e_) { \
        didThrow_ = true; \
        TS_ASSERT_EQUALS(string(expected_reason), string(e_.what())); \
    } \
    if (!didThrow_) TS_FAIL("didn't throw " #ExceptionType); \
} while (false)

using namespace std;

class DictParserTest : public CxxTest::TestSuite
{
public:

    void testEmpty() {
        istringstream stream("{}");
        DictParser parser(stream);
        DictParser::Property prop;
        TS_ASSERT_EQUALS(false, parser.getNextProperty(prop));
    }

    void testDontThrowAfterExpectedEOF() {
        istringstream stream("{}");
        DictParser parser(stream);
        DictParser::Property prop;
        TS_ASSERT_EQUALS(false, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(false, parser.getNextProperty(prop));
    }

    void testSimpleProperty() {
        istringstream stream("{name:val;}");
        DictParser parser(stream);
        DictParser::Property prop;
        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("name"), prop.name());
        TS_ASSERT_EQUALS(string("val"), prop.value());
    }

    void testBinaryProperty() {
        istringstream stream("{name(7):1234567;}");
        DictParser parser(stream);
        DictParser::Property prop;
        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("name"), prop.name());
        TS_ASSERT_EQUALS(string("1234567"), prop.value());
    }

    void testMultipleSimpleProperties() {
        istringstream stream("{name:val;name2:v2;name3:val3;}");
        DictParser parser(stream);
        DictParser::Property prop;

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("name"), prop.name());
        TS_ASSERT_EQUALS(string("val"), prop.value());

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("name2"), prop.name());
        TS_ASSERT_EQUALS(string("v2"), prop.value());

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("name3"), prop.name());
        TS_ASSERT_EQUALS(string("val3"), prop.value());
    }

    void testMultipleBinaryProperties() {
        istringstream stream("{name(7):1234567;name2(1):x;nn(3):abc;}");
        DictParser parser(stream);
        DictParser::Property prop;

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("name"), prop.name());
        TS_ASSERT_EQUALS(string("1234567"), prop.value());

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("name2"), prop.name());
        TS_ASSERT_EQUALS(string("x"), prop.value());

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("nn"), prop.name());
        TS_ASSERT_EQUALS(string("abc"), prop.value());
    }

    void testMixedProperties() {
        istringstream stream("{b1(2):ab;s1:xy;b2(3):abc;s2:rs;}");
        DictParser parser(stream);
        DictParser::Property prop;

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("b1"), prop.name());
        TS_ASSERT_EQUALS(string("ab"), prop.value());

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("s1"), prop.name());
        TS_ASSERT_EQUALS(string("xy"), prop.value());

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("b2"), prop.name());
        TS_ASSERT_EQUALS(string("abc"), prop.value());

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("s2"), prop.name());
        TS_ASSERT_EQUALS(string("rs"), prop.value());
    }

    void testEmptySimpleProperty() {
        istringstream stream("{name:;}");
        DictParser parser(stream);
        DictParser::Property prop;
        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("name"), prop.name());
        TS_ASSERT_EQUALS(string(""), prop.value());
    }

    void testEmptyBinaryProperty() {
        istringstream stream("{name(0):;}");
        DictParser parser(stream);
        DictParser::Property prop;
        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("name"), prop.name());
        TS_ASSERT_EQUALS(string(""), prop.value());
    }

    void testRepeatedNames() {
        istringstream stream("{n:;n:x;n(3):123;}");
        DictParser parser(stream);
        DictParser::Property prop;

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("n"), prop.name());
        TS_ASSERT_EQUALS(string(""), prop.value());

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("n"), prop.name());
        TS_ASSERT_EQUALS(string("x"), prop.value());

        TS_ASSERT_EQUALS(true, parser.getNextProperty(prop));
        TS_ASSERT_EQUALS(string("n"), prop.name());
        TS_ASSERT_EQUALS(string("123"), prop.value());
    }

    static void get_property(string input) {
        DictParser::Property prop;
        istringstream stream(input);
        DictParser parser(stream);
        parser.getNextProperty(prop);
    }

    void testThrowsOnMissingStartCurly() {
        MY_ASSERT_THROW(get_property("name:val;}"), DictParser::parse_error, "missing initial '{'");
    }

    void testThrowsOnEmptyName() {
        MY_ASSERT_THROW(get_property("{:val;}"), DictParser::parse_error, "missing name");
        MY_ASSERT_THROW(get_property("{(3):val;}"), DictParser::parse_error, "missing name");
    }

    void testThrowsOnEOFInName() {
        MY_ASSERT_THROW(get_property("{xyz"), DictParser::parse_error, "unexpected EOF when reading property name");
    }

    void testThrowsOnNonDigit() {
        MY_ASSERT_THROW(get_property("{a(a3):val;}"), DictParser::parse_error, "size must be digits");
        MY_ASSERT_THROW(get_property("{a(3a):val;}"), DictParser::parse_error, "size must be digits");
        MY_ASSERT_THROW(get_property("{a():val;}"), DictParser::parse_error, "size must be digits");
    }

    void testThrowsOnMissingColonAfterEndParentheses() {
        MY_ASSERT_THROW(get_property("{a(3)val;}"), DictParser::parse_error, "missing name separator ':'");
    }

    void testThrowsOnMissingSemicolonAfterSimpleValue() {
        MY_ASSERT_THROW(get_property("{a:val}"), DictParser::parse_error, "unexpected EOF when reading simple property value");
    }

    void testThrowsOnEOFInSize() {
        MY_ASSERT_THROW(get_property("{a("), DictParser::parse_error, "unexpected EOF when reading binary size");
        MY_ASSERT_THROW(get_property("{a(123"), DictParser::parse_error, "unexpected EOF when reading binary size");
    }

    void testThrowsOnMissingSemicolonAfterBinaryValue() {
        MY_ASSERT_THROW(get_property("{a(3):val}"), DictParser::parse_error, "missing binary property separator ';'");
        MY_ASSERT_THROW(get_property("{a(3):val"), DictParser::parse_error, "unexpected EOF when reading binary property separator ';'");
    }

    void testThrowsOnShortBinaryValue() {
        MY_ASSERT_THROW(get_property("{a(10):val;}"), DictParser::parse_error, "unexpected EOF when reading binary property value");
    }

    void testThrowsOnMissingEndCurly() {
        MY_ASSERT_THROW(get_property("{"), DictParser::parse_error, "missing dict separator '}'");
        MY_ASSERT_THROW(get_property("{a:val;"), DictParser::parse_error, "missing dict separator '}'");
    }
};

#endif /* DICTPARSERTEST_H */
