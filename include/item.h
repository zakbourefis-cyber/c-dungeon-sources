#ifndef ITEM_H
#define ITEM_H

/**
 * @file item.h
 * @brief Effets des objets, choisis par pointeur de fonction.
 *
 * Un objet est un Element dont le champ `type` est l'une des constantes
 * TYPE_*. Chaque type est associé, dans une table de item.c, à une fonction
 * de type FonctionEffet. Pour créer un nouveau type d'objet il suffit
 * d'écrire une fonction d'effet et d'ajouter une ligne à la table : la
 * fonction item_utiliser() n'a pas à être modifiée.
 */

#include <stddef.h>

#include "entity.h"

#define TYPE_SOIN "Soin"       /**< Rend `valeur` PV. */
#define TYPE_MANA "Mana"       /**< Rend `valeur` points de mana. */
#define TYPE_ARME "Arme"       /**< Augmente l'attaque de `valeur`. */
#define TYPE_ARMURE "Armure"   /**< Augmente la défense de `valeur`. */

/**
 * @brief Pointeur de fonction : effet d'un objet sur le joueur.
 *
 * @param joueur Joueur ciblé (non NULL).
 * @param valeur Puissance de l'objet.
 * @param message Tampon où écrire la description de l'effet.
 * @param taille Taille du tampon.
 * @return RES_OK si l'objet doit être consommé, RES_ERR_ACTION si l'effet
 *         est inutile (l'objet est alors conservé).
 */
typedef Resultat (*FonctionEffet)(Joueur *joueur, int valeur, char *message, size_t taille);

/**
 * @brief Utilise l'objet d'indice donné dans l'inventaire.
 *
 * Si l'effet réussit, l'objet est retiré (et libéré) de l'inventaire.
 *
 * @param joueur Joueur concerné.
 * @param indice Indice de l'objet à partir de 0.
 * @param message Tampon de description du résultat (peut être NULL si taille = 0).
 * @param taille Taille du tampon.
 * @return RES_OK, RES_ERR_NULL, RES_ERR_INDICE (pas d'objet),
 *         RES_ERR_ACTION (type inconnu ou effet inutile, objet conservé).
 */
Resultat item_utiliser(Joueur *joueur, size_t indice, char *message, size_t taille);

#endif
