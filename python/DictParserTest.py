import unittest
import io
from DictParser import DictParser


def suite():
    return unittest.TestSuite([
        unittest.TestLoader().loadTestsFromTestCase(DictParserTest),
    ])


def stream(text):
    return io.BytesIO(bytes(text))


class DictParserTest(unittest.TestCase):

    def testEmpty(self):
        parser = DictParser(stream("{}"))
        self.assertIsNone(parser.getNextProperty())

    def testDontThrowAfterExpectedEOF(self):
        parser = DictParser(stream("{}"))
        self.assertIsNone(parser.getNextProperty())
        self.assertIsNone(parser.getNextProperty())

    def testSimpleProperty(self):
        parser = DictParser(stream("{name:val;}"))
        prop = parser.getNextProperty()
        self.assertEqual("name", prop.name())
        self.assertEqual("val", prop.value())

    def testBinaryProperty(self):
        parser = DictParser(stream("{name(7):1234567;}"))
        prop = parser.getNextProperty()
        self.assertEqual("name", prop.name())
        self.assertEqual("1234567", prop.value())

    def testMultipleSimpleProperties(self):
        parser = DictParser(stream("{name:val;name2:v2;name3:val3;}"))

        prop = parser.getNextProperty()
        self.assertEqual("name", prop.name())
        self.assertEqual("val", prop.value())

        prop = parser.getNextProperty()
        self.assertEqual("name2", prop.name())
        self.assertEqual("v2", prop.value())

        prop = parser.getNextProperty()
        self.assertEqual("name3", prop.name())
        self.assertEqual("val3", prop.value())

    def testMultipleBinaryProperties(self):
        parser = DictParser(stream("{name(7):1234567;name2(1):x;nn(3):abc;}"))

        prop = parser.getNextProperty()
        self.assertEqual("name", prop.name())
        self.assertEqual("1234567", prop.value())

        prop = parser.getNextProperty()
        self.assertEqual("name2", prop.name())
        self.assertEqual("x", prop.value())

        prop = parser.getNextProperty()
        self.assertEqual("nn", prop.name())
        self.assertEqual("abc", prop.value())

    def testMixedProperties(self):
        parser = DictParser(stream("{b1(2):ab;s1:xy;b2(3):abc;s2:rs;}"))

        prop = parser.getNextProperty()
        self.assertEqual("b1", prop.name())
        self.assertEqual("ab", prop.value())

        prop = parser.getNextProperty()
        self.assertEqual("s1", prop.name())
        self.assertEqual("xy", prop.value())

        prop = parser.getNextProperty()
        self.assertEqual("b2", prop.name())
        self.assertEqual("abc", prop.value())

        prop = parser.getNextProperty()
        self.assertEqual("s2", prop.name())
        self.assertEqual("rs", prop.value())

    def testEmptySimpleProperty(self):
        parser = DictParser(stream("{name:;}"))
        prop = parser.getNextProperty()
        self.assertEqual("name", prop.name())
        self.assertEqual("", prop.value())

    def testEmptyBinaryProperty(self):
        parser = DictParser(stream("{name(0):;}"))
        prop = parser.getNextProperty()
        self.assertEqual("name", prop.name())
        self.assertEqual("", prop.value())

    def testRepeatedNames(self):
        parser = DictParser(stream("{n:;n:x;n(3):123;}"))

        prop = parser.getNextProperty()
        self.assertEqual("n", prop.name())
        self.assertEqual("", prop.value())

        prop = parser.getNextProperty()
        self.assertEqual("n", prop.name())
        self.assertEqual("x", prop.value())

        prop = parser.getNextProperty()
        self.assertEqual("n", prop.name())
        self.assertEqual("123", prop.value())

    def assertParseError(self, input, expectedMessage):
        try:
            DictParser(stream(input)).getNextProperty()
        except DictParser.ParseError as perr:
            self.assertEqual(expectedMessage, perr.args[0])
            return
        except Exception as ex:
            self.fail("expected ParseError('%s') but got >>%s<<" % (expectedMessage, str(ex)))
        self.fail("expected ParseError('%s') but nothing raised" % (expectedMessage))

    def testThrowsOnMissingStartCurly(self):
        self.assertParseError("name:val;}", "missing initial '{'")

    def testThrowsOnEmptyName(self):
        self.assertParseError("{:val;}", "missing name")
        self.assertParseError("{(3):val;}", "missing name")

    def testThrowsOnEOFInName(self):
        self.assertParseError("{xyz", "unexpected EOF when reading property name")

    def testThrowsOnNonDigit(self):
        self.assertParseError("{a(a3):val;}", "size must be digits")
        self.assertParseError("{a(3a):val;}", "size must be digits")
        self.assertParseError("{a():val;}", "size must be digits")

    def testThrowsOnMissingColonAfterEndParentheses(self):
        self.assertParseError("{a(3)val;}", "missing name separator ':'")

    def testThrowsOnMissingSemicolonAfterSimpleValue(self):
        self.assertParseError("{a:val}", "unexpected EOF when reading simple property value")

    def testThrowsOnEOFInSize(self):
        self.assertParseError("{a(", "unexpected EOF when reading binary size")
        self.assertParseError("{a(123", "unexpected EOF when reading binary size")

    def testThrowsOnMissingSemicolonAfterBinaryValue(self):
        self.assertParseError("{a(3):val}", "missing binary property separator ';'")
        self.assertParseError("{a(3):val", "unexpected EOF when reading binary property separator ';'")

    def testThrowsOnShortBinaryValue(self):
        self.assertParseError("{a(10):val;}", "unexpected EOF when reading binary property value")

    def testThrowsOnMissingEndCurly(self):
        self.assertParseError("{", "missing dict separator '}'")
        self.assertParseError("{a:val;", "missing dict separator '}'")
