#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"
#include "simplevector.h"

#define bool unsigned char

typedef struct{
    char** fileInputs;
    char* outputFile;
} CompileOptions;

bool isNum(char c){
    return c>='0' && c<='9';
}

bool isAlpha(char c){
    return (c>='a' && c<='z') || (c>='A' && c<='Z');
}

bool isAlphaNum(char c){
    return isNum(c)||isAlpha(c);
}

bool isIdentifierLegal(char c){
    return isAlphaNum(c) || c=='_';
}


bool isAssignableOperator(char c){
    return c=='+' || c=='-' || c=='/' || c=='*' || c=='%' || c=='!' || c=='&' || c=='|' || c=='^' || c=='~';
}

bool isOperator(char c){
    return isAssignableOperator(c) || c=='=' || c=='>' || c=='<' || c=='?' || c==':' || c=='.';
}

bool isDoubleOperator(char a, char b){
    return (b=='=' && (isAssignableOperator(a) || a=='>' || a=='<' || a=='=')) || (a==b && (a=='+' || a=='-' || a=='>' || a=='<' || a=='&' || a=='|')); 
}

bool isTripleOperator(char a, char b, char c){
    return (a=='>' && b=='>' && c=='=') || (a=='<' && b=='<' && c=='=');
}

bool isGrouping(char c){
    return c=='[' || c==']' || c=='{' || c=='}' || c=='(' || c==')';
}

char fpeek(FILE* file){
    char c = fgetc(file);
    ungetc(c,file);
    return c;
}

Token* parseNum(FILE* fptr, char* buffer, int* col, int* bufferi){
    while(isNum(buffer[(*bufferi)++]=fgetc(fptr)));

    if(buffer[*bufferi-1] == '.'){
        while(isNum(buffer[(*bufferi)++]=fgetc(fptr))) *col++;
        ungetc(buffer[*bufferi-1],fptr);
        buffer[*bufferi-1] = '\0';

        char* tokenStr = (char*) malloc(sizeof(char)**bufferi);
        strcpy(tokenStr, buffer);
        return token_make(DOUBLE_LIT, tokenStr);
    }
    
    ungetc(buffer[(*bufferi)-1],fptr);
    buffer[(*bufferi)-1] = '\0';
    char* tokenStr = (char*) malloc(sizeof(char)**bufferi);
    strcpy(tokenStr, buffer);
    return token_make(INT_LIT, tokenStr);
}

int main(int argc, char** argv){
    FILE* fptr;
    char buffer[256] = {'\0'};
    int bufferi = 0;

    fptr = fopen(argv[1],"r");
    if(fptr==NULL) return -1;

    const char* keywords[] = {
        "class",
        "struct",
        "enum",
        "return",
    };
    const int keywordsLen = 4;
    const char* basicTypes[] = {
        "int",
        "float",
        "string",
        "char",
        "bool"
    };
    Vector* typeRegistry = vector_from(sizeof(char*),5,basicTypes);

    Vector* tokens = vector_new(sizeof(Token));

    int row = 1;
    int col = 1;
    char c;
    CompilerState state = COMPILER_NORMAL;
    while((c=fgetc(fptr)) != EOF){
        buffer[0] = c;
        bufferi=1;
        if(isAlpha(c)){
            // Could be type, keyword, or identifier
            while(isIdentifierLegal(buffer[bufferi++]=fgetc(fptr))) col++;

            //Replace non-matching character onto stack
            ungetc(buffer[bufferi-1],fptr);
            buffer[bufferi-1] = '\0';

            char* tokenStr = (char*) malloc(sizeof(char)*bufferi);
            strcpy(tokenStr, buffer);

            for(int i=0;i<keywordsLen;i++){
                if(strcmp(keywords[i],buffer) == 0){
                    vector_append(tokens, token_make(KEYWORD, tokenStr));

                    if(state != COMPILER_NORMAL){
                        printf("Error: improper usage of keyword %s at Row: %d, Column: %d\n", buffer, row, col-bufferi+1);
                        return -1;
                    }
                    if(i<3){ // Type declarations
                        state = COMPILER_TYPEDECL;
                    }
                    goto _loop_end;
                }
            }
            for(int i=0;i<vector_size(typeRegistry);i++){
                if(strcmp(*(char**)vector_get(typeRegistry,i), buffer) == 0){
                    vector_append(tokens, token_make(TYPE_NAME, tokenStr));

                    if(state != COMPILER_NORMAL){
                        printf("Error: improper usage of type %s at Row: %d, Column: %d\n", buffer, row, col-bufferi+1);
                        return -1;
                    }
                    goto _loop_end;
                }
            }
            if(state == COMPILER_TYPEDECL){
                vector_append(typeRegistry, &tokenStr);
                vector_append(tokens, token_make(TYPE_NAME, tokenStr));
                state = COMPILER_NORMAL;
            } else if(state==COMPILER_NORMAL){
                vector_append(tokens, token_make(IDENTIFIER, tokenStr));
            }
        } else if(isNum(c)){
            vector_append(tokens, parseNum(fptr, buffer, &col, &bufferi));
        } else if(c=='+'){
            if(isNum(fpeek(fptr))){
                vector_append(tokens,parseNum(fptr,buffer,&col,&bufferi));
            } else {
                goto _parse_op;
            }
            col++;
        } else if(c=='-'){
            if(isNum(fpeek(fptr))){
                vector_append(tokens,parseNum(fptr,buffer,&col,&bufferi));
            } else {
                goto _parse_op;           
            }
            
        } else if(c=='"'){
            while((buffer[bufferi++]=fgetc(fptr)) != '"') col++;
            buffer[bufferi] = '\0';

            char* tokenStr = (char*) malloc(sizeof(char)*bufferi);
            strcpy(tokenStr, buffer);
            vector_append(tokens, token_make(STRING_LIT, tokenStr));
            col+=bufferi;
        } else if(c=='\n'){
            row++;
            col=1;
        } else if(c==';'){
            Token t = {TERMINATOR,";"};
            vector_append(tokens, &t);
            col++;
        } else if(isOperator(c)){
            _parse_op:
            char c2 = fgetc(fptr);
            char c3 = fgetc(fptr);


            if(isTripleOperator(c,c2,c3)){
                buffer[bufferi++] = c2;
                buffer[bufferi++] = c3;
            } else if(isDoubleOperator(c,c2)){
                buffer[bufferi++] = c2;
                ungetc(c3,fptr);
            } else {
                ungetc(c3,fptr);
                ungetc(c2,fptr);
            }
            buffer[bufferi] = '\0';
            char* tokenStr = (char*) malloc(sizeof(char)*bufferi);
            strcpy(tokenStr, buffer);
            vector_append(tokens, token_make(OPERATOR, tokenStr));
            col+=bufferi;
        } else if(isGrouping(c)){
            buffer[bufferi] = '\0';
            char* tokenStr = (char*) malloc(sizeof(char)*bufferi);
            strcpy(tokenStr, buffer);
            vector_append(tokens, token_make(GROUPING, tokenStr));
            col+=bufferi;
        } else {
            col++;
        }
        _loop_end:
        // printToken((Token*)vector_get(tokens,vector_size(tokens)-1));
    }
    for(int i=0;i<vector_size(tokens);i++){
        token_print((Token*)vector_get(tokens,i));
    }

    // vector_free_custom(tokens,token_free);
    // vector_free_custom(typeRegistry,free);
    return 0;
}