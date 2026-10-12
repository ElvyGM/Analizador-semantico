#include "parser.h"

namespace {

class Parser
{
public:
    explicit Parser(const QList<Token> &entrada)
    {
        for (const Token &token : entrada) {
            if (token.tipo != "COMENTARIO")
                tokens.append(token);
        }
    }

    ResultadoSintactico ejecutar()
    {
        agregarNodo("PROGRAMA");
        nivel++;
        while (!fin()) {
            const int posicionInicial = actual;
            sentencia();
            if (actual == posicionInicial)
                avanzar();
        }
        nivel--;

        ResultadoSintactico resultado;
        resultado.valido = errores.isEmpty();
        resultado.errores = errores;
        resultado.arbol = arbol.join('\n');
        return resultado;
    }

private:
    QList<Token> tokens;
    int actual = 0;
    int nivel = 0;
    QStringList errores;
    QStringList arbol;

    bool fin() const { return actual >= tokens.size(); }
    const Token *ver() const { return fin() ? nullptr : &tokens.at(actual); }
    const Token *anterior() const { return actual == 0 ? nullptr : &tokens.at(actual - 1); }

    void agregarNodo(const QString &texto)
    {
        arbol.append(QString(nivel * 2, ' ') + texto);
    }

    void error(const QString &mensaje)
    {
        const Token *token = ver();
        const int linea = token ? token->linea : (tokens.isEmpty() ? 1 : tokens.last().linea);
        const QString cerca = token ? QString(" cerca de '%1'").arg(token->lexema) : " al final del codigo";
        errores.append(QString("Linea %1: %2%3.").arg(linea).arg(mensaje, cerca));
    }

    bool coincide(const QString &lexema)
    {
        if (!fin() && ver()->lexema == lexema) {
            avanzar();
            return true;
        }
        return false;
    }

    bool tipo(const QString &tipoToken)
    {
        return !fin() && ver()->tipo == tipoToken;
    }

    Token avanzar()
    {
        if (!fin()) actual++;
        return tokens.at(actual - 1);
    }

    bool consumir(const QString &lexema, const QString &mensaje)
    {
        if (coincide(lexema)) return true;
        error(mensaje);
        return false;
    }

    bool esTipo() const
    {
        if (fin()) return false;
        const QString l = ver()->lexema;
        return l == "int" || l == "float" || l == "double" || l == "char" ||
               l == "bool" || l == "string" || l == "void";
    }

    void sentencia()
    {
        if (fin()) return;
        if (esTipo()) { declaracionOFuncion(); return; }
        if (coincide("if")) { sentenciaIf(); return; }
        if (coincide("while")) { sentenciaWhile(); return; }
        if (coincide("return")) { sentenciaReturn(); return; }
        if (coincide("{")) { bloqueAbierto(); return; }
        if (tipo("IDENTIFICADOR")) { asignacionOLlamada(); return; }
        error("Se esperaba una declaracion o sentencia");
        sincronizar();
    }

    void declaracionOFuncion()
    {
        const Token tokenTipo = avanzar();
        if (!tipo("IDENTIFICADOR")) {
            error("Se esperaba un identificador despues del tipo");
            sincronizar();
            return;
        }
        const Token nombre = avanzar();

        if (coincide("(")) {
            agregarNodo(QString("FUNCION %1 %2").arg(tokenTipo.lexema, nombre.lexema));
            nivel++;
            parametros();
            consumir(")", "Se esperaba ')' despues de los parametros");
            if (consumir("{", "Se esperaba '{' para iniciar el cuerpo de la funcion"))
                bloqueAbierto();
            nivel--;
            return;
        }

        agregarNodo(QString("DECLARACION %1 %2").arg(tokenTipo.lexema, nombre.lexema));
        nivel++;
        if (coincide("=")) {
            agregarNodo("INICIALIZADOR");
            nivel++;
            expresion();
            nivel--;
        }
        consumir(";", "Se esperaba ';' al final de la declaracion");
        nivel--;
    }

    void parametros()
    {
        if (!fin() && ver()->lexema == ")") return;
        do {
            if (!esTipo()) { error("Se esperaba el tipo del parametro"); return; }
            const Token t = avanzar();
            if (!tipo("IDENTIFICADOR")) { error("Se esperaba el nombre del parametro"); return; }
            agregarNodo(QString("PARAMETRO %1 %2").arg(t.lexema, avanzar().lexema));
        } while (coincide(","));
    }

    void bloqueAbierto()
    {
        agregarNodo("BLOQUE");
        nivel++;
        while (!fin() && ver()->lexema != "}")
            sentencia();
        consumir("}", "Se esperaba '}' para cerrar el bloque");
        nivel--;
    }

