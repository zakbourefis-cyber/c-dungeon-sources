#ifndef DUNGEON_H
#define DUNGEON_H

/**
 * @file dungeon.h
 * @brief Salles et donjon.
 *
 * Le donjon est un tableau dynamique de Salle (calloc au départ, realloc
 * quand il faut grandir). Les salles sont reliées par des INDICES et non par
 * des pointeurs : realloc peut déplacer le tableau en mémoire, ce qui
 * rendrait des pointeurs vers les salles invalides (pointeurs pendants).
 *
 * Propriété mémoire :
 *  - le Donjon possède son tableau de salles ;
 *  - chaque Salle possède son Ennemi (s'il y en a un) et sa liste d'objets.
 *  Tout est libéré par donjon_liberer().
 */

#include <stddef.h>

#include "enemy.h"
#include "erreurs.h"
#include "liste_element.h"

/** @brief Taille maximale du nom d'une salle (terminateur inclus). */
#define TAILLE_NOM_SALLE 50
/** @brief Taille maximale de la description d'une salle (terminateur inclus). */
#define TAILLE_DESC_SALLE 120
#define SALLE_AUCUNE (-1)   /**< Valeur de `voisins[d]` s'il n'y a pas de passage. */

/** @brief Les quatre directions possibles entre deux salles. */
typedef enum Direction {
    DIR_NORD,
    DIR_EST,
    DIR_SUD,
    DIR_OUEST,
    NB_DIRECTIONS
} Direction;

/** @brief Une salle du donjon. */
typedef struct Salle {
    char nom[TAILLE_NOM_SALLE];             /**< Nom affiché comme titre du panneau. */
    char description[TAILLE_DESC_SALLE];    /**< Texte affiché à la première visite. */
    const char *decor;              /**< Fichier ASCII art du décor (statique). */
    int voisins[NB_DIRECTIONS];     /**< Indices des salles voisines ou SALLE_AUCUNE. */
    Ennemi *ennemi;                 /**< Ennemi présent ou NULL. */
    Element *objets;                /**< Objets posés au sol (liste, peut être NULL). */
    int visitee;                    /**< 1 si le joueur y est déjà entré. */
    int finale;                     /**< 1 si vaincre son ennemi gagne la partie. */
} Salle;

/** @brief Le donjon : toutes les salles et la position du joueur. */
typedef struct Donjon {
    Salle *salles;       /**< Tableau dynamique de salles. */
    size_t nb_salles;    /**< Nombre de salles utilisées. */
    size_t capacite;     /**< Nombre de salles allouées. */
    size_t courante;     /**< Indice de la salle où se trouve le joueur. */
    size_t precedente;   /**< Indice de la salle d'où il vient (pour fuir). */
} Donjon;

/**
 * @brief Nom d'une direction.
 * @param direction Direction à nommer.
 * @return "nord", "est", "sud" ou "ouest" ; "?" si la direction est invalide.
 */
const char *direction_nom(Direction direction);

/**
 * @brief Direction opposée (nord <-> sud, est <-> ouest).
 * @param direction Direction de départ.
 * @return Direction opposée (la même si `direction` est invalide).
 */
Direction direction_opposee(Direction direction);

/**
 * @brief Prépare un donjon vide (alloue la capacité initiale avec calloc).
 *
 * @param donjon Donjon à initialiser.
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_MEMOIRE.
 */
Resultat donjon_init(Donjon *donjon);

/**
 * @brief Ajoute une salle (agrandit le tableau avec realloc si nécessaire).
 *
 * @param donjon Donjon initialisé.
 * @param nom Nom de la salle.
 * @param description Description affichée à l'arrivée.
 * @param decor Chemin du décor ASCII (chaîne statique, non copiée).
 * @param id Reçoit l'indice de la nouvelle salle (peut être NULL).
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_MEMOIRE (le donjon reste valide).
 */
Resultat donjon_ajouter_salle(Donjon *donjon, const char *nom, const char *description,
                              const char *decor, size_t *id);

/**
 * @brief Relie deux salles dans les deux sens (a --direction--> b et retour).
 *
 * @param donjon Donjon concerné.
 * @param a Indice de la salle de départ.
 * @param direction Direction de a vers b.
 * @param b Indice de la salle d'arrivée.
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_INDICE (salle ou direction invalide).
 */
Resultat donjon_relier(Donjon *donjon, size_t a, Direction direction, size_t b);

/**
 * @brief Crée un ennemi à partir d'un modèle et le place dans une salle.
 *
 * @param donjon Donjon concerné.
 * @param salle Indice de la salle.
 * @param modele Modèle d'ennemi (valeur de ModeleEnnemi).
 * @return RES_OK, RES_ERR_NULL, RES_ERR_INDICE, RES_ERR_MEMOIRE, ou
 *         RES_ERR_ACTION si la salle contient déjà un ennemi.
 */
Resultat donjon_placer_ennemi(Donjon *donjon, size_t salle, size_t modele);

/**
 * @brief Dépose un objet au sol dans une salle.
 *
 * @param donjon Donjon concerné.
 * @param salle Indice de la salle.
 * @param nom Nom de l'objet.
 * @param type Type de l'objet (voir item.h).
 * @param valeur Puissance de l'effet.
 * @return RES_OK, RES_ERR_NULL, RES_ERR_INDICE ou RES_ERR_MEMOIRE.
 */
Resultat donjon_deposer_objet(Donjon *donjon, size_t salle, const char *nom,
                              const char *type, int valeur);

/**
 * @brief Renvoie la salle d'indice donné.
 *
 * Le pointeur n'est valable que jusqu'au prochain donjon_ajouter_salle().
 *
 * @param donjon Donjon concerné.
 * @param id Indice de la salle.
 * @return Salle (appartient au donjon, ne pas libérer) ou NULL si invalide.
 */
Salle *donjon_salle(const Donjon *donjon, size_t id);

/**
 * @brief Déplace le joueur dans la direction demandée.
 *
 * Met à jour `precedente` et `courante`. Ne gère pas les ennemis : c'est le
 * rôle du moteur de jeu (jeu.h).
 *
 * @param donjon Donjon concerné.
 * @param direction Direction à emprunter.
 * @return RES_OK, RES_ERR_NULL, RES_ERR_INDICE (direction invalide) ou
 *         RES_ERR_INTROUVABLE (pas de passage dans cette direction).
 */
Resultat donjon_deplacer(Donjon *donjon, Direction direction);

/**
 * @brief Construit le donjon standard de 8 salles (grille 4x2) avec ses
 *        ennemis, ses objets et son boss final.
 *
 * @param donjon Donjon initialisé et vide (voir donjon_init()).
 * @return RES_OK ou l'erreur rencontrée (le donjon doit alors être libéré
 *         avec donjon_liberer()).
 */
Resultat donjon_construire_defaut(Donjon *donjon);

/**
 * @brief Libère le tableau, les ennemis et les objets de toutes les salles.
 *
 * Remet la structure à zéro : peut être appelée deux fois sans risque.
 *
 * @param donjon Donjon à libérer (peut être NULL).
 */
void donjon_liberer(Donjon *donjon);

#endif
