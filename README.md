DictParser
==========

DictParser is a tiny library
	for decoding rudimentary dictionary-like objects
	from a stream of bytes.
	The library contains two implementations,
	one in Python
	and one in C++.
	This document describes the format of an encoded dictionary object.

The Encoding
------------

We define _dictionary_ (or _dict_ for short)
	to mean a set of _properties_
	where each _property_ has a _name_ and a _value_.
	The set of properties may or may not be ordered
	and property names may or may not repeat;
	it's up to the user to define.

An _encoded dictionary_ is an ordered sequence of _encoded properties_
	enclosed in curly braces:
	`{`, `}`.
	The _empty dictionary_ is the string '`{}`'.
	The significance of the ordering of properties is user defined.

An _encoded property_ is either _simple_ or _binary_.

A _simple property_
	has a _name_ and a _value_
	(each a sequence of bytes)
	separated by a colon
	and terminated by a semicolon,
	e.g., '`name:value;`'.
	A simple property name must not be empty
	and must not contain parentheses (`(`, `)`) or colon (`:`).
	A simple property value must not contain a semicolon (`;`).

A _binary property_
	has the same structure as a simple property,
	but its name ends with the _length_ (in 8 bit bytes) of the value,
	in parentheses (`(`, `)`),
	e.g., '`hello(7): world!;`' and '`hello(6):world!;`'.
	A binary property value may contain any character.

Property names may be repeated,
	so that '`{a:x;a:y;a:z;}`'
	is a valid _encoded dictionary_ with three distinct properties.
	The interpretation of properties with identical names is user defined.

Note that white space characters are interpreted as any other characters;
	any line feed, space, tab, etc, will be interpreted verbatim,
	as part of a property name or value.


The API
-------

This library provides an interface
	to decode an _encoded dictionary_
	one property at a time.
	The interface is pretty straight forward.
	You instantiate a `DictParser`
	with a `stream`
	and call `getNextProperty()`
	until all available (or desired?) properties
	have been read.
	The parser instance will throw an exception
	on invalid input
	or stream errors.

Here's an example of how to parse
	the contents of a file
	in Python:

```python
import DictParser
import io

f = io.open("dict.txt", mode="r+b")
parser = DictParser.DictParser(f)
while True:
  prop = parser.getNextProperty()
  if not prop: break
  print "%s: %s" % (prop.name(), prop.value())
```

Here's the same example in C++:

```c++
#include <fstream>
#include "DictParser.h"

int main(int argc, char *argv[]) {
  std::fstream stream("dict.txt", std::ios::in|std::ios::binary);
  DictParser parser(stream);
  DictParser::Property prop;
  while (parser.getNextProperty(prop)) {
    std::cout << prop.name() << ": " << prop.value() << std::endl;
  }
  std::cout << std::flush;
  return 0;
}
```
