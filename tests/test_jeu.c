#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "item.h"
#include "jeu.h"

static void verifier_initialisation(void)
{
    Jeu jeu;

    assert(jeu_init(NULL, NULL) == RES_ERR_NULL);
    assert(jeu_init(&jeu, NULL) == RES_OK);
    assert(jeu.regles == regles_defaut());
    assert(jeu.etat == ETAT_EXPLORATION);
    assert(jeu_salle_courante(&jeu) == donjon_salle(&jeu.donjon, 0));
    assert(jeu_ennemi_courant(&jeu) == NULL);
    assert(compterElements(jeu.joueur.inventaire) == 2);
    assert(jeu.message[0] != '\0');
    jeu_liberer(&jeu);
    jeu_liberer(&jeu);        /* idempotent */
    jeu_liberer(NULL);
}

static void verifier_actions_hors_combat(void)
{
    Jeu jeu;

    jeu_init(&jeu, regles_trouver("simple"));
    assert(jeu_attaquer(&jeu) == RES_ERR_ACTION);
    assert(jeu_lancer_sort(&jeu) == RES_ERR_ACTION);
    assert(jeu_fuir(&jeu) == RES_ERR_ACTION);
    assert(jeu.tours == 0);

    assert(jeu_deplacer(&jeu, DIR_OUEST) == RES_ERR_INTROUVABLE);   /* mur */
    assert(jeu_salle_courante(&jeu) == donjon_salle(&jeu.donjon, 0));
    assert(strstr(jeu.message, "ouest") != NULL);

    assert(jeu_deplacer(NULL, DIR_NORD) == RES_ERR_NULL);
    assert(jeu_attaquer(NULL) == RES_ERR_NULL);
    jeu_liberer(&jeu);
}

static void verifier_ramassage(void)
{
    Jeu jeu;

    jeu_init(&jeu, NULL);
    assert(compterElements(jeu_salle_courante(&jeu)->objets) == 1);
    assert(jeu_ramasser(&jeu) == RES_OK);
    assert(jeu_salle_courante(&jeu)->objets == NULL);
    assert(compterElements(jeu.joueur.inventaire) == 3);
    assert(jeu_ramasser(&jeu) == RES_ERR_INTROUVABLE);               /* plus rien */

    /* Inventaire presque plein : les objets en trop restent au sol. */
    while (compterElements(jeu.joueur.inventaire) < TAILLE_MAX_INVENTAIRE - 1) {
        assert(ajouter_inventaire(&jeu.joueur, "Truc", TYPE_SOIN, 1) == RES_OK);
    }
    donjon_deposer_objet(&jeu.donjon, 0, "A", TYPE_SOIN, 1);
    donjon_deposer_objet(&jeu.donjon, 0, "B", TYPE_SOIN, 1);
    assert(jeu_ramasser(&jeu) == RES_OK);
    assert(compterElements(jeu.joueur.inventaire) == TAILLE_MAX_INVENTAIRE);
    assert(compterElements(jeu_salle_courante(&jeu)->objets) == 1);
    assert(jeu_ramasser(&jeu) == RES_ERR_ACTION);                    /* plein */
    jeu_liberer(&jeu);
}

static void verifier_utilisation_objet(void)
{
    Jeu jeu;

    jeu_init(&jeu, NULL);
    assert(jeu_utiliser_objet(&jeu, 0) == RES_ERR_ACTION);           /* PV déjà au max */
    assert(compterElements(jeu.joueur.inventaire) == 2);
    jeu.joueur.stats.hp = 20;
    assert(jeu_utiliser_objet(&jeu, 0) == RES_OK);
    assert(jeu.joueur.stats.hp == 60);
    assert(jeu_utiliser_objet(&jeu, 5) == RES_ERR_INDICE);
    assert(jeu.tours == 0);                                          /* hors combat : pas de tour */
    jeu_liberer(&jeu);
}

static void verifier_combat_et_fuite(void)
{
    Jeu jeu;
    int essais = 0;

    jeu_init(&jeu, regles_trouver("simple"));
    jeu_deplacer(&jeu, DIR_NORD);                       /* salle 1 : bonhomme de neige */
    assert(jeu.etat == ETAT_COMBAT);
    assert(jeu_ennemi_courant(&jeu) != NULL);

    /* Pendant le combat : pas de déplacement, pas de ramassage. */
    assert(jeu_deplacer(&jeu, DIR_EST) == RES_ERR_ACTION);
    assert(jeu_ramasser(&jeu) == RES_ERR_ACTION);
    assert(jeu_salle_courante(&jeu) == donjon_salle(&jeu.donjon, 1));

    /* Fuite : avec des PV énormes et beaucoup de défense on survit aux ratés. */
    jeu.joueur.stats.max_hp = jeu.joueur.stats.hp = 100000;
    while (jeu.etat == ETAT_COMBAT && essais++ < 200) {
        assert(jeu_fuir(&jeu) == RES_OK);
    }
    assert(jeu.etat == ETAT_EXPLORATION);
    assert(jeu.donjon.courante == 0);                   /* retour à la salle précédente */
    assert(donjon_salle(&jeu.donjon, 1)->ennemi != NULL);   /* l'ennemi est toujours là */

    /* Si on revient, le combat reprend. */
    jeu_deplacer(&jeu, DIR_NORD);
    assert(jeu.etat == ETAT_COMBAT);

    /* Sort sans mana : rien n'est consommé et l'ennemi ne joue pas. */
    jeu.joueur.mana = 0;
    {
        int hp = jeu.joueur.stats.hp;
        assert(jeu_lancer_sort(&jeu) == RES_ERR_ACTION);
        assert(jeu.joueur.stats.hp == hp);
    }

    /* Utiliser un objet en combat coûte le tour : l'ennemi riposte. */
    jeu.joueur.stats.hp = 50000;
    {
        int tours = jeu.tours;
        assert(jeu_utiliser_objet(&jeu, 0) == RES_OK);
        assert(jeu.tours == tours + 1);
        assert(jeu.joueur.stats.hp < 100000 && jeu.joueur.stats.hp > 50000);
    }
    jeu_liberer(&jeu);
}

