#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "display.h"
#include "jeu.h"

static void afficher_aide(const char *programme)
{
    size_t i;

    printf("Usage : %s [regles]\n\nRegles disponibles :\n", programme);
    for (i = 0; i < regles_nombre(); i++) {
        printf("  %s%s\n", regles_indice(i)->nom,
               regles_indice(i) == regles_defaut() ? "  (par defaut)" : "");
    }
    printf("\nTouches : fleches ou N/E/S/O pour se deplacer, R ramasser,\n"
           "1-9 utiliser un objet, A attaquer, S sort, F fuir, Q quitter.\n"
           "Terminal minimum : 80 colonnes x 24 lignes (plus grand = meilleur affichage).\n");
}

/* Relance une partie avec les mêmes règles. Retourne 0 si OK, -1 sinon. */
static int redemarrer(Jeu *jeu)
{
    const Regles *regles = jeu->regles;

    jeu_liberer(jeu);
    return jeu_init(jeu, regles) == RES_OK ? 0 : -1;
}

/* Traduit une touche en action. Retourne 1 pour quitter, 0 sinon. */
static int traiter_touche(Jeu *jeu, int touche)
{
    if (touche == 'q') {
        return 1;
    }
    if (touche == TOUCHE_AUCUNE) {
        return 0;
    }

    if (jeu->etat == ETAT_VICTOIRE || jeu->etat == ETAT_DEFAITE) {
        return touche == 'r' ? redemarrer(jeu) != 0 : 0;
    }

    if (touche >= '1' && touche <= '9') {
        jeu_utiliser_objet(jeu, (size_t)(touche - '1'));
        return 0;
    }

    if (jeu->etat == ETAT_COMBAT) {
        switch (touche) {
        case 'a': jeu_attaquer(jeu);    break;
        case 's': jeu_lancer_sort(jeu); break;
        case 'f': jeu_fuir(jeu);        break;
        default:
            snprintf(jeu->message, TAILLE_MESSAGE, "Commande inconnue en combat.");
            break;
        }
        return 0;
    }

    switch (touche) {
    case TOUCHE_HAUT:   case 'n': jeu_deplacer(jeu, DIR_NORD);  break;
    case TOUCHE_DROITE: case 'e': jeu_deplacer(jeu, DIR_EST);   break;
    case TOUCHE_BAS:    case 's': jeu_deplacer(jeu, DIR_SUD);   break;
    case TOUCHE_GAUCHE: case 'o': jeu_deplacer(jeu, DIR_OUEST); break;
    case 'r':                     jeu_ramasser(jeu);            break;
    default:
        snprintf(jeu->message, TAILLE_MESSAGE, "Commande inconnue.");
        break;
    }
    return 0;
}

int main(int argc, char **argv)
{
    Jeu jeu;
    const Regles *regles = regles_defaut();
    Resultat res;
    int quitter = 0;

    if (argc > 1) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            afficher_aide(argv[0]);
            return EXIT_SUCCESS;
        }
        regles = regles_trouver(argv[1]);
        if (regles == NULL) {
            fprintf(stderr, "Regles inconnues : %s\n\n", argv[1]);
            afficher_aide(argv[0]);
            return EXIT_FAILURE;
        }
    }

    srand((unsigned int)time(NULL));

    res = jeu_init(&jeu, regles);
    if (res != RES_OK) {
        fprintf(stderr, "Impossible de creer la partie : %s\n", resultat_message(res));
        return EXIT_FAILURE;
    }

    if (display_init() != 0) {
        fprintf(stderr, "Impossible d'initialiser l'affichage : terminal trop petit "
                        "(minimum 80x24) ou ncurses indisponible.\n");
        jeu_liberer(&jeu);
        return EXIT_FAILURE;
    }

    while (!quitter) {
        display_render(&jeu);
        quitter = traiter_touche(&jeu, display_get_action());
    }

    display_shutdown();
    jeu_liberer(&jeu);
    return EXIT_SUCCESS;
}
