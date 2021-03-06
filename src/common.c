/**
 * @file common.c
 * 
 * Knihovna pro obecné věci
 * 
 * IFJ Projekt 2018, Tým 13
 * 
 * @author <xforma14> Klára Formánková
 * @author <xlanco00> Jan Láncoš
 * @author <xsebel04> Vít Šebela
 * @author <xchalo16> Jan Chaloupka
 */

#include "common.h"

void *safeMalloc(size_t _Size){
	void *ret = malloc(_Size);
	if(ret == NULL){
		fprintf(stderr, "[INTERNAL] Fatal error - cannot allocate memory\n");
		exit(99);
	}
	return ret;
}

void *safeRealloc(void *_Block, size_t _Size){
	void *ret = realloc(_Block, _Size);
	if(ret == NULL){
		fprintf(stderr, "[INTERNAL] Fatal error - cannot allocate memory\n");
		exit(99);
	}
	return ret;
}

char *stringToInterpret(char *rawString){
	int rawLen = strlen(rawString);
	
	int max = rawLen + 1;
	int pos = 7;
	char *out = safeMalloc(sizeof(char) * (max + 20));
	strcpy(out, "string@");

	for(int i = 1; i < rawLen - 1; i++){
		if(max <= pos + 4){
			max += 100;
			out = safeRealloc(out, sizeof(char) * max);
		}
		
		out[pos] = '\\';
		if(rawString[i] == '\\'){
			i++;
			if(rawString[i] == '"'){
				out[pos] = '"';
				pos -= 3;
			}else if(rawString[i] == 'n'){
				out[pos+1] = '0';
				out[pos+2] = '1';
				out[pos+3] = '0';
			}else if(rawString[i] == 't'){
				out[pos+1] = '0';
				out[pos+2] = '0';
				out[pos+3] = '9';
			}else if(rawString[i] == '\\'){
				out[pos+1] = '0';
				out[pos+2] = '9';
				out[pos+3] = '2';
			}else if(rawString[i] == 'x'){
				int l = isxdigit(rawString[i+2])?2:1;
				char hex[3] = {0};
				hex[0] = rawString[i+1];
				if(l > 1) hex[1] = rawString[i+2];
				i += l;
				sprintf(&out[pos+1], "%03ld", strtol(hex, NULL, 16));
			}else{
				out[pos+1] = '0';
				out[pos+2] = '3';
				out[pos+3] = '2';
			}
			pos += 3;
		}else if(rawString[i] == '#'){
			out[pos+1] = '0';
			out[pos+2] = '3';
			out[pos+3] = '5';
			pos += 3;
		}else if(rawString[i] <= ' '){
			out[pos+1] = '0';
			out[pos+2] = '3';
			out[pos+3] = '2';
			pos += 3;
		}else{
			out[pos] = rawString[i];
		}

		pos++;
	}

	out[pos] = '\0';
	out = safeRealloc(out, sizeof(char) * (pos + 1));
	return out;
}

char *intToInterpret(char *rawInt){
	char *out = safeMalloc(sizeof(char) * 50);
	if(rawInt[0] == '0' && strlen(rawInt) > 1 && rawInt[1] == 'b'){
		// printf neumí rozeznat 0b
		sprintf(out, "int@%ld", strtol(&rawInt[2], NULL, 2));
	}else sprintf(out, "int@%ld", strtol(rawInt, NULL, 0));
	return safeRealloc(out, strlen(out));
}

char *floatToInterpret(char *rawFloat){
	char *out = safeMalloc(sizeof(char) * (50));
	sprintf(out, "float@%a", strtod(rawFloat, NULL));
	return safeRealloc(out, strlen(out));
}

char *trueToInterpret(){
	char *out = safeMalloc(sizeof(char) * 10);
	strcpy(out, "bool@true");
	return out;
}

char *falseToInterpret(){
	char *out = safeMalloc(sizeof(char) * 11);
	strcpy(out, "bool@false");
	return out;
}

char *nilToInterpret(){
	char *out = safeMalloc(sizeof(char) * 8);
	strcpy(out, "nil@nil");
	return out;
}

char *varToInterpret(char *id){
	char *out = safeMalloc(sizeof(char) * (strlen(id) + 4));

	strcpy(out, "LF@");
	strcat(out, id);

	return out;
}

char *funcToInterpret(char *id){
	char *out = safeMalloc(sizeof(char) * (strlen(id) + 4));

	strcpy(out, "GF@");
	strcat(out, id);

	return out;
}