    void sentenciaIf()
    {
        agregarNodo("IF");
        nivel++;
        consumir("(", "Se esperaba '(' despues de if");
        expresion();
        consumir(")", "Se esperaba ')' despues de la condicion");
        if (consumir("{", "Se esperaba '{' despues de la condicion")) bloqueAbierto();
        if (coincide("else")) {
            agregarNodo("ELSE");
            nivel++;
            if (coincide("if")) sentenciaIf();
            else if (consumir("{", "Se esperaba '{' despues de else")) bloqueAbierto();
            nivel--;
        }
        nivel--;
    }

    void sentenciaWhile()
    {
        agregarNodo("WHILE");
        nivel++;
        consumir("(", "Se esperaba '(' despues de while");
        expresion();
        consumir(")", "Se esperaba ')' despues de la condicion");
        if (consumir("{", "Se esperaba '{' despues de la condicion")) bloqueAbierto();
        nivel--;
    }

    void sentenciaReturn()
    {
        agregarNodo("RETURN");
        nivel++;
        if (!fin() && ver()->lexema != ";") expresion();
        consumir(";", "Se esperaba ';' despues de return");
        nivel--;
    }

    void asignacionOLlamada()
    {
        const Token nombre = avanzar();
        if (coincide("=")) {
            agregarNodo(QString("ASIGNACION %1").arg(nombre.lexema));
            nivel++;
            expresion();
            consumir(";", "Se esperaba ';' despues de la asignacion");
            nivel--;
            return;
        }
        if (coincide("(")) {
            agregarNodo(QString("LLAMADA %1").arg(nombre.lexema));
            nivel++;
            argumentos();
            consumir(")", "Se esperaba ')' despues de los argumentos");
            consumir(";", "Se esperaba ';' despues de la llamada");
            nivel--;
            return;
        }
        error("Se esperaba '=' o '(' despues del identificador");
        sincronizar();
    }

    void argumentos()
    {
        if (!fin() && ver()->lexema == ")") return;
        do { expresion(); } while (coincide(","));
    }

    void expresion() { igualdad(); }

    void igualdad()
    {
        comparacion();
        while (!fin() && (ver()->lexema == "==" || ver()->lexema == "!=")) {
            const QString op = avanzar().lexema;
            agregarNodo(QString("OPERADOR %1").arg(op));
            comparacion();
        }
    }

    void comparacion()
    {
        termino();
        while (!fin() && (ver()->lexema == "<" || ver()->lexema == "<=" ||
                          ver()->lexema == ">" || ver()->lexema == ">=")) {
            const QString op = avanzar().lexema;
            agregarNodo(QString("OPERADOR %1").arg(op));
            termino();
        }
    }

    void termino()
    {
        factor();
        while (!fin() && (ver()->lexema == "+" || ver()->lexema == "-")) {
            const QString op = avanzar().lexema;
            agregarNodo(QString("OPERADOR %1").arg(op));
            factor();
        }
    }

    void factor()
    {
        unario();
        while (!fin() && (ver()->lexema == "*" || ver()->lexema == "/" || ver()->lexema == "%")) {
            const QString op = avanzar().lexema;
            agregarNodo(QString("OPERADOR %1").arg(op));
            unario();
        }
    }

    void unario()
    {
        if (!fin() && (ver()->lexema == "!" || ver()->lexema == "-")) {
            agregarNodo(QString("UNARIO %1").arg(avanzar().lexema));
            unario();
            return;
        }
        primario();
    }

    void primario()
    {
        if (fin()) { error("Se esperaba una expresion"); return; }
        if (coincide("(")) {
            agregarNodo("AGRUPACION");
            nivel++;
            expresion();
            consumir(")", "Se esperaba ')' despues de la expresion");
            nivel--;
            return;
        }
        if (tipo("IDENTIFICADOR") || tipo("ENTERO") || tipo("REAL") ||
            tipo("CADENA") || tipo("BOOLEANO")) {
            const Token valor = avanzar();
            agregarNodo(QString("%1 %2").arg(valor.tipo, valor.lexema));
            return;
        }
        error("Se esperaba un identificador, literal o expresion entre parentesis");
        avanzar();
    }

    void sincronizar()
    {
        while (!fin()) {
            if (anterior() && anterior()->lexema == ";") return;
            if (ver()->lexema == "}" || esTipo() || ver()->lexema == "if" ||
                ver()->lexema == "while" || ver()->lexema == "return") return;
            avanzar();
        }
    }
};

}

ResultadoSintactico analizarSintaxis(const QList<Token> &tokens)
{
    return Parser(tokens).ejecutar();
}
