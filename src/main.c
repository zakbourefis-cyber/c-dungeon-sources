/*
 * Lanceur des tests unitaires (cible `make test`).
 * Le jeu lui-même démarre dans game_main.c.
 */
#include <stdio.h>

void test_liste_chainee(void);
void test_entity(void);
void test_combat(void);
void test_donjon(void);
void test_jeu(void);

int main(void)
{
    test_liste_chainee();
    test_entity();
    test_combat();
    test_donjon();
    test_jeu();
    puts("Tous les tests ok");
    return 0;
}
