#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "entity.h"
#include "item.h"

static void verifier_initialisation(void)
{
    Joueur j;

    assert(initialiser_joueur(NULL, "X") == RES_ERR_NULL);
    assert(initialiser_joueur(&j, NULL) == RES_ERR_NULL);
    assert(initialiser_joueur(&j, "Heros") == RES_OK);
    assert(strcmp(j.nom, "Heros") == 0);
    assert(j.niveau == 1);
    assert(j.stats.hp == j.stats.max_hp);
    assert(j.mana == j.max_mana);
    assert(j.inventaire == NULL);
    assert(joueur_est_vivant(&j));
    assert(!joueur_est_vivant(NULL));
}

static void verifier_inventaire(void)
{
    Joueur j;
    int i;

    initialiser_joueur(&j, "H");
    assert(ajouter_inventaire(NULL, "A", TYPE_SOIN, 1) == RES_ERR_NULL);
    assert(ajouter_inventaire(&j, NULL, TYPE_SOIN, 1) == RES_ERR_NULL);

    for (i = 0; i < TAILLE_MAX_INVENTAIRE; i++) {
        assert(ajouter_inventaire(&j, "Objet", TYPE_SOIN, i) == RES_OK);
    }
    assert(compterElements(j.inventaire) == TAILLE_MAX_INVENTAIRE);
    assert(ajouter_inventaire(&j, "Trop", TYPE_SOIN, 1) == RES_ERR_ACTION);
    assert(compterElements(j.inventaire) == TAILLE_MAX_INVENTAIRE);

    liberer_inventaire(&j);
    assert(j.inventaire == NULL);
    liberer_inventaire(&j);              /* deuxième appel : pas de double free */
    liberer_inventaire(NULL);
}

static void verifier_soin_et_experience(void)
{
    Joueur j;

    initialiser_joueur(&j, "H");
    j.stats.hp = 50;
    assert(joueur_soigner(&j, 20) == 20);
    assert(joueur_soigner(&j, 1000) == j.stats.max_hp - 70);
    assert(j.stats.hp == j.stats.max_hp);
    assert(joueur_soigner(&j, 10) == 0);
    assert(joueur_soigner(&j, -5) == 0);
    assert(joueur_soigner(NULL, 5) == 0);

    assert(joueur_xp_requise(&j) == 30);
    assert(joueur_gagner_experience(&j, 29) == 0);
    assert(j.niveau == 1);
    {
        int attaque = j.stats.attaque;
        int max_hp = j.stats.max_hp;
        assert(joueur_gagner_experience(&j, 1) == 1);
        assert(j.niveau == 2);
        assert(j.experience == 0);
        assert(j.stats.attaque == attaque + 2);
        assert(j.stats.max_hp == max_hp + 10);
    }
    assert(joueur_gagner_experience(&j, 60 + 90) == 2);   /* plusieurs niveaux d'un coup */
    assert(j.niveau == 4);
    assert(joueur_gagner_experience(&j, 0) == 0);
    assert(joueur_gagner_experience(NULL, 10) == 0);
}

static void verifier_effets_objets(void)
{
    Joueur j;
    char msg[128];
    int attaque;

    initialiser_joueur(&j, "H");
    attaque = j.stats.attaque;

    assert(item_utiliser(NULL, 0, msg, sizeof(msg)) == RES_ERR_NULL);
    assert(item_utiliser(&j, 0, msg, sizeof(msg)) == RES_ERR_INDICE);   /* inventaire vide */

    ajouter_inventaire(&j, "Potion", TYPE_SOIN, 40);
    ajouter_inventaire(&j, "Elixir", TYPE_MANA, 20);
    ajouter_inventaire(&j, "Epee", TYPE_ARME, 3);
    ajouter_inventaire(&j, "Bouclier", TYPE_ARMURE, 2);
    ajouter_inventaire(&j, "Bizarre", "Inconnu", 1);

    /* Soin inutile à PV max : refusé, objet conservé. */
    assert(item_utiliser(&j, 0, msg, sizeof(msg)) == RES_ERR_ACTION);
    assert(compterElements(j.inventaire) == 5);

    j.stats.hp = 10;
    assert(item_utiliser(&j, 0, msg, sizeof(msg)) == RES_OK);
    assert(j.stats.hp == 50);
    assert(strstr(msg, "Potion") != NULL);
    assert(compterElements(j.inventaire) == 4);

    j.mana = 0;
    assert(item_utiliser(&j, 0, msg, sizeof(msg)) == RES_OK);   /* Elixir est maintenant en 0 */
    assert(j.mana == 20);

    assert(item_utiliser(&j, 0, NULL, 0) == RES_OK);            /* Epee, sans tampon de message */
    assert(j.stats.attaque == attaque + 3);

    {
        int defense = j.stats.defense;
        assert(item_utiliser(&j, 0, msg, sizeof(msg)) == RES_OK);
        assert(j.stats.defense == defense + 2);
    }

    /* Type inconnu : refusé, objet conservé. */
    assert(item_utiliser(&j, 0, msg, sizeof(msg)) == RES_ERR_ACTION);
    assert(compterElements(j.inventaire) == 1);
    assert(item_utiliser(&j, 7, msg, sizeof(msg)) == RES_ERR_INDICE);

    liberer_inventaire(&j);
}

void test_entity(void)
{
    verifier_initialisation();
    verifier_inventaire();
    verifier_soin_et_experience();
    verifier_effets_objets();
    puts("[ok] joueur, inventaire, objets");
}
