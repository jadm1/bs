OS = linux
#OS = windows
#OS = apple


ifeq ($(OS),linux)
  LDPTHREAD = 
  LBM = -lm
  LBSOCKETS = 
  LBPTHREAD = -lpthread
else ifeq ($(OS),windows)
  LDPTHREAD = 
  LBM = 
  LBSOCKETS = -lws2_32
  #sometimes -lpthreadGC2 or -lpthread
  LBPTHREAD = 
else ifeq ($(OS),apple) # TODO
  LDPTHREAD = 
  LBM = -lm
  LBSOCKETS = 
  LBPTHREAD = -lpthread
else
  #ID specific include dirs
  #LD specific library dirs
  LDPTHREAD = 
  #LB specific libraries
  LBM = 
  LBSOCKETS = 
  LBPTHREAD = 
endif


CC = gcc

BDIR = bin
SDIR = src

CCFLAGS_DBG = -ggdb -Wall -O0
CCFLAGS_OPT = -O2 -funroll-loops
CCFLAGS_STD = 

CLFLAGS_STD = $(LBM)
CLFLAGS_NW = $(LBM) $(LBSOCKETS)
CLFLAGS_MT = $(LBM) $(LBPTHREAD) $(LDPTHREAD)
CLFLAGS_MTNW = $(LBM) $(LBPTHREAD) $(LBSOCKETS) $(LDPTHREAD)


# specify targets

# main target
BS_ODIR = $(SDIR)/bs
BS_OBJ = $(BS_ODIR)/bs.o
BS_INC = -I inc
BS_CCFLAGS = $(CCFLAGS_DBG)


# targets of example programs
SRVCLI_ODIR = $(SDIR)/serverclient
SRVCLI_INC = $(BS_INC)
SRVCLI_OBJ = $(SRVCLI_ODIR)/serverclient.o
SRVCLI_CCFLAGS = $(CCFLAGS_STD) -Wno-address
SRVCLI_BIN = $(BDIR)/bs-test
SRVCLI_BIN_OBJ = $(BS_OBJ) $(SRVCLI_OBJ)
SRVCLI_CLFLAGS = $(CLFLAGS_NW)

RECVWALL_ODIR = $(SDIR)/recvw-all
RECVWALL_INC = $(BS_INC)
RECVWALL_OBJ = $(RECVWALL_ODIR)/recvwall.o
RECVWALL_CCFLAGS = $(CCFLAGS_DBG)
RECVWALL_BIN = $(BDIR)/recvw-all
RECVWALL_BIN_OBJ = $(BS_OBJ) $(RECVWALL_OBJ)
RECVWALL_CLFLAGS = $(CLFLAGS_NW)

FILETSF_ODIR = $(SDIR)/file-tsf
FILETSF_INC = $(BS_INC)
FILETSF_OBJ = $(FILETSF_ODIR)/filetsf.o
FILETSF_CCFLAGS = $(CCFLAGS_DBG)
FILETSF_BIN = $(BDIR)/file-tsf
FILETSF_BIN_OBJ = $(BS_OBJ) $(FILETSF_OBJ)
FILETSF_CLFLAGS = $(CLFLAGS_NW)


# specify make rules

#all: bin # should be enough but obj->bin order not guaranteed
all: obj bin

bin: $(SRVCLI_BIN) $(RECVWALL_BIN) $(FILETSF_BIN)
obj: $(BS_OBJ) $(SRVCLI_OBJ) $(RECVWALL_OBJ) $(FILETSF_OBJ)

bs: $(BS_OBJ)

bs-test: $(SRVCLI_BIN)
recv-all: $(RECVWALL_BIN)
file-tsf: $(FILETSF_BIN)

# compiling objects
$(BS_ODIR)/%.o: $(BS_ODIR)/%.c
	$(CC) $(BS_CCFLAGS) -c $< -o $@ $(BS_INC)


$(SRVCLI_ODIR)/%.o: $(SRVCLI_ODIR)/%.c
	$(CC) $(SRVCLI_CCFLAGS) -c $< -o $@ $(SRVCLI_INC)

$(RECVWALL_ODIR)/%.o: $(RECVWALL_ODIR)/%.c
	$(CC) $(RECVWALL_CCFLAGS) -c $< -o $@ $(RECVWALL_INC)

$(FILETSF_ODIR)/%.o: $(FILETSF_ODIR)/%.c
	$(CC) $(FILETSF_CCFLAGS) -c $< -o $@ $(FILETSF_INC)


# linking programs
$(SRVCLI_BIN): $(SRVCLI_BIN_OBJ)
	$(CC) -o $(SRVCLI_BIN) $(SRVCLI_BIN_OBJ) $(SRVCLI_CLFLAGS)

$(RECVWALL_BIN): $(RECVWALL_BIN_OBJ)
	$(CC) -o $(RECVWALL_BIN) $(RECVWALL_BIN_OBJ) $(RECVWALL_CLFLAGS)

$(FILETSF_BIN): $(FILETSF_BIN_OBJ)
	$(CC) -o $(FILETSF_BIN) $(FILETSF_BIN_OBJ) $(FILETSF_CLFLAGS)

clean:
	rm $(SRVCLI_BIN) $(RECVWALL_BIN) $(FILETSF_BIN) -f
	rm $(BS_ODIR)/*.o -f
	rm $(RECVWALL_ODIR)/*.o -f
	rm $(FILETSF_ODIR)/*.o -f
	rm $(SDIR)/*.o -f

