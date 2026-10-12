#pragma once

#include <QString>

struct Token
{
    QString tipo;
    QString lexema;
    int linea = 1;
};

