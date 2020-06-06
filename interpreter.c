#include <stdbool.h>
#include "value.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "linkedlist.h"
#include "talloc.h"
//TODO: --letstar--, letrec, set!, --begin--, --and--, --or--
//prims: -,+,>,<
//note review error display in prim less than and greater than
//declaring functions for use
void printItem(Value *tree);
Value *lookupSymbol(Value *symbol, Frame *frame);
Value *evalIf(Value *args, Frame *frame);
Value *evalLet (Value *args, Frame *frame);
Value *evalLambda(Value *args, Frame *frame);
Value *apply(Value *function, Value *args);
Value *evalDefine(Value *args, Frame *frame);
void bind(char *name, Value *(*function)(struct Value *),Frame *frame);
Value *primitiveAdd(Value *args);
Value *primitiveNull(Value *args);
Value *primitiveCdr(Value *args);
Value *primitiveCar(Value *args);
Value *primitiveCons(Value *args);
Value *evalAnd(Value *args, Frame *frame);
Value *evalOr(Value *args, Frame *frame);
Value *evalLetStar(Value *args, Frame *frame);
Value *evalBegin(Value *args, Frame *frame);
Value *primitiveSubtract(Value *args);
Value *primitiveGreaterThan(Value *args);
Value *primitiveLessThan(Value *args);
Value *primitiveEqualsTo(Value *args);
//Evaluates Value tree with given Frame
Value *eval(Value *tree, Frame *frame){
	//non special forms: just return the Value struct
	switch(tree->type){
		case INT_TYPE:
			return tree;
			break;
		case STR_TYPE:
			return tree;
			break;
		case DOUBLE_TYPE:
			return tree;
			break;
		case BOOL_TYPE:
			return tree;
			break;
		case CLOSURE_TYPE:
			return tree;
			break;
		case PRIMITIVE_TYPE:
			return tree;
			break;
		//if symbol type, lookup the symbol and return ptr to value from given
		//frame
		case SYMBOL_TYPE:{
			Value *args = cdr(tree);
			return lookupSymbol(tree,frame);
		}
			break;
		case CONS_TYPE:
		{	//initialize function keyword and args
			Value *first = car(tree);
			Value *args = cdr(tree);

			//special forms
			if (!strcmp(first->s,"if")) {
        	return evalIf(args, frame);
      } else if (!strcmp(first->s,"let")){
					return evalLet(args,frame);
			} else if (!strcmp(first->s,"quote")) {
					if (length(args)==1){
						Value *newArgs = cdr(tree);
						return car(newArgs);
					}else {
						printf("EVAL ERROR IN QUOTE");
						texit(1);
					}
			} else if (!strcmp (first->s,"define")){
					return evalDefine(args,frame);
			} else if (!strcmp (first->s,"lambda")){
					return evalLambda(args,frame);
			} else if (!strcmp(first->s,"begin")){
					return evalBegin(args,frame);
			} else if (!strcmp(first->s,"and")){
					return evalAnd(args,frame);
			} else if (!strcmp(first->s,"let*")){
					return evalLetStar(args,frame);
			} 
			else if (!strcmp(first->s,"or")){
					return evalOr(args,frame);
			} else {
					Value *first = car(tree);
					Value *evaledOperator = eval(first, frame);
					Value *evaledArgs = makeNull();
					while (args->type != NULL_TYPE){
						evaledArgs = cons(eval(car(args),frame),evaledArgs);
						args = cdr(args);
					}
					if(evaledOperator->type == PRIMITIVE_TYPE){
                evaledArgs = reverse(evaledArgs);
                return evaledOperator->pf(evaledArgs);
            }
            else {
                return apply(evaledOperator, evaledArgs);
            }
			}
			break; 
	}
		default:
			printf("EVALUATION ERROR: OTHERTYPE");
			return tree;
			break;
	}
	printf("EVALUATION ERROR:ERROR");
	return tree;
}


