# Analizador Semantico Grafico

Aplicacion de escritorio desarrollada en C++ 17 con Qt Widgets. Integra analisis lexico, sintactico y semantico de un subconjunto didactico de C/C++. Todos los resultados se muestran en una interfaz grafica.

## Funcionalidad

- Editor grafico para escribir o cargar codigo.
- Tabla de tokens con lexema, categoria y linea.
- Analizador sintactico descendente recursivo.
- Tabla de simbolos con nombre, tipo, categoria, ambito y linea.
- Control de variables y parametros duplicados.
- Deteccion de identificadores y funciones no declarados.
- Comprobacion de tipos en asignaciones y expresiones.
- Validacion de condiciones booleanas.
- Comprobacion del tipo de retorno de las funciones.
- Mensajes de error con numero de linea.

## Lenguaje utilizado

El proyecto esta escrito en **C++ 17** y utiliza **Qt 6 Widgets** para la interfaz grafica. El analizador lexico produce tokens, el parser valida la gramatica y el modulo semantico recorre nuevamente la secuencia para construir ambitos y una tabla de simbolos.

## Lenguaje aceptado para las pruebas

- Tipos: `int`, `float`, `double`, `char`, `bool`, `string` y `void`.
- Variables con inicializacion opcional.
- Funciones y parametros tipados.
- Asignaciones y llamadas a funciones.
- Condicionales `if` y `else`.
- Ciclos `while`.
- Sentencias `return`.
- Expresiones aritmeticas, relacionales y de igualdad.
- Literales enteros, reales, cadenas y booleanos.
- Comentarios de linea y de bloque.

## Reglas semanticas

1. Un nombre no puede declararse dos veces dentro del mismo ambito.
2. Una variable debe declararse antes de ser utilizada.
3. Una funcion llamada debe existir en la tabla de simbolos.
4. El tipo de una expresion debe ser compatible con la variable de destino.
5. Los operadores aritmeticos requieren operandos numericos.
6. Los operadores relacionales producen un valor `bool`.
7. Las condiciones de `if` y `while` deben ser booleanas.
8. El valor de `return` debe ser compatible con el tipo declarado de la funcion.
9. Los parametros pertenecen al ambito de su funcion.
10. Las variables declaradas dentro de bloques pertenecen a su ambito local.

## Estructura del proyecto

- `lexer.cpp` y `lexer.l`: analisis lexico y especificacion de referencia.
- `lexerbridge.cpp` y `lexerbridge.h`: integracion del lexer.
- `parser.cpp` y `parser.h`: analisis sintactico.
- `semantic.cpp` y `semantic.h`: analisis semantico y tabla de simbolos.
- `token.h`: representacion de tokens.
- `mainwindow.cpp`, `mainwindow.h` y `mainwindow.ui`: interfaz grafica.
- `PRUEBAS.md`: programas validos e invalidos para la demostracion.
- `CMakeLists.txt`: configuracion del proyecto.

## Compilacion en Qt Creator

1. Abra `CMakeLists.txt` con Qt Creator.
2. Seleccione `Desktop Qt 6.11.2 MinGW 64-bit`.
3. Active Debug para desarrollo o Release para la entrega.
4. Pulse `Configure Project`.
5. Compile con `Ctrl + B` y ejecute con `Ctrl + R`.

## Uso

1. Escriba codigo o pulse **Cargar**.
2. Pulse **Analizar**.
3. Revise los tokens y el estado sintactico.
4. Consulte los errores semanticos.
5. Revise la tabla de simbolos y sus ambitos.

## Presentacion sugerida

1. Explicar las diferencias entre analisis lexico, sintactico y semantico.
2. Presentar la tabla de simbolos y el concepto de ambito.
3. Mostrar un programa valido y explicar cada simbolo registrado.
4. Probar una variable no declarada.
5. Probar una declaracion duplicada y una asignacion incompatible.
6. Explicar la validacion de `return`.
7. Cerrar con limitaciones y posibles mejoras.

## Limitaciones

El lenguaje es didactico y no implementa todas las reglas de C++. No ejecuta el programa, no genera codigo objeto y no comprueba la cantidad o los tipos individuales de los argumentos de una llamada.
