# Todo CLI

A small todo list program written in C. This project helped me practise structs, file handling, command-line arguments, and error checking.

## Features

- Add, list, complete, and delete todos
- Support titles with spaces
- Save data in a local `todos.txt` file

## Build

Requirements: a C11 compiler and `make`.

```sh
make
```

## Usage

```sh
./todo add "Read The C Programming Language"
./todo list
./todo done 1
./todo delete 1
```

Run `./todo` without a command to see the full usage message.
