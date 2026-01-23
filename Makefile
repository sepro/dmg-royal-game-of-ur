# Royal Game of Ur - Game Boy DMG Makefile
# GBDK-2020 build system

# GBDK paths
GBDK_HOME = /home/gbdev/gbdk
LCC = $(GBDK_HOME)/bin/lcc

# Project paths
SRCDIR = src
OBJDIR = build
INCDIR = include
ASSETDIR = assets/generated

# Compiler flags
CFLAGS = -Wa-l -Wl-m -Wl-j -I$(INCDIR)

# ROM name
TARGET = royal-ur.gb

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
ASSETS = $(wildcard $(ASSETDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(ASSETS:$(ASSETDIR)/%.c=$(OBJDIR)/%.o)

# Default target
all: $(TARGET)

# Link ROM
$(TARGET): $(OBJECTS)
	$(LCC) $(CFLAGS) -o $@ $^

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

# Compile asset files
$(OBJDIR)/%.o: $(ASSETDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

# Clean build artifacts
clean:
	rm -rf $(OBJDIR)/*.o $(OBJDIR)/*.lst $(OBJDIR)/*.map $(OBJDIR)/*.sym
	rm -f $(TARGET)

# Run in emulator (configure as needed)
run: $(TARGET)
	@echo "Launch $(TARGET) in your emulator"
	@echo "Recommended: BGB or SameBoy"

.PHONY: all clean run
