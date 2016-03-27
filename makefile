OS = linux
#OS = windows

GCCOPT = gcc -O2 -funroll-loops
GCCDEB = gcc -ggdb -Wall

#CCC = $(GCCOPT)
CCC = $(GCCDEB)

CFLAGS = 

ifeq ($(OS),linux)
  LIBS = -lm
else ifeq ($(OS),windows)
  LIBS = -lws2_32
else
  LIBS = 
endif


CCCLNFLAGS = $(LIBS)
CCCFLAGS = # -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-label

BDIR = bin
ODIR = obj
SDIR = src

BS_PROG = $(BDIR)/bs-test
BS_PROG_OBJ = $(ODIR)/bs.o $(ODIR)/serverclient.o

RECVWALL = $(BDIR)/recvw-all
RECVWALL_OBJ = $(ODIR)/bs.o $(ODIR)/recvwall.o

FILETSF = $(BDIR)/file-tsf
FILETSF_OBJ = $(ODIR)/bs.o $(ODIR)/filetsf.o

all: $(BS_PROG) $(RECVWALL) $(FILETSF)

$(ODIR)/%.o: $(SDIR)/%.c
	@echo compiling $*.c with $(CCC) $(CCCFLAGS)
	@$(CCC) $(CCCFLAGS) -c $< -o $@

$(BS_PROG): $(BS_PROG_OBJ)
	$(CCC) $(CCCFLAGS) -o $(BS_PROG) $(BS_PROG_OBJ) $(CCCLNFLAGS)

$(RECVWALL): $(RECVWALL_OBJ)
	$(CCC) $(CCCFLAGS) -o $(RECVWALL) $(RECVWALL_OBJ) $(CCCLNFLAGS)

$(FILETSF): $(FILETSF_OBJ)
	$(CCC) $(CCCFLAGS) -o $(FILETSF) $(FILETSF_OBJ) $(CCCLNFLAGS)


clean:
	rm $(BS_PROG) $(RECVWALL) $(FILETSF) -f
	rm $(ODIR)/* -f

