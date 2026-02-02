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

# Header Dependencies
# GBDK's lcc doesn't support automatic dependency generation (-MD -MP)
# Manual dependencies ensure header changes trigger recompilation
$(OBJDIR)/main.o: $(INCDIR)/game_types.h $(INCDIR)/title.h $(INCDIR)/opponent_select.h $(INCDIR)/difficulty_select.h $(INCDIR)/coinflip.h $(INCDIR)/game.h $(INCDIR)/endgame.h
$(OBJDIR)/title.o: $(INCDIR)/game_types.h $(INCDIR)/title.h $(INCDIR)/font.h $(INCDIR)/input.h $(INCDIR)/transition.h $(INCDIR)/random.h
$(OBJDIR)/opponent_select.o: $(INCDIR)/game_types.h $(INCDIR)/opponent_select.h $(INCDIR)/opponent_data.h $(INCDIR)/font.h $(INCDIR)/input.h $(INCDIR)/transition.h
$(OBJDIR)/difficulty_select.o: $(INCDIR)/game_types.h $(INCDIR)/difficulty_select.h $(INCDIR)/opponent_select.h $(INCDIR)/opponent_data.h $(INCDIR)/font.h $(INCDIR)/input.h $(INCDIR)/transition.h $(INCDIR)/portrait.h
$(OBJDIR)/coinflip.o: $(INCDIR)/game_types.h $(INCDIR)/coinflip.h $(INCDIR)/font.h $(INCDIR)/input.h $(INCDIR)/transition.h $(INCDIR)/random.h
$(OBJDIR)/transition.o: $(INCDIR)/game_types.h $(INCDIR)/transition.h
$(OBJDIR)/font.o: $(INCDIR)/font.h
$(OBJDIR)/input.o: $(INCDIR)/input.h
$(OBJDIR)/opponent_data.o: $(INCDIR)/opponent_data.h
$(OBJDIR)/random.o: $(INCDIR)/random.h
$(OBJDIR)/game.o: $(INCDIR)/game_types.h $(INCDIR)/game.h $(INCDIR)/coinflip.h $(INCDIR)/difficulty_select.h $(INCDIR)/opponent_data.h $(INCDIR)/font.h $(INCDIR)/input.h $(INCDIR)/board_state.h $(INCDIR)/vram_layout.h $(INCDIR)/random.h $(INCDIR)/portrait.h
$(OBJDIR)/board_state.o: $(INCDIR)/board_state.h $(INCDIR)/coinflip.h $(INCDIR)/vram_layout.h $(INCDIR)/game.h
$(OBJDIR)/endgame.o: $(INCDIR)/game_types.h $(INCDIR)/endgame.h $(INCDIR)/opponent_data.h $(INCDIR)/font.h $(INCDIR)/input.h $(INCDIR)/vram_layout.h $(INCDIR)/portrait.h
$(OBJDIR)/portrait.o: $(INCDIR)/portrait.h $(INCDIR)/opponent_data.h
