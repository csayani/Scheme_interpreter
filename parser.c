#include <stdbool.h>
#include "value.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "linkedlist.h"
#include "talloc.h"

Value *addToParseTree(Value *tree, int *depth, Value *token);

Value *parse(Value *tokens) {

    Value *tree = makeNull();
    int depth = 0;

    Value *current = tokens;
    assert(current != NULL && "Error (parse): null pointer");
		//current->type != NULL_TYPE
    while (current->type != NULL_TYPE) {
        Value *token = car(current);
        tree = addToParseTree(tree, &depth, token);
        current = cdr(current);
				assert(tree->type == CONS_TYPE);
    }
    if (depth != 0) {
				printf("ERROR");
				texit(0);
        //syntaxError();
    }
		Value *revTree; 
		revTree = reverse(tree);
		return revTree;
}

Value *addToParseTree(Value *tree, int *depth, Value *token){
	//Value *stack = malloc(sizeof(Value));
	if(token->type != CLOSE_TYPE){
		//printf("not close type");
		Value *item;
		item = cons(token,tree);
		if(token->type == OPEN_TYPE){
			(*depth)++;
		}
		assert(item->type == CONS_TYPE);
		return item;
	} else {
		assert(tree != NULL);
		(*depth)--;
		if (*depth < 0){
			printf("ERROR");
			texit(1);
		}

		Value *newList = makeNull();
		while (car(tree)->type != OPEN_TYPE){
			newList = cons(car(tree),newList);
			tree = cdr(tree);
		}
		tree = cdr(tree);
		Value *newHead = talloc(sizeof(Value));
		newHead = cons(newList, tree);
		return newHead;
	}
}


void printTree(Value *list){
	while (list->type != NULL_TYPE){
        if(car(list)->type == INT_TYPE){
            printf("%i\n",car(list)->i);
        } else if (car(list)-> type == CONS_TYPE){
					printf("(");
					printTree(car(list));
					printf(") ");
				} else if(car(list)->type == BOOL_TYPE){
            printf("#%s\n", car(list)->s);
            //printf(":boolean\n");
           // printf("%s",car(list)->s);
        } else if(car(list)->type == OPEN_TYPE){
            printf("%s\n",car(list)->s);
        } else if(car(list)->type == STR_TYPE){
            //printf(":string");
            printf("\"%s\"\n",car(list)->s);
        } else if(car(list)->type == DOUBLE_TYPE){
            printf("%lf\n",car(list)->d);
        } else if(car(list)->type == PTR_TYPE){
            printf("ptr");
        } else if(car(list)->type == CLOSE_TYPE){
            printf("%s\n",car(list)->s);
        } else if(car(list)->type == SYMBOL_TYPE){
            printf("%s\n", car(list)->s);
        } else {
            printf("()");
        }
        //printf("%p",cdr);
        list = cdr(list);
    } 
}