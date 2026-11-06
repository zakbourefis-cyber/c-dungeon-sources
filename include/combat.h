#ifndef COMBAT_H
#define COMBAT_H

/**
 * @file combat.h
 * @brief Combat et règles de calcul interchangeables.
 *
 * Les règles du jeu sont des pointeurs de fonctions regroupés dans une
 * structure Regles :
 *  - FonctionDegats : comment calculer les dégâts d'une attaque ;
 *  - FonctionScore  : comment calculer le score final.
 * Le code de combat ne connaît que ces pointeurs : choisir une autre Regles
 * change le jeu sans modifier une seule ligne de la logique de combat.
 */

#include <stddef.h>

#include "enemy.h"
#include "entity.h"

#define COUT_SORT 10          /**< Mana consommé par un sort. */
#define CHANCE_FUITE 50       /**< Probabilité de fuite réussie (en %). */
#define CHANCE_CRITIQUE 20    /**< Probabilité de coup critique (en %). */

/**
 * @brief Pointeur de fonction : calcule les dégâts d'une attaque.
 *
 * @param attaquant Stats de l'attaquant (lecture seule).
 * @param defenseur Stats du défenseur (lecture seule).
 * @return Dégâts infligés, toujours >= 1.
 */
typedef int (*FonctionDegats)(const Stats *attaquant, const Stats *defenseur);

/**
 * @brief Pointeur de fonction : calcule le score de la partie.
 *
 * @param joueur Joueur (lecture seule).
 * @param tours Nombre de tours de combat joués.
 * @param ennemis_vaincus Nombre d'ennemis vaincus.
 * @return Score (>= 0).
 */
typedef int (*FonctionScore)(const Joueur *joueur, int tours, int ennemis_vaincus);

/** @brief Jeu de règles : un nom et les fonctions de calcul associées. */
typedef struct Regles {
    const char *nom;         /**< Nom donné en ligne de commande. */
    FonctionDegats degats;   /**< Stratégie de calcul des dégâts. */
    FonctionScore score;     /**< Stratégie de calcul du score. */
} Regles;

/**
 * @brief Dégâts = attaque de l'attaquant (la défense est ignorée).
 * @param attaquant Stats de l'attaquant.
 * @param defenseur Stats du défenseur (non utilisées).
 * @return Dégâts infligés (>= 1).
 */
int degats_simples(const Stats *attaquant, const Stats *defenseur);

/**
 * @brief Dégâts = attaque - défense (minimum 1).
 * @param attaquant Stats de l'attaquant.
 * @param defenseur Stats du défenseur.
 * @return Dégâts infligés (>= 1).
 */
int degats_avec_defense(const Stats *attaquant, const Stats *defenseur);

/**
 * @brief Comme degats_avec_defense, doublés avec CHANCE_CRITIQUE % (rand()).
 * @param attaquant Stats de l'attaquant.
 * @param defenseur Stats du défenseur.
 * @return Dégâts infligés (>= 1), doublés en cas de coup critique.
 */
int degats_critique(const Stats *attaquant, const Stats *defenseur);

/**
 * @brief Score = 100 par ennemi vaincu + or possédé.
 * @param joueur Joueur (son or est utilisé).
 * @param tours Nombre de tours (non utilisé).
 * @param ennemis_vaincus Nombre d'ennemis vaincus.
 * @return Score.
 */
int score_simple(const Joueur *joueur, int tours, int ennemis_vaincus);

/**
 * @brief Score simple + bonus de rapidité (100 - tours) par ennemi vaincu.
 * @param joueur Joueur (son or est utilisé).
 * @param tours Nombre de tours de combat joués.
 * @param ennemis_vaincus Nombre d'ennemis vaincus.
 * @return Score.
 */
int score_rapidite(const Joueur *joueur, int tours, int ennemis_vaincus);

/**
 * @brief Nombre de jeux de règles disponibles.
 * @return Nombre de règles.
 */
size_t regles_nombre(void);

/**
 * @brief Jeu de règles d'indice donné.
 * @param indice Indice à partir de 0.
 * @return Règles trouvées (données statiques, ne pas libérer) ou NULL.
 */
const Regles *regles_indice(size_t indice);

/**
 * @brief Jeu de règles portant ce nom.
 * @param nom Nom recherché ("simple", "defense", "critique"...).
 * @return Règles trouvées (ne pas libérer) ou NULL si le nom est inconnu.
 */
const Regles *regles_trouver(const char *nom);

/**
 * @brief Jeu de règles utilisé par défaut.
 * @return Règles par défaut (ne pas libérer), jamais NULL.
 */
const Regles *regles_defaut(void);

/**
 * @brief Attaque normale du joueur contre un ennemi.
 *
 * @param joueur Attaquant.
 * @param ennemi Cible (ses PV sont réduits, jamais sous 0).
 * @param regles Règles de calcul des dégâts.
 * @param message Reçoit la description du coup.
 * @param taille Taille de `message`.
 * @return RES_OK ou RES_ERR_NULL.
 */
Resultat combat_attaque_joueur(Joueur *joueur, Ennemi *ennemi, const Regles *regles,
                               char *message, size_t taille);

/**
 * @brief Sort du joueur : attaque doublée pour le calcul, coûte COUT_SORT mana.
 *
 * @param joueur Lanceur du sort (son mana diminue de COUT_SORT).
 * @param ennemi Cible.
 * @param regles Règles de calcul des dégâts.
 * @param message Reçoit la description du résultat.
 * @param taille Taille de `message`.
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_ACTION (mana insuffisant, rien n'est
 *         consommé).
 */
Resultat combat_sort_joueur(Joueur *joueur, Ennemi *ennemi, const Regles *regles,
                            char *message, size_t taille);

/**
 * @brief Tour de l'ennemi : son pointeur `comportement` choisit l'action.
 *
 * @param joueur Cible des attaques éventuelles.
 * @param ennemi Ennemi qui joue.
 * @param regles Règles de calcul des dégâts.
 * @param message Reçoit la description de l'action.
 * @param taille Taille de `message`.
 * @return RES_OK ou RES_ERR_NULL.
 */
Resultat combat_tour_ennemi(Joueur *joueur, Ennemi *ennemi, const Regles *regles,
                            char *message, size_t taille);

#endif
