#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "item.h"
#include "jeu.h"

/* ---------- Messages ---------- */

/* Ajoute du texte formaté à la fin du message du jeu, sans jamais déborder. */
static void message_ajouter(Jeu *jeu, const char *format, ...)
{
    size_t longueur = strlen(jeu->message);
    va_list args;

    if (longueur >= TAILLE_MESSAGE - 1) {
        return;
    }
    if (longueur > 0 && longueur < TAILLE_MESSAGE - 2) {
        jeu->message[longueur++] = ' ';
        jeu->message[longueur] = '\0';
    }
    va_start(args, format);
    vsnprintf(jeu->message + longueur, TAILLE_MESSAGE - longueur, format, args);
    va_end(args);
}

static void message_remplacer(Jeu *jeu, const char *texte)
{
    snprintf(jeu->message, TAILLE_MESSAGE, "%s", texte);
}

static void mettre_a_jour_score(Jeu *jeu)
{
    jeu->score = jeu->regles->score(&jeu->joueur, jeu->tours, jeu->ennemis_vaincus);
}

/* ---------- Initialisation ---------- */

Resultat jeu_init(Jeu *jeu, const Regles *regles)
{
    Resultat res;

    if (jeu == NULL) {
        return RES_ERR_NULL;
    }

    memset(jeu, 0, sizeof(*jeu));
    jeu->regles = regles != NULL ? regles : regles_defaut();
    jeu->etat = ETAT_EXPLORATION;

    res = initialiser_joueur(&jeu->joueur, "Aventurier");
    if (res == RES_OK) {
        res = donjon_init(&jeu->donjon);
    }
    if (res == RES_OK) {
        res = donjon_construire_defaut(&jeu->donjon);
    }
    if (res == RES_OK) {
        res = ajouter_inventaire(&jeu->joueur, "Potion de soin", TYPE_SOIN, 40);
    }
    if (res == RES_OK) {
        res = ajouter_inventaire(&jeu->joueur, "Epee rouillee", TYPE_ARME, 2);
    }
    if (res != RES_OK) {
        jeu_liberer(jeu);
        return res;
    }

    message_remplacer(jeu, jeu_salle_courante(jeu)->description);
    mettre_a_jour_score(jeu);
    return RES_OK;
}

void jeu_liberer(Jeu *jeu)
{
    if (jeu == NULL) {
        return;
    }
    liberer_inventaire(&jeu->joueur);
    donjon_liberer(&jeu->donjon);
}

Salle *jeu_salle_courante(const Jeu *jeu)
{
    if (jeu == NULL) {
        return NULL;
    }
    return donjon_salle(&jeu->donjon, jeu->donjon.courante);
}

Ennemi *jeu_ennemi_courant(const Jeu *jeu)
{
    Salle *salle;

    if (jeu == NULL || jeu->etat != ETAT_COMBAT) {
        return NULL;
    }
    salle = jeu_salle_courante(jeu);
    return salle != NULL ? salle->ennemi : NULL;
}

/* ---------- Fin de combat / fin de partie ---------- */

/* Appelée quand l'ennemi courant vient de mourir : récompenses et nettoyage. */
static void ennemi_vaincu(Jeu *jeu, Salle *salle)
{
    Ennemi *ennemi = salle->ennemi;
    char nom[TAILLE_NOM_ENTITE];
    int niveaux;

    snprintf(nom, sizeof(nom), "%s", ennemi->nom);
    message_ajouter(jeu, "%s est vaincu ! +%d XP, +%d or.", nom, ennemi->xp, ennemi->gold);

    jeu->joueur.gold += ennemi->gold;
    niveaux = joueur_gagner_experience(&jeu->joueur, ennemi->xp);
    if (niveaux > 0) {
        message_ajouter(jeu, "Niveau %d atteint !", jeu->joueur.niveau);
    }

    ennemi_liberer(ennemi);
    salle->ennemi = NULL;      /* évite un pointeur pendant */
    jeu->ennemis_vaincus++;
    jeu->etat = ETAT_EXPLORATION;

    if (salle->finale) {
        jeu->etat = ETAT_VICTOIRE;
        message_ajouter(jeu, "VICTOIRE ! Le donjon est conquis.");
    }
}

