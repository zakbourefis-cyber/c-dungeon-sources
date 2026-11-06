#include <stdarg.h>
#include <stdio.h>

#include "debug.h"

void debug_log(const char *fichier, int ligne, const char *format, ...)
{
    FILE *journal = fopen(FICHIER_DEBUG, "a");
    va_list args;

    if (journal == NULL) {
        return;
    }
    fprintf(journal, "[%s:%d] ", fichier, ligne);
    va_start(args, format);
    vfprintf(journal, format, args);
    va_end(args);
    fputc('\n', journal);
    fclose(journal);
}
