#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>

#define USERNAME 6
#define PIN 5

typedef enum error_type {
    SUCCESS,
    INPUT_ERROR,
    MALLOC_ERROR,
    SYSTEM_ERROR
} error_type;

typedef enum command_t {
    LOGIN,
    REG,
    LOGOUT,
    TIME,
    DATE,
    HOWMUCH,
    SANC,
    EXIT,
    UNDEFINED
} command_t;

typedef struct usr_rec {
    char username[USERNAME+1];
    uint32_t pin;
    int32_t sanc;
} usr_rec;

typedef struct program_state {
    usr_rec* users;
    uint32_t count_users;
    uint32_t capacity;
    int32_t user_ind;
} program_state;

error_type check_input(program_state* bd, command_t command) {
    // Команды, доступные без авторизации
    if (command == LOGIN || command == REG) {
        return (bd->user_ind == -1) ? SUCCESS : INPUT_ERROR;
    }

    // Все остальные команды требуют авторизации
    if (bd->user_ind == -1) {
        return INPUT_ERROR;
    }

    // Особые проверки для отдельных команд
    switch (command) {
        case LOGOUT:
        case HOWMUCH:
        case SANC:
            return SUCCESS;

        case TIME:
        case DATE:
            // Проверяем санкции
            if (bd->users[bd->user_ind].sanc == 0) {
                return SUCCESS;
            }
            else if (bd->users[bd->user_ind].sanc > 0) {
                // Уменьшаем санкции только если они положительные
                bd->users[bd->user_ind].sanc--;
                return SUCCESS;
            }
            else if (bd->users[bd->user_ind].sanc == -1) {
                // Вечные санкции - команды доступны
                return SUCCESS;
            }
            return INPUT_ERROR;

        default:
            return INPUT_ERROR;
    }
}

command_t parse() {
    char buf[10];
    if (scanf("%9s", buf) != 1) {
        return UNDEFINED;
    }

    if (strcmp(buf, "login") == 0) return LOGIN;
    if (strcmp(buf, "reg") == 0) return REG;
    if (strcmp(buf, "logout") == 0) return LOGOUT;
    if (strcmp(buf, "time") == 0) return TIME;
    if (strcmp(buf, "date") == 0) return DATE;
    if (strcmp(buf, "howmuch") == 0) return HOWMUCH;
    if (strcmp(buf, "sanc") == 0) return SANC;
    if (strcmp(buf, "exit") == 0) return EXIT;

    return UNDEFINED;
}

error_type is_val_username(char* username) {
    if (username[0] == '\0' || strlen(username) > USERNAME) {
        return INPUT_ERROR;
    }
    for (int i = 0; username[i]; i++) {
        if (!isalnum(username[i])) {
            return INPUT_ERROR;
        }
    }
    return SUCCESS;
}

error_type is_val_pin(uint32_t pin) {
    return (pin > 0 && pin <= 99999) ? SUCCESS : INPUT_ERROR;
}

error_type login(program_state* bd) {
    char buf_name[USERNAME+1];
    uint32_t buf_pin;

    printf("Введите имя: ");
    if (scanf("%6s", buf_name) != 1) return INPUT_ERROR;

    printf("Введите пин: ");
    if (scanf("%u", &buf_pin) != 1) return INPUT_ERROR;

    if (is_val_username(buf_name) != SUCCESS || is_val_pin(buf_pin) != SUCCESS) {
        return INPUT_ERROR;
    }

    for (uint32_t i = 0; i < bd->count_users; i++) {
        if (strcmp(bd->users[i].username, buf_name) == 0 &&
            bd->users[i].pin == buf_pin) {
            bd->user_ind = i;
            return SUCCESS;
        }
    }

    return INPUT_ERROR;
}

