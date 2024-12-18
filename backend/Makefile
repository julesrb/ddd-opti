NAME := ddd-backend

# Directories
SRCDIR := src
INCDIR := include
BINDIR := build
LIBDIR := lib

WSSERVERDIR := $(LIBDIR)/wsServer
WSSERVER := $(WSSERVERDIR)/libws.a

# Source files
SRCS = main Log
SRCS += DroneControl DroneState DronePath
SRCS += SerialHandler WsServer
BINS := $(SRCS:%=$(BINDIR)/%.o)

CSRCS = 
CBINS := $(CSRCS:%=$(BINDIR)/%.o)

MKDIR := mkdir -p
RM := rm -fr

CXX := g++
CC := gcc
CFLAGS := -Wall -Wextra
CXXFLAGS := -Wall -Wextra -std=c++14
CPPFLAGS := -I$(INCDIR) -I$(WSSERVERDIR)/include
LDFLAGS := -L$(WSSERVERDIR)
LDLIBS := -lws
static: CFLAGS += -static -static-libgcc
static: CXXFLAGS += -static -static-libgcc -static-libstdc++
debug: CFLAGS += -ggdb3 -Og
debug: CXXFLAGS += -ggdb3 -Og
debug: CPPFLAGS += -DDEBUG_MODE=1

COLOUR_END := \033[0m
COLOUR_GREEN := \033[0;32m
COLOUR_RED := \033[0;31m
COLOUR_MAG := \033[0;35m
COLOUR_MAGB := \033[1;35m
COLOUR_CYN := \033[0;36m
COLOUR_CYNB := \033[1;36m

.PHONY: all static debug clean fclean re help

all: $(NAME)	# Compile all targets
static: re	# Compile static executable
debug: all	# Compile for debugging
clean:	# Clean up
	$(RM) $(BINDIR)
fclean: clean	# Full clean up
	$(RM) $(NAME)
re: fclean all	# Re-compile all
help:	# Print help on Makefile
	@awk 'BEGIN { \
	FS = ":.*#"; printf "Usage:\n\t$(COLOUR_CYNB)make $(COLOUR_MAGB)<target> \
	$(COLOUR_END)\n\nTargets:\n"; } \
	/^[a-zA-Z_0-9-]+:.*?#/ { \
	printf "$(COLOUR_MAGB)%-16s$(COLOUR_CYN)%s$(COLOUR_END)\n", $$1, $$2 } ' \
	Makefile

$(NAME): $(WSSERVER) $(BINS) $(CBINS)
	$(CXX) $(CPPFLAGS) $(BINS) $(CBINS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BINDIR)/%.o: $(SRCDIR)/%.cpp | $(BINDIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(BINDIR)/%.o: $(SRCDIR)/%.c | $(BINDIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BINDIR):
	$(MKDIR) $(BINDIR)

$(WSSERVER):
	$(MAKE) -C $(WSSERVERDIR) libws.a