Value *apply(Value *function, Value *args){
	//Check to make sure applying a procedure
	if (function->type != CLOSURE_TYPE){
		printf("EVALUATION ERROR:not calling on closure type");
		texit(1);
	}
	//create the Frame to bind args to
	Frame *newFrame = talloc(sizeof(Frame));
	newFrame->bindings = makeNull();
	newFrame->parent = function->cl.frame;
	Value *params = function->cl.paramNames;
	//check each param can be matched
	if (length(params) != length(args)){
		printf("ERROR: mismatch");
		texit(1);
	}
	//Creating the frame here
	while (params->type != NULL_TYPE){
		Value *val = car(args); 
		Value *newBinding = cons(car(params),val);
		newFrame->bindings = cons(newBinding, newFrame->bindings);
		params = cdr(params);
		args = cdr(args);
	}
	//Evaluate closure with newFrame
	Value *code = function->cl.functionCode;
	Value *evalbody = makeNull();
	evalbody = eval(car(code),newFrame);
	return evalbody;
}

//Looks up symbol in given frame
Value *lookupSymbol(Value *symbol, Frame *frame) {
	//check looking up a symbol type
	if (symbol->type != SYMBOL_TYPE){
		printf("EVALUATION ERROR: SYMBOL ERROR");
		texit(1);
	}
	//outer loop takes care of multiple/nested frames
	while (frame != NULL){
		Value *bindings = frame->bindings;
		//iterate through bindings and return the Value if the symbol is found
		while(bindings->type != NULL_TYPE){
			Value *curBind = car(bindings);
			//reminder: binding storage: car of curBind is symbol, cdr of curBind is the value
			if(!strcmp(symbol->s,(car(curBind)->s))){
				return cdr(curBind);
			} else {
				bindings = cdr(bindings);
			}
		}

		frame = frame->parent;
	}
	//at this point, symbol is not found in frame
	printf("EVALUATION ERROR: ");
	printf("SYMBOL NOT FOUND");
	texit(1);
	return frame->bindings;
}

//Evaluates special form if
Value *evalIf(Value *args, Frame *frame) {
	Value *result;
	//check that the proper amount of args are supplied
	if(length(args)<3){
		printf("ERROR:LENGTH LESS THAN 3");
		texit(1);
	}else if(length(args)== 0){
		printf("ERROR: LENGTH is zero");
		texit(1);
	}
	//evaluate the condition and check if t or f
	Value *compare = eval((car(args)),frame);
	if (!strcmp(compare->s,"t")){
		//return the evaluated second to last arg if true
		result = eval(car(cdr(args)),frame);
	}else if (!strcmp(compare->s,"f")){
		//return evaluated last arg if false
		result = eval((car(cdr(cdr(args)))),frame);
	}else {
			//error with boolean evaluation occured
			printf("ERROR: args->s not true or false\n");
			texit(1);
	}
	return result;
}

