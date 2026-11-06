#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "liste_element.h"

static void verifier_ajout_en_tete(void)
{
    Element *liste = NULL;

    ajouterElement(&liste, "Potion", "Soin", 20);
    ajouterElement(&liste, "Gobelin", "Monstre", 12);

    assert(liste != NULL);
    assert(strcmp(liste->nom, "Gobelin") == 0);
    assert(strcmp(liste->type, "Monstre") == 0);
    assert(liste->valeur == 12);
    assert(liste->suivant != NULL);
    assert(strcmp(liste->suivant->nom, "Potion") == 0);
    assert(liste->suivant->valeur == 20);

    libererListeElements(liste);
}

static void verifier_ajout_en_fin(void)
{
    Element *liste = NULL;

    ajouterElementFin(&liste, "Potion", "Soin", 20);
    ajouterElementFin(&liste, "Piege", "Environnement", 18);

    assert(liste != NULL);
    assert(strcmp(liste->nom, "Potion") == 0);
    assert(liste->valeur == 20);
    assert(liste->suivant != NULL);
    assert(strcmp(liste->suivant->nom, "Piege") == 0);
    assert(liste->suivant->valeur == 18);
    assert(liste->suivant->suivant == NULL);

    libererListeElements(liste);
}

static void verifier_recherches(void)
{
    Element *liste = NULL;

    ajouterElementFin(&liste, "Potion", "Soin", 20);
    ajouterElementFin(&liste, "Gobelin", "Monstre", 12);

    assert(rechercherElementNom(liste, "Gobelin") == liste->suivant);
    assert(rechercherElementNom(liste, "Inconnu") == NULL);
    assert(rechercherElementNom(liste, NULL) == NULL);
    assert(rechercherElementIndice(liste, 0) == liste);
    assert(rechercherElementIndice(liste, 1) == liste->suivant);
    assert(rechercherElementIndice(liste, 2) == NULL);

    libererListeElements(liste);
}

static void verifier_suppression(void)
{
    Element *liste = NULL;

    ajouterElementFin(&liste, "Potion", "Soin", 20);
    ajouterElementFin(&liste, "Gobelin", "Monstre", 12);
    ajouterElementFin(&liste, "Piege", "Environnement", 18);

    supprimerElement(&liste, "Gobelin");
    assert(liste != NULL);
    assert(strcmp(liste->nom, "Potion") == 0);
    assert(liste->suivant != NULL);
    assert(strcmp(liste->suivant->nom, "Piege") == 0);

    supprimerElement(&liste, "Potion");
    assert(liste != NULL);
    assert(strcmp(liste->nom, "Piege") == 0);

    supprimerElement(&liste, "Piege");
    assert(liste == NULL);

    libererListeElements(liste);
}

static void verifier_tirage_aleatoire(void)
{
    Element *liste = NULL;
    Element *elementTire;

    ajouterElementFin(&liste, "Potion", "Soin", 20);
    elementTire = tirageAleatoireElement(liste);

    assert(elementTire == liste);
    assert(tirageAleatoireElement(NULL) == NULL);

    libererListeElements(liste);
}

static void verifier_codes_erreur(void)
{
    Element *liste = NULL;

    assert(ajouterElement(NULL, "A", "T", 1) == RES_ERR_NULL);
    assert(ajouterElement(&liste, NULL, "T", 1) == RES_ERR_NULL);
    assert(ajouterElement(&liste, "A", NULL, 1) == RES_ERR_NULL);
    assert(ajouterElementFin(NULL, "A", "T", 1) == RES_ERR_NULL);
    assert(liste == NULL);

    assert(ajouterElementFin(&liste, "A", "T", 1) == RES_OK);
    assert(supprimerElement(&liste, "Inconnu") == RES_ERR_INTROUVABLE);
    assert(supprimerElement(NULL, "A") == RES_ERR_NULL);
    assert(supprimerElement(&liste, NULL) == RES_ERR_NULL);
    assert(supprimerElement(&liste, "A") == RES_OK);
    assert(liste == NULL);
}

static void verifier_troncature_des_noms(void)
{
    Element *liste = NULL;
    char long_nom[200];

    memset(long_nom, 'x', sizeof(long_nom) - 1);
    long_nom[sizeof(long_nom) - 1] = '\0';

    assert(ajouterElement(&liste, long_nom, long_nom, 0) == RES_OK);
    assert(strlen(liste->nom) == TAILLE_NOM_ELEMENT - 1);
    assert(strlen(liste->type) == TAILLE_TYPE_ELEMENT - 1);

    libererListeElements(liste);
}

static void verifier_suppression_par_indice(void)
{
    Element *liste = NULL;

    ajouterElementFin(&liste, "A", "T", 1);
    ajouterElementFin(&liste, "B", "T", 2);
    ajouterElementFin(&liste, "C", "T", 3);

    assert(supprimerElementIndice(&liste, 3) == RES_ERR_INDICE);
    assert(supprimerElementIndice(NULL, 0) == RES_ERR_NULL);
    assert(supprimerElementIndice(&liste, 1) == RES_OK);
    assert(compterElements(liste) == 2);
    assert(strcmp(liste->suivant->nom, "C") == 0);
    assert(supprimerElementIndice(&liste, 0) == RES_OK);
    assert(strcmp(liste->nom, "C") == 0);
    assert(supprimerElementIndice(&liste, 0) == RES_OK);
    assert(liste == NULL);
    assert(supprimerElementIndice(&liste, 0) == RES_ERR_INDICE);
}

static void verifier_transfert(void)
{
    Element *source = NULL;
    Element *destination = NULL;
    Element *noeud;

    ajouterElementFin(&source, "A", "T", 1);
    ajouterElementFin(&source, "B", "T", 2);
    ajouterElementFin(&source, "C", "T", 3);
    noeud = rechercherElementIndice(source, 1);

    assert(transfererElementIndice(&source, 1, &destination) == RES_OK);
    assert(destination == noeud);                 /* même nœud : aucune copie */
    assert(compterElements(source) == 2);
    assert(compterElements(destination) == 1);

    assert(transfererElementIndice(&source, 0, &destination) == RES_OK);
    assert(strcmp(destination->nom, "B") == 0);
    assert(strcmp(destination->suivant->nom, "A") == 0);
    assert(destination->suivant->suivant == NULL);

    assert(transfererElementIndice(&source, 5, &destination) == RES_ERR_INDICE);
    assert(transfererElementIndice(NULL, 0, &destination) == RES_ERR_NULL);
    assert(transfererElementIndice(&source, 0, NULL) == RES_ERR_NULL);
    assert(transfererElementIndice(&source, 0, &source) == RES_ERR_NULL);
    assert(compterElements(source) == 1);

    libererListeElements(source);
    libererListeElements(destination);
}

static void verifier_compter(void)
{
    Element *liste = NULL;

    assert(compterElements(NULL) == 0);
    ajouterElement(&liste, "A", "T", 1);
    ajouterElement(&liste, "B", "T", 2);
    assert(compterElements(liste) == 2);
    libererListeElements(liste);
}

void test_liste_chainee(void)
{
    verifier_ajout_en_tete();
    verifier_ajout_en_fin();
    verifier_recherches();
    verifier_suppression();
    verifier_tirage_aleatoire();
    verifier_codes_erreur();
    verifier_troncature_des_noms();
    verifier_suppression_par_indice();
    verifier_transfert();
    verifier_compter();
    puts("[ok] liste chainee");
}
