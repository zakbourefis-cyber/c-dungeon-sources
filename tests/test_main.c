#include "../include/entity.h"
#include "../include/ui.h"

int main(void){
    Joueur j;
    initialiser_joueur(&j, "Zakevin");
    ajouter_inventaire(&j, "Epée", 40);
    afficher_ui(&j);
    
    
}