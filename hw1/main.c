#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#define ARRAY_SIZE 64

#define MEMORY_ALLOCATION_ERROR -1
#define STDIN_READING_ERROR     -2
#define STDOUT_PRINT_ERROR      -3
#define MISSING_EOF             -4
#define FUNC_MEMCPY_ERROR       -5

void error_worker(int res) {
    switch (res) {
        case MEMORY_ALLOCATION_ERROR:
            printf("[error]\n");
            break;

            // ...

        default:
            printf("[error]\n");
            break;
    }
}

int read_line(char **str) {
    size_t size = ARRAY_SIZE;
    if (!(*str = (char *)malloc(size * sizeof(char)))) {
        return MEMORY_ALLOCATION_ERROR;
    }
    char *ptr = NULL, *str_ptr = *str;
    int reading_size = ARRAY_SIZE;

    while (fgets(str_ptr, reading_size, stdin)) {
        if ((ptr = strchr(str_ptr, '\n'))) {
            //if (!(*str = (char *)realloc(*str, (size_t)(ptr - *str + 1) * sizeof(char)))) {
            //    return MEMORY_ALLOCATION_ERROR;
            //}
            // valgrind is reporting error
            return 1;
        }

        size += ARRAY_SIZE;
        if (!(*str = (char *)realloc(*str, size * sizeof(char)))) {
            return MEMORY_ALLOCATION_ERROR;
        }

        str_ptr = *str + size - ARRAY_SIZE - 1;
        reading_size = ARRAY_SIZE + 1;
    }

    if (ferror(stdin)) {
        free(*str);
        *str = NULL;
        return STDIN_READING_ERROR;
    }

    ptr = strchr(*str, '\0');
    if (ptr == *str) {
        free(*str);
        *str = NULL;
        return 0;
    }
    if (!(*str = (char *)realloc(*str, (ptr - *str + 1) * sizeof(char)))) {
        return MEMORY_ALLOCATION_ERROR;
    }

    return 0;
}

int read_lines(char ***str, size_t *number_of_lines) {
    size_t i = 0, str_size = ARRAY_SIZE;
    if (!(*str = (char **)malloc(str_size * sizeof(char *)))) {
        return MEMORY_ALLOCATION_ERROR;
    }
    int read_line_res = 1;

    while((read_line_res = read_line(&((*str)[i++])))) {

        if (read_line_res < 0) {
            return read_line_res;
        }

        if (i == str_size) {
            str_size += ARRAY_SIZE;
            if (!(*str = (char **)realloc(*str, str_size * sizeof(char *)))) {
                return MEMORY_ALLOCATION_ERROR;
            }
        }
    }

    if ((*str)[i - 1] == NULL) {
        i--;
    }

    if (!(*str = (char **)realloc(*str, i * sizeof(char *)))) {
        return MEMORY_ALLOCATION_ERROR;
    }

    *number_of_lines = i;

    return 0;
}

int write_lines(char **str, const size_t number_of_lines) {

    for (size_t i = 0; i < number_of_lines; i++) {
        if (!fputs(str[i], stdout)) {
            return STDOUT_PRINT_ERROR;
        }
    }
    return 0;
}

int freeing_array_memory(char **str, size_t *number_of_lines) {

    for (size_t i = 0; i < *number_of_lines; i++) {
        free(str[i]);
    }
    free(str);
    *number_of_lines = 0;
    return 0;
}

int check_parentheses(const char *str) {
    size_t left = 0, right = 0, i = 0;

    while(str[i]) {
        if(str[i] == '(') {
            left++;
        } else if (str[i] == ')') {
            right++;
        }
        if (right > left) {
            return 0;
        }
        i++;
    }

    if (left == right) {
        return 1;
    }
    return 0;
}

int parse_parentheses(char **str, const size_t num_lines_str, char ***new_str) {

    size_t new_str_size = ARRAY_SIZE;
    char *ptr = NULL;
    size_t num_lines_new_str = 0;

    if (!(*new_str = (char **)malloc(new_str_size * sizeof(char *)))) {
        return MEMORY_ALLOCATION_ERROR;
    }

    for (size_t i = 0; i < num_lines_str; i++) {
        if (check_parentheses(str[i])) {
            ptr = strchr(str[i], '\0');
            if (!ptr) {
                freeing_array_memory(*new_str, &num_lines_new_str);
                return MISSING_EOF;
            }
            if (!((*new_str)[num_lines_new_str] = (char *)malloc((ptr - str[i] + 1) * sizeof(char)))) {
                freeing_array_memory(*new_str, &num_lines_new_str);
                return MEMORY_ALLOCATION_ERROR;
            }
            if (!memcpy((*new_str)[num_lines_new_str], str[i], (ptr - str[i] + 1))) {
                freeing_array_memory(*new_str, &num_lines_new_str);
                return FUNC_MEMCPY_ERROR;
            }
            if ((num_lines_new_str - 1) == new_str_size) {
                new_str_size += ARRAY_SIZE;
                if (!(*new_str = (char **)realloc(*new_str, new_str_size * sizeof(char *)))) {
                    freeing_array_memory(*new_str, &num_lines_new_str);
                    return MEMORY_ALLOCATION_ERROR;
                }
            }
            num_lines_new_str++;
        }
    }

    if (!(*new_str = (char **)realloc(*new_str, num_lines_new_str * sizeof(char *)))) {
        return MEMORY_ALLOCATION_ERROR;
    }

    return (int)num_lines_new_str;
}

int main() {
    char **str = NULL, **new_str = NULL;
    int res = 1;
    size_t number_of_lines_str = 0, number_of_lines_new_str = 0;

    if ((res = read_lines(&str, &number_of_lines_str))) {
        freeing_array_memory(str, &number_of_lines_str);
        error_worker(res);
        return 0;
    }

    res = parse_parentheses(str, number_of_lines_str, &new_str);
    if (res > 0) {
        number_of_lines_new_str = (size_t)res;
    } else {
        freeing_array_memory(str, &number_of_lines_str);
        error_worker(res);
        return 0;
    }

    if ((res = write_lines(new_str, number_of_lines_new_str))) {
        freeing_array_memory(str, &number_of_lines_str);
        freeing_array_memory(new_str, &number_of_lines_new_str);
        error_worker(res);
        return 0;
    }

    freeing_array_memory(str, &number_of_lines_str);
    freeing_array_memory(new_str, &number_of_lines_new_str);
    return 0;
}
