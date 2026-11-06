#define _XOPEN_SOURCE_EXTENDED
#include <locale.h>
#include <wchar.h>
#include <ncursesw/curses.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "display.h"
#include "item.h"

#define MIN_COLS 80
#define MIN_ROWS 24
#define FOOTER_HEIGHT 5

static WINDOW *hero_win = NULL;
static WINDOW *environment_win = NULL;
static WINDOW *monster_win = NULL;
static WINDOW *footer_win = NULL;
static int layout_ready = 0;

static void destroy_windows(void)
{
    if (hero_win != NULL) {
        delwin(hero_win);
        hero_win = NULL;
    }
    if (environment_win != NULL) {
        delwin(environment_win);
        environment_win = NULL;
    }
    if (monster_win != NULL) {
        delwin(monster_win);
        monster_win = NULL;
    }
    if (footer_win != NULL) {
        delwin(footer_win);
        footer_win = NULL;
    }
    layout_ready = 0;
}

static int create_windows(void)
{
    int rows;
    int cols;
    int panel_height;
    int hero_width;
    int environment_width;
    int monster_width;

    destroy_windows();
    getmaxyx(stdscr, rows, cols);

    if (cols < MIN_COLS || rows < MIN_ROWS) {
        erase();
        mvaddstr(0, 0, "Terminal trop petit pour C-Dungeon.");
        mvaddstr(1, 0, "Il faut au minimum 80 colonnes et 24 lignes.");
        mvaddstr(2, 0, "Agrandissez le terminal puis relancez le jeu.");
        refresh();
        return -1;
    }

    panel_height = rows - FOOTER_HEIGHT;
    hero_width = cols / 3;
    environment_width = cols / 3;
    monster_width = cols - hero_width - environment_width;

    hero_win = newwin(panel_height, hero_width, 0, 0);
    environment_win = newwin(panel_height, environment_width, 0, hero_width);
    monster_win = newwin(panel_height, monster_width, 0,
                         hero_width + environment_width);
    footer_win = newwin(FOOTER_HEIGHT, cols, panel_height, 0);

    if (hero_win == NULL || environment_win == NULL ||
        monster_win == NULL || footer_win == NULL) {
        destroy_windows();
        return -1;
    }

    keypad(stdscr, TRUE);
    keypad(footer_win, TRUE);
    layout_ready = 1;
    return 0;
}

int display_init(void)
{
    setlocale(LC_ALL, "");

    if (initscr() == NULL) {
        return -1;
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_GREEN, -1);
        init_pair(2, COLOR_RED, -1);
        init_pair(3, COLOR_CYAN, -1);
        init_pair(4, COLOR_YELLOW, -1);
        init_pair(5, COLOR_WHITE, -1);
    }

    if (create_windows() != 0) {
        endwin();
        return -1;
    }
    return 0;
}

/* Affiche un fichier UTF-8 dans une fenêtre, sans dépasser ses limites. */
static void draw_ascii_file(
    WINDOW *win, const char *path, int start_y, int start_x,
    int max_lines, int max_width
)
{
    FILE *file;
    wchar_t line[512];
    int line_number = 0;

    if (path == NULL) {
        mvwaddstr(win, start_y, start_x, "(Aucun decor)");
        return;
    }

    file = fopen(path, "r");
    if (file == NULL) {
        mvwaddstr(win, start_y, start_x, "(Fichier introuvable)");
        return;
    }

    while (line_number < max_lines &&
           fgetws(line, (int)(sizeof(line) / sizeof(line[0])), file) != NULL) {
        int length = (int)wcslen(line);

        while (length > 0 &&
               (line[length - 1] == L'\n' || line[length - 1] == L'\r')) {
            line[--length] = L'\0';
        }

        if (length > max_width) {
            length = max_width;
        }
        if (length > 0) {
            mvwaddnwstr(win, start_y + line_number, start_x, line, length);
        }
        line_number++;
    }

    fclose(file);
}

