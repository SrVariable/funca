# funca

**funca** is a program that gets the signatures/definitions/whatever you wanna call it, of your projects.

# Installation

Clone the repository
```
git clone https://github.com/SrVariable/funca
```

Navigate to the project
```
cd funca
```

Compile the program
```
cc -Wall -Wextra -Werror -o funca funca.c
```

You could also:
```
make
```

# Usage

Run the program
```
./funca <paths>
```

To get every function of the current directory
```
./funca
```

To get every function of a specific file
```
./funca funca.c
```
