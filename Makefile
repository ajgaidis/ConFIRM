ROOT = /home/ajg/install
CXX  = g++

SRCDIR=src
BINDIR=bin
LIBDIR=lib

#code_coop_linux
TESTS = callback_linux \
	convention \
	cppeh \
	data_symbl \
	fptr \
	jit \
	load_time_dynlnk_linux \
	mem \
	multithreading_linux64 \
	ret \
	run_time_dynlnk \
	signal \
	switch \
	tail_call \
	unmatched_pair \
	vtbl_call
BINS = $(addprefix $(BINDIR)/, $(TESTS))

INC = inc
INC_LIB = $(addprefix $(LIBDIR)/lib, $(addsuffix .so, $(INC)))
SETUP = setup
SETUP_LIB = $(addprefix $(LIBDIR)/lib, $(addsuffix .so, $(SETUP)))
LIBS = $(INC_LIB) $(SETUP_LIB)

DBG = -g -Wall -Werror
LIB_CFLAGS  = $(DBG) -fPIC
LIB_LDFLAGS = -shared
BIN_CFLAGS  = $(DBG) -fPIE
BIN_LDFLAGS = -pie -Wl,--as-needed,-rpath,\$$ORIGIN/../lib \
	      -lpthread -ldl -L$(LIBDIR) -l$(INC) -l$(SETUP)

.PHONY: all clean run
.SILENT: run

all: $(LIBS) $(BINS)

# run all test binaries
run: $(BINS)
	$(foreach test, $(BINS), printf "\ntest: $(test)\n"; ./$(test);)

# build support libraries
$(SETUP_LIB): $(SRCDIR)/$(SETUP).cpp
	$(CXX) $(LIB_CFLAGS) $^ -o $@ $(LIB_LDFLAGS)

$(INC_LIB): $(SETUP_LIB) $(SRCDIR)/$(INC).cpp
	$(CXX) $(LIB_CFLAGS) $^ -o $@ $(LIB_LDFLAGS) -L$(BINDIR)

# build test binaries
$(BINDIR)/code_coop_linux: $(LIBS) $(SRCDIR)/code_coop_linux.cpp
	$(CXX) $(BIN_CFLAGS)  $^ -o $@ $(BIN_LDFLAGS) \
		`pkg-config gtkmm-3.0 --cflags --libs`

$(BINDIR)/%: $(LIBS) $(SRCDIR)/%.cpp
	$(CXX) $(BIN_CFLAGS) $^ -o $@ $(BIN_LDFLAGS)

clean:
	rm -f $(LIBS) $(BINS) 
