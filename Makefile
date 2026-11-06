# ---------------------------------------------------------------------------
# C-Dungeon
#   make                 construit les bibliothèques, les tests et le jeu
#   make test            compile et exécute les tests unitaires
#   make run             lance le jeu   (RULES=simple|defense|critique)
#   make MODE=release    compile en mode release (-O2 -DNDEBUG, sans debug)
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
# libListeElement.a : liste chaînée générique, sans dépendance
# libcdungeon.a     : logique du jeu (joueur, ennemis, combat, donjon, moteur)
LIB_LISTE := libListeElement.a
LIB_JEU   := libcdungeon.a

LISTE_SRC := src/liste_element.c
JEU_SRC   := src/debug.c src/erreurs.c src/entity.c src/item.c src/enemy.c src/combat.c \
             src/evenement.c src/dungeon.c src/jeu.c

LISTE_OBJ := $(LISTE_SRC:src/%.c=$(BUILD)/%.o)
JEU_OBJ   := $(JEU_SRC:src/%.c=$(BUILD)/%.o)

# --- Exécutables ---------------------------------------------------------------
GAME      := c-dungeon
TEST_BIN  := run_tests

GAME_SRC  := src/game_main.c src/display.c
GAME_OBJ  := $(GAME_SRC:src/%.c=$(BUILD)/%.o)

TEST_SRC  := src/main.c $(wildcard tests/test_*.c)
TEST_SRC  := $(filter-out tests/test_main.c,$(TEST_SRC))
TEST_OBJ  := $(patsubst src/%.c,$(BUILD)/%.o,$(filter src/%,$(TEST_SRC))) \
             $(patsubst tests/%.c,$(BUILD)/tests/%.o,$(filter tests/%,$(TEST_SRC)))

LIBS_LINK := -L. -lcdungeon -lListeElement
NCURSES   := -lncursesw

.PHONY: all lib test game run memcheck docs clean help

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

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Les tests utilisent assert : on force -UNDEBUG pour qu'ils restent actifs
# même en mode release.
$(BUILD)/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -UNDEBUG -MMD -MP -c $< -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

game: $(GAME)
²
run: $(GAME)
	./$(GAME) $(RULES)

memcheck: $(TEST_BIN)
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(TEST_BIN)

docs:
	doxygen Doxyfile

clean:
	rm -rf build $(GAME) $(TEST_BIN) $(LIB_LISTE) $(LIB_JEU) docs cdungeon-debug.log

help:
	@sed -n '3,10p' Makefile

-include $(shell find $(BUILD) -name '*.d' 2>/dev/null)
