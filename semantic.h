#pragma once

#include "token.h"

#include <QList>
#include <QString>
#include <QStringList>

struct SimboloSemantico
{
    QString nombre;
    QString tipo;
    QString categoria;
    QString ambito;
    int linea = 1;
};

struct ResultadoSemantico
{
    bool valido = false;
    QStringList errores;
    QList<SimboloSemantico> simbolos;
};

ResultadoSemantico analizarSemantica(const QList<Token> &tokens);
