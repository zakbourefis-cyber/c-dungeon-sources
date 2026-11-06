#include <stdlib.h>
#include <string.h>

#include "enemy.h"

#define SEUIL_SOIN_POURCENT 30
#define CHANCE_HESITATION 25

/* Modèle de monstre : les valeurs d'équilibrage sont toutes ici. */
typedef struct ModeleDef {
    const char *nom;
    int hp, attaque, defense;
    int xp, gold, soins;
    const char *asset;
    ComportementEnnemi comportement;
} ModeleDef;

static const ModeleDef MODELES[NB_MODELES_ENNEMIS] = {
    [MODELE_BONHOMME_NEIGE] = { "Bonhomme de neige", 30, 9, 1,  15,  10, 0,
                                "include/assets/monsters/snowman.txt",   comportement_agressif },
    [MODELE_SQUELETTE]      = { "Squelette",         40, 12, 2,  20,  15, 0,
                                "include/assets/monsters/squelette.txt", comportement_agressif },
    [MODELE_VAMPIRE]        = { "Vampire",           52, 16, 3,  30,  20, 2,
                                "include/assets/monsters/vampire.txt",   comportement_prudent },
    [MODELE_ALIEN]          = { "Alien",             56, 17, 3,  35,  25, 0,
                                "include/assets/monsters/alien.txt",     comportement_capricieux },
    [MODELE_EVIL]           = { "Entite maudite",    72, 21, 5,  50,  40, 0,
                                "include/assets/monsters/Evil.txt",      comportement_agressif },
    [MODELE_DRAGON]         = { "Dragon",           120, 25, 6, 100, 100, 2,
                                "include/assets/monsters/dragon.txt",    comportement_prudent },
};

Resultat ennemi_creer(size_t modele, Ennemi **sortie)
{
    Ennemi *ennemi;
    const ModeleDef *def;

    if (sortie == NULL) {
        return RES_ERR_NULL;
    }
    *sortie = NULL;

    if (modele >= NB_MODELES_ENNEMIS) {
        return RES_ERR_INDICE;
    }

    ennemi = calloc(1, sizeof(*ennemi));
    if (ennemi == NULL) {
        return RES_ERR_MEMOIRE;
    }

    def = &MODELES[modele];
    strncpy(ennemi->nom, def->nom, TAILLE_NOM_ENTITE - 1);
    ennemi->stats.hp = def->hp;
    ennemi->stats.max_hp = def->hp;
    ennemi->stats.attaque = def->attaque;
    ennemi->stats.defense = def->defense;
    ennemi->xp = def->xp;
    ennemi->gold = def->gold;
    ennemi->soins_restants = def->soins;
    ennemi->asset = def->asset;
    ennemi->comportement = def->comportement;

    *sortie = ennemi;
    return RES_OK;
}

void ennemi_liberer(Ennemi *ennemi)
{
    free(ennemi);
}

int ennemi_est_vivant(const Ennemi *ennemi)
{
    return ennemi != NULL && ennemi->stats.hp > 0;
}

ActionEnnemi comportement_agressif(const struct Ennemi *ennemi)
{
    (void)ennemi;
    return ENNEMI_ATTAQUE;
}

ActionEnnemi comportement_prudent(const struct Ennemi *ennemi)
{
    if (ennemi->soins_restants > 0 &&
        ennemi->stats.hp * 100 < ennemi->stats.max_hp * SEUIL_SOIN_POURCENT) {
        return ENNEMI_SOIN;
    }
    return ENNEMI_ATTAQUE;
}

ActionEnnemi comportement_capricieux(const struct Ennemi *ennemi)
{
    (void)ennemi;
    if (rand() % 100 < CHANCE_HESITATION) {
        return ENNEMI_HESITE;
    }
    return ENNEMI_ATTAQUE;
}
