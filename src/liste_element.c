#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liste_element.h"

/* Crée un nœud isolé. Retourne NULL si un argument est NULL ou si malloc échoue. */
static Element *creer_element(const char *nomElement, const char *nomType, int valeur)
{
    Element *element;

    if (nomElement == NULL || nomType == NULL) {
        return NULL;
    }

    element = malloc(sizeof(*element));
    if (element == NULL) {
        return NULL;
    }

    strncpy(element->nom, nomElement, TAILLE_NOM_ELEMENT - 1);
    element->nom[TAILLE_NOM_ELEMENT - 1] = '\0';
    strncpy(element->type, nomType, TAILLE_TYPE_ELEMENT - 1);
    element->type[TAILLE_TYPE_ELEMENT - 1] = '\0';
    element->valeur = valeur;
    element->suivant = NULL;

    return element;
}

Resultat ajouterElement(Element **liste, const char *nomElement, const char *nomType, int valeur)
{
    Element *nouvelElement;

    if (liste == NULL || nomElement == NULL || nomType == NULL) {
        return RES_ERR_NULL;
    }

    nouvelElement = creer_element(nomElement, nomType, valeur);
    if (nouvelElement == NULL) {
        return RES_ERR_MEMOIRE;
    }

    nouvelElement->suivant = *liste;
    *liste = nouvelElement;
    return RES_OK;
}

Resultat ajouterElementFin(Element **liste, const char *nomElement, const char *nomType, int valeur)
{
    Element *nouvelElement;
    Element *dernierElement;

    if (liste == NULL || nomElement == NULL || nomType == NULL) {
        return RES_ERR_NULL;
    }

    nouvelElement = creer_element(nomElement, nomType, valeur);
    if (nouvelElement == NULL) {
        return RES_ERR_MEMOIRE;
    }

    if (*liste == NULL) {
        *liste = nouvelElement;
        return RES_OK;
    }

    dernierElement = *liste;
    while (dernierElement->suivant != NULL) {
        dernierElement = dernierElement->suivant;
    }
    dernierElement->suivant = nouvelElement;
    return RES_OK;
}

Resultat supprimerElement(Element **liste, const char *nomElement)
{
    Element *elementActuel;
    Element *elementPrecedent = NULL;

    if (liste == NULL || nomElement == NULL) {
        return RES_ERR_NULL;
    }

    elementActuel = *liste;
    while (elementActuel != NULL &&
           strcmp(elementActuel->nom, nomElement) != 0) {
        elementPrecedent = elementActuel;
        elementActuel = elementActuel->suivant;
    }

    if (elementActuel == NULL) {
        return RES_ERR_INTROUVABLE;
    }

    if (elementPrecedent == NULL) {
        *liste = elementActuel->suivant;
    } else {
        elementPrecedent->suivant = elementActuel->suivant;
    }

    free(elementActuel);
    return RES_OK;
}

/* Détache le nœud d'indice `indice` et le renvoie (NULL si indice invalide). */
static Element *detacher_element_indice(Element **liste, size_t indice)
{
    Element *precedent = NULL;
    Element *actuel = *liste;

    while (actuel != NULL && indice > 0) {
        precedent = actuel;
        actuel = actuel->suivant;
        indice--;
    }

    if (actuel == NULL) {
        return NULL;
    }

    if (precedent == NULL) {
        *liste = actuel->suivant;
    } else {
        precedent->suivant = actuel->suivant;
    }
    actuel->suivant = NULL;
    return actuel;
}

Resultat supprimerElementIndice(Element **liste, size_t indice)
{
    Element *detache;

    if (liste == NULL) {
        return RES_ERR_NULL;
    }

    detache = detacher_element_indice(liste, indice);
    if (detache == NULL) {
        return RES_ERR_INDICE;
    }

    free(detache);
    return RES_OK;
}

Resultat transfererElementIndice(Element **source, size_t indice, Element **destination)
{
    Element *detache;
    Element *dernier;

    if (source == NULL || destination == NULL || source == destination) {
        return RES_ERR_NULL;
    }

    detache = detacher_element_indice(source, indice);
    if (detache == NULL) {
        return RES_ERR_INDICE;
    }

    if (*destination == NULL) {
        *destination = detache;
        return RES_OK;
    }

    dernier = *destination;
    while (dernier->suivant != NULL) {
        dernier = dernier->suivant;
    }
    dernier->suivant = detache;
    return RES_OK;
}

size_t compterElements(const Element *liste)
{
    size_t nombre = 0;

    while (liste != NULL) {
        nombre++;
        liste = liste->suivant;
    }
    return nombre;
}

void afficherElements(const Element *liste)
{
    while (liste != NULL) {
        printf("Nom : %s | Type : %s | Valeur : %d\n",
               liste->nom, liste->type, liste->valeur);
        liste = liste->suivant;
    }
}

void libererListeElements(Element *liste)
{
    while (liste != NULL) {
        Element *suivant = liste->suivant;
        free(liste);
        liste = suivant;
    }
}

Element *rechercherElementNom(Element *liste, const char *nomElement)
{
    if (nomElement == NULL) {
        return NULL;
    }

    while (liste != NULL) {
        if (strcmp(liste->nom, nomElement) == 0) {
            return liste;
        }
        liste = liste->suivant;
    }

    return NULL;
}

Element *rechercherElementIndice(Element *liste, size_t indice)
{
    size_t indiceActuel = 0;

    while (liste != NULL && indiceActuel < indice) {
        liste = liste->suivant;
        indiceActuel++;
    }

    return liste;
}

Element *tirageAleatoireElement(Element *liste)
{
    size_t nombreElements = compterElements(liste);

    if (nombreElements == 0) {
        return NULL;
    }

    return rechercherElementIndice(liste, (size_t)rand() % nombreElements);
}
