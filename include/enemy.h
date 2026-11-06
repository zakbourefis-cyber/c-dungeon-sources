#ifndef ENEMY_H
#define ENEMY_H

/**
 * @file enemy.h
 * @brief Ennemis et comportements (pointeurs de fonctions).
 *
 * Chaque ennemi possède un pointeur de fonction `comportement` qui décide de
 * son action à chaque tour. Changer d'IA = changer ce pointeur.
 *
 * Règle mémoire : un Ennemi est alloué par ennemi_creer() et appartient à la
 * Salle qui le contient ; il est libéré par ennemi_liberer().
 */

#include <stddef.h>

#include "entity.h"

/** @brief Actions possibles d'un ennemi pendant son tour. */
typedef enum ActionEnnemi {
    ENNEMI_ATTAQUE,   /**< Frappe le joueur. */
    ENNEMI_SOIN,      /**< Se soigne (consomme un soin). */
    ENNEMI_HESITE     /**< Rate son tour. */
} ActionEnnemi;

struct Ennemi;

/**
 * @brief Pointeur de fonction : choisit l'action d'un ennemi.
 *
 * @param ennemi Ennemi qui joue (lecture seule).
 * @return Action choisie.
 */
typedef ActionEnnemi (*ComportementEnnemi)(const struct Ennemi *ennemi);

/** @brief Un monstre du donjon. */
typedef struct Ennemi {
    char nom[TAILLE_NOM_ENTITE];  /**< Nom affiché. */
    Stats stats;                  /**< Caractéristiques de combat. */
    int xp;                       /**< XP donnée au joueur à la défaite. */
    int gold;                     /**< Or donné à la défaite. */
    int soins_restants;           /**< Nombre de soins encore utilisables. */
    const char *asset;            /**< Fichier ASCII art (chaîne statique). */
    ComportementEnnemi comportement;  /**< Pointeur de fonction : son « intelligence ». */
} Ennemi;

/** @brief Identifiants des modèles d'ennemis disponibles. */
typedef enum ModeleEnnemi {
    MODELE_BONHOMME_NEIGE,
    MODELE_SQUELETTE,
    MODELE_VAMPIRE,
    MODELE_ALIEN,
    MODELE_EVIL,
    MODELE_DRAGON,
    NB_MODELES_ENNEMIS
} ModeleEnnemi;

/**
 * @brief Alloue un ennemi à partir d'un modèle.
 *
 * @param modele Indice du modèle (valeur de ModeleEnnemi).
 * @param sortie Reçoit le nouvel ennemi (l'appelant en devient propriétaire).
 * @return RES_OK, RES_ERR_NULL, RES_ERR_INDICE (modèle inconnu) ou
 *         RES_ERR_MEMOIRE. En cas d'erreur `*sortie` vaut NULL.
 */
Resultat ennemi_creer(size_t modele, Ennemi **sortie);

/**
 * @brief Libère un ennemi.
 * @param ennemi Ennemi à libérer (NULL accepté, sans effet).
 */
void ennemi_liberer(Ennemi *ennemi);

/**
 * @brief Indique si l'ennemi est encore en vie.
 * @param ennemi Ennemi testé (NULL accepté).
 * @return 1 s'il a des PV, 0 sinon ou si `ennemi` vaut NULL.
 */
int ennemi_est_vivant(const Ennemi *ennemi);

/**
 * @brief Comportement : attaque à chaque tour.
 * @param ennemi Ennemi qui joue (non utilisé).
 * @return Toujours ENNEMI_ATTAQUE.
 */
ActionEnnemi comportement_agressif(const struct Ennemi *ennemi);

/**
 * @brief Comportement : se soigne sous 30 % de PV s'il lui reste des soins.
 * @param ennemi Ennemi qui joue.
 * @return ENNEMI_SOIN ou ENNEMI_ATTAQUE.
 */
ActionEnnemi comportement_prudent(const struct Ennemi *ennemi);

/**
 * @brief Comportement : hésite (rate son tour) une fois sur quatre.
 * @param ennemi Ennemi qui joue (non utilisé).
 * @return ENNEMI_HESITE ou ENNEMI_ATTAQUE (utilise rand()).
 */
ActionEnnemi comportement_capricieux(const struct Ennemi *ennemi);

#endif
