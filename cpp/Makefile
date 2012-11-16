CXXTESTDIR = ../deps/cxxtest
CXXTESTGEN = $(CXXTESTDIR)/bin/cxxtestgen
CXXFLAGS = -g

test: runner
	./runner

runner: runner.cpp DictParser.o
	$(CXX) $(CXXFLAGS) -I$(CXXTESTDIR) -o $@ $^

runner.cpp: DictParserTest.h
	$(CXXTESTGEN) --error-printer -o $@ $<

DictParser.o: DictParser.cpp DictParser.h

clean:
	@rm -f *.o
	@rm -r runner runner.dSYM runner.cpp
