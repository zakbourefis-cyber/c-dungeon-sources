#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dungeon.h"
#include "item.h"

static void verifier_directions(void)
{
    int d;

    for (d = 0; d < NB_DIRECTIONS; d++) {
        assert(direction_opposee(direction_opposee((Direction)d)) == (Direction)d);
        assert(direction_opposee((Direction)d) != (Direction)d);
        assert(strcmp(direction_nom((Direction)d), "?") != 0);
    }
    assert(strcmp(direction_nom(NB_DIRECTIONS), "?") == 0);
}

static void verifier_croissance_du_tableau(void)
{
    Donjon d;
    size_t id = 99;
    size_t i;
    char nom[20];

    assert(donjon_init(NULL) == RES_ERR_NULL);
    assert(donjon_init(&d) == RES_OK);
    assert(d.nb_salles == 0 && d.capacite > 0);

    assert(donjon_ajouter_salle(&d, NULL, "d", "x.txt", &id) == RES_ERR_NULL);
    assert(donjon_ajouter_salle(NULL, "n", "d", "x.txt", &id) == RES_ERR_NULL);

    for (i = 0; i < 20; i++) {                        /* force plusieurs realloc */
        snprintf(nom, sizeof(nom), "Salle %zu", i);
        assert(donjon_ajouter_salle(&d, nom, "desc", "x.txt", &id) == RES_OK);
        assert(id == i);
    }
    assert(d.nb_salles == 20 && d.capacite >= 20);
    for (i = 0; i < 20; i++) {                        /* les données ont survécu aux realloc */
        snprintf(nom, sizeof(nom), "Salle %zu", i);
        assert(strcmp(donjon_salle(&d, i)->nom, nom) == 0);
        assert(donjon_salle(&d, i)->voisins[DIR_NORD] == SALLE_AUCUNE);
    }
    assert(donjon_salle(&d, 20) == NULL);

    donjon_liberer(&d);
    assert(d.salles == NULL && d.nb_salles == 0);
    donjon_liberer(&d);                               /* idempotent */
    donjon_liberer(NULL);
}

static void verifier_liaisons_et_deplacements(void)
{
    Donjon d;
    size_t a, b;

    donjon_init(&d);
    donjon_ajouter_salle(&d, "A", "a", "x.txt", &a);
    donjon_ajouter_salle(&d, "B", "b", "x.txt", &b);

    assert(donjon_relier(&d, a, DIR_EST, b) == RES_OK);
    assert(donjon_salle(&d, a)->voisins[DIR_EST] == (int)b);
    assert(donjon_salle(&d, b)->voisins[DIR_OUEST] == (int)a);   /* sens inverse */
    assert(donjon_relier(&d, a, DIR_EST, 42) == RES_ERR_INDICE);
    assert(donjon_relier(&d, 42, DIR_EST, b) == RES_ERR_INDICE);
    assert(donjon_relier(&d, a, NB_DIRECTIONS, b) == RES_ERR_INDICE);
    assert(donjon_relier(NULL, a, DIR_EST, b) == RES_ERR_NULL);

    assert(donjon_deplacer(&d, DIR_NORD) == RES_ERR_INTROUVABLE);   /* mur */
    assert(d.courante == a);
    assert(donjon_deplacer(&d, NB_DIRECTIONS) == RES_ERR_INDICE);
    assert(donjon_deplacer(&d, DIR_EST) == RES_OK);
    assert(d.courante == b && d.precedente == a);
    assert(donjon_deplacer(&d, DIR_OUEST) == RES_OK);
    assert(d.courante == a && d.precedente == b);
    assert(donjon_deplacer(NULL, DIR_EST) == RES_ERR_NULL);

    donjon_liberer(&d);
}

static void verifier_ennemis_et_objets(void)
{
    Donjon d;
    size_t a;

    donjon_init(&d);
    donjon_ajouter_salle(&d, "A", "a", "x.txt", &a);

    assert(donjon_placer_ennemi(&d, a, MODELE_ALIEN) == RES_OK);
    assert(donjon_salle(&d, a)->ennemi != NULL);
    assert(donjon_placer_ennemi(&d, a, MODELE_ALIEN) == RES_ERR_ACTION);   /* déjà occupée */
    assert(donjon_placer_ennemi(&d, 9, MODELE_ALIEN) == RES_ERR_INDICE);
    assert(donjon_placer_ennemi(&d, a + 0, NB_MODELES_ENNEMIS) == RES_ERR_ACTION);
    assert(donjon_placer_ennemi(NULL, a, MODELE_ALIEN) == RES_ERR_NULL);

    assert(donjon_deposer_objet(&d, a, "Potion", TYPE_SOIN, 10) == RES_OK);
    assert(donjon_deposer_objet(&d, a, "Epee", TYPE_ARME, 2) == RES_OK);
    assert(compterElements(donjon_salle(&d, a)->objets) == 2);
    assert(donjon_deposer_objet(&d, 9, "X", TYPE_SOIN, 1) == RES_ERR_INDICE);
    assert(donjon_deposer_objet(&d, a, NULL, TYPE_SOIN, 1) == RES_ERR_NULL);

    donjon_liberer(&d);   /* libère ennemi et objets : à vérifier avec valgrind */
}

/* Parcours en largeur : toutes les salles doivent être atteignables depuis le départ. */
static void verifier_donjon_par_defaut(void)
{
    Donjon d;
    int vu[64] = { 0 };
    size_t file[64];
    size_t tete = 0, queue = 0, atteintes = 0, i;
    int dir, nb_ennemis = 0, nb_finales = 0;

    donjon_init(&d);
    assert(donjon_construire_defaut(&d) == RES_OK);
    assert(d.nb_salles == 8);
    assert(d.courante == 0 && donjon_salle(&d, 0)->visitee);
    assert(donjon_salle(&d, 0)->ennemi == NULL);          /* départ sans danger */

    file[queue++] = 0;
    vu[0] = 1;
    while (tete < queue) {
        const Salle *s = donjon_salle(&d, file[tete++]);
        atteintes++;
        for (dir = 0; dir < NB_DIRECTIONS; dir++) {
            int v = s->voisins[dir];
            if (v != SALLE_AUCUNE && !vu[v]) {
                /* Les liaisons sont symétriques. */
                assert(donjon_salle(&d, (size_t)v)->voisins[direction_opposee((Direction)dir)]
                       == (int)file[tete - 1]);
                vu[v] = 1;
                file[queue++] = (size_t)v;
            }
        }
    }
    assert(atteintes == d.nb_salles);

    for (i = 0; i < d.nb_salles; i++) {
        const Salle *s = donjon_salle(&d, i);
        assert(s->decor != NULL && s->nom[0] != '\0');
        if (s->ennemi != NULL) nb_ennemis++;
        if (s->finale) {
            nb_finales++;
            assert(s->ennemi != NULL);
        }
    }
    assert(nb_ennemis == 6);
    assert(nb_finales == 1);

    donjon_liberer(&d);
}

void test_donjon(void)
{
    verifier_directions();
    verifier_croissance_du_tableau();
    verifier_liaisons_et_deplacements();
    verifier_ennemis_et_objets();
    verifier_donjon_par_defaut();
    puts("[ok] donjon, salles, memoire dynamique");
}
