#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "dungeon.h"
#include "item.h"

#define CAPACITE_INITIALE 4

#define DECOR_CHATEAU "include/assets/environnement/castle.txt"
#define DECOR_GLACE   "include/assets/environnement/snow_mountain.txt"
#define DECOR_DESERT  "include/assets/environnement/desert.txt"
#define DECOR_NUCLEAIRE "include/assets/environnement/nuclear.txt"
#define DECOR_VAISSEAU "include/assets/environnement/spaceship.txt"
#define DECOR_ESPACE  "include/assets/environnement/space.txt"

const char *direction_nom(Direction direction)
{
    switch (direction) {
    case DIR_NORD:  return "nord";
    case DIR_EST:   return "est";
    case DIR_SUD:   return "sud";
    case DIR_OUEST: return "ouest";
    default:        return "?";
    }
}

Direction direction_opposee(Direction direction)
{
    switch (direction) {
    case DIR_NORD:  return DIR_SUD;
    case DIR_SUD:   return DIR_NORD;
    case DIR_EST:   return DIR_OUEST;
    case DIR_OUEST: return DIR_EST;
    default:        return direction;
    }
}

Resultat donjon_init(Donjon *donjon)
{
    if (donjon == NULL) {
        return RES_ERR_NULL;
    }

    memset(donjon, 0, sizeof(*donjon));
    donjon->salles = calloc(CAPACITE_INITIALE, sizeof(Salle));
    if (donjon->salles == NULL) {
        return RES_ERR_MEMOIRE;
    }
    donjon->capacite = CAPACITE_INITIALE;
    return RES_OK;
}

Resultat donjon_ajouter_salle(Donjon *donjon, const char *nom, const char *description,
                              const char *decor, size_t *id)
{
    Salle *salle;
    int d;

    if (donjon == NULL || donjon->salles == NULL || nom == NULL ||
        description == NULL || decor == NULL) {
        return RES_ERR_NULL;
    }

    if (donjon->nb_salles == donjon->capacite) {
        size_t nouvelle_capacite = donjon->capacite * 2;
        /* Toujours passer par un pointeur temporaire : si realloc échoue,
         * l'ancien tableau reste valide et ne fuit pas. */
        Salle *nouveau = realloc(donjon->salles, nouvelle_capacite * sizeof(Salle));

        if (nouveau == NULL) {
            return RES_ERR_MEMOIRE;
        }
        memset(nouveau + donjon->capacite, 0,
               (nouvelle_capacite - donjon->capacite) * sizeof(Salle));
        donjon->salles = nouveau;
        donjon->capacite = nouvelle_capacite;
        DEBUG_LOG("donjon agrandi : capacite %zu", nouvelle_capacite);
    }

    salle = &donjon->salles[donjon->nb_salles];
    memset(salle, 0, sizeof(*salle));
    strncpy(salle->nom, nom, TAILLE_NOM_SALLE - 1);
    strncpy(salle->description, description, TAILLE_DESC_SALLE - 1);
    salle->decor = decor;
    for (d = 0; d < NB_DIRECTIONS; d++) {
        salle->voisins[d] = SALLE_AUCUNE;
    }

    if (id != NULL) {
        *id = donjon->nb_salles;
    }
    donjon->nb_salles++;
    return RES_OK;
}

Salle *donjon_salle(const Donjon *donjon, size_t id)
{
    if (donjon == NULL || donjon->salles == NULL || id >= donjon->nb_salles) {
        return NULL;
    }
    return &donjon->salles[id];
}

Resultat donjon_relier(Donjon *donjon, size_t a, Direction direction, size_t b)
{
    Salle *salle_a;
    Salle *salle_b;

    if (donjon == NULL) {
        return RES_ERR_NULL;
    }
    salle_a = donjon_salle(donjon, a);
    salle_b = donjon_salle(donjon, b);
    if (salle_a == NULL || salle_b == NULL || (int)direction < 0 ||
        direction >= NB_DIRECTIONS) {
        return RES_ERR_INDICE;
    }

    salle_a->voisins[direction] = (int)b;
    salle_b->voisins[direction_opposee(direction)] = (int)a;
    return RES_OK;
}

Resultat donjon_placer_ennemi(Donjon *donjon, size_t salle, size_t modele)
{
    Salle *s;

    if (donjon == NULL) {
        return RES_ERR_NULL;
    }
    s = donjon_salle(donjon, salle);
    if (s == NULL) {
        return RES_ERR_INDICE;
    }
    if (s->ennemi != NULL) {
        return RES_ERR_ACTION;
    }
    return ennemi_creer(modele, &s->ennemi);
}

Resultat donjon_deposer_objet(Donjon *donjon, size_t salle, const char *nom,
                              const char *type, int valeur)
{
    Salle *s;

    if (donjon == NULL || nom == NULL || type == NULL) {
        return RES_ERR_NULL;
    }
    s = donjon_salle(donjon, salle);
    if (s == NULL) {
        return RES_ERR_INDICE;
    }
    return ajouterElementFin(&s->objets, nom, type, valeur);
}

Resultat donjon_deplacer(Donjon *donjon, Direction direction)
{
    const Salle *actuelle;
    int cible;

    if (donjon == NULL) {
        return RES_ERR_NULL;
    }
    if ((int)direction < 0 || direction >= NB_DIRECTIONS) {
        return RES_ERR_INDICE;
    }
    actuelle = donjon_salle(donjon, donjon->courante);
    if (actuelle == NULL) {
        return RES_ERR_INDICE;
    }

    cible = actuelle->voisins[direction];
    if (cible == SALLE_AUCUNE) {
        return RES_ERR_INTROUVABLE;
    }

    donjon->precedente = donjon->courante;
    donjon->courante = (size_t)cible;
    return RES_OK;
}

