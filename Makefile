# Compiler and archiver
CC = gcc
AR = ar

# Directories
INCDIR = inc
SRCDIR = src
OBJDIR = obj
LIBDIR = lib
BUILD  = build
TESTDIR = test

# Files
LIB = $(LIBDIR)/libz_thpool.a
TEST_LIB = $(LIBDIR)/libtestlib.a
TEST_PROGRAM =  $(BUILD)/z_thpool_test

# Compiler and linker flags
CFLAGS = -std=c99 -I$(INCDIR) -D_DEFAULT_SOURCE
LDFLAGS = -L$(LIBDIR) -lz_thpool -ltestlib -lpthread

# List all source files
SRC = $(wildcard $(SRCDIR)/*.c)
TEST_SRC = $(wildcard $(TESTDIR)/*.c)

# Generate object files in obj directory
OBJ = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRC))
TEST_OBJ = $(patsubst $(TESTDIR)/%.c, $(OBJDIR)/%.o, $(TEST_SRC))

# Default target
all: $(LIB) $(TEST_LIB) $(TEST_PROGRAM)

# Rule to create the library
$(LIB): $(OBJ)
	@mkdir -p $(BUILD)
	@mkdir -p $(LIBDIR)
	$(AR) rcs $@ $^

# Rule to create the test library
$(TEST_LIB): $(TEST_OBJ)
	@mkdir -p $(LIBDIR)
	$(AR) rcs $@ $^

# Rule to compile source files into objects
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile test source files into objects
$(OBJDIR)/%.o: $(TESTDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to create the main program
$(TEST_PROGRAM): $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Clean up all generated files
clean:
	rm -rf $(OBJDIR) $(LIBDIR) $(TEST_PROGRAM)

.PHONY: all clean