static void verifier_victoire_complete(void)
{
    Jeu jeu;
    static const Direction trajet[] = { DIR_NORD, DIR_EST, DIR_EST, DIR_EST };
    size_t i;

    jeu_init(&jeu, regles_trouver("critique"));
    jeu.joueur.stats.attaque = 1000;                    /* chaque ennemi tombe en un coup */
    jeu.joueur.stats.max_hp = jeu.joueur.stats.hp = 100000;

    for (i = 0; i < sizeof(trajet) / sizeof(trajet[0]); i++) {
        assert(jeu_deplacer(&jeu, trajet[i]) == RES_OK);
        while (jeu.etat == ETAT_COMBAT) {
            assert(jeu_attaquer(&jeu) == RES_OK);
        }
    }

    assert(jeu.etat == ETAT_VICTOIRE);
    assert(jeu.ennemis_vaincus == 3);                   /* salles 1, 5 et 7 */
    assert(jeu.joueur.gold >= 10 + 25 + 100);
    assert(jeu.joueur.niveau > 1);
    assert(jeu.score >= 300);
    assert(jeu.score == jeu.regles->score(&jeu.joueur, jeu.tours, jeu.ennemis_vaincus));
    assert(jeu_salle_courante(&jeu)->ennemi == NULL);
    assert(strstr(jeu.message, "VICTOIRE") != NULL);

    /* Plus aucune action possible après la fin. */
    assert(jeu_deplacer(&jeu, DIR_OUEST) == RES_ERR_ACTION);
    assert(jeu_attaquer(&jeu) == RES_ERR_ACTION);
    assert(jeu_utiliser_objet(&jeu, 0) == RES_ERR_ACTION);
    jeu_liberer(&jeu);
}

static void verifier_defaite(void)
{
    Jeu jeu;

    jeu_init(&jeu, regles_trouver("defense"));
    jeu_deplacer(&jeu, DIR_NORD);
    assert(jeu.etat == ETAT_COMBAT);

    jeu.joueur.stats.hp = 1;                            /* après l'éventuel événement d'entrée */
    jeu.joueur.stats.defense = 0;
    jeu.joueur.stats.attaque = 1;                       /* ne tue pas le bonhomme de neige */
    assert(jeu_attaquer(&jeu) == RES_OK);

    assert(jeu.etat == ETAT_DEFAITE);
    assert(jeu.joueur.stats.hp == 0);
    assert(strstr(jeu.message, "DEFAITE") != NULL);
    assert(jeu_attaquer(&jeu) == RES_ERR_ACTION);
    assert(jeu_fuir(&jeu) == RES_ERR_ACTION);
    jeu_liberer(&jeu);
}

static void verifier_evenements(void)
{
    Joueur j;
    char msg[128];
    size_t i;
    int declenches = 0, essais;

    initialiser_joueur(&j, "H");
    assert(evenement_nombre() >= 3);
    assert(evenement_appliquer(evenement_nombre(), &j, msg, sizeof(msg)) == RES_ERR_INDICE);
    assert(evenement_appliquer(0, NULL, msg, sizeof(msg)) == RES_ERR_NULL);

    for (i = 0; i < evenement_nombre(); i++) {
        j.stats.hp = 50;
        j.mana = 0;
        assert(evenement_appliquer(i, &j, msg, sizeof(msg)) == RES_OK);
        assert(msg[0] != '\0');
    }

    /* Le piège ne tue jamais, même à 1 PV. */
    for (essais = 0; essais < 200; essais++) {
        j.stats.hp = 1;
        evenement_appliquer(1, &j, msg, sizeof(msg));
        assert(j.stats.hp >= 1);
    }

    srand(7);
    for (essais = 0; essais < 1000; essais++) {
        if (evenement_declencher(&j, msg, sizeof(msg))) {
            declenches++;
        }
    }
    assert(declenches > 250 && declenches < 550);       /* ~ PROBA_EVENEMENT % */
    assert(evenement_declencher(NULL, msg, sizeof(msg)) == 0);
}

void test_jeu(void)
{
    srand(2024);
    verifier_initialisation();
    verifier_actions_hors_combat();
    verifier_ramassage();
    verifier_utilisation_objet();
    verifier_combat_et_fuite();
    verifier_victoire_complete();
    verifier_defaite();
    verifier_evenements();
    puts("[ok] moteur de jeu, evenements, victoire et defaite");
}
