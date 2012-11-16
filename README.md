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

The central idea is a _list of properties_,
  similar to a `dict` in Python.
  It's not not the same
  as repeated keys are allowed.
  We'll reuse the name _dictionary_,
  or _dict_ for short.

A _dictionary_ is a (possibly ordered) sequence of _properties_
	enclosed in curly braces:
	`{`, `}`.
	The _empty dictionary_ is the string '`{}`'.
	The significance of the ordering of properties is user defined.

There are _simple_ and _binary_ properties.

A _simple property_
	has a _name_ and a _value_ separated by a colon
	and is terminated by a semicolon,
	e.g., '`name:value;`'.
	A simple property name must not be empty
	and must not contain parentheses or colons: `(`, `)` and `:`.
	A simple property value must not contain a semicolon: `;`.

A _binary property_
	has the same structure as a simple property,
	but its name ends with the _length_ (in 8 bit bytes) of the value,
	in parentheses,
	e.g., '`hello(7): world!;`'.</p>

As mentioned,
	property names may be repeated,
	so that '`{a:x;a:y;a:z;}`'
	is a valid _dict_ with three distinct properties.
	The interpretation of properties with identical names is user defined.

Note that white space characters are interpreted as any other characters;
	any line feed, space, tab, etc, will be interpreted verbatim,
	as part of a property name or value.
