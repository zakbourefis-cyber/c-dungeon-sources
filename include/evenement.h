#ifndef EVENEMENT_H
#define EVENEMENT_H

/**
 * @file evenement.h
 * @brief Événements aléatoires déclenchés à la première visite d'une salle.
 *
 * Chaque événement est une fonction de type FonctionEvenement rangée dans une
 * table. Ajouter un événement = écrire la fonction et ajouter une ligne.
 */

#include <stddef.h>

#include "entity.h"

#define PROBA_EVENEMENT 40   /**< Probabilité (en %) qu'un événement survienne. */

/**
 * @brief Pointeur de fonction : applique un événement au joueur.
 *
 * @param joueur Joueur concerné (non NULL).
 * @param message Reçoit la description de l'événement.
 * @param taille Taille de `message`.
 * @return RES_OK.
 */
typedef Resultat (*FonctionEvenement)(Joueur *joueur, char *message, size_t taille);

/**
 * @brief Nombre d'événements disponibles.
 * @return Taille de la table d'événements.
 */
size_t evenement_nombre(void);

/**
 * @brief Applique l'événement d'indice donné (déterministe, utile aux tests).
 *
 * @param indice Indice de l'événement (inférieur à evenement_nombre()).
 * @param joueur Joueur concerné.
 * @param message Reçoit la description de l'événement.
 * @param taille Taille de `message`.
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_INDICE.
 */
Resultat evenement_appliquer(size_t indice, Joueur *joueur, char *message, size_t taille);

/**
 * @brief Tire au sort (PROBA_EVENEMENT %) puis applique un événement aléatoire.
 *
 * @param joueur Joueur concerné.
 * @param message Reçoit la description de l'événement s'il a lieu.
 * @param taille Taille de `message`.
 * @return 1 si un événement a eu lieu (message rempli), 0 sinon.
 */
int evenement_declencher(Joueur *joueur, char *message, size_t taille);

#endif
