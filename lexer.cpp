#include "token.h"

#include <QList>
#include <QSet>
#include <QString>

namespace {
const QSet<QString> palabrasReservadas = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if", "int",
    "long", "register", "return", "short", "signed", "sizeof", "static", "struct",
    "switch", "typedef", "union", "unsigned", "void", "volatile", "while", "class",
    "public", "private", "protected", "new", "delete", "namespace", "using"
};

void agregar(QList<Token> &tokens, const QString &tipo, const QString &lexema, int linea)
{
    tokens.append({tipo, lexema, linea});
}
}

QList<Token> ejecutarLexer(const QString &codigo)
{
    QList<Token> tokens;
    int i = 0;
    int linea = 1;

    while (i < codigo.size()) {
        const QChar c = codigo.at(i);

        if (c.isSpace()) {
            if (c == '\n') ++linea;
            ++i;
            continue;
        }

        if (c == '/' && i + 1 < codigo.size() && codigo.at(i + 1) == '/') {
            const int inicio = i;
            while (i < codigo.size() && codigo.at(i) != '\n') ++i;
            agregar(tokens, "COMENTARIO", codigo.mid(inicio, i - inicio), linea);
            continue;
        }

        if (c == '/' && i + 1 < codigo.size() && codigo.at(i + 1) == '*') {
            const int inicio = i;
            const int lineaInicio = linea;
            i += 2;
            while (i < codigo.size() && !(codigo.at(i) == '*' && i + 1 < codigo.size() && codigo.at(i + 1) == '/')) {
                if (codigo.at(i) == '\n') ++linea;
                ++i;
            }
            if (i < codigo.size()) i += 2;
            agregar(tokens, "COMENTARIO", codigo.mid(inicio, i - inicio), lineaInicio);
            continue;
        }

        if (c.isLetter() || c == '_') {
            const int inicio = i++;
            while (i < codigo.size() && (codigo.at(i).isLetterOrNumber() || codigo.at(i) == '_')) ++i;
            const QString lexema = codigo.mid(inicio, i - inicio);
            QString tipo = palabrasReservadas.contains(lexema) ? "PALABRA RESERVADA" : "IDENTIFICADOR";
            if (lexema == "true" || lexema == "false") tipo = "BOOLEANO";
            agregar(tokens, tipo, lexema, linea);
            continue;
        }

        if (c.isDigit()) {
            const int inicio = i++;
            bool punto = false;
            while (i < codigo.size() && (codigo.at(i).isDigit() || (!punto && codigo.at(i) == '.'))) {
                if (codigo.at(i) == '.') punto = true;
                ++i;
            }
            agregar(tokens, punto ? "REAL" : "ENTERO", codigo.mid(inicio, i - inicio), linea);
            continue;
        }

        if (c == '"' || c == '\'') {
            const QChar comilla = c;
            const int inicio = i++;
            const int lineaInicio = linea;
            bool cerrada = false;
            while (i < codigo.size()) {
                if (codigo.at(i) == '\\' && i + 1 < codigo.size()) { i += 2; continue; }
                if (codigo.at(i) == comilla) { ++i; cerrada = true; break; }
                if (codigo.at(i) == '\n') ++linea;
                ++i;
            }
            agregar(tokens, cerrada ? "CADENA" : "ERROR LEXICO", codigo.mid(inicio, i - inicio), lineaInicio);
            continue;
        }

        const QString doble = codigo.mid(i, 2);
        static const QSet<QString> operadoresDobles = {"==", "!=", "<=", ">=", "++", "--", "&&", "||", "+=", "-=", "*=", "/="};
        if (operadoresDobles.contains(doble)) {
            agregar(tokens, "OPERADOR", doble, linea);
            i += 2;
            continue;
        }

        if (QString("+-*/%=<>!&|").contains(c)) {
            agregar(tokens, "OPERADOR", QString(c), linea);
        } else if (QString("(){}[];,.: ").contains(c) && c != ' ') {
            agregar(tokens, "DELIMITADOR", QString(c), linea);
        } else {
            agregar(tokens, "ERROR LEXICO", QString(c), linea);
        }
        ++i;
    }

    return tokens;
}

