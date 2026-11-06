#ifndef LISTE_ELEMENT_H
#define LISTE_ELEMENT_H

/**
 * @file liste_element.h
 * @brief Liste chaînée générique d'éléments (nom, type, valeur).
 *
 * Ce module est réutilisé pour l'inventaire du joueur et pour les objets
 * posés au sol dans chaque salle du donjon.
 *
 * Règle mémoire : la liste possède ses nœuds. Celui qui détient le pointeur
 * de tête (`Element *`) est responsable d'appeler libererListeElements().
 */

#include <stddef.h>

#include "erreurs.h"

/** @brief Taille maximale du nom d'un élément (terminateur inclus). */
#define TAILLE_NOM_ELEMENT 50
/** @brief Taille maximale du type d'un élément (terminateur inclus). */
#define TAILLE_TYPE_ELEMENT 50

/** @brief Élément d'une liste chaînée. */
typedef struct Element {
    char nom[TAILLE_NOM_ELEMENT];     /**< Nom de l'élément. */
    char type[TAILLE_TYPE_ELEMENT];   /**< Catégorie (ex. "Soin"). */
    int valeur;                       /**< Valeur associée (puissance, quantité...). */
    struct Element *suivant;          /**< Élément suivant ou NULL en fin de liste. */
} Element;

/**
 * @brief Ajoute un élément au début de la liste.
 *
 * @param liste Adresse du pointeur de tête.
 * @param nomElement Nom du nouvel élément (copié, tronqué si trop long).
 * @param nomType Type du nouvel élément (copié).
 * @param valeur Valeur associée.
 * @return RES_OK, RES_ERR_NULL (argument NULL) ou RES_ERR_MEMOIRE.
 *         En cas d'erreur la liste n'est pas modifiée.
 */
Resultat ajouterElement(Element **liste, const char *nomElement, const char *nomType, int valeur);

/**
 * @brief Ajoute un élément à la fin de la liste.
 *
 * @param liste Adresse du pointeur de tête.
 * @param nomElement Nom du nouvel élément (copié, tronqué si trop long).
 * @param nomType Type du nouvel élément (copié).
 * @param valeur Valeur associée.
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_MEMOIRE (liste inchangée).
 */
Resultat ajouterElementFin(Element **liste, const char *nomElement, const char *nomType, int valeur);

/**
 * @brief Supprime (et libère) le premier élément portant le nom indiqué.
 *
 * @param liste Adresse du pointeur de tête.
 * @param nomElement Nom recherché.
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_INTROUVABLE.
 */
Resultat supprimerElement(Element **liste, const char *nomElement);

/**
 * @brief Supprime (et libère) l'élément situé à un indice donné.
 *
 * @param liste Adresse du pointeur de tête.
 * @param indice Indice à partir de 0.
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_INDICE.
 */
Resultat supprimerElementIndice(Element **liste, size_t indice);

/**
 * @brief Déplace un nœud d'une liste vers la fin d'une autre, sans allocation.
 *
 * Le nœud est détaché de `source` puis rattaché à `destination` : aucune
 * copie, aucun malloc, donc aucun risque d'échec mémoire.
 *
 * @param source Adresse du pointeur de tête de la liste d'origine.
 * @param indice Indice du nœud à déplacer dans `source`.
 * @param destination Adresse du pointeur de tête de la liste d'arrivée.
 * @return RES_OK, RES_ERR_NULL ou RES_ERR_INDICE.
 */
Resultat transfererElementIndice(Element **source, size_t indice, Element **destination);

/**
 * @brief Compte les éléments de la liste.
 *
 * @param liste Premier élément (peut être NULL).
 * @return Nombre d'éléments.
 */
size_t compterElements(const Element *liste);

/**
 * @brief Affiche tous les éléments de la liste sur la sortie standard.
 *
 * @param liste Premier élément (peut être NULL).
 */
void afficherElements(const Element *liste);

/**
 * @brief Libère tous les éléments de la liste.
 *
 * @param liste Premier élément (peut être NULL). Après l'appel, l'appelant
 *        doit considérer son pointeur comme invalide.
 */
void libererListeElements(Element *liste);

/**
 * @brief Recherche le premier élément correspondant à un nom.
 *
 * @param liste Premier élément de la liste.
 * @param nomElement Nom recherché.
 * @return Élément trouvé (appartient à la liste, ne pas libérer) ou NULL.
 */
Element *rechercherElementNom(Element *liste, const char *nomElement);

/**
 * @brief Recherche un élément à partir de son indice.
 *
 * @param liste Premier élément de la liste.
 * @param indice Indice recherché, à partir de 0.
 * @return Élément trouvé (ne pas libérer) ou NULL si l'indice est invalide.
 */
Element *rechercherElementIndice(Element *liste, size_t indice);

/**
 * @brief Retourne un élément choisi aléatoirement (utilise rand()).
 *
 * @param liste Premier élément de la liste.
 * @return Élément choisi (ne pas libérer) ou NULL si la liste est vide.
 */
Element *tirageAleatoireElement(Element *liste);

#endif
