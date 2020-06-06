#include <stdbool.h>
#include "value.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "talloc.h"

Value *makeNull(){
	Value *myValue = talloc(sizeof(Value));
	myValue->type = NULL_TYPE;
	myValue->c.car = NULL;
	myValue->c.cdr = 0;
	return myValue;
}

Value *cons(Value *newCar, Value *newCdr){
	Value *myValue = talloc(sizeof(Value));
	myValue->type = CONS_TYPE;
	myValue->c.car = newCar;
	myValue->c.cdr = newCdr;
	return myValue;
}

Value *cdr(Value *list){
	assert(list != NULL);
	return list->c.cdr;
}

Value *car(Value *list){
	assert(list != NULL);
	return list->c.car;
}

bool isNull(Value *value){
	Value *val = 0;
	val = value;
	assert(val != NULL);
	if (val->type == NULL_TYPE){
		return 1;
	}else{
		return 0;
	}
}

void display(Value *list){
	Value *ptr = 0;
	ptr = list;
	while(ptr->type != NULL_TYPE){
		if (car(ptr)== NULL){
			ptr = cdr(ptr);
		}
		if (car(ptr)->type == INT_TYPE){
			printf("%i\n",car(ptr)->i);
			ptr = cdr(ptr);
		} else if (car(ptr)->type == DOUBLE_TYPE){
			printf("%g\n",car(ptr)->d);
			ptr = cdr(ptr);
		} else if (car(ptr)->type == STR_TYPE){
			printf("%s\n",car(ptr)->s);
			ptr = cdr(ptr);
		} else{
			printf("NULL OR OTHER");
			ptr = cdr(ptr);
		}
	}
}

/*
Value *reverse(Value *list){
	Value *first = NULL;
	Value *tmp = NULL;
	while(list != NULL){
		tmp = cons(car(list),tmp);
		list = cdr(list);
	} 
	tmp = cdr(tmp);
	return tmp;
}
*/

Value *reverse(Value *list){
	Value *ptr = makeNull();
	while (list->type != NULL_TYPE){
		ptr = cons(car(list),ptr);
		list = cdr(list);
	}
	return ptr;
}

int length(Value *value){
	int counter=0;
	counter = 0;
	while(value->type != NULL_TYPE){
		counter++;
		value = value->c.cdr;
	}
	return counter;
}