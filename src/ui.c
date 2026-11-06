#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/entity.h"
#include "../include/ui.h"



#ifndef DISPLAY_H
#define DISPLAY_H

#include "entity.h"

/*
 * Initialise ncurses et prepare les panneaux.
 * Retourne 0 en cas de succes, -1 en cas d'erreur.
 */
int display_init(void);

/* Affiche le heros, l'environnement, le monstre et les commandes. */
void display_render(
    const Joueur *joueur,
    const char *environment_path,
    const char *monster_name,
    const char *monster_path,
    int monster_hp,
    int monster_max_hp,
    const char *message
);

/* Attend une touche et retourne sa version minuscule. */
int display_get_action(void);

/* Termine ncurses et restaure le terminal. */
void display_shutdown(void);

#endif
