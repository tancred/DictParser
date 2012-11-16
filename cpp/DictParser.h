#ifndef __DictParser_h__
#define __DictParser_h__

// http://www.mr-edd.co.uk/blog/beginners_guide_streambuf
// http://stackoverflow.com/questions/7107025/unbuffered-std-streambuf-implementation
// http://bytes.com/topic/c/answers/134171-unbuffered-basic_streambuf-implementation
// http://uw714doc.sco.com/en/SDK_clib/CONTENTS.html

//TODO: Parse dict from stream, but read at most maxBytes.
// DictParser(std::istream& stream, size_t maxBytes);

//TODO: skip dict, i.e., skip past the whole dict in the input.
//TODO: optimization: make skip effective, i.e., don't store data.

//TODO: optimization: stream-based property parsing.
// * implement a custom istream/streambuf pair that can wrap an existing istream, but read at most maxBytes.
// * istream& DictParser::Property::valueStream() is the default.
// * string DictParser::Property::value() based on valueStream().

#include <string>
#include <stdexcept>
#include <iostream>
#include <stdint.h>

class DictParser {
    public:
        class parse_error : public std::runtime_error {
            public:
                explicit parse_error(const std::string& what) : std::runtime_error(what) {}
                virtual ~parse_error() throw() {}
        };

        class Property : public std::pair<std::string, std::string> {
            // Prefer name() and value() for .first and .second; we
            // want to move to a stream based variant in the future.
            public:
                std::string name() const;
                std::string value() const;
                //std::istream valueStream() const;
        };

    private:
        std::istream& input;
        int state;

    public:
        DictParser(std::istream& stream) : input(stream), state(0) {};

        bool getNextProperty(Property&);
        // Returns true if a property was read.
        // Returns false if input is exhausted (and no property read).
        // Throws parse_error on malformed parameters, including premature EOF.
    
    private:
        bool getName(std::string& name);
        void getValue(std::string& value);

        void matchOpenCurly();
        void matchNameStart();
        bool getNameValue(std::string& name);
        void getSimpleValue(std::string& value);
        void getBinaryValue(std::string& value);
        uint64_t getBinarySize();
        void matchBinaryNameSeparator();
        void getBinaryData(std::string& value, uint64_t size);
        void matchBinaryPropertySeparator();
};


#endif /* __DictParser_h__ */