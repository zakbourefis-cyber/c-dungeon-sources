# ---------------------------------------------------------------------------
# C-Dungeon
#   make                 construit les bibliothèques, les tests et le jeu
#   make test            compile et exécute les tests unitaires
#   make run             lance le jeu ncurses (RULES=simple|defense|critique)
#   make gtk             compile et lance l'interface GTK 3
#   make MODE=release    compile en mode release
#   make memcheck        tests sous Valgrind
#   make docs            génère la documentation Doxygen dans docs/html
#   make clean           supprime tout ce qui est généré
# ---------------------------------------------------------------------------

CC ?= cc
MODE ?= debug
RULES ?=

WARNINGS := -Wall -Wextra -Werror -std=c11 -Iinclude

ifeq ($(MODE),debug)
    CFLAGS := $(WARNINGS) -g -O0 -DDEBUG
else ifeq ($(MODE),release)
    CFLAGS := $(WARNINGS) -O2 -DNDEBUG
else
    $(error MODE doit valoir debug ou release)
endif

BUILD := build/$(MODE)

# --- Bibliothèques statiques -------------------------------------------------
LIB_LISTE := libListeElement.a
LIB_JEU   := libcdungeon.a

LISTE_SRC := src/liste_element.c
JEU_SRC   := src/debug.c src/erreurs.c src/entity.c src/item.c src/enemy.c src/combat.c \
             src/evenement.c src/dungeon.c src/jeu.c

LISTE_OBJ := $(LISTE_SRC:src/%.c=$(BUILD)/%.o)
JEU_OBJ   := $(JEU_SRC:src/%.c=$(BUILD)/%.o)

# --- Exécutables -------------------------------------------------------------
GAME      := c-dungeon
GTK_GAME  := c-dungeon-gtk
TEST_BIN  := run_tests

GAME_SRC  := src/game_main.c src/display.c
GAME_OBJ  := $(GAME_SRC:src/%.c=$(BUILD)/%.o)
GTK_SRC   := src/gtk_main.c
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0 2>/dev/null)
GTK_LIBS   := $(shell pkg-config --libs gtk+-3.0 2>/dev/null)

TEST_SRC  := src/main.c $(wildcard tests/test_*.c)
TEST_SRC  := $(filter-out tests/test_main.c,$(TEST_SRC))
TEST_OBJ  := $(patsubst src/%.c,$(BUILD)/%.o,$(filter src/%,$(TEST_SRC))) \
             $(patsubst tests/%.c,$(BUILD)/tests/%.o,$(filter tests/%,$(TEST_SRC)))

LIBS_LINK := -L. -lcdungeon -lListeElement
NCURSES   := -lncursesw

.PHONY: all lib test game run gtk memcheck docs clean help

all: lib $(TEST_BIN) $(GAME)

lib: $(LIB_LISTE) $(LIB_JEU)

$(LIB_LISTE): $(LISTE_OBJ)
	ar rcs $@ $^

$(LIB_JEU): $(JEU_OBJ)
	ar rcs $@ $^

$(TEST_BIN): $(TEST_OBJ) $(LIB_JEU) $(LIB_LISTE)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJ) $(LIBS_LINK)

$(GAME): $(GAME_OBJ) $(LIB_JEU) $(LIB_LISTE)
	$(CC) $(CFLAGS) -o $@ $(GAME_OBJ) $(LIBS_LINK) $(NCURSES)

# GTK est une interface séparée : le moteur et le jeu ncurses restent inchangés.
$(GTK_GAME): $(GTK_SRC) $(LIB_JEU) $(LIB_LISTE)
	@test -n "$(GTK_LIBS)" || (echo "GTK 3 absent : installez libgtk-3-dev et pkg-config."; exit 1)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -o $@ $(GTK_SRC) $(LIBS_LINK) $(GTK_LIBS)

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Les tests utilisent assert : on force -UNDEBUG même en mode release.
$(BUILD)/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -UNDEBUG -MMD -MP -c $< -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

game: $(GAME)

run: $(GAME)
	./$(GAME) $(RULES)

gtk: $(GTK_GAME)
	./$(GTK_GAME) $(RULES)

memcheck: $(TEST_BIN)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(TEST_BIN)

docs:
	doxygen Doxyfile

clean:
	rm -rf build $(GAME) $(GTK_GAME) $(TEST_BIN) $(LIB_LISTE) $(LIB_JEU) docs cdungeon-debug.log

help:
	@sed -n '3,11p' Makefile

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
