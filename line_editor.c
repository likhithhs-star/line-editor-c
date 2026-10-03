#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_CAPACITY 10
#define MAX_INPUT 1024

typedef struct {
    char **lines;
    int count;
    int capacity;
} Document;

typedef struct {
    char **lines;
    int count;
} Snapshot;

static Snapshot undo_state = {NULL, 0};
static int undo_available = 0;

/* ---------- Utility Functions ---------- */

char *duplicate_string(const char *source) {
    size_t len = strlen(source);
    char *copy = malloc(len + 1);

    if (copy == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    strcpy(copy, source);
    return copy;
}

void trim_newline(char *text) {
    text[strcspn(text, "\r\n")] = '\0';
}

/* ---------- Document Management ---------- */

void init_document(Document *doc) {
    doc->lines = malloc(INITIAL_CAPACITY * sizeof(char *));

    if (doc->lines == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    doc->count = 0;
    doc->capacity = INITIAL_CAPACITY;
}

void free_document(Document *doc) {
    for (int i = 0; i < doc->count; i++) {
        free(doc->lines[i]);
    }

    free(doc->lines);
    doc->lines = NULL;
    doc->count = 0;
    doc->capacity = 0;
}

void ensure_capacity(Document *doc) {
    if (doc->count < doc->capacity) {
        return;
    }

    int new_capacity = doc->capacity * 2;

    char **new_lines = realloc(
        doc->lines,
        new_capacity * sizeof(char *)
    );

    if (new_lines == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    doc->lines = new_lines;
    doc->capacity = new_capacity;
}

void clear_document(Document *doc) {
    for (int i = 0; i < doc->count; i++) {
        free(doc->lines[i]);
    }

    doc->count = 0;
}

/* ---------- Undo ---------- */

void clear_undo(void) {
    for (int i = 0; i < undo_state.count; i++) {
        free(undo_state.lines[i]);
    }

    free(undo_state.lines);

    undo_state.lines = NULL;
    undo_state.count = 0;
    undo_available = 0;
}

void save_undo_state(const Document *doc) {
    clear_undo();

    if (doc->count == 0) {
        undo_state.lines = NULL;
        undo_state.count = 0;
        undo_available = 1;
        return;
    }

    undo_state.lines = malloc(doc->count * sizeof(char *));

    if (undo_state.lines == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    undo_state.count = doc->count;

    for (int i = 0; i < doc->count; i++) {
        undo_state.lines[i] = duplicate_string(doc->lines[i]);
    }

    undo_available = 1;
}

void perform_undo(Document *doc) {
    if (!undo_available) {
        printf("Nothing to undo.\n");
        return;
    }

    clear_document(doc);

    while (doc->capacity < undo_state.count) {
        int new_capacity = doc->capacity * 2;

        char **new_lines = realloc(
            doc->lines,
            new_capacity * sizeof(char *)
        );

        if (new_lines == NULL) {
            fprintf(stderr, "Memory allocation failed.\n");
            exit(EXIT_FAILURE);
        }

        doc->lines = new_lines;
        doc->capacity = new_capacity;
    }

    for (int i = 0; i < undo_state.count; i++) {
        doc->lines[i] = duplicate_string(undo_state.lines[i]);
    }

    doc->count = undo_state.count;

    clear_undo();

    printf("Last action undone.\n");
}

/* ---------- Core Features ---------- */

void display_document(const Document *doc) {
    if (doc->count == 0) {
        printf("[Document is empty]\n");
        return;
    }

    printf("\n----- Document -----\n");

    for (int i = 0; i < doc->count; i++) {
        printf("%3d | %s\n", i + 1, doc->lines[i]);
    }

    printf("--------------------\n");
    printf("Total lines: %d\n\n", doc->count);
}

void insert_line(Document *doc, int line_number, const char *text) {
    if (line_number < 1 || line_number > doc->count + 1) {
        printf("Invalid line number. Use 1 to %d.\n", doc->count + 1);
        return;
    }

    save_undo_state(doc);
    ensure_capacity(doc);

    for (int i = doc->count; i >= line_number; i--) {
        doc->lines[i] = doc->lines[i - 1];
    }

    doc->lines[line_number - 1] = duplicate_string(text);
    doc->count++;

    printf("Line inserted successfully.\n");
}

void delete_line(Document *doc, int line_number) {
    if (doc->count == 0) {
        printf("Cannot delete from an empty document.\n");
        return;
    }

    if (line_number < 1 || line_number > doc->count) {
        printf("Invalid line number. Use 1 to %d.\n", doc->count);
        return;
    }

    save_undo_state(doc);

    free(doc->lines[line_number - 1]);

    for (int i = line_number - 1; i < doc->count - 1; i++) {
        doc->lines[i] = doc->lines[i + 1];
    }

    doc->count--;

    printf("Line deleted successfully.\n");
}

/* ---------- Save / Load ---------- */

int save_file(const Document *doc, const char *filename) {
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        printf("Could not open '%s' for writing.\n", filename);
        return 0;
    }

    for (int i = 0; i < doc->count; i++) {
        fprintf(file, "%s\n", doc->lines[i]);
    }

    fclose(file);

    printf("Document saved to '%s'.\n", filename);
    return 1;
}

int load_file(Document *doc, const char *filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Could not open '%s' for reading.\n", filename);
        return 0;
    }

    Document temporary;
    init_document(&temporary);

    char buffer[MAX_INPUT];

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        trim_newline(buffer);

        ensure_capacity(&temporary);

        temporary.lines[temporary.count++] =
            duplicate_string(buffer);
    }

    fclose(file);

    save_undo_state(doc);
    clear_document(doc);

    for (int i = 0; i < temporary.count; i++) {
        ensure_capacity(doc);

        doc->lines[doc->count++] =
            duplicate_string(temporary.lines[i]);
    }

    free_document(&temporary);

    printf("Document loaded from '%s'.\n", filename);
    return 1;
}

/* ---------- Search ---------- */

void search_document(const Document *doc, const char *phrase) {
    if (phrase == NULL || strlen(phrase) == 0) {
        printf("Search phrase cannot be empty.\n");
        return;
    }

    int found = 0;

    for (int i = 0; i < doc->count; i++) {
        if (strstr(doc->lines[i], phrase) != NULL) {
            printf("Found on line %d: %s\n",
                   i + 1,
                   doc->lines[i]);

            found = 1;
        }
    }

    if (!found) {
        printf("Phrase not found.\n");
    }
}

/* ---------- Find & Replace ---------- */

void replace_in_line(
    Document *doc,
    int line_index,
    const char *old_text,
    const char *new_text,
    int *replacement_count
) {
    char *line = doc->lines[line_index];

    size_t old_len = strlen(old_text);
    size_t new_len = strlen(new_text);

    if (old_len == 0) {
        return;
    }

    int occurrences = 0;
    char *position = line;

    while ((position = strstr(position, old_text)) != NULL) {
        occurrences++;
        position += old_len;
    }

    if (occurrences == 0) {
        return;
    }

    size_t original_length = strlen(line);

    size_t new_length;

    if (new_len >= old_len) {
        new_length =
            original_length +
            (size_t)occurrences * (new_len - old_len) +
            1;
    } else {
        new_length =
            original_length -
            (size_t)occurrences * (old_len - new_len) +
            1;
    }

    char *new_line = malloc(new_length);

    if (new_line == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    char *destination = new_line;
    position = line;

    while (1) {
        char *next = strstr(position, old_text);

        if (next == NULL) {
            strcpy(destination, position);
            break;
        }

        size_t prefix_length = (size_t)(next - position);

        memcpy(destination, position, prefix_length);
        destination += prefix_length;

        memcpy(destination, new_text, new_len);
        destination += new_len;

        position = next + old_len;
    }

    free(doc->lines[line_index]);
    doc->lines[line_index] = new_line;

    *replacement_count += occurrences;
}

void find_and_replace(
    Document *doc,
    const char *old_text,
    const char *new_text
) {
    if (strlen(old_text) == 0) {
        printf("Text to replace cannot be empty.\n");
        return;
    }

    int total = 0;

    for (int i = 0; i < doc->count; i++) {
        if (strstr(doc->lines[i], old_text) != NULL) {
            total++;
        }
    }

    if (total == 0) {
        printf("Text '%s' was not found.\n", old_text);
        return;
    }

    save_undo_state(doc);

    int replacements = 0;

    for (int i = 0; i < doc->count; i++) {
        replace_in_line(
            doc,
            i,
            old_text,
            new_text,
            &replacements
        );
    }

    printf("%d replacement(s) made.\n", replacements);
}

/* ---------- Statistics ---------- */

void show_statistics(const Document *doc) {
    long words = 0;
    long characters = 0;

    for (int i = 0; i < doc->count; i++) {
        characters += (long)strlen(doc->lines[i]);

        int inside_word = 0;

        for (size_t j = 0; j < strlen(doc->lines[i]); j++) {
            if (isspace((unsigned char)doc->lines[i][j])) {
                inside_word = 0;
            } else if (!inside_word) {
                words++;
                inside_word = 1;
            }
        }
    }

    printf("\nDocument Statistics\n");
    printf("-------------------\n");
    printf("Lines      : %d\n", doc->count);
    printf("Words      : %ld\n", words);
    printf("Characters : %ld\n\n", characters);
}

/* ---------- Help ---------- */

void show_help(void) {
    printf("\n========== LINE EDITOR HELP ==========\n");
    printf("i N TEXT       Insert TEXT at line N\n");
    printf("d N            Delete line N\n");
    printf("p              Display the document\n");
    printf("w FILE         Save document to FILE\n");
    printf("r FILE         Load document from FILE\n");
    printf("s TEXT         Search for TEXT\n");
    printf("f OLD|NEW      Replace OLD with NEW everywhere\n");
    printf("u              Undo the last modification\n");
    printf("c              Show line, word and character counts\n");
    printf("h              Show this help\n");
    printf("q              Quit the editor\n");
    printf("======================================\n\n");

    printf("Examples:\n");
    printf("  i 1 Hello world\n");
    printf("  i 2 This is a line editor\n");
    printf("  d 1\n");
    printf("  p\n");
    printf("  w document.txt\n");
    printf("  r document.txt\n");
    printf("  s editor\n");
    printf("  f editor|EDITOR\n");
    printf("  u\n");
    printf("  c\n\n");
}

/* ---------- Command Processing ---------- */

void process_command(
    Document *doc,
    char *input,
    int *running
) {
    trim_newline(input);

    if (strlen(input) == 0) {
        return;
    }

    char command = input[0];

    /* Insert */
    if (command == 'i') {
        int line_number;
        char text[MAX_INPUT];

        if (sscanf(input + 1, "%d %[^\n]",
                   &line_number, text) == 2) {

            insert_line(doc, line_number, text);

        } else {
            printf("Usage: i LINE TEXT\n");
        }
    }

    /* Delete */
    else if (command == 'd') {
        int line_number;

        if (sscanf(input + 1, "%d", &line_number) == 1) {
            delete_line(doc, line_number);
        } else {
            printf("Usage: d LINE\n");
        }
    }

    /* Display */
    else if (command == 'p') {
        display_document(doc);
    }

    /* Save */
    else if (command == 'w') {
        char filename[MAX_INPUT];

        if (sscanf(input + 1, "%1023s", filename) == 1) {
            save_file(doc, filename);
        } else {
            printf("Usage: w FILE\n");
        }
    }

    /* Load */
    else if (command == 'r') {
        char filename[MAX_INPUT];

        if (sscanf(input + 1, "%1023s", filename) == 1) {
            load_file(doc, filename);
        } else {
            printf("Usage: r FILE\n");
        }
    }

    /* Search */
    else if (command == 's') {
        char *phrase = input + 1;

        while (*phrase == ' ') {
            phrase++;
        }

        search_document(doc, phrase);
    }

    /* Find & Replace */
    else if (command == 'f') {
        char *data = input + 1;

        while (*data == ' ') {
            data++;
        }

        char *separator = strchr(data, '|');

        if (separator == NULL) {
            printf("Usage: f OLD|NEW\n");
            return;
        }

        *separator = '\0';

        char *old_text = data;
        char *new_text = separator + 1;

        while (*old_text == ' ') {
            old_text++;
        }

        find_and_replace(doc, old_text, new_text);
    }

    /* Undo */
    else if (command == 'u') {
        perform_undo(doc);
    }

    /* Statistics */
    else if (command == 'c') {
        show_statistics(doc);
    }

    /* Help */
    else if (command == 'h') {
        show_help();
    }

    /* Quit */
    else if (command == 'q') {
        *running = 0;
        printf("Exiting Line Editor.\n");
    }

    /* Unknown */
    else {
        printf("Unknown command. Type 'h' for help.\n");
    }
}

/* ---------- Main ---------- */

int main(void) {
    Document document;

    init_document(&document);

    int running = 1;
    char input[MAX_INPUT];

    printf("=====================================\n");
    printf("       COMMAND-LINE LINE EDITOR      \n");
    printf("=====================================\n");
    printf("Type 'h' for help.\n\n");

    while (running) {
        printf("editor> ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }

        process_command(
            &document,
            input,
            &running
        );
    }

    clear_undo();
    free_document(&document);

    return 0;
}