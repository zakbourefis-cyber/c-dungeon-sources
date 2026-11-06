#include <stdio.h>
#include <stdlib.h>

#include "evenement.h"

static Resultat evenement_tresor(Joueur *joueur, char *message, size_t taille)
{
    int or_trouve = 10 + rand() % 21;

    joueur->gold += or_trouve;
    snprintf(message, taille, "Vous trouvez un coffre : +%d or.", or_trouve);
    return RES_OK;
}

static Resultat evenement_piege(Joueur *joueur, char *message, size_t taille)
{
    int degats = 5 + rand() % 8;

    /* Un piège blesse mais ne tue jamais : il laisse au moins 1 PV. */
    if (degats >= joueur->stats.hp) {
        degats = joueur->stats.hp - 1;
    }
    joueur->stats.hp -= degats;
    snprintf(message, taille, "Un piege vous blesse : -%d PV.", degats);
    return RES_OK;
}

static Resultat evenement_fontaine(Joueur *joueur, char *message, size_t taille)
{
    int avant = joueur->mana;

    joueur->mana += 15;
    if (joueur->mana > joueur->max_mana) {
        joueur->mana = joueur->max_mana;
    }
    snprintf(message, taille, "Une fontaine magique vous rend %d mana.", joueur->mana - avant);
    return RES_OK;
}

static Resultat evenement_benediction(Joueur *joueur, char *message, size_t taille)
{
    int rendus = joueur_soigner(joueur, 15);

    if (rendus == 0) {
        snprintf(message, taille, "Une benediction vous entoure : vous etes deja en pleine forme.");
    } else {
        snprintf(message, taille, "Une benediction vous soigne : +%d PV.", rendus);
    }
    return RES_OK;
}

static const FonctionEvenement TABLE_EVENEMENTS[] = {
    evenement_tresor,
    evenement_piege,
    evenement_fontaine,
    evenement_benediction,
};

#define NB_EVENEMENTS (sizeof(TABLE_EVENEMENTS) / sizeof(TABLE_EVENEMENTS[0]))

size_t evenement_nombre(void)
{
    return NB_EVENEMENTS;
}

Resultat evenement_appliquer(size_t indice, Joueur *joueur, char *message, size_t taille)
{
    if (joueur == NULL) {
        return RES_ERR_NULL;
    }
    if (indice >= NB_EVENEMENTS) {
        return RES_ERR_INDICE;
    }
    return TABLE_EVENEMENTS[indice](joueur, message, taille);
}

int evenement_declencher(Joueur *joueur, char *message, size_t taille)
{
    if (joueur == NULL || rand() % 100 >= PROBA_EVENEMENT) {
        return 0;
    }
    return evenement_appliquer((size_t)rand() % NB_EVENEMENTS, joueur, message, taille) == RES_OK;
}
