# funca

**funca** es un programa que obtiene las firmas/declaraciones/como lo quieras llamar, de tus proyectos.

# Instalación

Clona el repositorio
```
git clone https://github.com/SrVariable/funca
```

Navega al proyecto
```
cd funca
```

Compila el programa
```
cc -Wall -Wextra -Werror -o funca funca.c
```

También puedes:
```
make
```

# Uso

Ejecuta el program
```
./funca <rutas>
```

Para obtener todas las funciones del directorio actual
```
./funca .
```

Para obtener todas las funciones de un archivo específico
```
./funca funca.c
```
