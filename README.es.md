# ✅ funca

**funca** es un programa que obtiene las firmas/declaraciones/como lo quieras llamar, de tus proyectos. La versión actual detecta funciones que utilizan corchetes <kbd>{}</kbd>. En el futuro será capaz de detectar más formatos.
> [!WARNING]
>
> El proyecto sigue en desarrollo, úsalo bajo tu propia responsabilidad.
>
> La versión actual es ideal para proyectos simples/intermedios.

# 🛠 Instalación

Clona el repositorio
```
git clone https://github.com/SrVariable/funca
```

Navega al proyecto
```
cd funca
```

Compila el programa o bien usando el compilador directamente
```
cc -Wall -Wextra -Werror -o funca funca.c
```

O usando make
```
make
```

# 💡 Uso

Ejecuta el programa
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
