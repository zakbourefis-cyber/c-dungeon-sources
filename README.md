# C-Dungeon

RPG en terminal écrit en C (projet L3 Informatique). Le joueur explore un donjon
de 8 salles, combat des monstres, ramasse et utilise des objets, gagne de
l'expérience et affronte un dragon.

## Compiler et jouer

Prérequis : `gcc`/`cc`, `make`, `libncursesw5-dev` (Linux/WSL).
Optionnels : `valgrind` (`make memcheck`), `doxygen` (`make docs`), GTK 3 (`libgtk-3-dev` et `pkg-config`) pour l'interface graphique.

```bash
make            # bibliothèques + tests + jeu
make test       # tests unitaires
make run        # lance le jeu ncurses (règles par défaut)
make gtk        # compile et lance l'interface graphique GTK 3
make gtk RULES=critique  # interface GTK avec les règles critiques
make run RULES=critique
make MODE=release   # compilation optimisée, sans traces de debug
make memcheck   # tests sous Valgrind
make docs       # documentation Doxygen dans docs/html/index.html
make clean
```

**Lancez le jeu depuis la racine du projet** (les décors et monstres ASCII sont
chargés avec des chemins relatifs `include/assets/...`) et dans un terminal
d'au moins **80×24** ; plus grand = meilleur affichage (les dessins des
monstres demandent environ 40 lignes).

## Comment jouer

| Touche | Action |
|---|---|
| Flèches ou `N` `E` `S` `O` | se déplacer (hors combat) |
| `R` | ramasser tous les objets de la salle |
| `1`-`9` | utiliser l'objet correspondant de l'inventaire (coûte le tour en combat) |
| `A` | attaquer |
| `S` | lancer un sort (dégâts doublés, 10 mana) |
| `F` | tenter de fuir (50 %) vers la salle précédente |
| `R` (fin de partie) | rejouer |
| `Q` | quitter |

Le donjon est une grille 4×2 ; la salle de départ est en bas à gauche, le
**dragon** garde la salle en haut à droite. Vaincre le dragon gagne la partie.
Chaque salle donne un événement aléatoire à la première visite (coffre, piège,
fontaine, bénédiction). La salle du Vide stellaire est facultative mais
dangereuse. Les sorts sont indispensables : pensez à garder du mana.

### Règles de calcul interchangeables

`./c-dungeon [simple|defense|critique]` (par défaut `defense`)

| Règles | Dégâts | Score | Difficulté |
|---|---|---|---|
| `simple` | l'attaque brute, défense ignorée (pour tout le monde) | 100/ennemi + or | difficile |
| `defense` | attaque − défense (minimum 1) | 100/ennemi + or | équilibrée |
| `critique` | comme `defense`, ×2 avec 20 % de chance | + bonus de rapidité | variable |

## Architecture

```
include/ + src/
  erreurs      codes de retour (enum Resultat) communs à tous les modules
  debug        macro DEBUG_LOG (journal cdungeon-debug.log, mode debug seulement)
  liste_element  liste chaînée générique ................ libListeElement.a
  entity       Stats, Joueur, inventaire (liste d'Element)
  item         effets des objets  ..................... pointeurs de fonctions
  enemy        ennemis et comportements ............... pointeurs de fonctions
  combat       calcul des dégâts / du score, règles ... pointeurs de fonctions
  evenement    événements aléatoires .................. pointeurs de fonctions
  dungeon      salles, donjon (calloc/realloc)
  jeu          moteur de partie (machine à états) ..... libcdungeon.a (avec les 6 modules ci-dessus)
  display      interface ncursesw (lecture seule de Jeu)
  game_main.c  boucle principale : touches -> actions jeu_*
tests/         un fichier de tests par module ; src/main.c les lance tous
```

* **La logique du jeu ne dépend pas de ncurses** : `libcdungeon.a` se teste seule
  (`make test`) ; `display.c` ne fait que lire la structure `Jeu`.
* `libListeElement.a` ne dépend de rien ; `libcdungeon.a` dépend d'elle.

### Pointeurs de fonctions

