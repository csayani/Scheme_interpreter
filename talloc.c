#include <stdbool.h>
#include "value.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
static bool headNULL = true;
Value *head;

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size) {
	//init pointer
	void *ptr;
	//create head if LL is null
	if(headNULL) {
		head = malloc(sizeof(Value));
		head->type = NULL_TYPE;
		headNULL = false;
		//check
		//printf("Constructed head succesfully\n");
	}
	//allocate space for ptr;
	ptr = malloc(size);
	//create struct and init value struct to add
	Value *toAdd = malloc(sizeof(Value));
	toAdd->type = CONS_TYPE;
	toAdd->c.car = malloc(sizeof(Value));
	toAdd->c.car->type = PTR_TYPE;
	toAdd->c.car->p = ptr;
	toAdd->c.cdr = head;
	head = toAdd;
	return ptr;
}

void tfree() {
	if (headNULL){
		int i = 0;
	} else {
	//iterate until we hit the last node (null type)
	while (head->type != NULL_TYPE) {
		//save value of cdr
		Value *next = head->c.cdr;
		//free data (car) and cons type Value stsruct
		free(head->c.car->p);
		free(head->c.car);
		free(head);
		//iterate
		head = next;
	}

	}
	//free the last null type node
	if (head != NULL){
			free(head);
	}

	//set headNULL true since list is deleted
	headNULL = true;
}

void texit(int status){
	//call the two functions in the order specified in the header file
	tfree();
	exit(0);
}