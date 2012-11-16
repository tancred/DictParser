import io
import sys

def val2str(i):
  if i == None: return "?"
  return str(i)

class Property(object):
    def __init__(self,name,value):
        self._name = name
        self._value = value
    
    def name(self):
        return self._name;
    
    def value(self):
        return self._value;

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        return "<DictParser.Property: name='%s' value='%s'>" % (val2str(self._name), val2str(self._value))

    def __unicode__(self):
        return unicode(self.__str__())

class State:
    (START, CURLY, NAME_START, NAME_DATA, SIMPLE, BINARY_SIZE, BINARY_DATA, END) = range(8)

class DictParser(object):
    
    class ParseError(Exception):
        pass
    
    def __init__(self, inputStream):
        self.state = State.START
        self.input = io.BufferedReader(inputStream)
    
    def getNextProperty(self):
        name = self.parseName()
        if name == None: return None # EOF
        value = self.parseValue()
        return Property(name,value)
    
    #
    # Internal methods after this point
    #
    
    def parseName(self):
        if self.state == State.END: return None
        if self.state in [State.START, State.CURLY]: self.matchOpenCurly()
        if self.state == State.NAME_START: self.matchNameStart()
        return self.parseNameValue()
    
    def parseValue(self):
        val = None
        if   self.state == State.SIMPLE: val = self.parseSimpleValue()
        elif self.state == State.BINARY_SIZE: val = self.parseBinaryValue()
        self.matchNameStart()
        return val
    
    def parseNameValue(self):
        if self.state == State.END: return None
        if self.state != State.NAME_DATA: self.barf("invalid state: not at name data")
        
        nameBuf = []
        while True:
            c = self.readOrBarf("unexpected EOF when reading property name")
            if c == b':':
                self.state = State.SIMPLE
                break
            elif c == b'(':
                self.state = State.BINARY_SIZE
                break
            nameBuf.append(c)        
        return b''.join(nameBuf)
    
    def parseSimpleValue(self):
        valBuf = []
        while True:
            c = self.readOrBarf("unexpected EOF when reading simple property value")
            if c == b';':
                self.state = State.NAME_START
                break
            valBuf.append(c)
        return b''.join(valBuf)
    
    def parseBinaryValue(self):
        val = self.parseBinaryData( self.parseBinarySize() )
        self.matchBinaryPropertySeparator()
        self.state = State.NAME_START
        return val
    
    def parseBinarySize(self):
        digits = []
        digit_range = range(ord('0'),ord('9')+1)
        
        while True:
            c = self.readOrBarf("unexpected EOF when reading binary size")
            if c == b')': break
            if ord(c) not in digit_range:
                self.barf("size must be digits")
            digits.append(c)
        if len(digits) == 0: self.barf("size must be digits")

        self.matchBinaryNameSeparator()
        
        self.state = State.BINARY_DATA
        return int(b''.join(digits))
    
    def parseBinaryData(self, size):
        val = self.input.read(size)
        if len(val) != size: self.barf("unexpected EOF when reading binary property value")
        return val
    
    def matchOpenCurly(self):
        c = self.peekOrBarf("unexpected EOF when reading open curly")
        if c != b'{': self.barf("missing initial '{'")
        self.input.read(1)
        self.state = State.NAME_START
    
    def matchNameStart(self):
        if self.state != State.NAME_START: self.barf("invalid state: not at name start")
        
        lookahead = self.peekOrBarf("missing dict separator '}'")
        
        if lookahead == b'}':
            self.input.read(1)
            self.state = State.END
            return
        
        if lookahead in [b'(', b':']: self.barf("missing name")
        self.state = State.NAME_DATA
    
    def matchBinaryNameSeparator(self):
        c = self.peekOrBarf("unexpected EOF when reading binary value separator ':'")
        if c != b':': self.barf("missing name separator ':'")
        self.input.read(1)
    
    def matchBinaryPropertySeparator(self):
        c = self.peekOrBarf("unexpected EOF when reading binary property separator ';'")
        if c != b';': self.barf("missing binary property separator ';'")
        self.input.read(1)
    
    def peekOrBarf(self, message):
        buf = self.input.peek(1)
        if len(buf) < 1: self.barf(message)
        return buf[0]
    
    def readOrBarf(self, message):
        c = self.input.read(1)
        if len(c) != 1: self.barf(message)
        return c
    
    def barf(self,message):
        raise DictParser.ParseError(message)
