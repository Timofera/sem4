#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "errors.h"
#define USERNAME 6
#define ADMIN_CODE 12345

typedef unsigned int uint;

typedef enum command_t{
    LOGIN,
    REG,
    LOGOUT,
    TIME,
    DATE,
    HOWMUCH,
    SANC,
    EXIT,
    UNDEFINED //некорректный ввод
}command_t;

typedef struct usr_rec{
    char username[USERNAME];
    uint pin;
    int sanc;
}usr_rec;


typedef struct program_state{
    usr_rec* users;
    uint count_users;
    uint capacity;
    int user_ind;
}program_state;

error_type check_input(program_state* bd,command_t command){
    if(command == LOGIN || command == REG){
        if(bd->user_ind != -1)
            return INPUT_ERROR;
        else
            return SUCCESS;
    }
    if(command == LOGOUT){
        if(bd->user_ind == -1){
            return INPUT_ERROR;//уже вышел
        }
        else
            return SUCCESS;
    }
    if(command == TIME || command == DATE || command == HOWMUCH || command == SANC ){
        if(bd->user_ind != -1) {
            if ((bd->users[bd->user_ind].sanc != 0)) {
                if (bd->users[bd->user_ind].sanc != -1)
                    if (bd->users[bd->user_ind].sanc)
                        bd->users[bd->user_ind].sanc--;
                return SUCCESS;
            }
            else
                return BAN;
        }
        else
            return INPUT_ERROR;
    }
    if(command == EXIT)
        return SUCCESS;
    if(command == UNDEFINED)
        return INPUT_ERROR;

}

command_t parse(){
    char buf[10];
    scanf("%9s",buf);
    if (strcmp(buf, "login") == 0)
        return LOGIN;
    if (strcmp(buf, "reg") == 0)
        return REG;
    if (strcmp(buf, "logout") == 0)
        return LOGOUT;
    if (strcmp(buf, "time") == 0)
        return TIME;
    if (strcmp(buf, "date") == 0)
        return DATE;
    if (strcmp(buf, "howmuch") == 0)
        return HOWMUCH;
    if (strcmp(buf, "sanc") == 0)
        return SANC;
    if (strcmp(buf, "exit") == 0)
        return EXIT;
    else
        return UNDEFINED;
}

error_type login(program_state* bd);
error_type logout(program_state* bd);
error_type reg(program_state* bd);
error_type time_(program_state* bd);
error_type date_(program_state* bd);
error_type howmuch(program_state* bd);
error_type sanc(program_state* bd);
error_type is_val_username(char* name);
error_type is_val_pin(uint pin);


error_type login(program_state* bd){
    char buf_name[7];
    uint buf_pin;
    printf("Введите имя\n");
    scanf("%6s",buf_name);
    printf("Введите пин\n");
    scanf("%u",&buf_pin);
    if((is_val_username(buf_name) != SUCCESS) || (is_val_pin(buf_pin) != SUCCESS))
        return INPUT_ERROR;
    else{
        for(int i = 0; i < bd->count_users;i++){
            if(strcmp(bd->users[i].username,buf_name) == 0 && bd->users[i].pin == buf_pin){
                bd->user_ind = i;
                return SUCCESS;
            }
        }
        printf("неверный логин или пороль\n");
    }

}

error_type call_func(command_t code_func,program_state* bd){
    switch (code_func) {
        case LOGIN:
            login(bd);
            break;
        case REG:
            reg(bd);
            break;
        case LOGOUT:
            logout(bd);
            break;
        case TIME:
            time_(bd);
            break;
        case DATE:
            date_(bd);
            break;
        case HOWMUCH:
            howmuch(bd);
            break;
        case SANC:
            sanc(bd);
            break;
    }
}

error_type is_val_username(char* username) {
    for (int i = 0; username[i]; i++) {
        if (!isalnum(username[i])) {
            return INPUT_ERROR;
        }
    }
    return SUCCESS;
}

error_type is_val_pin(uint pin) {
    if (pin > 100000 ) {
        return INPUT_ERROR;
    }
    return SUCCESS;
}

error_type logout(program_state* state) {
    state->user_ind = -1;
    return SUCCESS;
}