//evaluate let special form
Value *evalLet (Value *args, Frame *frame){
	//init frame to transfer bindings into
	Frame *newFrame = talloc(sizeof(Frame));
	newFrame->parent = frame;
	newFrame->bindings = makeNull();
	Value *listofbindings = car(args);
	//iterate through list of bindings and copy into 
	//the new frame
	while (listofbindings->type != NULL_TYPE){
		//check iterating through symbol types
		if (car(car(listofbindings))->type != SYMBOL_TYPE){
			printf("EVALUATION ERROR: symbol");
			texit(1);
		}
		Value *curPair = car(listofbindings);
		Value *val = eval(car(cdr(curPair)),frame);
		Value *pair = cons(car(curPair),val);
		newFrame->bindings = cons(pair, newFrame->bindings);
		listofbindings = cdr(listofbindings);
	}
	Value *body = cdr(args);
	if (body->type != NULL_TYPE){
		while(cdr(body)->type != NULL_TYPE){
			eval(car(body),newFrame);
			body = cdr(body);
		}
		return eval(car(body),newFrame);
	}
	printf("EVALUATION ERROR: at end");
	return args;
}
//evaluate lambda special form and returns closure
Value *evalLambda(Value *args, Frame *frame){
	//error check on length of args and type
	if (args->type != CONS_TYPE){
		printf("EVALUATION ERROR: TYPE");
		texit(1);
	}
	if (length(args)!=2){
		printf("EVALUATION ERROR: TOO FEW ARGS");
		texit(1);
	}
  //init closure
	Value *closure = talloc(sizeof(Value));
  closure->type = CLOSURE_TYPE;
	//store list of params values
  Value *paramList = car(args);
  closure->cl.paramNames = makeNull();
	//store params in closure
  while (paramList->type != NULL_TYPE) {
		if (car(paramList)->type != SYMBOL_TYPE){
			printf("EVALUATION ERROR: not symbol");
			texit(1);
		}
  closure->cl.paramNames = cons(car(paramList), closure->cl.paramNames);
	paramList = cdr(paramList);
	}
	//store the function and frame in closure
	Value *functionCode = cdr(args);
  closure->cl.functionCode = functionCode;
  closure->cl.frame = frame;
  return closure;
}
//Function that constructs main frame and 
//evaluates elements of the tree
void interpret(Value *tree){
	//init the main frame and its bindings
	Frame *main = talloc(sizeof(Frame));
	main->parent = NULL;
	main->bindings = makeNull();
	//bind primitives
	bind("+",primitiveAdd,main);
	bind("null?",primitiveNull,main);
	bind("car",primitiveCar,main);
  bind("cdr",primitiveCdr,main);
	bind("cons",primitiveCons,main);
	bind("-",primitiveSubtract,main);
	bind(">",primitiveGreaterThan,main);
	bind("<",primitiveLessThan,main);
	bind("=",primitiveEqualsTo,main);
	//iterate through tree create frame through
	//evaluating elements and consing
	while (tree->type != NULL_TYPE){
		Value *result = makeNull();
		Value *next = eval(car(tree),main);
		result = cons(next, result);
		//keep list structure 
		if (car(result)->type == NULL_TYPE){
			printf("()");
		}
		if (car(result)->type != VOID_TYPE) {
			result = reverse(result);
			printItem(result);
			printf("\n");
		}
		tree = cdr(tree);
	}
}
Value *evalDefine (Value *args, Frame *frame){
	//error checking
	if (cdr(args)==NULL){
		printf("EVALUATION ERROR: NO BINDINGS AT ALL");
		texit(1);
	}
	if ((car(args))->type != SYMBOL_TYPE){
		printf("EVALUATION ERROR: MUST BIND TO SYMBOL");
		texit(1);
	}
	if(cdr(cdr(args))==NULL){
		printf("EVALUATION ERROR: NULL ERROR");
		texit(1);
	}
	// takes bindings and modifies global frame
	Value *expression = eval(car(cdr(args)),frame);
	Value *newBinds = cons(car(args),expression);
	frame->bindings = cons(newBinds,frame->bindings);
	Value *toReturn = talloc(sizeof(Value));
	toReturn->type = VOID_TYPE;
	return toReturn;
}
//printing function tree, reworked from linkedlist.c
void printItem(Value *tree) {
 if (tree->type == NULL_TYPE){
		printf("()");
	} else {
	while (tree->type != NULL_TYPE) {
		if (tree->type != CONS_TYPE) {
			printItem(tree);
			break;
		}
		switch (car(tree)->type) {
		case INT_TYPE:
			printf("%i\n",car(tree)->i);
			break;
		case STR_TYPE:
			printf("%c",'"');
			printf("%s",car(tree)->s);
			printf("%c",'"');
			printf(" ");
			break;
		case DOUBLE_TYPE:
			printf("%f\n",car(tree)->d);
			break;
		case PTR_TYPE:
			printf("%p\n",car(tree)->p);
			break;
		case NULL_TYPE:
			printf(" ");
			break;
		case OPEN_TYPE:
			printf("(");
			break;
		case CLOSE_TYPE:
			printf(")");
			break;
		case BOOL_TYPE:
			printf("#%s",car(tree)->s);
			break;
		case SYMBOL_TYPE:
			printf("%s ",car(tree)->s);
			break;
		case CLOSURE_TYPE:
			printf("#<procedure>");
			break;
		case CONS_TYPE:
			printf("(");
			if(cdr(car(tree))->type != NULL_TYPE && cdr(car(tree))->type != CONS_TYPE){
                    Value *null = makeNull();
                    printItem(cons(car(car(tree)), null));
										//note for self: space after period makes test case 5 pass
                    printf(". ");
                    printItem(cons(cdr(car(tree)), null));
                }else {
                    printItem(car(tree));
                }
			printf(")");
			break;
		default:
			printf(" ");
			break;
	}
	tree = cdr(tree);
	}
	}
}
// adds primitives to main (top level) binding list
void bind(char *name, Value *(*function)(struct Value *),Frame *frame){
	//init value, set pf to point to passed in function, set type as primitive
	Value *val = talloc(sizeof(Value));
	val->type = PRIMITIVE_TYPE;
	val->pf = function;
	//init the symbol to bind the functions to, set the s attribute to the passed in name
	Value *symbol = talloc(sizeof(Value));
	symbol->type = STR_TYPE;
	symbol->s = name;
	Value *newBind = cons(symbol,val);
	//actual adding to frame occurs here
	frame->bindings = cons(newBind, frame->bindings);
}

