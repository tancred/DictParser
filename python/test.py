import unittest
import sys
import DictParserTest

if __name__ == '__main__':
    suite = unittest.TestSuite([
        DictParserTest.suite()
    ])
    runner = unittest.TextTestRunner()
    result = runner.run(suite)
    sys.exit(not result.wasSuccessful())
