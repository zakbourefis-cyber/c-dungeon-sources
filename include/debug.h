#ifndef DEBUG_H
#define DEBUG_H

/**
 * @file debug.h
 * @brief Macros de débogage, actives uniquement si DEBUG est défini.
 *
 * `make MODE=debug` définit DEBUG ; `make MODE=release` ne le définit pas et
 * les appels DEBUG_LOG disparaissent complètement du binaire.
 *
 * Les messages sont écrits dans le fichier FICHIER_DEBUG (et non à l'écran),
 * car écrire sur stderr abîmerait l'affichage ncurses.
 */

/** @brief Nom du fichier de journal (créé dans le répertoire courant). */
#define FICHIER_DEBUG "cdungeon-debug.log"

#if defined(__GNUC__)
/** @brief Permet à GCC de vérifier le format de debug_log comme printf. */
#define DEBUG_FORMAT_PRINTF __attribute__((format(printf, 3, 4)))
#else
/** @brief Sans effet avec un compilateur autre que GCC/Clang. */
#define DEBUG_FORMAT_PRINTF
#endif

/**
 * @brief Ajoute une ligne au fichier de journal (ne fait rien si le fichier
 *        ne peut pas être ouvert). Utiliser la macro DEBUG_LOG.
 *
 * @param fichier Nom du fichier source appelant.
 * @param ligne Numéro de ligne appelant.
 * @param format Format de type printf.
 */
void debug_log(const char *fichier, int ligne, const char *format, ...) DEBUG_FORMAT_PRINTF;

#ifdef DEBUG
/** @brief Écrit un message formaté dans le journal (mode debug uniquement). */
#define DEBUG_LOG(...) debug_log(__FILE__, __LINE__, __VA_ARGS__)
#else
/** @brief Sans effet en mode release. */
#define DEBUG_LOG(...) ((void)0)
#endif

#endif
