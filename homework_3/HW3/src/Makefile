# g++ -std=c++11 main.cpp LCN.cpp -o main
EXEDIR = ../bin
EXE = hw3

HEADERS = total.hpp
SOURCES = main.cpp input.cpp Floorplan.cpp

CFLAGS = -std=c++11 -O3 --optimize

INPUTDIR = ../testcase
# INPUTFILENAME = sample
# INPUTFILENAME = public1
# INPUTFILENAME = public2
# INPUTFILENAME = public3
INPUTFILENAME = public4

OUTPUTDIR = ../output
OUTPUTFILENAME = $(INPUTFILENAME).floorplan

CHECKDIR = ../check
CHECKFILENAME = $(INPUTFILENAME)

all: build

build: $(HEADERS) $(SOURCES)
	g++ $(CFLAGS) $(SOURCES) -o $(EXEDIR)/$(EXE)

run: 
#### if debug needed, add -d flag & CHECK argv ####
#	$(EXEDIR)/$(EXE) $(INPUTDIR)/$(INPUTFILENAME).txt $(OUTPUTDIR)/$(OUTPUTFILENAME) -d $(CHECKDIR)/$(CHECKFILENAME)
	$(EXEDIR)/$(EXE) $(INPUTDIR)/$(INPUTFILENAME).txt $(OUTPUTDIR)/$(OUTPUTFILENAME)

verify:
	../verifier/verify $(INPUTDIR)/$(INPUTFILENAME).txt $(OUTPUTDIR)/$(OUTPUTFILENAME)

test: run verify

allin: build run verify

clean:
	rm $(EXEDIR)/$(EXE)
	rm $(OUTPUTDIR)/*.floorplan
