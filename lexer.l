%option noyywrap yylineno
%{
/* Especificacion de referencia. La compilacion usa lexer.cpp para no depender
   de una instalacion externa de Flex en Windows. */
%}
%%
[ \t\r\n]+                    ;
"//"[^\n]*                   ;
"/*"([^*]|\*+[^*/])*\*+"/"  ;
[0-9]+(\.[0-9]+)?            ;
[A-Za-z_][A-Za-z0-9_]*       ;
"\""([^\\\n]|\\.)*"\""       ;
.                            ;
%%