static void draw_bar(
    WINDOW *win, int y, int x, int width,
    int value, int max_value, int color_pair
)
{
    int i;
    int filled;

    if (width <= 0) {
        return;
    }
    if (max_value <= 0) {
        value = 0;
        max_value = 1;
    }
    if (value < 0) {
        value = 0;
    }
    if (value > max_value) {
        value = max_value;
    }

    filled = value * width / max_value;
    wattron(win, COLOR_PAIR(color_pair));
    for (i = 0; i < width; i++) {
        mvwaddnwstr(win, y, x + i, i < filled ? L"█" : L"░", 1);
    }
    wattroff(win, COLOR_PAIR(color_pair));
}

/* Barre de valeur avec un libellé "NOM x/y" au-dessus. */
static void draw_stat(WINDOW *win, int y, const char *label, int value, int max_value,
                      int color_pair)
{
    int width = getmaxx(win) - 4;

    mvwprintw(win, y, 2, "%s %d/%d", label, value, max_value);
    draw_bar(win, y + 1, 2, width > 0 ? width : 1, value, max_value, color_pair);
}

static void draw_hero(const Jeu *jeu)
{
    const Joueur *joueur = &jeu->joueur;
    const Element *objet = joueur->inventaire;
    int numero = 1;
    int y = 11;

    werase(hero_win);
    box(hero_win, 0, 0);
    wattron(hero_win, A_BOLD);
    mvwaddstr(hero_win, 0, 2, " HEROS ");
    wattroff(hero_win, A_BOLD);

    mvwprintw(hero_win, 1, 2, "%.30s  Niv.%d", joueur->nom, joueur->niveau);
    draw_stat(hero_win, 2, "PV", joueur->stats.hp, joueur->stats.max_hp, 2);
    draw_stat(hero_win, 4, "MANA", joueur->mana, joueur->max_mana, 3);
    mvwprintw(hero_win, 6, 2, "ATQ %d   DEF %d", joueur->stats.attaque, joueur->stats.defense);
    mvwprintw(hero_win, 7, 2, "XP %d/%d   Or %d", joueur->experience,
              joueur_xp_requise(joueur), joueur->gold);
    mvwprintw(hero_win, 8, 2, "Score : %d", jeu->score);

    wattron(hero_win, A_BOLD);
    mvwaddstr(hero_win, 10, 2, "INVENTAIRE");
    wattroff(hero_win, A_BOLD);

    if (objet == NULL) {
        mvwaddstr(hero_win, y, 2, "(vide)");
    }
    while (objet != NULL && y < getmaxy(hero_win) - 1) {
        mvwprintw(hero_win, y, 2, "%d. %.*s", numero, getmaxx(hero_win) - 8, objet->nom);
        y++;
        numero++;
        objet = objet->suivant;
    }
    wrefresh(hero_win);
}

static void draw_environment(const Jeu *jeu)
{
    const Salle *salle = jeu_salle_courante(jeu);
    int height;
    int width;
    int dir;
    char sorties[64] = "";

    getmaxyx(environment_win, height, width);
    werase(environment_win);
    box(environment_win, 0, 0);
    wattron(environment_win, A_BOLD);
    mvwprintw(environment_win, 0, 2, " %.*s ", width - 6, salle->nom);
    wattroff(environment_win, A_BOLD);
    draw_ascii_file(environment_win, salle->decor, 2, 2, height - 7, width - 4);

    for (dir = 0; dir < NB_DIRECTIONS; dir++) {
        if (salle->voisins[dir] != SALLE_AUCUNE) {
            strncat(sorties, direction_nom((Direction)dir), sizeof(sorties) - strlen(sorties) - 2);
            strncat(sorties, " ", sizeof(sorties) - strlen(sorties) - 1);
        }
    }
    mvwprintw(environment_win, height - 3, 2, "Sorties : %.*s", width - 14, sorties);
    mvwprintw(environment_win, height - 2, 2, "Au sol : %d objet(s)",
              (int)compterElements(salle->objets));
    wrefresh(environment_win);
}

