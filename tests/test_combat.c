#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "combat.h"
#include "enemy.h"

static void verifier_calculs_de_degats(void)
{
    Stats att = { 50, 50, 12, 0 };
    Stats def = { 50, 50, 0, 5 };
    Stats faible = { 50, 50, 3, 0 };
    int vu_normal = 0, vu_critique = 0, i;

    assert(degats_simples(&att, &def) == 12);          /* ignore la défense */
    assert(degats_avec_defense(&att, &def) == 7);      /* 12 - 5 */
    assert(degats_avec_defense(&faible, &def) == 1);   /* jamais moins de 1 */

    for (i = 0; i < 500; i++) {
        int d = degats_critique(&att, &def);
        assert(d == 7 || d == 14);
        if (d == 7) vu_normal = 1; else vu_critique = 1;
    }
    assert(vu_normal && vu_critique);
}

static void verifier_calculs_de_score(void)
{
    Joueur j;

    initialiser_joueur(&j, "H");
    j.gold = 30;
    assert(score_simple(&j, 10, 2) == 230);
    assert(score_rapidite(&j, 10, 2) == 230 + 90 * 2);
    assert(score_rapidite(&j, 500, 2) == 230);          /* plus de bonus */
    assert(score_rapidite(&j, 10, 0) == 30);
}

static void verifier_table_de_regles(void)
{
    size_t i;

    assert(regles_nombre() == 3);
    assert(regles_indice(regles_nombre()) == NULL);
    assert(regles_trouver("inconnu") == NULL);
    assert(regles_trouver(NULL) == NULL);
    assert(regles_defaut() != NULL);
    for (i = 0; i < regles_nombre(); i++) {
        const Regles *r = regles_indice(i);
        assert(r != NULL && r->degats != NULL && r->score != NULL);
        assert(regles_trouver(r->nom) == r);
    }
    assert(regles_trouver("simple")->degats == degats_simples);
    assert(regles_trouver("defense")->degats == degats_avec_defense);
    assert(regles_trouver("critique")->degats == degats_critique);
}

/* Même code de combat, règles différentes => résultat différent. */
static void verifier_changement_de_strategie(void)
{
    Joueur j;
    Ennemi *a = NULL, *b = NULL;
    char msg[128];

    initialiser_joueur(&j, "H");               /* attaque 10 */
    assert(ennemi_creer(MODELE_SQUELETTE, &a) == RES_OK);   /* défense 2, 35 PV */
    assert(ennemi_creer(MODELE_SQUELETTE, &b) == RES_OK);

    assert(combat_attaque_joueur(&j, a, regles_trouver("simple"), msg, sizeof(msg)) == RES_OK);
    assert(combat_attaque_joueur(&j, b, regles_trouver("defense"), msg, sizeof(msg)) == RES_OK);
    assert(a->stats.hp == a->stats.max_hp - 10);                       /* défense ignorée */
    assert(b->stats.hp == b->stats.max_hp - (10 - b->stats.defense));  /* défense prise en compte */
    assert(strstr(msg, "Squelette") != NULL);

    ennemi_liberer(a);
    ennemi_liberer(b);
}

static void verifier_attaque_et_erreurs(void)
{
    Joueur j;
    Ennemi *e = NULL;
    char msg[128];
    const Regles *r = regles_trouver("simple");

    initialiser_joueur(&j, "H");
    ennemi_creer(MODELE_BONHOMME_NEIGE, &e);

    assert(combat_attaque_joueur(NULL, e, r, msg, sizeof(msg)) == RES_ERR_NULL);
    assert(combat_attaque_joueur(&j, NULL, r, msg, sizeof(msg)) == RES_ERR_NULL);
    assert(combat_attaque_joueur(&j, e, NULL, msg, sizeof(msg)) == RES_ERR_NULL);

    j.stats.attaque = 1000;                       /* les PV ne descendent jamais sous 0 */
    assert(combat_attaque_joueur(&j, e, r, msg, sizeof(msg)) == RES_OK);
    assert(e->stats.hp == 0);
    assert(!ennemi_est_vivant(e));
    assert(!ennemi_est_vivant(NULL));

    ennemi_liberer(e);
    ennemi_liberer(NULL);
}

