#ifndef ERREURS_H
#define ERREURS_H

/**
 * @file erreurs.h
 * @brief Codes de retour communs à tous les modules de C-Dungeon.
 *
 * Les fonctions de bibliothèque ne terminent jamais le programme (pas de
 * exit) et n'écrivent rien dans le terminal : elles renvoient un Resultat
 * et laissent l'appelant décider quoi faire.
 */

/** @brief Code de retour des fonctions pouvant échouer. */
typedef enum Resultat {
    RES_OK = 0,           /**< Succès. */
    RES_ERR_NULL,         /**< Un pointeur obligatoire valait NULL. */
    RES_ERR_MEMOIRE,      /**< Échec d'allocation (malloc/calloc/realloc). */
    RES_ERR_INDICE,       /**< Indice ou identifiant hors limites. */
    RES_ERR_INTROUVABLE,  /**< Objet, passage ou ressource inexistant. */
    RES_ERR_ACTION        /**< Action impossible dans l'état actuel du jeu. */
} Resultat;

/**
 * @brief Donne une description lisible d'un code de retour.
 *
 * @param r Code à décrire.
 * @return Chaîne statique (ne pas libérer), jamais NULL.
 */
const char *resultat_message(Resultat r);

#endif
