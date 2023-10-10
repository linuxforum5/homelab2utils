# Simple makefile for utils
CC=gcc
WCC=i686-w64-mingw32-gcc
SRC=src
WBIN=win32
BIN=bin
INSTALL_DIR=~/.local/bin

all: h2bas2htp htpinfo htp2h2wav h2htp2htp1 h2CreateHtp htp2bas

h2bas2htp: $(SRC)/h2bas2htp.c
	$(CC) -o $(BIN)/h2bas2htp $(SRC)/h2bas2htp.c
	$(WCC) -o $(WBIN)/h2bas2htp $(SRC)/h2bas2htp.c

h2CreateHtp: $(SRC)/h2CreateHtp.c
	$(CC) -o $(BIN)/h2CreateHtp $(SRC)/h2CreateHtp.c $(SRC)/lib/params.c $(SRC)/lib/htp.c $(SRC)/lib/fs.c $(SRC)/lib/basic.c $(SRC)/lib/labels.c
	$(WCC) -o $(WBIN)/h2CreateHtp $(SRC)/h2CreateHtp.c $(SRC)/lib/params.c $(SRC)/lib/htp.c $(SRC)/lib/fs.c $(SRC)/lib/basic.c $(SRC)/lib/labels.c

htp2h2wav: $(SRC)/htp2h2wav.c
	$(CC) -o $(BIN)/htp2h2wav $(SRC)/htp2h2wav.c
	$(WCC) -o $(WBIN)/htp2h2wav $(SRC)/htp2h2wav.c

h2htp2htp1: $(SRC)/h2htp2htp1.c
	$(CC) -o $(BIN)/h2htp2htp1 $(SRC)/h2htp2htp1.c $(SRC)/lib/htp.c
	$(WCC) -o $(WBIN)/h2htp2htp1 $(SRC)/h2htp2htp1.c $(SRC)/lib/htp.c

htpinfo: $(SRC)/htpinfo.c
	$(CC) -o $(BIN)/htpinfo $(SRC)/htpinfo.c
	$(WCC) -o $(WBIN)/htpinfo $(SRC)/htpinfo.c

htp2bas: $(SRC)/htp2bas.c
	$(CC) -o $(BIN)/htp2bas $(SRC)/htp2bas.c
	$(WCC) -o $(WBIN)/htp2bas $(SRC)/htp2bas.c

clean:
	rm -f $(WBIN)/* $(BIN)/* *~ $(SRC)/*~ 

install:
	cp $(BIN)/* $(INSTALL_DIR)/
