# Pruebas del Analizador Semantico

## Programa valido

```cpp
int main() {
    int edad = 22;
    double promedio = 89.5;
    bool aprobado = promedio >= 70;

    if (aprobado == true) {
        edad = edad + 1;
    }

    return edad;
}
```

Resultado esperado: sintaxis y semantica validas. La tabla debe incluir `main`, `edad`, `promedio` y `aprobado`.

## Variable no declarada

```cpp
int main() {
    int edad = 22;
    total = edad + 1;
    return edad;
}
```

Resultado esperado: `total` no fue declarada.

## Declaracion duplicada

```cpp
int main() {
    int edad = 22;
    int edad = 30;
    return edad;
}
```

Resultado esperado: `edad` ya fue declarada en el mismo ambito.

## Tipos incompatibles

```cpp
int main() {
    int cantidad = 10;
    string nombre = "Elvy";
    cantidad = nombre;
    return cantidad;
}
```

Resultado esperado: no se puede asignar un valor `string` a una variable `int`.

## Condicion no booleana

```cpp
int main() {
    int cantidad = 10;
    if (cantidad) {
        cantidad = cantidad + 1;
    }
    return cantidad;
}
```

Resultado esperado: la condicion debe ser `bool`.

## Retorno incompatible

```cpp
int main() {
    string mensaje = "hola";
    return mensaje;
}
```

Resultado esperado: la funcion retorna `int`, pero `return` entrega `string`.
