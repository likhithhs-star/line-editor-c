# Line Editor in C

## Overview

This project implements a command-line Line Editor in C.

The editor allows users to create and modify a text document directly from the terminal. It supports basic line editing operations as well as additional document-management features.

## Features

### Core Features
- Insert a line at a specified position
- Delete a line
- Display the complete document

### Advanced Features
- Save document to a file
- Load document from a file
- Search for text
- Find and replace text
- Undo the last modification
- Display line, word and character counts
- Built-in help menu

## Commands

| Command | Description |
|---|---|
| `i N TEXT` | Insert TEXT at line N |
| `d N` | Delete line N |
| `p` | Display the document |
| `w FILE` | Save document to FILE |
| `r FILE` | Load document from FILE |
| `s TEXT` | Search for TEXT |
| `f OLD\|NEW` | Replace OLD with NEW |
| `u` | Undo the last modification |
| `c` | Show line, word and character counts |
| `h` | Show help |
| `q` | Quit the editor |

## Example

```text
editor> i 1 Hello world
editor> i 2 This is my line editor
editor> p

----- Document -----
  1 | Hello world
  2 | This is my line editor
--------------------
Total lines: 2