#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <gtk/gtk.h>

#include "jeu.h"
#include "liste_element.h"

typedef struct ApplicationJeu {
    Jeu jeu;
    GtkWidget *window;
    GtkWidget *room_label;
    GtkWidget *description_label;
    GtkWidget *player_label;
    GtkWidget *enemy_label;
    GtkWidget *inventory_combo;
    GtkWidget *message_label;
    GtkWidget *move_buttons[NB_DIRECTIONS];
    GtkWidget *pickup_button;
    GtkWidget *attack_button;
    GtkWidget *spell_button;
    GtkWidget *run_button;
    GtkWidget *use_button;
    int initialized;
} ApplicationJeu;

static void actualiser_interface(ApplicationJeu *app);

static void afficher_erreur(ApplicationJeu *app, const char *message)
{
    gtk_label_set_text(GTK_LABEL(app->message_label), message);
}

static void action_deplacer(GtkWidget *widget, gpointer data)
{
    ApplicationJeu *app = data;
    Direction direction = (Direction)GPOINTER_TO_INT(
        g_object_get_data(G_OBJECT(widget), "direction"));
    (void)jeu_deplacer(&app->jeu, direction);
    actualiser_interface(app);
}

static void action_ramasser(GtkWidget *widget, gpointer data)
{
    ApplicationJeu *app = data;
    (void)widget;
    (void)jeu_ramasser(&app->jeu);
    actualiser_interface(app);
}

static void action_attaquer(GtkWidget *widget, gpointer data)
{
    ApplicationJeu *app = data;
    (void)widget;
    (void)jeu_attaquer(&app->jeu);
    actualiser_interface(app);
}

static void action_sort(GtkWidget *widget, gpointer data)
{
    ApplicationJeu *app = data;
    (void)widget;
    (void)jeu_lancer_sort(&app->jeu);
    actualiser_interface(app);
}

static void action_fuir(GtkWidget *widget, gpointer data)
{
    ApplicationJeu *app = data;
    (void)widget;
    (void)jeu_fuir(&app->jeu);
    actualiser_interface(app);
}

static void action_utiliser(GtkWidget *widget, gpointer data)
{
    ApplicationJeu *app = data;
    int indice = gtk_combo_box_get_active(GTK_COMBO_BOX(app->inventory_combo));
    (void)widget;

    if (indice >= 0) {
        (void)jeu_utiliser_objet(&app->jeu, (size_t)indice);
    } else {
        afficher_erreur(app, "Choisissez un objet dans l'inventaire.");
    }
    actualiser_interface(app);
}

static void action_recommencer(GtkWidget *widget, gpointer data)
{
    ApplicationJeu *app = data;
    Resultat resultat;
    (void)widget;

    if (app->initialized) {
        jeu_liberer(&app->jeu);
        app->initialized = 0;
    }
    resultat = jeu_init(&app->jeu, app->jeu.regles);
    if (resultat == RES_OK) {
        app->initialized = 1;
        actualiser_interface(app);
    } else {
        afficher_erreur(app, "Impossible de recommencer la partie.");
    }
}

static void actualiser_interface(ApplicationJeu *app)
{
    Salle *salle = jeu_salle_courante(&app->jeu);
    Ennemi *ennemi = jeu_ennemi_courant(&app->jeu);
    Element *objet;
    char texte[512];
    size_t index = 0;
    int en_combat = app->jeu.etat == ETAT_COMBAT;
    int partie_finie = app->jeu.etat == ETAT_VICTOIRE ||
                       app->jeu.etat == ETAT_DEFAITE;
    int direction;

    if (salle != NULL) {
        snprintf(texte, sizeof(texte), "Salle : %s", salle->nom);
        gtk_label_set_text(GTK_LABEL(app->room_label), texte);
        gtk_label_set_text(GTK_LABEL(app->description_label), salle->description);
    }

    snprintf(texte, sizeof(texte),
             "%s  |  Niveau %d  |  PV : %d/%d  |  Mana : %d/%d\n"
             "Attaque : %d  |  Défense : %d  |  Or : %d  |  Score : %d",
             app->jeu.joueur.nom, app->jeu.joueur.niveau,
             app->jeu.joueur.stats.hp, app->jeu.joueur.stats.max_hp,
             app->jeu.joueur.mana, app->jeu.joueur.max_mana,
             app->jeu.joueur.stats.attaque, app->jeu.joueur.stats.defense,
             app->jeu.joueur.gold, app->jeu.score);
    gtk_label_set_text(GTK_LABEL(app->player_label), texte);

    if (ennemi != NULL) {
        snprintf(texte, sizeof(texte), "Ennemi : %s  |  PV : %d/%d",
                 ennemi->nom, ennemi->stats.hp, ennemi->stats.max_hp);
    } else {
        snprintf(texte, sizeof(texte), "Aucun ennemi dans cette salle.");
    }
    gtk_label_set_text(GTK_LABEL(app->enemy_label), texte);

    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(app->inventory_combo));
    for (objet = app->jeu.joueur.inventaire; objet != NULL; objet = objet->suivant) {
        snprintf(texte, sizeof(texte), "%s (%s, valeur %d)",
                 objet->nom, objet->type, objet->valeur);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app->inventory_combo), texte);
        index++;
    }
    if (index > 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(app->inventory_combo), 0);
    }

    gtk_widget_set_sensitive(app->use_button, index > 0 && !partie_finie);
    gtk_widget_set_sensitive(app->pickup_button,
                             app->jeu.etat == ETAT_EXPLORATION && !partie_finie);
    gtk_widget_set_sensitive(app->attack_button, en_combat);
    gtk_widget_set_sensitive(app->spell_button, en_combat);
    gtk_widget_set_sensitive(app->run_button, en_combat);
    for (direction = 0; direction < NB_DIRECTIONS; direction++) {
        gtk_widget_set_sensitive(app->move_buttons[direction],
                                 app->jeu.etat == ETAT_EXPLORATION);
    }

    if (partie_finie) {
        snprintf(texte, sizeof(texte), "%s  Partie terminée. Cliquez sur Recommencer.",
                 app->jeu.etat == ETAT_VICTOIRE ? "VICTOIRE !" : "DÉFAITE.");
        gtk_label_set_text(GTK_LABEL(app->message_label), texte);
    } else {
        gtk_label_set_text(GTK_LABEL(app->message_label), app->jeu.message);
    }
}

