#include <stdio.h>
#include <string.h>

#include "entity.h"

#define PV_DE_DEPART 100
#define MANA_DE_DEPART 50
#define ATTAQUE_DE_DEPART 10
#define DEFENSE_DE_DEPART 3
#define XP_PAR_NIVEAU 30

Resultat initialiser_joueur(Joueur *joueur, const char *nom)
{
    if (joueur == NULL || nom == NULL) {
        return RES_ERR_NULL;
    }

    memset(joueur, 0, sizeof(*joueur));
    strncpy(joueur->nom, nom, TAILLE_NOM_ENTITE - 1);
    joueur->nom[TAILLE_NOM_ENTITE - 1] = '\0';
    joueur->stats.hp = PV_DE_DEPART;
    joueur->stats.max_hp = PV_DE_DEPART;
    joueur->stats.attaque = ATTAQUE_DE_DEPART;
    joueur->stats.defense = DEFENSE_DE_DEPART;
    joueur->mana = MANA_DE_DEPART;
    joueur->max_mana = MANA_DE_DEPART;
    joueur->gold = 0;
    joueur->niveau = 1;
    joueur->experience = 0;
    joueur->inventaire = NULL;
    return RES_OK;
}

Resultat ajouter_inventaire(Joueur *joueur, const char *nom, const char *type, int valeur)
{
    if (joueur == NULL || nom == NULL || type == NULL) {
        return RES_ERR_NULL;
    }
    if (compterElements(joueur->inventaire) >= TAILLE_MAX_INVENTAIRE) {
        return RES_ERR_ACTION;
    }
    return ajouterElementFin(&joueur->inventaire, nom, type, valeur);
}

void afficher_inventaire(const Joueur *joueur)
{
    if (joueur == NULL) {
        return;
    }
    afficherElements(joueur->inventaire);
}

void liberer_inventaire(Joueur *joueur)
{
    if (joueur == NULL) {
        return;
    }
    libererListeElements(joueur->inventaire);
    joueur->inventaire = NULL;
}

int joueur_est_vivant(const Joueur *joueur)
{
    return joueur != NULL && joueur->stats.hp > 0;
}

int joueur_soigner(Joueur *joueur, int points)
{
    int avant;

    if (joueur == NULL || points <= 0) {
        return 0;
    }
    avant = joueur->stats.hp;
    joueur->stats.hp += points;
    if (joueur->stats.hp > joueur->stats.max_hp) {
        joueur->stats.hp = joueur->stats.max_hp;
    }
    return joueur->stats.hp - avant;
}

int joueur_xp_requise(const Joueur *joueur)
{
    if (joueur == NULL) {
        return 0;
    }
    return joueur->niveau * XP_PAR_NIVEAU;
}

int joueur_gagner_experience(Joueur *joueur, int xp)
{
    int niveaux = 0;

    if (joueur == NULL || xp <= 0) {
        return 0;
    }

    joueur->experience += xp;
    while (joueur->experience >= joueur_xp_requise(joueur)) {
        joueur->experience -= joueur_xp_requise(joueur);
        joueur->niveau++;
        joueur->stats.max_hp += 10;
        joueur->stats.hp += 10;
        joueur->stats.attaque += 2;
        joueur->stats.defense += 1;
        joueur->max_mana += 5;
        joueur->mana += 5;
        niveaux++;
    }
    return niveaux;
}
