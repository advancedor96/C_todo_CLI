#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TODOS 100
#define TITLE_SIZE 100
#define DATA_FILE "todos.txt"

typedef struct {
    char title[TITLE_SIZE];
    int completed;
} Todo;

static void print_usage(const char *program)
{
    printf("Usage:\n");
    printf("  %s list\n", program);
    printf("  %s add <title>\n", program);
    printf("  %s done <number>\n", program);
    printf("  %s delete <number>\n", program);
}

static void list_todos(const Todo todos[], int count)
{
    if (count == 0) {
        printf("No todos yet. Add one with: todo add <title>\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        printf("%d. [%c] %s\n", i + 1, todos[i].completed ? 'x' : ' ', todos[i].title);
    }
}

static int add_todo(Todo todos[], int *count, const char *title)
{
    size_t title_length = strlen(title);

    if (*count >= MAX_TODOS) {
        fprintf(stderr, "Todo list is full (maximum %d items).\n", MAX_TODOS);
        return 0;
    }
    if (title_length == 0 || title_length >= TITLE_SIZE) {
        fprintf(stderr, "Title must contain 1 to %d characters.\n", TITLE_SIZE - 1);
        return 0;
    }
    if (strchr(title, '|') != NULL) {
        fprintf(stderr, "Title cannot contain '|'.\n");
        return 0;
    }

    snprintf(todos[*count].title, TITLE_SIZE, "%s", title);
    todos[*count].completed = 0;
    (*count)++;
    return 1;
}

static int delete_todo(Todo todos[], int *count, int index)
{
    if (index < 0 || index >= *count) {
        return 0;
    }

    memmove(&todos[index], &todos[index + 1],
            (size_t)(*count - index - 1) * sizeof(Todo));
    (*count)--;
    return 1;
}

static int load_todos(Todo todos[], int *count)
{
    FILE *file = fopen(DATA_FILE, "r");
    char line[TITLE_SIZE + 16];

    if (file == NULL) {
        return errno == ENOENT;
    }

    while (*count < MAX_TODOS && fgets(line, sizeof(line), file) != NULL) {
        char *separator = strchr(line, '|');
        char *title;
        char *end;
        long completed;

        if (separator == NULL || strchr(separator + 1, '\n') == NULL) {
            continue;
        }
        *separator = '\0';
        errno = 0;
        completed = strtol(line, &end, 10);
        if (errno != 0 || end == line || *end != '\0' || (completed != 0 && completed != 1)) {
            continue;
        }
        title = separator + 1;
        title[strcspn(title, "\n")] = '\0';
        if (title[0] == '\0' || strlen(title) >= TITLE_SIZE) {
            continue;
        }

        snprintf(todos[*count].title, TITLE_SIZE, "%s", title);
        todos[*count].completed = (int)completed;
        (*count)++;
    }

    fclose(file);
    return 1;
}

static int save_todos(const Todo todos[], int count)
{
    FILE *file = fopen(DATA_FILE, "w");

    if (file == NULL) {
        perror("Failed to open data file");
        return 0;
    }
    for (int i = 0; i < count; i++) {
        fprintf(file, "%d|%s\n", todos[i].completed, todos[i].title);
    }
    fclose(file);
    return 1;
}

static int parse_index(const char *text, int count, int *index)
{
    char *end;
    long number;

    errno = 0;
    number = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || number < 1 || number > count) {
        return 0;
    }
    *index = (int)number - 1;
    return 1;
}

static int build_title(int argc, char *argv[], char *title)
{
    size_t used = 0;

    for (int i = 2; i < argc; i++) {
        size_t part_length = strlen(argv[i]);
        size_t separator_length = i == 2 ? 0 : 1;

        if (used + separator_length + part_length >= TITLE_SIZE) {
            return 0;
        }
        if (separator_length != 0) {
            title[used++] = ' ';
        }
        memcpy(title + used, argv[i], part_length);
        used += part_length;
    }
    title[used] = '\0';
    return used > 0 && strchr(title, '|') == NULL;
}

int main(int argc, char *argv[])
{
    Todo todos[MAX_TODOS];
    int count = 0;

    if (!load_todos(todos, &count)) {
        perror("Failed to load data file");
        return 1;
    }
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "list") == 0) {
        list_todos(todos, count);
        return 0;
    }
    if (strcmp(argv[1], "add") == 0) {
        char title[TITLE_SIZE];

        if (!build_title(argc, argv, title) || !add_todo(todos, &count, title)) {
            fprintf(stderr, "Usage: %s add <title>\n", argv[0]);
            return 1;
        }
        if (!save_todos(todos, count)) {
            return 1;
        }
        printf("Added: %s\n", title);
        return 0;
    }
    if (strcmp(argv[1], "done") == 0 || strcmp(argv[1], "delete") == 0) {
        int index;

        if (argc != 3 || !parse_index(argv[2], count, &index)) {
            fprintf(stderr, "Usage: %s %s <number>\n", argv[0], argv[1]);
            return 1;
        }
        if (strcmp(argv[1], "done") == 0) {
            todos[index].completed = !todos[index].completed;
            printf("Marked %d as %s: %s\n", index + 1,
                   todos[index].completed ? "done" : "pending", todos[index].title);
        } else {
            printf("Deleted: %s\n", todos[index].title);
            delete_todo(todos, &count, index);
        }
        return save_todos(todos, count) ? 0 : 1;
    }

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    print_usage(argv[0]);
    return 1;
}

