# g++ -std=c++11 main.cpp LCN.cpp -o main
EXEDIR = ../bin
EXE = hw2

HEADERS = input.hpp Partitioning.hpp
SOURCES = main.cpp input.cpp Partitioning.cpp

CFLAGS = -std=c++14 -O3 --optimize -I /usr/local/include/boost/

INPUTDIR = ../testcase
INPUTFILENAME = public6

OUTPUTDIR = ../output
OUTPUTFILENAME = $(INPUTFILENAME).out

CHECKDIR = ../check
CHECKFILENAME = $(INPUTFILENAME)

all: build run verify

build: $(HEADERS) $(SOURCES)
	g++ $(CFLAGS) $(SOURCES) -o $(EXEDIR)/$(EXE)

run: 
#### if debug needed, add -d flag & CHECK argv ####
#	$(EXEDIR)/$(EXE) $(INPUTDIR)/$(INPUTFILENAME).txt $(OUTPUTDIR)/$(OUTPUTFILENAME) -d $(CHECKDIR)/$(CHECKFILENAME)
	$(EXEDIR)/$(EXE) $(INPUTDIR)/$(INPUTFILENAME).txt $(OUTPUTDIR)/$(OUTPUTFILENAME)

verify:
	../verifier/verify $(INPUTDIR)/$(INPUTFILENAME).txt $(OUTPUTDIR)/$(OUTPUTFILENAME)

test: run verify

clean:
	rm $(EXEDIR)/$(EXE)
