#include "authorization.h"

char sysuser[10][50] = {"root"};
char syspass[10][50] = {"123456"};
char loginuser[50];

int checkuser(){
    char user[50], pass[50];
    printf("username:");
    scanf("%s", user);
    printf("password:");
    scanf("%s", pass);
    int i = 0;
    for(; i < 11; i++){
        if(!strcmp(user, sysuser[i]))
            break;
        if(i == 10){
            printf("user doesn't exit, try again\n");
            checkuser();
        }
    }
    if(!strcmp(pass, syspass[i])){
        strcpy(loginuser, sysuser[i]);
        return 1;
    }
    printf("user doesn't exit, try again\n");
    checkuser();
}

int adduser(){
    char user[50], pass[50];
    printf("username:");
    scanf("%s", user);
    printf("password:");
    scanf("%s", pass);
    int i = 0;
    for(; i< 10; i++){
        if(strlen(sysuser[i]) == 0)
            break;
    }
    strcpy(sysuser[i], user);
    strcpy(syspass[i], pass);
    printf("用户添加成功\n");
}