static void draw_monster(const Jeu *jeu)
{
    const Ennemi *ennemi = jeu_salle_courante(jeu)->ennemi;
    int height;
    int width;

    getmaxyx(monster_win, height, width);
    werase(monster_win);
    box(monster_win, 0, 0);
    wattron(monster_win, A_BOLD);
    mvwaddstr(monster_win, 0, 2, " MONSTRE ");
    wattroff(monster_win, A_BOLD);

    if (ennemi == NULL) {
        mvwaddstr(monster_win, 2, 2, "Aucun ennemi ici.");
    } else {
        mvwprintw(monster_win, 2, 2, "%.*s", width - 4, ennemi->nom);
        draw_stat(monster_win, 3, "PV", ennemi->stats.hp, ennemi->stats.max_hp, 2);
        draw_ascii_file(monster_win, ennemi->asset, 6, 2, height - 8, width - 4);
    }
    wrefresh(monster_win);
}

/* Écrit un texte sur plusieurs lignes en coupant aux espaces (ASCII uniquement). */
static void draw_wrapped(WINDOW *win, int y, int x, int width, int max_lines, const char *text)
{
    int line;
    size_t length = strlen(text);
    size_t start = 0;

    for (line = 0; line < max_lines && start < length; line++) {
        size_t count = length - start;

        if (count > (size_t)width) {
            count = (size_t)width;
            while (count > 0 && text[start + count] != ' ') {
                count--;
            }
            if (count == 0) {
                count = (size_t)width;      /* mot plus long que la ligne */
            }
        }
        mvwaddnstr(win, y + line, x, text + start, (int)count);
        start += count;
        while (start < length && text[start] == ' ') {
            start++;
        }
    }
}

static void draw_footer(const Jeu *jeu)
{
    int width = getmaxx(footer_win) - 4;
    char commandes[160];

    switch (jeu->etat) {
    case ETAT_COMBAT:
        snprintf(commandes, sizeof(commandes),
                 "[A] Attaquer  [S] Sort (%d mana)  [1-9] Objet  [F] Fuir  [Q] Quitter",
                 COUT_SORT);
        break;
    case ETAT_VICTOIRE:
    case ETAT_DEFAITE:
        snprintf(commandes, sizeof(commandes), "%s - Score : %d   [R] Rejouer  [Q] Quitter",
                 jeu->etat == ETAT_VICTOIRE ? "VICTOIRE" : "DEFAITE", jeu->score);
        break;
    case ETAT_EXPLORATION:
    default:
        snprintf(commandes, sizeof(commandes),
                 "[N/E/S/O ou fleches] Aller  [R] Ramasser  [1-9] Utiliser  [Q] Quitter");
        break;
    }

    werase(footer_win);
    box(footer_win, 0, 0);
    wattron(footer_win, A_BOLD);
    mvwaddnstr(footer_win, 1, 2, commandes, width);
    wattroff(footer_win, A_BOLD);
    draw_wrapped(footer_win, 2, 2, width, 2, jeu->message);
    wrefresh(footer_win);
}

/* Recrée les fenêtres après un redimensionnement du terminal. */
static void rebuild_after_resize(void)
{
    endwin();
    refresh();
    clear();
    create_windows();
}

void display_render(const Jeu *jeu)
{
    if (jeu == NULL || jeu_salle_courante(jeu) == NULL) {
        return;
    }

    if (is_term_resized(LINES, COLS)) {
        rebuild_after_resize();
    }
    if (!layout_ready) {
        return;
    }

    draw_hero(jeu);
    draw_environment(jeu);
    draw_monster(jeu);
    draw_footer(jeu);
}

int display_get_action(void)
{
    int key;

    if (!layout_ready) {
        /* Terminal trop petit : on attend un redimensionnement ou Q. */
        key = getch();
        if (key == KEY_RESIZE) {
            rebuild_after_resize();
        }
        return (key == 'q' || key == 'Q') ? 'q' : TOUCHE_AUCUNE;
    }

    key = wgetch(footer_win);
    switch (key) {
    case KEY_RESIZE:
        rebuild_after_resize();
        return TOUCHE_AUCUNE;
    case KEY_UP:    return TOUCHE_HAUT;
    case KEY_DOWN:  return TOUCHE_BAS;
    case KEY_LEFT:  return TOUCHE_GAUCHE;
    case KEY_RIGHT: return TOUCHE_DROITE;
    default:
        break;
    }
    if (key < 0 || key > 255) {
        return TOUCHE_AUCUNE;
    }
    return tolower(key);
}

void display_shutdown(void)
{
    destroy_windows();
    endwin();
}