static GtkWidget *ajouter_bouton(GtkWidget *box, const char *label,
                                 GCallback callback, gpointer data)
{
    GtkWidget *button = gtk_button_new_with_label(label);
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 3);
    g_signal_connect(button, "clicked", callback, data);
    return button;
}

static void detruire_fenetre(GtkWidget *widget, gpointer data)
{
    ApplicationJeu *app = data;
    (void)widget;

    if (app->initialized) {
        jeu_liberer(&app->jeu);
        app->initialized = 0;
    }
    gtk_main_quit();
}

int main(int argc, char **argv)
{
    ApplicationJeu app = {0};
    const Regles *regles = NULL;
    GtkWidget *root;
    GtkWidget *section;
    GtkWidget *row;
    GtkWidget *label;
    int i;
    const char *noms_directions[NB_DIRECTIONS] = {
        "Nord", "Est", "Sud", "Ouest"
    };

    gtk_init(&argc, &argv);
    srand((unsigned int)time(NULL));

    if (argc > 1) {
        regles = regles_trouver(argv[1]);
        if (regles == NULL) {
            fprintf(stderr, "Règles inconnues : %s (simple, defense, critique)\n", argv[1]);
            return EXIT_FAILURE;
        }
    }
    if (jeu_init(&app.jeu, regles) != RES_OK) {
        fprintf(stderr, "Impossible d'initialiser le jeu.\n");
        return EXIT_FAILURE;
    }
    app.initialized = 1;

    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app.window), "C-Dungeon — interface GTK");
    gtk_window_set_default_size(GTK_WINDOW(app.window), 620, 560);
    gtk_container_set_border_width(GTK_CONTAINER(app.window), 12);
    g_signal_connect(app.window, "destroy", G_CALLBACK(detruire_fenetre), &app);

    root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_add(GTK_CONTAINER(app.window), root);

    label = gtk_label_new("C-Dungeon");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), label, FALSE, FALSE, 0);

    app.room_label = gtk_label_new("");
    gtk_widget_set_halign(app.room_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), app.room_label, FALSE, FALSE, 0);

    app.description_label = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(app.description_label), TRUE);
    gtk_widget_set_halign(app.description_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), app.description_label, FALSE, FALSE, 0);

    app.player_label = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(app.player_label), TRUE);
    gtk_widget_set_halign(app.player_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), app.player_label, FALSE, FALSE, 0);

    app.enemy_label = gtk_label_new("");
    gtk_widget_set_halign(app.enemy_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), app.enemy_label, FALSE, FALSE, 0);

    section = gtk_frame_new("Déplacement");
    gtk_box_pack_start(GTK_BOX(root), section, FALSE, FALSE, 0);
    row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_set_border_width(GTK_CONTAINER(row), 6);
    gtk_container_add(GTK_CONTAINER(section), row);
    for (i = 0; i < NB_DIRECTIONS; i++) {
        app.move_buttons[i] = ajouter_bouton(row, noms_directions[i],
                                             G_CALLBACK(action_deplacer), &app);
        g_object_set_data(G_OBJECT(app.move_buttons[i]), "direction", GINT_TO_POINTER(i));
    }

    row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_pack_start(GTK_BOX(root), row, FALSE, FALSE, 0);
    app.pickup_button = ajouter_bouton(row, "Ramasser", G_CALLBACK(action_ramasser), &app);
    app.attack_button = ajouter_bouton(row, "Attaquer", G_CALLBACK(action_attaquer), &app);
    app.spell_button = ajouter_bouton(row, "Lancer un sort", G_CALLBACK(action_sort), &app);
    app.run_button = ajouter_bouton(row, "Fuir", G_CALLBACK(action_fuir), &app);

    section = gtk_frame_new("Inventaire");
    gtk_box_pack_start(GTK_BOX(root), section, FALSE, FALSE, 0);
    row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_container_set_border_width(GTK_CONTAINER(row), 6);
    gtk_container_add(GTK_CONTAINER(section), row);
    app.inventory_combo = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(row), app.inventory_combo, TRUE, TRUE, 0);
    app.use_button = ajouter_bouton(row, "Utiliser", G_CALLBACK(action_utiliser), &app);

    app.message_label = gtk_label_new("");
    gtk_label_set_line_wrap(GTK_LABEL(app.message_label), TRUE);
    gtk_widget_set_halign(app.message_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(root), app.message_label, TRUE, TRUE, 4);

    (void)ajouter_bouton(root, "Recommencer", G_CALLBACK(action_recommencer), &app);

    actualiser_interface(&app);
    gtk_widget_show_all(app.window);
    gtk_main();
    return EXIT_SUCCESS;
}
