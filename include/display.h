#ifndef DISPLAY_H
#define DISPLAY_H

/**
 * @file display.h
 * @brief Interface ncursesw : affichage de l'état du jeu et lecture des touches.
 *
 * Ce module ne fait que LIRE la structure Jeu : aucune règle du jeu n'est
 * codée ici (voir jeu.h pour la logique).
 */

#include "jeu.h"

#define TOUCHE_AUCUNE 0      /**< Aucune action (redimensionnement, touche inconnue). */
#define TOUCHE_HAUT 1001     /**< Flèche haut. */
#define TOUCHE_BAS 1002      /**< Flèche bas. */
#define TOUCHE_GAUCHE 1003   /**< Flèche gauche. */
#define TOUCHE_DROITE 1004   /**< Flèche droite. */

/**
 * @brief Initialise ncurses et prépare les panneaux.
 *
 * @return 0 en cas de succès, -1 si ncurses ne démarre pas ou si le terminal
 *         est plus petit que 80 colonnes x 24 lignes (ncurses est alors
 *         refermé : l'appelant peut écrire sur stderr).
 */
int display_init(void);

/**
 * @brief Dessine héros, salle, monstre et commandes pour l'état courant.
 *
 * @param jeu Partie à afficher (lecture seule).
 */
void display_render(const Jeu *jeu);

/**
 * @brief Attend une touche.
 *
 * @return Une lettre en minuscule, un chiffre, une constante TOUCHE_* pour les
 *         flèches, ou TOUCHE_AUCUNE.
 */
int display_get_action(void);

/** @brief Termine ncurses et restaure le terminal. */
void display_shutdown(void);

#endif