Value *primitiveAdd(Value *args) {
		//init sum, and boolean flag for if a real number is encountered
    double result = 0;
		bool real = false;
		//in both ifs, iterate through args and increment result accordingly
    while(args->type != NULL_TYPE) {
        if(car(args)->type == INT_TYPE){
						result += (double) car(args)->i;
            args = cdr(args);
        } else if (car(args)->type == DOUBLE_TYPE){
						//if real number is encountered, adjust flag so double type is returned
						real = true;
            result += car(args)->d;
            args = cdr(args);
        } else {
            printf("EVALUATION ERROR : arguments given are not ints or doubles");
            texit(1);
        }
    }
		//return a int type or double type based on value of boolean flag
		if (real == true){
			Value *sum = talloc(sizeof(Value));
			sum->d = result;
			sum->type = DOUBLE_TYPE;
			return sum;
		} else {
			Value *sum = talloc(sizeof(Value));
			sum->type = INT_TYPE;
			sum->i = (int) result;
			return sum;
		}
}

Value *primitiveNull(Value *args) {
	//error check number of args
    if(length(args) != 1){
        printf("EVALUATION ERROR : null requires one input");
        texit(1);
    }
		//check if the arg is null type, return Value struct of Boolean Type to 
		//reflect result
    Value *toReturn = talloc(sizeof(Value));
    toReturn->type = BOOL_TYPE;
    if (car(args)->type == NULL_TYPE) {
        toReturn->s = "t";
    } else {
        toReturn->s = "f";
    }
    return toReturn;
}

/* primitives for pairs below are fairly straightforward. Error check then return the correct element given tree structure. Car and Cdr defs can be found in linkedlist.c*/
Value *primitiveCdr(Value *args) {
    if(length(args) != 1){
        printf("EVALUATION ERROR: cdr requires one arg");
        texit(1);
    }
    if(args->type != CONS_TYPE){
        printf("EVALUATION ERROR: cdr requires cons type arg");
        texit(1);
    }
    return cdr(car(args));
}
Value *primitiveCar(Value *args) {
    if(length(args) != 1){
        printf("EVALUATION ERROR : car requires one input");
        texit(1);
    }
    if(car(args)->type != CONS_TYPE){
        printf("EVALUATION ERROR : car requires cons type arg");
        texit(1);
    }
    return car(car(args));
}
Value *primitiveCons(Value *args){
		if (length(args) != 2){
			printf("EVALUATION ERROR: incorrect number of args");
			texit(1);
		}
    if(cdr(args)->type == CONS_TYPE){
        return cons(car(args), car(cdr(args)));
    } else{
        return cons(car(args), cdr(args));
    }
}

