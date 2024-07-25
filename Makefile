# compiler and flags
CC      = gcc
DEBUG	= -ggdb
CSTD	=
WARN	= -Wall -Wextra
CDEFS	=
SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LIBS = $(shell sdl2-config --libs) -lSDL2_image
CFLAGS	= -I$(SRCDIR) $(DEBUG) $(WARN) $(CSTD) $(CDEFS) $(SDL2_CFLAGS) -O3

# directories
SRCDIR = src
BUILDDIR = build
BINDIR = $(BUILDDIR)/bin
OBJDIR = $(BUILDDIR)/obj

# source and object files
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
TARGET = $(BINDIR)/chess

# targets
all: $(TARGET)

# link the final executable
$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJS) -o $@ $(SDL2_LIBS)

# compile source files to object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(SRCDIR)/%.h
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# clean up build artifacts
clean:
	rm -rf $(BUILDDIR)

# Phony targets
.PHONY: all clean