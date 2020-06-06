#include <stdbool.h>
#include "value.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "linkedlist.h"
#include "talloc.h"

//adding to test display tokens within tokenize
void displayTokens(Value *list);
int isDig(char c);

Value *tokenize(){
    char charRead;
    Value *list = makeNull();
    charRead = (char)fgetc(stdin);

    while (charRead != EOF){

        Value *token = talloc(sizeof(Value));
        token->type = NULL_TYPE;

        if (charRead == ' '){
            int q = 0;
        } else if (charRead == ')'){
            token->type = CLOSE_TYPE;
            token->s = ")";
        } else if (charRead == '('){
            token->type = OPEN_TYPE;
            token->s = "(";
        } else if (charRead == '[' || charRead == ']' || charRead == '@'){
            printf("ERROR:invalid char");
            texit(0);
        } 
        else if (charRead == '#') {
            charRead = (char)fgetc(stdin);
            if (charRead == 't' ||charRead == 'f' || charRead == 'T' ||charRead == 'F'){
                token->type = BOOL_TYPE;
                if (charRead == 't' || charRead == 'T'){
                    token->s = "t";
                } else {
                    token->s = "f";
                }
            } else {
                printf("ERROR: INVALID BOOL\n");
            }
        } else if (charRead == '+'){
            token->type = SYMBOL_TYPE;
            token->s = "+";
        } else if (charRead == '-'){
            charRead = (char)fgetc(stdin);
            if (isalpha(charRead)==1 || isDig(charRead)==1 || charRead != ' '){
                int i = 0, x = 0;
                double doub = 0;
                bool isDoub = false;
                char *arr = talloc(sizeof(char)*300);
								//isalpha(charRead)== 1 || isDig(charRead)==1 || charRead != ' '
                while (charRead != ' ' && charRead != '\377' && charRead != ';' && charRead != '"'){
                    if (charRead == '.'){
                        isDoub = true;
                    }
                    arr[i] = charRead;
                    charRead = (char)fgetc(stdin);
                    i++;
                }
               
                arr[i] = '\0';
                if (isDoub){
                    token->type = DOUBLE_TYPE;
                    sscanf(arr,"%lf",&doub);
                    doub = doub*-1;
                    token->d = doub;
                } else {
                    token->type = INT_TYPE;
                    sscanf(arr,"%i",&x);
                    x = x*-1;
                    token->i = x;
                }
            } else if (charRead == ' '){
                token->type = SYMBOL_TYPE;
                token->s = "-";
            } else {
                printf("charRead not space or alphanum\n");
            }
        } else if (charRead == '*'){
            token->type = SYMBOL_TYPE;
            token->s = "*";
        } else if (isalpha(charRead) != 0 && charRead != ' '){
            int i =0;
            char *arr = talloc(sizeof(char)*300);
            while (isalpha(charRead) != 0 || isDig(charRead) == 1|| charRead == '-' || charRead == '?') {
                
                arr[i] = charRead;
                charRead = (char)fgetc(stdin);
                i++;
            }
            arr[i] = '\0';
            token->type = SYMBOL_TYPE;
            token->s = arr;
            //COMPENSATION
            if (charRead == ')'){
                ungetc(')',stdin);
            }
            //printf("%c :looking for paren\n",charRead);
        } else if (isDig(charRead) == 1){
            int i= 0, x = 0;
            bool isDoub = false;
            double doub = 0;
            char *arrA = talloc(sizeof(char)*300);
            while (isDig(charRead) == 1 || charRead == '.'){
                arrA[i] = charRead;
                if (charRead == '.'){
                    isDoub = true;
                }
                charRead = (char)fgetc(stdin);
                i++;
            }
            //COMPENSATION
            if (charRead == ')'){
                ungetc(')',stdin);
            }
            arrA[i] = '\0';
            if (isDoub){
                //printf("DETECTING DOUBLE");
                token->type = DOUBLE_TYPE;
                sscanf(arrA,"%lf",&doub);
                token->d = doub;
            } else {
                token->type = INT_TYPE;
                sscanf(arrA,"%i",&x);
                token->i = x;
            }
        } else if (charRead == '"'){
            charRead = (char)fgetc(stdin);
            //printf("HERE: %c\n",charRead);
            int i = 0;
            char *str = talloc(sizeof(char)* 300);
            while (charRead != '"'){
                str[i] = charRead;
                charRead = (char)fgetc(stdin);
                i++;
            }
            str[i] = '\0';
            token->type = STR_TYPE;
            token->s = str;
        } else if (charRead == '>'){
            token->type = SYMBOL_TYPE;
            token->s = ">";
        } else if (charRead == '<'){
            token->type = SYMBOL_TYPE;
            token->s = "<";
        } else if (charRead == '/'){
            token->type = SYMBOL_TYPE;
            token->s = "/";
        } else if (charRead == '='){
            token->type = SYMBOL_TYPE;
            token->s = "=";
        } else if (charRead == ';'){
            charRead = (char)fgetc(stdin);
            if (charRead == ';'){
                int i = 0;
                //char *arr = malloc(sizeof(char)*300);
                while (charRead != '\n'){
                    //arr[i] = charRead;
                    charRead = (char)fgetc(stdin);
                    i++;
                }
                //arr[i] = '\0';
                //token->type = STR_TYPE;
                //token->s = arr;
            }
        }
        if(token->type != NULL_TYPE){
            list = cons(token,list);
        }

       //list = cons(token,list);

        charRead = (char)fgetc(stdin);
    }
    Value *revList = reverse(list);
    return revList;
}


void displayTokens(Value *list){
    //list->type != NULL_TYPE
    //list->type == CONS_TYPE
    while (list->type != NULL_TYPE){
        if(car(list)->type == INT_TYPE){
            printf("%i:integer\n",car(list)->i);
        } else if(car(list)->type == BOOL_TYPE){
            printf("#%s:boolean\n", car(list)->s);
            //printf(":boolean\n");
           // printf("%s",car(list)->s);
        } else if(car(list)->type == OPEN_TYPE){
            printf("%s:open\n",car(list)->s);
        } else if(car(list)->type == STR_TYPE){
            //printf(":string");
            printf("\"%s\":string\n",car(list)->s);
        } else if(car(list)->type == DOUBLE_TYPE){
            printf("%lf:double\n",car(list)->d);
        } else if(car(list)->type == PTR_TYPE){
            printf("ptr");
        } else if(car(list)->type == CLOSE_TYPE){
            printf("%s:close\n",car(list)->s);
        } else if(car(list)->type == SYMBOL_TYPE){
            printf("%s:symbol\n", car(list)->s);
        } else if (car(list)->type == CONS_TYPE){
            printf("CONS\n");
            displayTokens(car(list));
        } 
        else {
            printf("else");
        }
        //printf("%p",cdr);
        list = cdr(list);
    } 
   //printf("in display");
}

int isDig(char c){
    if (c == ')' || c == '('){
        //printf("CHECKIGN IS DIG");
        return 0;
    }
    if (c == '1'){
        return 1;
    } else if (c == '2') {
        return 1;
    } else if (c == '3') {
        return 1;
    } else if (c == '4') {
        return 1;
    } else if (c == '5') {
        return 1;
    } else if (c == '6') {
        return 1;
    } else if (c == '7') {
        return 1;
    } else if (c == '8') {
        return 1;
    }else if (c == '9') {
        return 1;
    }else if (c == '0') {
        return 1;
    } else {
        return 0;
    }
    return 0;
}