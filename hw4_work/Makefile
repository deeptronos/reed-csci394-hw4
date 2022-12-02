UNAME := $(shell uname)
TARGET=dwislpyc
CXX=clang++
YACCFLAGS=-d -v
ifeq ($(UNAME), Darwin)
	YACC=/usr/local/opt/bison/bin/bison
	LEX=/usr/local/opt/flex/bin/flex
	INCLUDES=-I/usr/local/opt/flex/include
	LDFLAGS=-L/usr/local/opt/flex/lib -L/usr/local/opt/bison/lib -lfl -ly
else
	YACC=bison
	LEX=flex
	INCLUDES=
	LDFLAGS=
endif
CXXFLAGS=-Wall -Wextra -pedantic -Wno-c11-extensions -std=c++17 -g $(INCLUDES)
YACC_YACC=dwislpy-bison.tab.hh location.hh position.hh stack.hh dwislpy-bison.tab.cc dwislpy-bison.output
OBJ=$(SRC:.cc=.o)

all:  $(TARGET)

dwislpyc: dwislpy-flex.o dwislpy-bison.tab.o dwislpyc.o dwislpy-ast.o dwislpy-check.o dwislpy-inst.o dwislpy-mips.o dwislpy-util.o 
		$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

lexer: dwislpy-flex.cc

dwislpy-flex.cc: dwislpy-flex.ll dwislpy-flex.hh dwislpy-util.hh parser
		$(LEX) -o $@ dwislpy-flex.ll

parser: dwislpy-bison.tab.cc dwislpy-bison.tab.hh

dwislpy-bison.tab.cc: dwislpy-bison.yy dwislpy-ast.hh dwislpy-check.hh dwislpy-util.hh dwislpy-main.hh
		$(YACC) $(YACCFLAGS) dwislpy-bison.yy

%.o: %.cc %.hh
		$(CXX) $(CXXFLAGS) $(OPTFLAGS) -c -o $@ $<

dwislpy-ast.o: dwislpy-check.hh

clean:
		touch $(YACC_YACC) dwislpy-flex.cc foo.o foo~ $(TARGET)
		rm -f *~ *.o $(YACC_YACC) dwislpy-flex.cc $(TARGET)
		touch stack.hh position.hh location.hh
		rm -f stack.hh position.hh location.hh