error_type reg(program_state* bd) {
    char buf_name[USERNAME+1];
    uint32_t buf_pin;

    printf("Введите имя: ");
    if (scanf("%6s", buf_name) != 1) return INPUT_ERROR;

    printf("Введите пин: ");
    if (scanf("%u", &buf_pin) != 1) return INPUT_ERROR;

    if (is_val_username(buf_name) != SUCCESS || is_val_pin(buf_pin) != SUCCESS) {
        return INPUT_ERROR;
    }

    for (uint32_t i = 0; i < bd->count_users; i++) {
        if (strcmp(bd->users[i].username, buf_name) == 0) {
            return INPUT_ERROR;
        }
    }

    if (bd->count_users >= bd->capacity) {
        bd->capacity *= 2;
        usr_rec* new_users = realloc(bd->users, sizeof(usr_rec) * bd->capacity);
        if (!new_users) return MALLOC_ERROR;
        bd->users = new_users;
    }

    strcpy(bd->users[bd->count_users].username, buf_name);
    bd->users[bd->count_users].pin = buf_pin;
    bd->users[bd->count_users].sanc = 0;
    bd->count_users++;

    return SUCCESS;
}

error_type logout(program_state* bd) {
    bd->user_ind = -1;
    return SUCCESS;
}

error_type show_time(program_state* bd) {
    time_t now = time(NULL);
    if (now == -1) return SYSTEM_ERROR;

    struct tm* tm = localtime(&now);
    if (!tm) return SYSTEM_ERROR;

    printf("Текущее время: %02d:%02d:%02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
    return SUCCESS;
}

error_type show_date(program_state* bd) {
    time_t now = time(NULL);
    if (now == -1) return SYSTEM_ERROR;

    struct tm* tm = localtime(&now);
    if (!tm) return SYSTEM_ERROR;

    printf("Текущая дата: %02d.%02d.%04d\n", tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900);
    return SUCCESS;
}

error_type show_howmuch(program_state* bd) {
    if (bd->users[bd->user_ind].sanc == -1) {
        printf("Установлены вечные санкции\n");
    }
    else if (bd->users[bd->user_ind].sanc == 0) {
        printf("Санкции отсутствуют\n");
    }
    else {
        printf("Осталось санкций: %d\n", bd->users[bd->user_ind].sanc);
    }
    return SUCCESS;
}

error_type add_sanc(program_state* bd) {
    int32_t amount;
    printf("Введите количество санкций (0 - снять, -1 - вечные, >0 - обычные): ");

    if (scanf("%d", &amount) != 1) {
        while (getchar() != '\n');
        return INPUT_ERROR;
    }

    if (amount < -1) {
        return INPUT_ERROR;
    }

    bd->users[bd->user_ind].sanc = amount;

    if (amount == -1) {
        printf("Установлены вечные санкции\n");
    }
    else if (amount == 0) {
        printf("Санкции сняты\n");
    }
    else {
        printf("Установлено %d санкций\n", amount);
    }

    return SUCCESS;
}

error_type call_func(command_t code_func, program_state* bd) {
    switch (code_func) {
        case LOGIN: return login(bd);
        case REG: return reg(bd);
        case LOGOUT: return logout(bd);
        case TIME: return show_time(bd);
        case DATE: return show_date(bd);
        case HOWMUCH: return show_howmuch(bd);
        case SANC: return add_sanc(bd);
        case EXIT: exit(0);
        default: return INPUT_ERROR;
    }
}

int main() {
    program_state bd = {
            .users = malloc(sizeof(usr_rec) * 10),
            .capacity = 10,
            .user_ind = -1,
            .count_users = 0
    };

    if (!bd.users) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return MALLOC_ERROR;
    }

    while (1) {
        printf("%s> ", (bd.user_ind == -1) ? "" : bd.users[bd.user_ind].username);

        command_t command = parse();
        error_type check = check_input(&bd, command);

        if (check != SUCCESS) {
            printf("Ошибка: некорректная команда или недостаточно прав\n");
            continue;
        }

        error_type result = call_func(command, &bd);
        if (result != SUCCESS) {
            const char* messages[] = {
                    [INPUT_ERROR] = "Ошибка ввода данных",
                    [MALLOC_ERROR] = "Ошибка памяти",
                    [SYSTEM_ERROR] = "Системная ошибка"
            };
            printf("Ошибка: %s\n", messages[result]);
        }
    }

    free(bd.users);
    return 0;
}