error_type time_(program_state* bd) {
    time_t now = time(NULL);
    if (now == -1) return DEF_ERROR;
    struct tm* tm = localtime(&now);
    if (!tm) return DEF_ERROR;
    printf("Текущее время: %02d:%02d:%02d\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
    return SUCCESS;
}

error_type date_(program_state* bd) {
    time_t now = time(NULL);
    if (now == -1) return DEF_ERROR;
    struct tm* tm = localtime(&now);
    if (!tm) return DEF_ERROR;
    printf("Текущая дата: %02d.%02d.%04d\n", tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900);
    return SUCCESS;
}

error_type reg(program_state* bd){
    usr_rec user_data;
    printf("Введите имя пользователя\n");
    scanf("%6s",user_data.username);
    if(is_val_username(user_data.username) != SUCCESS){
        printf("uncor name\n");
        return INPUT_ERROR;
    }
    for (int i = 0; i < bd->count_users; ++i) {
        if(strcmp(user_data.username,bd->users[i].username) == 0){
            printf("alrg exst\n");
            return INPUT_ERROR;
        }
    }
    printf("Введите pin\n");
    scanf("%u",&user_data.pin);
    if(is_val_pin(user_data.pin) != SUCCESS){
        printf("uncor pin\n");
        return INPUT_ERROR;
    }
    if( bd->count_users > bd->capacity - 1){
        usr_rec* tmp = realloc(bd->users, bd->capacity * 2);
        if(tmp == NULL){
            printf("realloc error\n");
            return REALLOC_ERROR;
        }
        bd->capacity *= 2;
        bd->users = tmp;
    }
    user_data.sanc = -1;
    bd->users[bd->count_users] = user_data;
    bd->count_users++;
    return SUCCESS;
}

error_type sanc(program_state* bd){
    int code;
    int count_sanc = 0;
    usr_rec user_data;
    int ind = -1;
    printf("Введите имя пользователя для санк\n");
    scanf("%6s",user_data.username);
    if(is_val_username(user_data.username) != SUCCESS){
        printf("incor name\n");
        return INPUT_ERROR;
    }
    for (int i = 0; i < bd->count_users; ++i) {
        if(strcmp(user_data.username,bd->users[i].username) == 0){
            ind = i;
        }
    }
    if(ind == -1){
        printf("not exst\n");
        return INPUT_ERROR;
    }
    printf("введите кол-во санк\n");
    scanf("%d",&count_sanc);
    if(count_sanc < -1){
        printf("incor sanc\n");
        return INPUT_ERROR;
    }
    printf("введите код\n");
    scanf("%d",&code);
    if(code != ADMIN_CODE){
        printf("inc code\n");
        return INPUT_ERROR;
    }
    bd->users[ind].sanc = count_sanc;
    return SUCCESS;
}

error_type howmuch(program_state* bd) {
    struct tm enter_time;
    char flag[3];
    printf("введите дату в формате: dd.mm.yyyy -<flag>\n");
    scanf("%02d.%02d.%04d %2s", &enter_time.tm_mday, &enter_time.tm_mon, &enter_time.tm_year,flag); //segfault
    if (enter_time.tm_mday > 31 || enter_time.tm_mday < 0 || enter_time.tm_mon > 12 || enter_time.tm_mon < 0 ||
        enter_time.tm_year < 1970) {
        printf("inc date\n");
        return INPUT_ERROR;
    }
    enter_time.tm_year -= 1900;
    enter_time.tm_mon --;
    enter_time.tm_hour = enter_time.tm_isdst = enter_time.tm_min = enter_time.tm_sec =enter_time.tm_wday =enter_time.tm_yday = 0;
    time_t aim_time = mktime(&enter_time);
    time_t now = time(NULL);
    if (now == -1) return DEF_ERROR;

    unsigned long long res = fabs(difftime(aim_time,now));
    if(flag[0] == '-'){
        if(flag[1] == 's'){
            printf("%llu\n",res);
        }
        if(flag[1] == 'm'){
            printf("%llu\n",res/ 60);
        }
        if(flag[1] == 'h'){
            printf("%llu\n",res / (60 * 60));
        }
        if(flag[1] == 'y'){
            printf("%llu\n",res / (unsigned long long)((3600 * 24 * 365.25 )));
        }
    }
    else{
        printf("inc flag\n");
        return INPUT_ERROR;
    }
}

int main(){
    program_state bd;
    bd.capacity = 10;
    bd.users = malloc(sizeof(usr_rec)* bd.capacity);
    if(bd.users == NULL)
        return MALLOC_ERROR;
    bd.user_ind = -1;
    bd.count_users = 0;
    while (1){
        if(bd.user_ind == -1)
            printf(">");
        else
            printf("%s>",bd.users[bd.user_ind].username);
        command_t command = parse();
        if(command == EXIT){
            free(bd.users);
            return 0;
        }
        error_type flag_input = check_input(&bd,command);
        if(flag_input != SUCCESS){
            if(flag_input == BAN)
                printf("ходы кончились\n");
            if(flag_input == INPUT_ERROR)
                printf("incorrect command\n");
            continue;
        }
        call_func(command,&bd);
    }
}