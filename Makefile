# Royal Game of Ur - Game Boy DMG Makefile
# GBDK-2020 build system

# GBDK paths
GBDK_HOME = /home/gbdev/gbdk
LCC = $(GBDK_HOME)/bin/lcc

# Project paths
OBJDIR = build
INCDIR = include

# Compiler flags
CFLAGS = -Wa-l -Wl-m -Wl-j -I$(INCDIR)

# ROM name
TARGET = royal-ur.gb

# Source files (organized by subfolder)
SOURCES = src/main.c \
          $(wildcard src/screens/*.c) \
          $(wildcard src/link/*.c) \
          $(wildcard src/logic/*.c) \
          $(wildcard src/util/*.c)

# Asset files (organized by subfolder)
ASSETS = $(wildcard assets/generated/title/*.c) \
         $(wildcard assets/generated/ui/*.c) \
         $(wildcard assets/generated/portraits/*.c) \
         $(wildcard assets/generated/coins/*.c) \
         $(wildcard assets/generated/board/*.c) \
         $(wildcard assets/generated/pieces/*.c) \
         $(wildcard assets/generated/dice/*.c)

# All .o files go flat into build/ using notdir
OBJECTS = $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(SOURCES) $(ASSETS)))

# Default target
all: $(TARGET)

# Link ROM
$(TARGET): $(OBJECTS)
	$(LCC) $(CFLAGS) -o $@ $^

# Pattern rules - one per source subdirectory
$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: src/screens/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: src/link/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: src/logic/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: src/util/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: assets/generated/title/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: assets/generated/ui/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: assets/generated/portraits/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: assets/generated/coins/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: assets/generated/board/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: assets/generated/pieces/%.c
	@mkdir -p $(OBJDIR)
	$(LCC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: assets/generated/dice/%.c
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
$(OBJDIR)/main.o: $(INCDIR)/game_types.h $(INCDIR)/screens/title.h $(INCDIR)/screens/opponent_select.h $(INCDIR)/screens/difficulty_select.h $(INCDIR)/screens/coinflip.h $(INCDIR)/screens/game.h $(INCDIR)/screens/endgame.h $(INCDIR)/link/link_connect.h $(INCDIR)/link/link_profile.h $(INCDIR)/util/font.h
$(OBJDIR)/title.o: $(INCDIR)/game_types.h $(INCDIR)/screens/title.h $(INCDIR)/util/font.h $(INCDIR)/util/input.h $(INCDIR)/util/transition.h $(INCDIR)/util/random.h
$(OBJDIR)/opponent_select.o: $(INCDIR)/game_types.h $(INCDIR)/screens/opponent_select.h $(INCDIR)/util/opponent_data.h $(INCDIR)/util/font.h $(INCDIR)/util/input.h $(INCDIR)/util/transition.h
$(OBJDIR)/difficulty_select.o: $(INCDIR)/game_types.h $(INCDIR)/screens/difficulty_select.h $(INCDIR)/screens/opponent_select.h $(INCDIR)/util/opponent_data.h $(INCDIR)/util/font.h $(INCDIR)/util/input.h $(INCDIR)/util/transition.h $(INCDIR)/util/portrait.h $(INCDIR)/util/screen_utils.h
$(OBJDIR)/coinflip.o: $(INCDIR)/game_types.h $(INCDIR)/screens/coinflip.h $(INCDIR)/util/font.h $(INCDIR)/util/input.h $(INCDIR)/util/transition.h $(INCDIR)/util/random.h $(INCDIR)/util/screen_utils.h
$(OBJDIR)/transition.o: $(INCDIR)/game_types.h $(INCDIR)/util/transition.h
$(OBJDIR)/font.o: $(INCDIR)/util/font.h
$(OBJDIR)/input.o: $(INCDIR)/util/input.h
$(OBJDIR)/opponent_data.o: $(INCDIR)/util/opponent_data.h
$(OBJDIR)/random.o: $(INCDIR)/util/random.h
$(OBJDIR)/game.o: $(INCDIR)/game_types.h $(INCDIR)/screens/game.h $(INCDIR)/screens/coinflip.h $(INCDIR)/screens/difficulty_select.h $(INCDIR)/util/opponent_data.h $(INCDIR)/util/font.h $(INCDIR)/util/input.h $(INCDIR)/logic/board_state.h $(INCDIR)/vram_layout.h $(INCDIR)/util/random.h $(INCDIR)/util/portrait.h $(INCDIR)/logic/ai.h $(INCDIR)/link/link.h $(INCDIR)/link/link_profile.h
$(OBJDIR)/board_state.o: $(INCDIR)/logic/board_state.h $(INCDIR)/screens/coinflip.h $(INCDIR)/vram_layout.h $(INCDIR)/screens/game.h
$(OBJDIR)/ai.o: $(INCDIR)/logic/ai.h $(INCDIR)/logic/board_state.h $(INCDIR)/screens/game.h $(INCDIR)/screens/difficulty_select.h $(INCDIR)/screens/opponent_select.h $(INCDIR)/util/random.h
$(OBJDIR)/endgame.o: $(INCDIR)/game_types.h $(INCDIR)/screens/endgame.h $(INCDIR)/util/opponent_data.h $(INCDIR)/util/font.h $(INCDIR)/util/input.h $(INCDIR)/vram_layout.h $(INCDIR)/util/portrait.h $(INCDIR)/util/screen_utils.h $(INCDIR)/screens/game.h $(INCDIR)/link/link.h
$(OBJDIR)/portrait.o: $(INCDIR)/util/portrait.h $(INCDIR)/util/opponent_data.h
$(OBJDIR)/screen_utils.o: $(INCDIR)/util/screen_utils.h
$(OBJDIR)/link.o: $(INCDIR)/link/link.h
$(OBJDIR)/link_connect.o: $(INCDIR)/link/link_connect.h $(INCDIR)/link/link.h $(INCDIR)/game_types.h $(INCDIR)/util/font.h $(INCDIR)/util/input.h $(INCDIR)/util/transition.h $(INCDIR)/screens/coinflip.h $(INCDIR)/util/screen_utils.h
$(OBJDIR)/link_profile.o: $(INCDIR)/link/link_profile.h $(INCDIR)/game_types.h $(INCDIR)/link/link.h $(INCDIR)/util/opponent_data.h $(INCDIR)/util/font.h $(INCDIR)/util/input.h $(INCDIR)/util/transition.h $(INCDIR)/util/portrait.h $(INCDIR)/util/screen_utils.h $(INCDIR)/screens/coinflip.h $(INCDIR)/screens/game.h