Value *evalAnd(Value *args, Frame *frame){
	if(length(args) != 2){
		printf("EVALUATION ERROR: NOT GIVEN TWO ARGS");
		texit(1);
	}
	Value *first = eval(car(args),frame);
	Value *second = eval(car(cdr(args)),frame);
	Value *toReturn = talloc(sizeof(Value));
	toReturn->type = BOOL_TYPE;
	if (first->type == BOOL_TYPE && second->type == BOOL_TYPE){
		if (!strcmp(first->s,"t") && !strcmp(second->s,"t")){
			toReturn->s = "t";
		} else {
			toReturn->s = "f";
		}
	} else {
		printf("EVALUATION ERROR: NOT GIVEN BOOL ARGS");
		texit(1);
	}
	return toReturn;
}

Value *evalOr(Value *args, Frame *frame){
	if(length(args) != 2) {
    printf("EVALUATION ERROR: WRONG NUM ARGS");
		texit(1);
	}

	Value *first = eval(car(args), frame);
	Value *second = eval(car(cdr(args)), frame);
	Value *toReturn = talloc(sizeof(Value));
	toReturn->type = BOOL_TYPE;
	if(first->type == BOOL_TYPE && second->type == BOOL_TYPE) {
		if ( !strcmp(first->s,"t") || !strcmp(second->s,"t") ){
			toReturn->s = "t";
		} else {
			toReturn->s = "f";
		}
	} else {
			printf("EVALUATION ERROR");
			texit(1);
	}
	return toReturn;
	}

Value *evalLetStar(Value *args, Frame *frame){
  if(length(args) != 2){
    printf("EVALUATION ERROR : INCORRECT NUMBER OF ARGS");
    texit(1);
  }
  Frame *curFrame = talloc(sizeof(Frame));
  curFrame->bindings = makeNull();
  curFrame->parent = frame;
  Value *listOfBindings = car(args);
  while(listOfBindings->type != NULL_TYPE){
    Value *pair = car(listOfBindings);
    
		if (pair->type != CONS_TYPE){
			printf("ERROR: NOT CONS TYPE");
			texit(1);
		} if (length(pair) != 2) {
			printf("ERROR: bindings wrong length");
		} if (car(pair)->type != SYMBOL_TYPE){
			printf("ERROR: assigning to not a symbol");
			texit(1);
		}

    curFrame->bindings = cons(eval(car(cdr(pair)), curFrame),
			      curFrame->bindings);
    curFrame->bindings = cons(car(pair), curFrame->bindings);
    listOfBindings = cdr(listOfBindings);
  }
  return evalBegin(cdr(args), curFrame);
}

Value *evalBegin(Value *args, Frame *frame){
	Value *val;
	while(args->type != NULL_TYPE){
		val = eval(car(args),frame);
		args = cdr(args);
	}
	return val;
}

Value *primitiveSubtract(Value *args){
	double result = 0.0;
	double first = 0.0;
	double second = 0.0;
	bool real = false;

	if (car(args)->type == INT_TYPE){
		first = (double)car(args)->i;
		result += first;
	} else if (car(args)->type == DOUBLE_TYPE){
		real = true;
		first = car(args)->d;
		result += car(args)->d;
	} else {
		printf("ERROR: FIRST ARG NOT DOUBLE OR INT");
	}

	if (car(cdr(args))->type == INT_TYPE){
		second = (double)car(cdr(args))->i;
		result -= second;
	} else if (car(cdr(args))->type == DOUBLE_TYPE){
		real = true;
		second = car(cdr(args))->d;
		result -= second;
	} else {
		printf("ERROR: ARGS NOT DOUBLES OR INT");
		texit(1);
	}

	Value *toReturn = talloc(sizeof(Value));
	if (real){
		toReturn->type = DOUBLE_TYPE;
		toReturn->d = result;
		return toReturn;
	} else {
		toReturn->type = INT_TYPE;
		toReturn->i = (int) result;
		return toReturn;
	}
}