/* Description d'une salle du donjon standard. */
typedef struct DefSalle {
    const char *nom;
    const char *description;
    const char *decor;
    int modele;    /* ModeleEnnemi, ou -1 s'il n'y a pas d'ennemi */
} DefSalle;

/*
 * Grille 4 x 2 (le nord est en haut) :
 *     [1 Glace]     [3 Reliques]  [5 Hangar]    [7 Dragon]
 *     [0 Hall]      [2 Desert]    [4 Crypte]    [6 Vide]
 */
static const DefSalle SALLES_DEFAUT[] = {
    { "Hall du chateau", "Un hall froid et silencieux. Votre aventure commence ici.",
      DECOR_CHATEAU, -1 },
    { "Couloir de glace", "Le vent glacial siffle entre des parois gelees.",
      DECOR_GLACE, MODELE_BONHOMME_NEIGE },
    { "Desert des ossements", "Des ossements blanchis couvrent le sable brulant.",
      DECOR_DESERT, MODELE_SQUELETTE },
    { "Salle des reliques", "Des armes anciennes reposent sur des socles de pierre.",
      DECOR_CHATEAU, -1 },
    { "Crypte radioactive", "Une lueur verte baigne des cercueils entrouverts.",
      DECOR_NUCLEAIRE, MODELE_VAMPIRE },
    { "Hangar du vaisseau", "Un vaisseau echoue grince dans l'obscurite.",
      DECOR_VAISSEAU, MODELE_ALIEN },
    { "Vide stellaire", "Plus de sol : seulement des etoiles et une presence hostile.",
      DECOR_ESPACE, MODELE_EVIL },
    { "Antre du dragon", "Une chaleur etouffante : le gardien du donjon vous attend.",
      DECOR_CHATEAU, MODELE_DRAGON },
};

#define NB_SALLES_DEFAUT (sizeof(SALLES_DEFAUT) / sizeof(SALLES_DEFAUT[0]))

Resultat donjon_construire_defaut(Donjon *donjon)
{
    size_t i;
    size_t id;
    Resultat res;

    if (donjon == NULL) {
        return RES_ERR_NULL;
    }

    for (i = 0; i < NB_SALLES_DEFAUT; i++) {
        res = donjon_ajouter_salle(donjon, SALLES_DEFAUT[i].nom, SALLES_DEFAUT[i].description,
                                   SALLES_DEFAUT[i].decor, &id);
        if (res != RES_OK) {
            return res;
        }
        if (SALLES_DEFAUT[i].modele >= 0) {
            res = donjon_placer_ennemi(donjon, id, (size_t)SALLES_DEFAUT[i].modele);
            if (res != RES_OK) {
                return res;
            }
        }
    }

    /* Rangée du bas (0-2-4-6) et du haut (1-3-5-7), puis les liaisons nord-sud. */
    if ((res = donjon_relier(donjon, 0, DIR_EST, 2)) != RES_OK ||
        (res = donjon_relier(donjon, 2, DIR_EST, 4)) != RES_OK ||
        (res = donjon_relier(donjon, 4, DIR_EST, 6)) != RES_OK ||
        (res = donjon_relier(donjon, 1, DIR_EST, 3)) != RES_OK ||
        (res = donjon_relier(donjon, 3, DIR_EST, 5)) != RES_OK ||
        (res = donjon_relier(donjon, 5, DIR_EST, 7)) != RES_OK ||
        (res = donjon_relier(donjon, 0, DIR_NORD, 1)) != RES_OK ||
        (res = donjon_relier(donjon, 2, DIR_NORD, 3)) != RES_OK ||
        (res = donjon_relier(donjon, 4, DIR_NORD, 5)) != RES_OK ||
        (res = donjon_relier(donjon, 6, DIR_NORD, 7)) != RES_OK) {
        return res;
    }

    /* Objets au sol. */
    if ((res = donjon_deposer_objet(donjon, 0, "Potion de soin", TYPE_SOIN, 40)) != RES_OK ||
        (res = donjon_deposer_objet(donjon, 2, "Potion de mana", TYPE_MANA, 30)) != RES_OK ||
        (res = donjon_deposer_objet(donjon, 3, "Epee d'acier", TYPE_ARME, 3)) != RES_OK ||
        (res = donjon_deposer_objet(donjon, 3, "Bouclier runique", TYPE_ARMURE, 2)) != RES_OK ||
        (res = donjon_deposer_objet(donjon, 4, "Elixir de vie", TYPE_SOIN, 50)) != RES_OK ||
        (res = donjon_deposer_objet(donjon, 5, "Potion de soin", TYPE_SOIN, 40)) != RES_OK ||
        (res = donjon_deposer_objet(donjon, 6, "Armure stellaire", TYPE_ARMURE, 3)) != RES_OK ||
        (res = donjon_deposer_objet(donjon, 1, "Potion de mana", TYPE_MANA, 30)) != RES_OK) {
        return res;
    }

    donjon->salles[7].finale = 1;
    donjon->courante = 0;
    donjon->precedente = 0;
    donjon->salles[0].visitee = 1;
    return RES_OK;
}

void donjon_liberer(Donjon *donjon)
{
    size_t i;

    if (donjon == NULL) {
        return;
    }

    for (i = 0; i < donjon->nb_salles; i++) {
        ennemi_liberer(donjon->salles[i].ennemi);
        libererListeElements(donjon->salles[i].objets);
    }
    free(donjon->salles);
    memset(donjon, 0, sizeof(*donjon));
}