Un pointeur de fonction est une variable qui contient l'adresse d'une fonction ;
on l'appelle comme une fonction. Le code appelant ne sait pas laquelle il
exécute : on change le comportement en changeant le pointeur.

| Type | Où | Choisit… |
|---|---|---|
| `FonctionDegats` | `combat.h` | le calcul des dégâts (`degats_simples`, `degats_avec_defense`, `degats_critique`) |
| `FonctionScore` | `combat.h` | le calcul du score (`score_simple`, `score_rapidite`) |
| `ComportementEnnemi` | `enemy.h` | l'IA d'un ennemi (`comportement_agressif`, `_prudent`, `_capricieux`) |
| `FonctionEffet` | `item.h` | l'effet d'un type d'objet (soin, mana, arme, armure) |
| `FonctionEvenement` | `evenement.h` | l'événement d'une première visite |

La structure `Regles` regroupe `degats` et `score` : `combat_attaque_joueur()`
appelle `regles->degats(...)` sans connaître la stratégie. **Pour ajouter une
règle** : écrire la fonction, ajouter une ligne dans `TABLE_REGLES`
(`combat.c`). Même principe pour un nouveau type d'objet (`TABLE_EFFETS`
dans `item.c`), un événement (`TABLE_EVENEMENTS`) ou un monstre
(`MODELES` dans `enemy.c`).

### Mémoire dynamique : qui possède quoi ?

| Ressource | Allocation | Propriétaire | Libération |
|---|---|---|---|
| nœuds de liste (`Element`) | `malloc` | la liste (inventaire du joueur ou objets d'une salle) | `libererListeElements` |
| objet qui change de propriétaire | aucune (le nœud est déplacé) | nouvelle liste | `transfererElementIndice` |
| `Ennemi` | `calloc` (`ennemi_creer`) | sa `Salle` | `ennemi_liberer` (mis à NULL à sa mort) |
| tableau de `Salle` | `calloc` puis `realloc` | le `Donjon` | `donjon_liberer` (libère aussi ennemis et objets) |

Les salles sont reliées par **indices** et non par pointeurs, car `realloc`
peut déplacer le tableau et rendrait des pointeurs invalides.
Toutes les allocations sont testées ; `realloc` passe par un pointeur
temporaire pour ne pas perdre le tableau en cas d'échec.

### Gestion des erreurs

Aucune fonction de bibliothèque n'appelle `exit()` ni n'écrit dans le terminal :
elles renvoient un `Resultat` (`RES_OK`, `RES_ERR_NULL`, `RES_ERR_MEMOIRE`,
`RES_ERR_INDICE`, `RES_ERR_INTROUVABLE`, `RES_ERR_ACTION`) documenté dans
chaque fonction (Doxygen). `resultat_message()` donne un texte lisible.
Seul `game_main.c` décide d'afficher une erreur ou de quitter.

### Préprocesseur

* Gardes d'inclusion dans tous les en-têtes.
* `make MODE=debug` (défaut) : `-g -O0 -DDEBUG`, active `DEBUG_LOG`
  (écrit dans `cdungeon-debug.log`, jamais à l'écran).
* `make MODE=release` : `-O2 -DNDEBUG`, `DEBUG_LOG` disparaît du binaire.
* Les tests sont toujours compilés avec `-UNDEBUG` pour que leurs `assert`
  restent actifs même en release.
* Compilation stricte partout : `-Wall -Wextra -Werror -std=c11`.

## Tests

`make test` exécute 5 suites (liste chaînée, joueur/objets, combat/règles/
comportements, donjon/mémoire dynamique, moteur de jeu avec victoire, défaite
et fuite complètes), sans aucune dépendance à ncurses. `make memcheck` les
passe sous Valgrind.

## Fichiers hérités à nettoyer (non utilisés)

`src/ui.c`, `include/ui.h`, `tests/test_main.c`, `src/actions.c`,
`include/actions.h`, `include/utils.h` ne sont référencés par aucune cible du
Makefile (voir le rapport d'audit). Les binaires versionnés (`build/`,
`c-dungeon`, `test_main`, `include/entity.exe`) sont désormais ignorés par
`.gitignore` mais doivent être retirés de l'index avec `git rm --cached`.

