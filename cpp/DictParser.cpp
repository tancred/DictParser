#include "DictParser.h"
#include <sstream>
#include <stdio.h>

enum DictParserParseStates {
    STATE_START = 0,
    STATE_CURLY,
    STATE_NAME_START,
    STATE_NAME_DATA,
    STATE_SIMPLE,
    STATE_BINARY_SIZE,
    STATE_BINARY_DATA,
    STATE_END
};

static void barf(std::string msg) {
    throw DictParser::parse_error(msg);
}

bool DictParser::getNextProperty(Property& property) {
    std::string name;
    std::string value;
    if (!getName(name)) return false; // EOF
    getValue(value);
    property.first = name;
    property.second = value;
    return true;
}

static int peekOrBarf(std::istream& stream, std::string msg) {
    int c = stream.peek();
    if (c == EOF) barf(msg);
    return c;
}

bool DictParser::getName(std::string& name) {
    if (state == STATE_END) return false;
    if (state == STATE_START || state == STATE_CURLY) matchOpenCurly();
    if (state == STATE_NAME_START) matchNameStart();
    return getNameValue(name);
}

void DictParser::matchNameStart() {
    if (state != STATE_NAME_START) { barf("invalid state: not at name start"); }

    int lookahead = peekOrBarf(input, "missing dict separator '}'");

    if (lookahead == '}') {
        input.get();
        state = STATE_END;
        return;
    }

    if (lookahead == '(' || lookahead == ':') barf("missing name");
    state = STATE_NAME_DATA;
}

void DictParser::matchOpenCurly() {
    int c = peekOrBarf(input, "unexpected EOF when reading open curly");
    if (c != '{') barf("missing initial '{'");
    input.get();
    state = STATE_NAME_START;
}

bool DictParser::getNameValue(std::string& name) {
    if (state == STATE_END) return false;
    if (state != STATE_NAME_DATA) barf("invalid state: not at name data");
    
    name.clear();
    while (input.good()) {
        char c;
        input.get(c);
        if (input.fail()) barf("unexpected EOF when reading property name");
        if (c == ':') {
            state = STATE_SIMPLE;
            break;
        } else if (c == '(') {
            state = STATE_BINARY_SIZE;
            break;
        }
        name.push_back(c);
    }
    return true;
}

void DictParser::getValue(std::string& value) {
    if (state == STATE_SIMPLE) { getSimpleValue(value); }
    if (state == STATE_BINARY_SIZE) { getBinaryValue(value); }
    matchNameStart();
}

void DictParser::getSimpleValue(std::string& value) {
    value.clear();
    while (input.good()) {
        char c;
        input.get(c);
        if (input.fail()) barf("unexpected EOF when reading simple property value");
        if (c == ';') { state = STATE_NAME_START; break; }
        value.push_back(c);
    }
}

void DictParser::getBinaryValue(std::string& value) {
    uint64_t size = getBinarySize();
    getBinaryData(value, size);
    matchBinaryPropertySeparator();
    state = STATE_NAME_START;
}

void DictParser::matchBinaryPropertySeparator() {
    int c = peekOrBarf(input, "unexpected EOF when reading binary property separator ';'");
    if (c != ';') barf("missing binary property separator ';'");
    input.get();
}

uint64_t DictParser::getBinarySize() {
    std::string digits;

    while (input.good()) {
        char c;
        input.get(c);
        if (input.fail()) barf("unexpected EOF when reading binary size");
        if (c == ')') break;
        if (c < '0' || c > '9') barf("size must be digits");
        digits.push_back(c);
    }
    if (digits.length() == 0) barf("size must be digits");
    matchBinaryNameSeparator();
    
    std::istringstream digitStream(digits);
    uint64_t size = 0;
    digitStream >> size;
    
    state = STATE_BINARY_DATA;
    return size;
}

void DictParser::getBinaryData(std::string& value, uint64_t size) {
    value.clear();
    for (uint64_t i=0; i<size; i++) {
        char c;
        input.get(c);
        if (input.fail()) barf("unexpected EOF when reading binary property value");
        value.push_back(c);
    }
}

void DictParser::matchBinaryNameSeparator() {
    int c = peekOrBarf(input, "unexpected EOF when reading binary value separator ':'");
    if (c != ':') barf("missing name separator ':'");
    input.get();
}

std::string DictParser::Property::name() const {
    return (*this).first;
}

std::string DictParser::Property::value() const {
    return (*this).second;
}
