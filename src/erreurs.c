#include "erreurs.h"

const char *resultat_message(Resultat r)
{
    switch (r) {
    case RES_OK:             return "Succes";
    case RES_ERR_NULL:       return "Pointeur NULL";
    case RES_ERR_MEMOIRE:    return "Allocation memoire impossible";
    case RES_ERR_INDICE:     return "Indice invalide";
    case RES_ERR_INTROUVABLE:return "Element introuvable";
    case RES_ERR_ACTION:     return "Action impossible";
    }
    return "Erreur inconnue";
}
