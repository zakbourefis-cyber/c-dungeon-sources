#ifndef JEU_H
#define JEU_H

/**
 * @file jeu.h
 * @brief Moteur de partie : état du jeu et actions du joueur.
 *
 * Ce module ne dépend PAS de ncurses : il peut être testé seul. L'interface
 * (display.h) lit la structure Jeu pour l'afficher, et game_main.c traduit
 * les touches en appels aux fonctions jeu_*.
 *
 * Chaque action renvoie un Resultat et écrit dans `jeu->message` une phrase
 * décrivant ce qui s'est passé (ou pourquoi l'action est impossible).
 */

#include "combat.h"
#include "dungeon.h"
#include "entity.h"
#include "erreurs.h"
#include "evenement.h"

/** @brief Taille du tampon du message affiché en bas de l'écran. */
#define TAILLE_MESSAGE 256

/** @brief État général de la partie. */
typedef enum EtatJeu {
    ETAT_EXPLORATION,   /**< Le joueur peut se déplacer et ramasser. */
    ETAT_COMBAT,        /**< Un ennemi est présent dans la salle courante. */
    ETAT_VICTOIRE,      /**< Le boss final est vaincu. */
    ETAT_DEFAITE        /**< Le joueur est mort. */
} EtatJeu;

/** @brief Tout l'état d'une partie. */
typedef struct Jeu {
    Joueur joueur;                 /**< Le héros. */
    Donjon donjon;                 /**< Les salles, les ennemis et les objets. */
    const Regles *regles;          /**< Règles de calcul en vigueur (non possédées). */
    EtatJeu etat;                  /**< Exploration, combat, victoire ou défaite. */
    int tours;                     /**< Tours de combat joués. */
    int ennemis_vaincus;           /**< Nombre d'ennemis tués. */
    int score;                     /**< Recalculé avec regles->score. */
    char message[TAILLE_MESSAGE];  /**< Dernier message à afficher. */
} Jeu;

/**
 * @brief Prépare une nouvelle partie (joueur, donjon standard, inventaire).
 *
 * @param jeu Partie à initialiser.
 * @param regles Règles à utiliser (si NULL, regles_defaut()).
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_MEMOIRE. En cas d'erreur, tout ce
 *         qui avait été alloué est déjà libéré.
 */
Resultat jeu_init(Jeu *jeu, const Regles *regles);

/**
 * @brief Libère toutes les ressources d'une partie (idempotent).
 * @param jeu Partie à libérer (NULL accepté, sans effet).
 */
void jeu_liberer(Jeu *jeu);

/**
 * @brief Salle où se trouve le joueur.
 * @param jeu Partie concernée.
 * @return Salle courante (ne pas libérer), NULL si `jeu` vaut NULL.
 */
Salle *jeu_salle_courante(const Jeu *jeu);

/**
 * @brief Ennemi à combattre.
 * @param jeu Partie concernée.
 * @return Ennemi courant (appartient à la salle, ne pas libérer) ou NULL si
 *         l'on n'est pas en combat.
 */
Ennemi *jeu_ennemi_courant(const Jeu *jeu);

/**
 * @brief Se déplacer dans une direction (exploration uniquement).
 *
 * À la première visite d'une salle, un événement aléatoire peut se produire.
 * Si la salle contient un ennemi, l'état passe en ETAT_COMBAT.
 *
 * @param jeu Partie en cours.
 * @param direction Direction à emprunter.
 * @return RES_OK, RES_ERR_NULL, RES_ERR_INTROUVABLE (mur) ou RES_ERR_ACTION
 *         (combat en cours ou partie terminée).
 */
Resultat jeu_deplacer(Jeu *jeu, Direction direction);

/**
 * @brief Ramasse tous les objets de la salle (exploration uniquement).
 *
 * Les objets qui ne tiennent pas dans l'inventaire restent au sol.
 *
 * @param jeu Partie en cours.
 * @return RES_OK, RES_ERR_NULL, RES_ERR_INTROUVABLE (rien au sol) ou
 *         RES_ERR_ACTION (combat, partie finie ou inventaire plein).
 */
Resultat jeu_ramasser(Jeu *jeu);

/**
 * @brief Utilise l'objet d'indice donné. En combat, cela coûte le tour.
 *
 * @param jeu Partie en cours.
 * @param indice Indice de l'objet dans l'inventaire, à partir de 0.
 * @return RES_OK, RES_ERR_NULL, RES_ERR_INDICE (objet inexistant) ou
 *         RES_ERR_ACTION (effet inutile, partie finie).
 */
Resultat jeu_utiliser_objet(Jeu *jeu, size_t indice);

/**
 * @brief Attaque normale (combat uniquement), puis riposte de l'ennemi.
 * @param jeu Partie en cours.
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_ACTION (pas en combat, partie finie).
 */
Resultat jeu_attaquer(Jeu *jeu);

/**
 * @brief Sort puissant (combat uniquement, coûte du mana), puis riposte.
 * @param jeu Partie en cours.
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_ACTION (pas en combat, mana
 *         insuffisant, partie finie).
 */
Resultat jeu_lancer_sort(Jeu *jeu);

/**
 * @brief Tente de fuir vers la salle précédente (CHANCE_FUITE %).
 *
 * En cas d'échec l'ennemi riposte.
 *
 * @param jeu Partie en cours.
 * @return RES_OK (fuite tentée, réussie ou non), RES_ERR_NULL ou
 *         RES_ERR_ACTION (pas en combat).
 */
Resultat jeu_fuir(Jeu *jeu);

#endif