Value *primitiveGreaterThan(Value *args){
	if (length(args)!=2){
		printf("ERROR: INCORRECT NUM ARGS GIVEN TO GREATER THAN");
		texit(1);
	}

	Value *first = car(args);
	Value *second = car(cdr(args));
	Value *toReturn = talloc(sizeof(Value));
	toReturn->type = BOOL_TYPE;

	if (first->type == INT_TYPE){
			if (second->type == INT_TYPE){
				if (first->i > second->i){
					toReturn->s = "t";
				} else {
					toReturn->s = "f";
				}
			} else if (second->type == DOUBLE_TYPE){
				if (first->i > second->d){
					toReturn->s = "t";
				} else {
					toReturn->s = "f";
				}
			} else {
					printf("EVALUATION ERROR : incorrect comparision types");
					texit(1);
			}
	} else if (first->type  == DOUBLE_TYPE) {
			if (second->type == INT_TYPE) {
				if (first->d > second->i){
					toReturn->s = "t";
				} else {
					toReturn->s = "f";
				} 
			} else if (second->type == DOUBLE_TYPE){
					if (first->d > second->d) {
						toReturn->s = "t";
					} else {
						toReturn->s = "f";
					}
			} 
	} else {
		printf("EVALUATION ERROR");
		texit(1);
	}
	return toReturn;
}

Value *primitiveLessThan(Value *args){

	if (length(args)!=2){
		printf("ERROR: INCORRECT NUM ARGS GIVEN TO GREATER THAN");
		texit(1);
	}

	Value *first = car(args);
	Value *second = car(cdr(args));
	Value *toReturn = talloc(sizeof(Value));
	toReturn->type = BOOL_TYPE;

	if (first->type == INT_TYPE){
			if (second->type == INT_TYPE){
				if (first->i < second->i){
					toReturn->s = "t";
				} else {
					toReturn->s = "f";
				}
			} else if (second->type == DOUBLE_TYPE){
				if (first->i < second->d){
					toReturn->s = "t";
				} else {
					toReturn->s = "f";
				}
			} else {
					printf("EVALUATION ERROR : incorrect comparision types");
					texit(1);
			}
	} else if (first->type  == DOUBLE_TYPE) {
			if (second->type == INT_TYPE) {
				if (first->d < second->i){
					toReturn->s = "t";
				} else {
					toReturn->s = "f";
				} 
			} else if (second->type == DOUBLE_TYPE){
					if (first->d < second->d) {
						toReturn->s = "t";
					} else {
						toReturn->s = "f";
					}
			} 
	} else {
		printf("EVALUATION ERROR");
		texit(1);
	}
	return toReturn;
}

Value *primitiveEqualsTo(Value *args) {
	if (length(args) != 2){
		printf("ERROR: INCORRECT NUMBER ARGS");
		texit(1);
	}
	double first;
	double second;

	if (car(args)->type == INT_TYPE){
		first = (double) car(args)->i;
	} else if (car(args)->type == DOUBLE_TYPE){
		first = car(args)->d;
	} else {
		printf("ERROR: First arg not correct type");
		texit(1);
	}

	if (car(cdr(args))->type == INT_TYPE){
		second = (double) car(cdr(args))->i;
	} else if (car(cdr(args))->type == DOUBLE_TYPE){
		second = car(cdr(args))->d;
	} else {
		printf("ERROR: SECOND ARG WRONG TYPE");
		texit(1);
	}
	Value *toReturn = talloc(sizeof(Value));
	toReturn->type = BOOL_TYPE;
	if (first == second){
		toReturn->s = "t";
	} else {
		toReturn->s = "f";
	}
	return toReturn;
}