/* Riposte de l'ennemi courant ; peut faire passer la partie en défaite. */
static void tour_ennemi(Jeu *jeu, Ennemi *ennemi)
{
    char texte[TAILLE_MESSAGE];

    if (combat_tour_ennemi(&jeu->joueur, ennemi, jeu->regles, texte, sizeof(texte)) == RES_OK) {
        message_ajouter(jeu, "%s", texte);
    }
    if (!joueur_est_vivant(&jeu->joueur)) {
        jeu->etat = ETAT_DEFAITE;
        message_ajouter(jeu, "Vous etes mort... DEFAITE.");
    }
}

/* Vérifie qu'une action de combat est permise ; sinon écrit le message d'erreur. */
static Resultat verifier_combat(Jeu *jeu)
{
    if (jeu->etat == ETAT_VICTOIRE || jeu->etat == ETAT_DEFAITE) {
        message_remplacer(jeu, "La partie est terminee.");
        return RES_ERR_ACTION;
    }
    if (jeu->etat != ETAT_COMBAT || jeu_ennemi_courant(jeu) == NULL) {
        message_remplacer(jeu, "Il n'y a personne a combattre.");
        return RES_ERR_ACTION;
    }
    return RES_OK;
}

/* Fin d'un tour d'action du joueur en combat : résolution puis riposte. */
static void conclure_tour(Jeu *jeu, Salle *salle)
{
    jeu->tours++;
    if (!ennemi_est_vivant(salle->ennemi)) {
        ennemi_vaincu(jeu, salle);
    } else {
        tour_ennemi(jeu, salle->ennemi);
    }
    mettre_a_jour_score(jeu);
}

/* ---------- Actions ---------- */

Resultat jeu_deplacer(Jeu *jeu, Direction direction)
{
    Salle *salle;
    char evenement[TAILLE_MESSAGE];
    Resultat res;

    if (jeu == NULL) {
        return RES_ERR_NULL;
    }
    if (jeu->etat == ETAT_VICTOIRE || jeu->etat == ETAT_DEFAITE) {
        message_remplacer(jeu, "La partie est terminee.");
        return RES_ERR_ACTION;
    }
    if (jeu->etat == ETAT_COMBAT) {
        message_remplacer(jeu, "Un ennemi vous bloque le passage ! Combattez ou fuyez.");
        return RES_ERR_ACTION;
    }

    res = donjon_deplacer(&jeu->donjon, direction);
    if (res == RES_ERR_INTROUVABLE) {
        snprintf(jeu->message, TAILLE_MESSAGE, "Aucun passage vers le %s.",
                 direction_nom(direction));
        return res;
    }
    if (res != RES_OK) {
        message_remplacer(jeu, "Deplacement impossible.");
        return res;
    }

    salle = jeu_salle_courante(jeu);
    jeu->message[0] = '\0';

    if (!salle->visitee) {
        salle->visitee = 1;
        if (evenement_declencher(&jeu->joueur, evenement, sizeof(evenement))) {
            message_ajouter(jeu, "%s", evenement);
        }
        if (salle->ennemi == NULL) {
            message_ajouter(jeu, "%s", salle->description);
        }
    } else if (salle->ennemi == NULL) {
        message_ajouter(jeu, "Vous revenez dans : %s.", salle->nom);
    }

    if (salle->ennemi != NULL) {
        jeu->etat = ETAT_COMBAT;
        message_ajouter(jeu, "%s surgit !", salle->ennemi->nom);
    }
    return RES_OK;
}

Resultat jeu_ramasser(Jeu *jeu)
{
    Salle *salle;
    int ramasses = 0;
    char nom[TAILLE_NOM_ELEMENT];

    if (jeu == NULL) {
        return RES_ERR_NULL;
    }
    if (jeu->etat != ETAT_EXPLORATION) {
        message_remplacer(jeu, jeu->etat == ETAT_COMBAT ? "Impossible de ramasser en plein combat."
                                                        : "La partie est terminee.");
        return RES_ERR_ACTION;
    }

    salle = jeu_salle_courante(jeu);
    if (salle->objets == NULL) {
        message_remplacer(jeu, "Il n'y a rien a ramasser ici.");
        return RES_ERR_INTROUVABLE;
    }

    jeu->message[0] = '\0';
    while (salle->objets != NULL &&
           compterElements(jeu->joueur.inventaire) < TAILLE_MAX_INVENTAIRE) {
        snprintf(nom, sizeof(nom), "%s", salle->objets->nom);
        /* Déplacement du nœud sans allocation : il ne peut pas échouer. */
        if (transfererElementIndice(&salle->objets, 0, &jeu->joueur.inventaire) != RES_OK) {
            break;
        }
        message_ajouter(jeu, "Ramasse : %s.", nom);
        ramasses++;
    }

    if (salle->objets != NULL) {
        message_ajouter(jeu, "Inventaire plein (%d objets max), le reste est laisse au sol.",
                        TAILLE_MAX_INVENTAIRE);
        return ramasses > 0 ? RES_OK : RES_ERR_ACTION;
    }
    return RES_OK;
}