static void verifier_sort(void)
{
    Joueur j;
    Ennemi *e = NULL;
    char msg[128];
    const Regles *r = regles_trouver("simple");

    initialiser_joueur(&j, "H");
    ennemi_creer(MODELE_DRAGON, &e);

    assert(combat_sort_joueur(&j, e, r, msg, sizeof(msg)) == RES_OK);
    assert(j.mana == j.max_mana - COUT_SORT);
    assert(e->stats.hp == e->stats.max_hp - 20);   /* attaque doublée */

    j.mana = COUT_SORT - 1;
    assert(combat_sort_joueur(&j, e, r, msg, sizeof(msg)) == RES_ERR_ACTION);
    assert(j.mana == COUT_SORT - 1);               /* rien consommé */
    assert(e->stats.hp == e->stats.max_hp - 20);

    ennemi_liberer(e);
}

static void verifier_comportements_ennemis(void)
{
    Joueur j;
    Ennemi *vamp = NULL, *alien = NULL, *squelette = NULL;
    char msg[128];
    const Regles *r = regles_trouver("defense");
    int vu_hesite = 0, vu_attaque = 0, i;

    initialiser_joueur(&j, "H");
    ennemi_creer(MODELE_VAMPIRE, &vamp);
    ennemi_creer(MODELE_ALIEN, &alien);
    ennemi_creer(MODELE_SQUELETTE, &squelette);

    /* Agressif : toujours attaque. */
    assert(squelette->comportement(squelette) == ENNEMI_ATTAQUE);
    assert(combat_tour_ennemi(&j, squelette, r, msg, sizeof(msg)) == RES_OK);
    assert(j.stats.hp == j.stats.max_hp - (squelette->stats.attaque - j.stats.defense));

    /* Prudent : attaque en forme, se soigne quand il est faible, puis plus de soins. */
    assert(vamp->comportement(vamp) == ENNEMI_ATTAQUE);
    vamp->stats.hp = 5;
    assert(vamp->comportement(vamp) == ENNEMI_SOIN);
    assert(combat_tour_ennemi(&j, vamp, r, msg, sizeof(msg)) == RES_OK);
    assert(vamp->stats.hp == 5 + vamp->stats.max_hp / 4);
    assert(vamp->soins_restants == 1);
    vamp->stats.hp = 5;
    vamp->soins_restants = 0;
    assert(vamp->comportement(vamp) == ENNEMI_ATTAQUE);

    /* Capricieux : parfois il rate son tour. */
    for (i = 0; i < 300; i++) {
        ActionEnnemi a = alien->comportement(alien);
        if (a == ENNEMI_HESITE) vu_hesite = 1;
        if (a == ENNEMI_ATTAQUE) vu_attaque = 1;
    }
    assert(vu_hesite && vu_attaque);

    /* Le joueur ne descend jamais sous 0 PV. */
    j.stats.hp = 1;
    squelette->stats.attaque = 999;
    combat_tour_ennemi(&j, squelette, r, msg, sizeof(msg));
    assert(j.stats.hp == 0);

    assert(combat_tour_ennemi(NULL, squelette, r, msg, sizeof(msg)) == RES_ERR_NULL);

    ennemi_liberer(vamp);
    ennemi_liberer(alien);
    ennemi_liberer(squelette);
}

static void verifier_creation_ennemis(void)
{
    Ennemi *e = (Ennemi *)1;
    size_t m;

    assert(ennemi_creer(NB_MODELES_ENNEMIS, &e) == RES_ERR_INDICE);
    assert(e == NULL);
    assert(ennemi_creer(0, NULL) == RES_ERR_NULL);

    for (m = 0; m < NB_MODELES_ENNEMIS; m++) {
        assert(ennemi_creer(m, &e) == RES_OK);
        assert(e->stats.hp > 0 && e->stats.hp == e->stats.max_hp);
        assert(e->asset != NULL && e->comportement != NULL && e->nom[0] != '\0');
        ennemi_liberer(e);
    }
}

void test_combat(void)
{
    srand(12345);
    verifier_calculs_de_degats();
    verifier_calculs_de_score();
    verifier_table_de_regles();
    verifier_changement_de_strategie();
    verifier_attaque_et_erreurs();
    verifier_sort();
    verifier_comportements_ennemis();
    verifier_creation_ennemis();
    puts("[ok] combat, regles, comportements");
}
