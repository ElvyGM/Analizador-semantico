#include "semantic.h"

#include <QHash>
#include <QSet>

namespace {

class AnalizadorSemantico
{
public:
    explicit AnalizadorSemantico(const QList<Token> &entrada)
    {
        for (const Token &token : entrada) {
            if (token.tipo != "COMENTARIO")
                tokens.append(token);
        }
        abrirAmbito("global");
    }

    ResultadoSemantico ejecutar()
    {
        registrarFunciones();
        actual = 0;
        while (!fin()) sentencia();

        ResultadoSemantico resultado;
        resultado.valido = errores.isEmpty();
        resultado.errores = errores;
        resultado.simbolos = listaSimbolos;
        return resultado;
    }

private:
    QList<Token> tokens;
    int actual = 0;
    QStringList errores;
    QList<SimboloSemantico> listaSimbolos;
    QList<QHash<QString, SimboloSemantico>> tablaPorAmbito;
    QStringList nombresAmbitos;
    QString tipoRetornoActual;
    int contadorBloques = 0;

    bool fin() const { return actual >= tokens.size(); }
    const Token *ver(int desplazamiento = 0) const
    {
        const int indice = actual + desplazamiento;
        return indice >= 0 && indice < tokens.size() ? &tokens.at(indice) : nullptr;
    }
    Token avanzar() { return tokens.at(actual++); }
    bool coincide(const QString &lexema)
    {
        if (!fin() && ver()->lexema == lexema) { actual++; return true; }
        return false;
    }

    bool esTipo(const Token *token) const
    {
        if (!token) return false;
        static const QSet<QString> tipos = {"int", "float", "double", "char", "bool", "string", "void"};
        return tipos.contains(token->lexema);
    }

    void abrirAmbito(const QString &nombre)
    {
        tablaPorAmbito.append(QHash<QString, SimboloSemantico>{});
        nombresAmbitos.append(nombre);
    }
    void cerrarAmbito()
    {
        if (tablaPorAmbito.size() > 1) {
            tablaPorAmbito.removeLast();
            nombresAmbitos.removeLast();
        }
    }

    void informar(const Token &token, const QString &mensaje)
    {
        errores.append(QString("Linea %1: %2").arg(token.linea).arg(mensaje));
    }

    bool declarar(const Token &nombre, const QString &tipo, const QString &categoria)
    {
        QHash<QString, SimboloSemantico> &ambito = tablaPorAmbito.last();
        if (ambito.contains(nombre.lexema)) {
            informar(nombre, QString("'%1' ya fue declarado en el ambito %2.")
                             .arg(nombre.lexema, nombresAmbitos.last()));
            return false;
        }
        const SimboloSemantico simbolo{nombre.lexema, tipo, categoria, nombresAmbitos.last(), nombre.linea};
        ambito.insert(nombre.lexema, simbolo);
        listaSimbolos.append(simbolo);
        return true;
    }

    const SimboloSemantico *buscar(const QString &nombre) const
    {
        for (int i = tablaPorAmbito.size() - 1; i >= 0; --i) {
            auto it = tablaPorAmbito.at(i).constFind(nombre);
            if (it != tablaPorAmbito.at(i).constEnd()) return &it.value();
        }
        return nullptr;
    }

    void registrarFunciones()
    {
        for (int i = 0; i + 2 < tokens.size(); ++i) {
            if (esTipo(&tokens.at(i)) && tokens.at(i + 1).tipo == "IDENTIFICADOR" &&
                tokens.at(i + 2).lexema == "(") {
                const Token nombre = tokens.at(i + 1);
                if (!tablaPorAmbito[0].contains(nombre.lexema)) {
                    const SimboloSemantico simbolo{nombre.lexema, tokens.at(i).lexema, "funcion", "global", nombre.linea};
                    tablaPorAmbito[0].insert(nombre.lexema, simbolo);
                    listaSimbolos.append(simbolo);
                }
            }
        }
    }

    void sentencia()
    {
        if (fin()) return;
        const int inicio = actual;
        if (esTipo(ver())) declaracionOFuncion();
        else if (coincide("if")) sentenciaIf();
        else if (coincide("while")) sentenciaWhile();
        else if (coincide("return")) sentenciaReturn();
        else if (coincide("{")) bloque("bloque");
        else if (ver()->tipo == "IDENTIFICADOR") asignacionOLlamada();
        else actual++;
        if (actual == inicio) actual++;
    }

