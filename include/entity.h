#ifndef ENTITY_H
#define ENTITY_H

/**
 * @file entity.h
 * @brief Statistiques communes et joueur (avec son inventaire).
 *
 * L'inventaire est une liste chaînée d'Element (voir liste_element.h) :
 * un objet du jeu est un Element dont `type` indique la catégorie
 * (voir item.h) et `valeur` la puissance de l'effet.
 */

#include "erreurs.h"
#include "liste_element.h"

/** @brief Taille maximale d'un nom d'entité (terminateur inclus). */
#define TAILLE_NOM_ENTITE 50
#define TAILLE_MAX_INVENTAIRE 9   /**< Un chiffre 1-9 par objet à l'écran. */

/** @brief Caractéristiques de combat partagées par le joueur et les ennemis. */
typedef struct Stats {
    int hp;        /**< Points de vie actuels (0 = mort). */
    int max_hp;    /**< Points de vie maximum. */
    int attaque;   /**< Puissance d'attaque. */
    int defense;   /**< Réduction des dégâts (selon la stratégie de calcul). */
} Stats;

/** @brief Le héros contrôlé par le joueur. */
typedef struct Joueur {
    char nom[TAILLE_NOM_ENTITE];  /**< Nom du héros. */
    Stats stats;                  /**< PV, attaque et défense. */
    int mana;                     /**< Mana actuel. */
    int max_mana;                 /**< Mana maximum. */
    int gold;                     /**< Or possédé. */
    int niveau;                   /**< Niveau actuel (1 au départ). */
    int experience;          /**< XP accumulée vers le niveau suivant. */
    Element *inventaire;     /**< Liste possédée par le joueur (NULL = vide). */
} Joueur;

/**
 * @brief Initialise un joueur de niveau 1 avec un inventaire vide.
 *
 * @param joueur Joueur à initialiser (ne doit pas contenir d'inventaire alloué).
 * @param nom Nom du héros (copié, tronqué si trop long).
 * @return RES_OK ou RES_ERR_NULL.
 */
Resultat initialiser_joueur(Joueur *joueur, const char *nom);

/**
 * @brief Ajoute un objet à la fin de l'inventaire.
 *
 * @param joueur Joueur concerné.
 * @param nom Nom de l'objet.
 * @param type Catégorie de l'objet (voir item.h).
 * @param valeur Puissance de l'effet.
 * @return RES_OK, RES_ERR_NULL, RES_ERR_MEMOIRE, ou RES_ERR_ACTION si
 *         l'inventaire contient déjà TAILLE_MAX_INVENTAIRE objets.
 */
Resultat ajouter_inventaire(Joueur *joueur, const char *nom, const char *type, int valeur);

/**
 * @brief Affiche l'inventaire sur la sortie standard (débogage/tests).
 * @param joueur Joueur concerné (NULL accepté, sans effet).
 */
void afficher_inventaire(const Joueur *joueur);

/**
 * @brief Libère l'inventaire et remet le pointeur à NULL.
 *
 * Peut être appelée plusieurs fois sans risque (pas de double libération).
 *
 * @param joueur Joueur concerné (NULL accepté, sans effet).
 */
void liberer_inventaire(Joueur *joueur);

/**
 * @brief Indique si le joueur est en vie.
 * @param joueur Joueur testé (NULL accepté).
 * @return 1 s'il a des PV, 0 sinon ou si `joueur` vaut NULL.
 */
int joueur_est_vivant(const Joueur *joueur);

/**
 * @brief Soigne le joueur sans dépasser max_hp.
 *
 * @param joueur Joueur à soigner.
 * @param points PV à rendre (> 0).
 * @return Nombre de PV réellement rendus (0 si joueur NULL ou déjà au maximum).
 */
int joueur_soigner(Joueur *joueur, int points);

/**
 * @brief XP nécessaire pour passer du niveau actuel au suivant.
 * @param joueur Joueur concerné.
 * @return XP requise (0 si `joueur` vaut NULL).
 */
int joueur_xp_requise(const Joueur *joueur);

/**
 * @brief Donne de l'expérience et gère les montées de niveau.
 *
 * À chaque niveau : +10 PV max (et +10 PV), +2 attaque, +1 défense,
 * +5 mana max (et +5 mana).
 *
 * @param joueur Joueur qui gagne de l'expérience.
 * @param xp Expérience gagnée.
 * @return Nombre de niveaux gagnés (0 si joueur NULL ou xp <= 0).
 */
int joueur_gagner_experience(Joueur *joueur, int xp);

#endif
