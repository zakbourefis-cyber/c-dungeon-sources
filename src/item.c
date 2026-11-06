#include <stdio.h>
#include <string.h>

#include "item.h"

static Resultat effet_soin(Joueur *joueur, int valeur, char *message, size_t taille)
{
    int rendus;

    if (joueur->stats.hp >= joueur->stats.max_hp) {
        snprintf(message, taille, "Vos PV sont deja au maximum.");
        return RES_ERR_ACTION;
    }
    rendus = joueur_soigner(joueur, valeur);
    snprintf(message, taille, "+%d PV.", rendus);
    return RES_OK;
}

static Resultat effet_mana(Joueur *joueur, int valeur, char *message, size_t taille)
{
    int avant = joueur->mana;

    if (joueur->mana >= joueur->max_mana) {
        snprintf(message, taille, "Votre mana est deja au maximum.");
        return RES_ERR_ACTION;
    }
    joueur->mana += valeur;
    if (joueur->mana > joueur->max_mana) {
        joueur->mana = joueur->max_mana;
    }
    snprintf(message, taille, "+%d mana.", joueur->mana - avant);
    return RES_OK;
}

static Resultat effet_arme(Joueur *joueur, int valeur, char *message, size_t taille)
{
    joueur->stats.attaque += valeur;
    snprintf(message, taille, "Attaque +%d.", valeur);
    return RES_OK;
}

static Resultat effet_armure(Joueur *joueur, int valeur, char *message, size_t taille)
{
    joueur->stats.defense += valeur;
    snprintf(message, taille, "Defense +%d.", valeur);
    return RES_OK;
}

/* Table type d'objet -> fonction d'effet. Ajouter une ligne = nouveau type. */
typedef struct EffetItem {
    const char *type;
    FonctionEffet effet;
} EffetItem;

static const EffetItem TABLE_EFFETS[] = {
    { TYPE_SOIN,   effet_soin   },
    { TYPE_MANA,   effet_mana   },
    { TYPE_ARME,   effet_arme   },
    { TYPE_ARMURE, effet_armure },
};

#define NB_EFFETS (sizeof(TABLE_EFFETS) / sizeof(TABLE_EFFETS[0]))

Resultat item_utiliser(Joueur *joueur, size_t indice, char *message, size_t taille)
{
    Element *objet;
    FonctionEffet effet = NULL;
    char detail[100];
    char nom[TAILLE_NOM_ELEMENT];
    Resultat res;
    size_t i;

    if (joueur == NULL) {
        return RES_ERR_NULL;
    }

    objet = rechercherElementIndice(joueur->inventaire, indice);
    if (objet == NULL) {
        return RES_ERR_INDICE;
    }

    for (i = 0; i < NB_EFFETS; i++) {
        if (strcmp(TABLE_EFFETS[i].type, objet->type) == 0) {
            effet = TABLE_EFFETS[i].effet;
            break;
        }
    }
    if (effet == NULL) {
        return RES_ERR_ACTION;
    }

    /* On garde le nom : l'élément sera libéré avant d'écrire le message. */
    strcpy(nom, objet->nom);
    detail[0] = '\0';
    res = effet(joueur, objet->valeur, detail, sizeof(detail));
    if (res != RES_OK) {
        if (message != NULL && taille > 0) {
            snprintf(message, taille, "%s", detail);
        }
        return res;
    }

    supprimerElementIndice(&joueur->inventaire, indice);
    if (message != NULL && taille > 0) {
        snprintf(message, taille, "Vous utilisez %s : %s", nom, detail);
    }
    return RES_OK;
}