void generateBaseCode(){
	printf(".IFJcode18\n\
\n\
DEFVAR GF@$tmp\n\
DEFVAR GF@$tmp2\n\
\n\
JUMP $main\n\
\n\
LABEL $checkIfBool\n\
	CALL $getType\n\
	JUMPIFNEQ $printTypeError TF@%%return string@bool\n\
	RETURN\n\
\n\
LABEL $checkIfLtGt\n\
	CALL $getType\n\
	JUMPIFEQ $checkIfLtGt$end TF@%%return string@string\n\
	JUMPIFEQ $checkIfLtGt$end TF@%%return string@int\n\
	JUMPIFNEQ $printTypeError TF@%%return string@float\n\
	LABEL $checkIfLtGt$end\n\
	RETURN\n\
\n\
LABEL $checkIfEql\n\
	CALL $getType\n\
	\n\
	JUMPIFEQ $decideEqlOp$diff TF@%%return string@different\n\
	PUSHS TF@rv\n\
	PUSHS TF@lv\n\
	RETURN\n\
	\n\
	LABEL $decideEqlOp$diff\n\
	PUSHS bool@false\n\
	PUSHS bool@true\n\
	RETURN\n\
\n\
LABEL $decideDivOp\n\
	JUMPIFEQ $decideDivOp$float TF@%%return string@float\n\
	IDIVS\n\
	RETURN\n\
	LABEL $decideDivOp$float\n\
	DIVS\n\
	RETURN\n\
\n\
LABEL $checkIfAdd\n\
	CALL $getType\n\
	JUMPIFEQ $checkIfAdd$string TF@%%return string@string\n\
	JUMPIFEQ $checkIfAdd$num TF@%%return string@int\n\
	JUMPIFNEQ $printTypeError TF@%%return string@float\n\
	LABEL $checkIfAdd$num\n\
	ADDS\n\
	RETURN\n\
	LABEL $checkIfAdd$string\n\
	POPS GF@$tmp2\n\
	POPS GF@$tmp\n\
	CONCAT GF@$tmp GF@$tmp GF@$tmp2\n\
	PUSHS GF@$tmp\n\
	RETURN\n\
\n\
LABEL $checkIfNum\n\
	CALL $getType\n\
	JUMPIFEQ $checkIfNum$end TF@%%return string@int\n\
	JUMPIFNEQ $printTypeError TF@%%return string@float\n\
	LABEL $checkIfNum$end\n\
	RETURN\n\
\n\
LABEL $getType\n\
	CREATEFRAME\n\
	DEFVAR TF@%%return\n\
	DEFVAR TF@lv\n\
	DEFVAR TF@lt\n\
	DEFVAR TF@rv\n\
	DEFVAR TF@rt\n\
	POPS TF@rv\n\
	POPS TF@lv\n\
	TYPE TF@lt TF@lv\n\
	TYPE TF@rt TF@rv\n\
	\n\
	# defaultne nejsou stejneho typu\n\
	MOVE TF@%%return string@different\n\
	\n\
	JUMPIFEQ $getType$if$1$1 TF@lt TF@rt\n\
	JUMPIFEQ $getType$if$1$2 TF@lt string@int\n\
	JUMPIFEQ $getType$if$1$3 TF@rt string@int\n\
	JUMP $getType$if$1$end\n\
	LABEL $getType$if$1$1 # jsou stejneho typu\n\
		MOVE TF@%%return TF@lt\n\
	JUMP $getType$if$1$end\n\
	LABEL $getType$if$1$2 # lt je int\n\
		JUMPIFNEQ $getType$if$2 TF@rt string@float\n\
			MOVE TF@%%return TF@rt\n\
			INT2FLOAT TF@lv TF@lv\n\
		LABEL $getType$if$2\n\
	JUMP $getType$if$1$end\n\
	LABEL $getType$if$1$3 # rt je int\n\
		JUMPIFNEQ $getType$if$3 TF@lt string@float\n\
			MOVE TF@%%return TF@lt\n\
			INT2FLOAT TF@rv TF@rv\n\
		LABEL $getType$if$3\n\
	LABEL $getType$if$1$end\n\
	\n\
	PUSHS TF@lv\n\
	PUSHS TF@rv\n\
	RETURN\n\
\n\
LABEL $printTypeError\n\
	WRITE string@\\010[RUNTIME]\\032Type\\032error\\032-\\032incompatible\\032types\\032(\n\
	WRITE TF@lt\n\
	WRITE string@\\032with\\032\n\
	WRITE TF@rt\n\
	WRITE string@)\n\
	EXIT int@4\n\
\n\
LABEL $checkDivByZero\n\
	POPS GF@$tmp\n\
	TYPE GF@$tmp2 GF@$tmp\n\
	JUMPIFEQ $checkDivByZero$float GF@$tmp2 string@float\n\
	JUMPIFNEQ $checkDivByZero$end GF@$tmp int@0\n\
	JUMP $checkDivByZero$err\n\
	LABEL $checkDivByZero$float\n\
	JUMPIFNEQ $checkDivByZero$end GF@$tmp float@0x0p+0\n\
	LABEL $checkDivByZero$err\n\
	WRITE string@\\010[RUNTIME]\\032Divide\\032by\\032zero\\032error\\010\n\
	EXIT int@9\n\
	LABEL $checkDivByZero$end\n\
	PUSHS GF@$tmp\n\
	RETURN\n\
\n\
LABEL $checkIfReturnBool\n\
	TYPE GF@$tmp TF@$return\n\
	JUMPIFEQ $checkIfReturnBool$end GF@$tmp string@bool\n\
	WRITE string@\\010[RUNTIME]\\032Type\\032error\\032-\\032condition\\032in\\032statement\\032must\\032be\\032bool\\010\n\
	EXIT int@4\n\
	LABEL $checkIfReturnBool$end\n\
	RETURN\n\
\n\
\n\
LABEL inputi\n\
	PUSHFRAME\n\
	DEFVAR LF@$return\n\
	READ LF@$return int\n\
	POPFRAME\n\
	RETURN\n\
\n\
LABEL inputf\n\
	PUSHFRAME\n\
	DEFVAR LF@$return\n\
	READ LF@$return float\n\
	POPFRAME\n\
	RETURN\n\
\n\
LABEL inputs\n\
	PUSHFRAME\n\
	DEFVAR LF@$return\n\
	READ LF@$return string\n\
	POPFRAME\n\
	RETURN\n\
\n");
	printf("\
LABEL length\n\
	PUSHFRAME\n\
	DEFVAR LF@$return\n\
	DEFVAR LF@type\n\
	TYPE LF@type LF@%%1\n\
	JUMPIFEQ length$cont LF@type string@string\n\
		WRITE string@\\010[RUNTIME]\\032Type\\032error\\032-\\032Function\\032length()\\032expected\\032type\\032string\\010\n\
		EXIT int@4\n\
	LABEL length$cont\n\
	STRLEN LF@$return LF@%%1\n\
	POPFRAME\n\
	RETURN\n\
\n\
LABEL chr\n\
	PUSHFRAME\n\
	DEFVAR LF@$return\n\
	DEFVAR LF@type\n\
	TYPE LF@type LF@%%1\n\
	JUMPIFEQ chr$cont LF@type string@int\n\
		WRITE string@\\010[RUNTIME]\\032Type\\032error\\032-\\032Function\\032chr()\\032expected\\032type\\032int\\010\n\
		EXIT int@4\n\
	LABEL chr$cont\n\
	INT2CHAR LF@$return LF@%%1\n\
	POPFRAME\n\
	RETURN\n\
\n\
LABEL ord\n\
	PUSHFRAME\n\
	DEFVAR LF@$return\n\
	TYPE GF@$tmp LF@%%2\n\
	JUMPIFEQ ord$cont GF@$tmp string@int\n\
		WRITE string@\\010[RUNTIME]\\032Type\\032error\\032-\\032Function\\032ord()\\032expected\\032second\\032parameter\\032type\\032int\\010\n\
		EXIT int@4\n\
	LABEL ord$cont\n\
	DEFVAR LF@len\n\
	MOVE LF@$return nil@nil\n\
	CREATEFRAME\n\
	DEFVAR TF@%%1\n\
	MOVE TF@%%1 LF@%%1\n\
	CALL length\n\
	MOVE LF@len TF@$return\n\
	CLEARS\n\
	PUSHS LF@%%2\n\
	PUSHS int@0\n\
	LTS\n\
	PUSHS LF@%%2\n\
	PUSHS LF@len\n\
	LTS\n\
	NOTS\n\
	ORS\n\
	POPS TF@$return\n\
	JUMPIFEQ ord$nil TF@$return bool@true\n\
	STRI2INT LF@$return LF@%%1 LF@%%2\n\
	LABEL ord$nil\n\
	POPFRAME\n\
	RETURN\n\
\n\
LABEL substr\n\
	PUSHFRAME\n\
	DEFVAR LF@$return\n\
	MOVE LF@$return nil@nil\n\
	DEFVAR LF@len\n\
	CREATEFRAME\n\
	DEFVAR TF@%%1\n\
	MOVE TF@%%1 LF@%%1\n\
	CALL length\n\
	MOVE LF@len TF@$return\n\
	CLEARS\n\
	PUSHS LF@%%2\n\
	PUSHS int@0\n\
	LTS\n\
	PUSHS LF@%%2\n\
	PUSHS LF@len\n\
	GTS\n\
	#NOTS\n\
	PUSHS LF@%%3\n\
	PUSHS int@0\n\
	LTS\n\
	ORS\n\
	ORS\n\
	POPS GF@$tmp\n\
	JUMPIFEQ substr$1 GF@$tmp bool@true\n\
	MOVE LF@$return string@\n\
	ADD LF@%%3 LF@%%3 LF@%%2\n\
	LABEL substr$2$cmp\n\
	CLEARS\n\
	PUSHS LF@%%2\n\
	PUSHS LF@%%3\n\
	LTS\n\
	PUSHS LF@%%2\n\
	PUSHS LF@len\n\
	LTS\n\
	ANDS\n\
	POPS GF@$tmp\n\
	JUMPIFNEQ substr$2$end GF@$tmp bool@true\n\
	GETCHAR GF@$tmp LF@%%1 LF@%%2\n\
	CONCAT LF@$return LF@$return GF@$tmp\n\
	ADD LF@%%2 LF@%%2 int@1\n\
	JUMP substr$2$cmp\n\
	LABEL substr$2$end\n\
	LABEL substr$1\n\
	\n\
	POPFRAME\n\
	RETURN\n\
	\n\
\n\
# Konec preddefinovanych funkci\n\
LABEL $main$main\n\
\n");
}