Resultat jeu_utiliser_objet(Jeu *jeu, size_t indice)
{
    Salle *salle;
    char texte[TAILLE_MESSAGE];
    Resultat res;

    if (jeu == NULL) {
        return RES_ERR_NULL;
    }
    if (jeu->etat == ETAT_VICTOIRE || jeu->etat == ETAT_DEFAITE) {
        message_remplacer(jeu, "La partie est terminee.");
        return RES_ERR_ACTION;
    }

    res = item_utiliser(&jeu->joueur, indice, texte, sizeof(texte));
    switch (res) {
    case RES_OK:
        message_remplacer(jeu, texte);
        break;
    case RES_ERR_INDICE:
        message_remplacer(jeu, "Cet objet n'existe pas.");
        return res;
    case RES_ERR_ACTION:
        message_remplacer(jeu, texte[0] != '\0' ? texte : "Cet objet ne sert a rien.");
        return res;
    default:
        message_remplacer(jeu, "Utilisation impossible.");
        return res;
    }

    if (jeu->etat == ETAT_COMBAT) {   /* utiliser un objet en combat coûte le tour */
        salle = jeu_salle_courante(jeu);
        conclure_tour(jeu, salle);
    }
    return RES_OK;
}

Resultat jeu_attaquer(Jeu *jeu)
{
    Salle *salle;
    char texte[TAILLE_MESSAGE];
    Resultat res;

    if (jeu == NULL) {
        return RES_ERR_NULL;
    }
    res = verifier_combat(jeu);
    if (res != RES_OK) {
        return res;
    }

    salle = jeu_salle_courante(jeu);
    res = combat_attaque_joueur(&jeu->joueur, salle->ennemi, jeu->regles, texte, sizeof(texte));
    if (res != RES_OK) {
        message_remplacer(jeu, "Attaque impossible.");
        return res;
    }
    message_remplacer(jeu, texte);
    conclure_tour(jeu, salle);
    return RES_OK;
}

Resultat jeu_lancer_sort(Jeu *jeu)
{
    Salle *salle;
    char texte[TAILLE_MESSAGE];
    Resultat res;

    if (jeu == NULL) {
        return RES_ERR_NULL;
    }
    res = verifier_combat(jeu);
    if (res != RES_OK) {
        return res;
    }

    salle = jeu_salle_courante(jeu);
    res = combat_sort_joueur(&jeu->joueur, salle->ennemi, jeu->regles, texte, sizeof(texte));
    message_remplacer(jeu, texte);
    if (res != RES_OK) {
        return res;     /* mana insuffisant : le tour n'est pas consommé */
    }
    conclure_tour(jeu, salle);
    return RES_OK;
}

Resultat jeu_fuir(Jeu *jeu)
{
    Salle *salle;
    Resultat res;

    if (jeu == NULL) {
        return RES_ERR_NULL;
    }
    res = verifier_combat(jeu);
    if (res != RES_OK) {
        return res;
    }

    if (rand() % 100 < CHANCE_FUITE) {
        jeu->donjon.courante = jeu->donjon.precedente;
        jeu->etat = ETAT_EXPLORATION;
        message_remplacer(jeu, "Vous prenez la fuite !");
        DEBUG_LOG("fuite reussie vers la salle %zu", jeu->donjon.courante);
        return RES_OK;
    }

    salle = jeu_salle_courante(jeu);
    message_remplacer(jeu, "Vous n'arrivez pas a fuir !");
    conclure_tour(jeu, salle);
    return RES_OK;
}
