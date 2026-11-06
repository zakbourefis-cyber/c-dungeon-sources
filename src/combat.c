#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "combat.h"
#include "debug.h"

/* ---------- Stratégies de dégâts ---------- */

int degats_simples(const Stats *attaquant, const Stats *defenseur)
{
    (void)defenseur;
    return attaquant->attaque > 0 ? attaquant->attaque : 1;
}

int degats_avec_defense(const Stats *attaquant, const Stats *defenseur)
{
    int degats = attaquant->attaque - defenseur->defense;
    return degats > 0 ? degats : 1;
}

int degats_critique(const Stats *attaquant, const Stats *defenseur)
{
    int degats = degats_avec_defense(attaquant, defenseur);

    if (rand() % 100 < CHANCE_CRITIQUE) {
        degats *= 2;
    }
    return degats;
}

/* ---------- Stratégies de score ---------- */

int score_simple(const Joueur *joueur, int tours, int ennemis_vaincus)
{
    (void)tours;
    return ennemis_vaincus * 100 + joueur->gold;
}

int score_rapidite(const Joueur *joueur, int tours, int ennemis_vaincus)
{
    int bonus = tours < 100 ? 100 - tours : 0;
    return score_simple(joueur, tours, ennemis_vaincus) + bonus * ennemis_vaincus;
}

/* ---------- Jeux de règles ---------- */

static const Regles TABLE_REGLES[] = {
    { "simple",   degats_simples,      score_simple   },
    { "defense",  degats_avec_defense, score_simple   },
    { "critique", degats_critique,     score_rapidite },
};

#define NB_REGLES (sizeof(TABLE_REGLES) / sizeof(TABLE_REGLES[0]))

size_t regles_nombre(void)
{
    return NB_REGLES;
}

const Regles *regles_indice(size_t indice)
{
    return indice < NB_REGLES ? &TABLE_REGLES[indice] : NULL;
}

const Regles *regles_trouver(const char *nom)
{
    size_t i;

    if (nom == NULL) {
        return NULL;
    }
    for (i = 0; i < NB_REGLES; i++) {
        if (strcmp(TABLE_REGLES[i].nom, nom) == 0) {
            return &TABLE_REGLES[i];
        }
    }
    return NULL;
}

const Regles *regles_defaut(void)
{
    return &TABLE_REGLES[1];
}

/* ---------- Combat ---------- */

/* Applique des dégâts à l'ennemi sans passer sous 0 PV. */
static void blesser_ennemi(Ennemi *ennemi, int degats)
{
    ennemi->stats.hp -= degats;
    if (ennemi->stats.hp < 0) {
        ennemi->stats.hp = 0;
    }
}

Resultat combat_attaque_joueur(Joueur *joueur, Ennemi *ennemi, const Regles *regles,
                               char *message, size_t taille)
{
    int degats;

    if (joueur == NULL || ennemi == NULL || regles == NULL || regles->degats == NULL) {
        return RES_ERR_NULL;
    }

    degats = regles->degats(&joueur->stats, &ennemi->stats);
    blesser_ennemi(ennemi, degats);
    DEBUG_LOG("attaque joueur: %d degats sur %s (reste %d)", degats, ennemi->nom, ennemi->stats.hp);
    snprintf(message, taille, "Vous frappez %s : -%d PV.", ennemi->nom, degats);
    return RES_OK;
}

Resultat combat_sort_joueur(Joueur *joueur, Ennemi *ennemi, const Regles *regles,
                            char *message, size_t taille)
{
    Stats puissance;
    int degats;

    if (joueur == NULL || ennemi == NULL || regles == NULL || regles->degats == NULL) {
        return RES_ERR_NULL;
    }
    if (joueur->mana < COUT_SORT) {
        snprintf(message, taille, "Pas assez de mana (%d requis).", COUT_SORT);
        return RES_ERR_ACTION;
    }

    joueur->mana -= COUT_SORT;
    puissance = joueur->stats;
    puissance.attaque *= 2;
    degats = regles->degats(&puissance, &ennemi->stats);
    blesser_ennemi(ennemi, degats);
    DEBUG_LOG("sort joueur: %d degats sur %s (reste %d)", degats, ennemi->nom, ennemi->stats.hp);
    snprintf(message, taille, "Votre sort touche %s : -%d PV.", ennemi->nom, degats);
    return RES_OK;
}

Resultat combat_tour_ennemi(Joueur *joueur, Ennemi *ennemi, const Regles *regles,
                            char *message, size_t taille)
{
    ActionEnnemi action;
    int degats;
    int soin;

    if (joueur == NULL || ennemi == NULL || regles == NULL ||
        regles->degats == NULL || ennemi->comportement == NULL) {
        return RES_ERR_NULL;
    }

    action = ennemi->comportement(ennemi);   /* appel via pointeur de fonction */

    switch (action) {
    case ENNEMI_SOIN:
        soin = ennemi->stats.max_hp / 4;
        ennemi->stats.hp += soin;
        if (ennemi->stats.hp > ennemi->stats.max_hp) {
            ennemi->stats.hp = ennemi->stats.max_hp;
        }
        ennemi->soins_restants--;
        snprintf(message, taille, "%s se soigne : +%d PV.", ennemi->nom, soin);
        break;
    case ENNEMI_HESITE:
        snprintf(message, taille, "%s hesite et rate son tour.", ennemi->nom);
        break;
    case ENNEMI_ATTAQUE:
    default:
        degats = regles->degats(&ennemi->stats, &joueur->stats);
        joueur->stats.hp -= degats;
        if (joueur->stats.hp < 0) {
            joueur->stats.hp = 0;
        }
        DEBUG_LOG("%s attaque: %d degats (joueur reste %d)", ennemi->nom, degats, joueur->stats.hp);
        snprintf(message, taille, "%s vous frappe : -%d PV.", ennemi->nom, degats);
        break;
    }
    return RES_OK;
}
