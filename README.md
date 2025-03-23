# ✅ funca

**funca** is a program that gets the signatures/declarations/whatever you wanna call it, of your projects. The current version detects functions that uses curly brackets <kbd>{}</kbd>. In the future it will be able to detect more formats.
> [!WARNING]
> 
> The project is still on development, so use at your own risk.
> 
> The current version is ideal for simple/mid-level projects.

# 🛠 Installation

Clone the repository
```
git clone https://github.com/SrVariable/funca
```

Navigate to the project
```
cd funca
```

Compile the program whether using the compiler directly
```
cc -Wall -Wextra -Werror -o funca funca.c
```

Or using make
```
make
```

# 💡 Usage

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
