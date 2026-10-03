# Line Editor — Design Document

## 1. Problem Statement

The objective is to implement a command-line Line Editor in C that allows users to create, modify, display and manage a text document line by line.

The editor supports the required operations:

- Insert a line
- Delete a line
- Display the document

It also provides additional functionality:

- Save and load
- Search
- Find and replace
- Undo
- Document statistics
- Help

---

## 2. Data Structure Design

The document is represented using a dynamic array of strings.

### Document Structure

```c
typedef struct {
    char **lines;
    int count;
    int capacity;
} Document;