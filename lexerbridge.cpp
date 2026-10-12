#include "lexerbridge.h"

QList<Token> ejecutarLexer(const QString &codigo);

QList<Token> analizarConFlex(const QString &codigo)
{
    return ejecutarLexer(codigo);
}

