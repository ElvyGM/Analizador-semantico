#pragma once

#include "token.h"

#include <QList>
#include <QString>
#include <QStringList>

struct ResultadoSintactico
{
    bool valido = false;
    QStringList errores;
    QString arbol;
};

ResultadoSintactico analizarSintaxis(const QList<Token> &tokens);