    void declaracionOFuncion()
    {
        const QString tipoDeclarado = avanzar().lexema;
        if (fin() || ver()->tipo != "IDENTIFICADOR") return;
        const Token nombre = avanzar();

        if (coincide("(")) {
            const QString retornoAnterior = tipoRetornoActual;
            tipoRetornoActual = tipoDeclarado;
            abrirAmbito(QString("funcion %1").arg(nombre.lexema));
            parametros();
            coincide(")");
            if (coincide("{")) bloqueContenido();
            cerrarAmbito();
            tipoRetornoActual = retornoAnterior;
            return;
        }

        declarar(nombre, tipoDeclarado, "variable");
        if (coincide("=")) {
            const QString tipoValor = expresion();
            validarAsignacion(nombre, tipoDeclarado, tipoValor);
        }
        coincide(";");
    }

    void parametros()
    {
        if (ver() && ver()->lexema == ")") return;
        while (!fin()) {
            if (!esTipo(ver())) return;
            const QString tipoParametro = avanzar().lexema;
            if (fin() || ver()->tipo != "IDENTIFICADOR") return;
            declarar(avanzar(), tipoParametro, "parametro");
            if (!coincide(",")) return;
        }
    }

    void bloque(const QString &prefijo)
    {
        abrirAmbito(QString("%1 %2").arg(prefijo).arg(++contadorBloques));
        bloqueContenido();
        cerrarAmbito();
    }

    void bloqueContenido()
    {
        while (!fin() && ver()->lexema != "}") sentencia();
        coincide("}");
    }

    void sentenciaIf()
    {
        coincide("(");
        const QString condicion = expresion();
        validarCondicion(condicion);
        coincide(")");
        if (coincide("{")) bloque("if");
        if (coincide("else")) {
            if (coincide("if")) sentenciaIf();
            else if (coincide("{")) bloque("else");
        }
    }

    void sentenciaWhile()
    {
        coincide("(");
        const QString condicion = expresion();
        validarCondicion(condicion);
        coincide(")");
        if (coincide("{")) bloque("while");
    }

    void sentenciaReturn()
    {
        const Token palabraReturn = tokens.at(actual - 1);
        QString tipoValor = "void";
        if (!fin() && ver()->lexema != ";") tipoValor = expresion();
        coincide(";");
        if (tipoRetornoActual.isEmpty()) {
            informar(palabraReturn, "return solo puede utilizarse dentro de una funcion.");
        } else if (!compatibles(tipoRetornoActual, tipoValor)) {
            informar(palabraReturn, QString("la funcion retorna %1, pero return entrega %2.")
                                         .arg(tipoRetornoActual, tipoValor));
        }
    }

    void asignacionOLlamada()
    {
        const Token nombre = avanzar();
        if (coincide("=")) {
            const SimboloSemantico *simbolo = buscar(nombre.lexema);
            const QString tipoValor = expresion();
            if (!simbolo) informar(nombre, QString("la variable '%1' no fue declarada.").arg(nombre.lexema));
            else if (simbolo->categoria == "funcion") informar(nombre, QString("'%1' es una funcion y no una variable.").arg(nombre.lexema));
            else validarAsignacion(nombre, simbolo->tipo, tipoValor);
            coincide(";");
            return;
        }
        if (coincide("(")) {
            const SimboloSemantico *simbolo = buscar(nombre.lexema);
            if (!simbolo || simbolo->categoria != "funcion")
                informar(nombre, QString("la funcion '%1' no fue declarada.").arg(nombre.lexema));
            argumentos();
            coincide(")");
            coincide(";");
            return;
        }
    }

    void argumentos()
    {
        if (ver() && ver()->lexema == ")") return;
        do { expresion(); } while (coincide(","));
    }

    QString expresion() { return igualdad(); }
    QString igualdad()
    {
        QString izquierda = comparacion();
        while (ver() && (ver()->lexema == "==" || ver()->lexema == "!=")) {
            const Token operador = avanzar();
            const QString derecha = comparacion();
            if (!compatibles(izquierda, derecha) && izquierda != "error" && derecha != "error")
                informar(operador, QString("no se pueden comparar %1 y %2.").arg(izquierda, derecha));
            izquierda = "bool";
        }
        return izquierda;
    }
    QString comparacion()
    {
        QString izquierda = termino();
        while (ver() && (ver()->lexema == "<" || ver()->lexema == "<=" || ver()->lexema == ">" || ver()->lexema == ">=")) {
            const Token operador = avanzar();
            const QString derecha = termino();
            if ((!esNumerico(izquierda) || !esNumerico(derecha)) && izquierda != "error" && derecha != "error")
                informar(operador, "los operadores relacionales requieren valores numericos.");
            izquierda = "bool";
        }
        return izquierda;
    }
    QString termino()
    {
        QString izquierda = factor();
        while (ver() && (ver()->lexema == "+" || ver()->lexema == "-")) {
            const Token operador = avanzar();
            const QString derecha = factor();
            izquierda = combinarNumericos(operador, izquierda, derecha);
        }
        return izquierda;
    }
    QString factor()
    {
        QString izquierda = unario();
        while (ver() && (ver()->lexema == "*" || ver()->lexema == "/" || ver()->lexema == "%")) {
            const Token operador = avanzar();
            const QString derecha = unario();
            izquierda = combinarNumericos(operador, izquierda, derecha);
        }
        return izquierda;
    }
    QString unario()
    {
        if (ver() && (ver()->lexema == "!" || ver()->lexema == "-")) {
            const Token operador = avanzar();
            const QString tipo = unario();
            if (operador.lexema == "!" && tipo != "bool" && tipo != "error")
                informar(operador, "el operador ! requiere un valor booleano.");
            if (operador.lexema == "-" && !esNumerico(tipo) && tipo != "error")
                informar(operador, "el operador - requiere un valor numerico.");
            return operador.lexema == "!" ? "bool" : tipo;
        }
        return primario();
    }
    QString primario()
    {
        if (fin()) return "error";
        if (coincide("(")) {
            const QString tipo = expresion();
            coincide(")");
            return tipo;
        }
        const Token token = avanzar();
        if (token.tipo == "ENTERO") return "int";
        if (token.tipo == "REAL") return "double";
        if (token.tipo == "CADENA") return "string";
        if (token.tipo == "BOOLEANO") return "bool";
        if (token.tipo == "IDENTIFICADOR") {
            const SimboloSemantico *simbolo = buscar(token.lexema);
            if (!simbolo) {
                informar(token, QString("el identificador '%1' no fue declarado.").arg(token.lexema));
                return "error";
            }
            if (ver() && ver()->lexema == "(") {
                avanzar();
                argumentos();
                coincide(")");
                return simbolo->tipo;
            }
            return simbolo->tipo;
        }
        return "error";
    }

    bool esNumerico(const QString &tipo) const
    {
        return tipo == "char" || tipo == "int" || tipo == "float" || tipo == "double";
    }
    bool compatibles(const QString &destino, const QString &origen) const
    {
        if (destino == "error" || origen == "error") return true;
        if (destino == origen) return true;
        if (esNumerico(destino) && esNumerico(origen)) {
            if (destino == "double") return true;
            if (destino == "float") return origen != "double";
            if (destino == "int") return origen == "int" || origen == "char";
        }
        return false;
    }
    QString combinarNumericos(const Token &operador, const QString &a, const QString &b)
    {
        if (a == "error" || b == "error") return "error";
        if (!esNumerico(a) || !esNumerico(b)) {
            informar(operador, QString("el operador %1 requiere operandos numericos, no %2 y %3.")
                                  .arg(operador.lexema, a, b));
            return "error";
        }
        if (a == "double" || b == "double") return "double";
        if (a == "float" || b == "float") return "float";
        return "int";
    }
    void validarAsignacion(const Token &token, const QString &destino, const QString &origen)
    {
        if (!compatibles(destino, origen))
            informar(token, QString("no se puede asignar un valor %1 a '%2' de tipo %3.")
                                .arg(origen, token.lexema, destino));
    }
    void validarCondicion(const QString &tipo)
    {
        const Token token = actual > 0 ? tokens.at(actual - 1) : tokens.first();
        if (tipo != "bool" && tipo != "error")
            informar(token, QString("la condicion debe ser bool, no %1.").arg(tipo));
    }
};

}

ResultadoSemantico analizarSemantica(const QList<Token> &tokens)
{
    return AnalizadorSemantico(tokens).ejecutar();
}
