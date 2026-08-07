#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"
#include "simplevector.h"

#define bool unsigned char

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

int hexValue(char c){
    return (c>='a' && c<='z') ? c-'a'+10 :
           (c>='A' && c<='Z') ? c-'A'+10 :
           (c>='0' && c<='9') ? c-'0' :
           -1;
}

char fpeek(FILE* file){
    char c = fgetc(file);
    ungetc(c,file);
    return c;
}

typedef struct {
    const char* src;
    FILE* fptr;
    size_t line;
    size_t col;

    Vector* tokens;
} Lexer;

Lexer* lexer_new(const char* src){
    Lexer* lexer = (Lexer*)malloc(sizeof(lexer));
    lexer->src = src;
    lexer->fptr = fopen(src,"r");
    lexer->line = 1;
    lexer->col = 1;
    lexer->tokens = vector_new(sizeof(Token));
    return lexer;
}

void emit(Lexer* lexer, TokenType type){
    Token* t = token_make(type,NULL);
    t->row = lexer->line;
    t->col = lexer->col;
    vector_append(lexer->tokens, t);
}

void emit_val(Lexer* lexer, TokenType type, char* val){
    Token* t = token_make(type,val);
    t->row = lexer->line;
    t->col = lexer->col;
    vector_append(lexer->tokens, t);
}

void emit_token(Lexer* lexer, Token* token){
    token->row = lexer->line;
    token->col = lexer->col;
    vector_append(lexer->tokens, token);
}

bool match(Lexer* lexer, char c){
    if(fpeek(lexer->fptr) != c){
        return 0;
    }

    fgetc(lexer->fptr);
    return 1;
}

char consume(Lexer* lexer){
    char c = fgetc(lexer->fptr); 
    if(c=='\n'){
        lexer->line++;
        lexer->col=1;
    } else {
        lexer->col++;
    }
    return c;
}

char peek(Lexer* lexer){
    char c = fgetc(lexer->fptr);
    ungetc(c, lexer->fptr);
    return c;
}

void panic(){
    exit(EXIT_FAILURE);
}

void parseChar(Lexer* l){
    Token* t = token_new(TOK_CHAR_LIT);
    if(match(l,'\\')){
        if(match(l,'a')){
            t->charVal = '\a';
        } else if(match(l,'b')){
            t->charVal = '\b';
        } else if(match(l,'f')){
            t->charVal = '\f';
        } else if(match(l,'n')){
            t->charVal = '\n';
        } else if(match(l,'r')){
            t->charVal = '\r';
        } else if(match(l,'t')){
            t->charVal = '\t';
        } else if(match(l,'v')){
            t->charVal = '\v';
        } else if(match(l,'\'')){
            t->charVal = '\'';
        } else if(match(l,'\"')){
            t->charVal = '\"';
        } else if(match(l,'\\')){
            t->charVal = '\\';
        } else if(match(l,'x')){
            if(hexValue(peek(l)) != -1){
                t->charVal = 0x10*hexValue(consume(l));
            } else {
                panic();
            }
            if(hexValue(peek(l)) != -1){
                t->charVal += hexValue(consume(l));
            } else {
                panic();
            }
            
            if(hexValue(peek(l)) != -1){
                printf("Sorry, I don't know how to do unicode yet :(");
            }
        } else if(peek(l) >= '0' && peek(l) <= '7'){
            t->charVal = 64 * consume(l)-'0';
            if(peek(l) >= '0' && peek(l) <= '7') {
                t->charVal = 8*consume(l)-'0';
            } else {
                panic();
            }

            if(peek(l) >= '0' && peek(l) <= '9'){
                t->charVal = consume(l)-'0';
            } else {
                panic();
            }
        } else {
            printf("Error: unknown escape sequence \"\\%c\"",peek(l));
        }
    } else {
        t->charVal = consume(l);
    }

    if(!match(l,'\'')){
        panic();
    }
    emit_token(l,t);
}

Vector* lexer_lexFile(char* fstring){
    char buffer[256] = {'\0'};
    int bufferi = 0;

    const char* keywords[] = {
        "class",
        "struct",
        "enum",
        "return",
    };
    const int keywordsLen = 4;

    Lexer* l = lexer_new(fstring);

    char c;
    while(peek(l) != EOF){
        c = consume(l);
        buffer[0] = c;
        bufferi=1;
        if(isAlpha(c)){
            // Could be type, keyword, or identifier
            while(isIdentifierLegal(peek(l))){
                buffer[bufferi++]=consume(l);
            }

            buffer[bufferi] = '\0';

            char* tokenStr = (char*) malloc(sizeof(char)*bufferi);
            strcpy(tokenStr, buffer);

            for(int i=0;i<keywordsLen;i++){
                if(strcmp(keywords[i],buffer) == 0){
                    emit_val(l,TOK_KEYWORD,tokenStr);
                    goto _loop_end;
                }
            }
            emit_val(l,TOK_IDENTIFIER,tokenStr);
            
        } else if(isNum(c)){
            while(isNum(peek(l))){
                buffer[bufferi++]=consume(l);
            }
            if(match(l,'.')){
                while(isNum(peek(l))){
                    buffer[bufferi++]=consume(l);
                }
                buffer[bufferi]='\0';
                Token* t = token_new(TOK_DOUBLE_LIT);
                t->doubleVal = atof(buffer);
                emit_token(l, t);
                continue;
            }
            buffer[bufferi]='\0';
            Token* t = token_new(TOK_INT_LIT);
            t->intVal = atoi(buffer);
            emit_token(l,t);
        } else {
            switch(c){
                case '+':
                    emit(l, match(l,'+') ? TOK_PLUS_PLUS : 
                            match(l,'=') ? TOK_PLUS_EQUAL:
                            TOK_PLUS);
                    break;
                case '-':
                    emit(l, match(l,'-') ? TOK_MINUS_MINUS :
                            match(l,'=') ? TOK_MINUS_EQUAL :
                            match(l,'>') ? TOK_MINUS_GT :
                            TOK_MINUS);
                    break;
                case '*':
                    emit(l, match(l,'=') ? TOK_STAR_EQUAL : TOK_STAR);
                    break;
                case '/':
                    emit(l, match(l,'=') ? TOK_SLASH_EQUAL : TOK_SLASH);
                    break;
                case '%':
                    emit(l, match(l,'=') ? TOK_MOD_EQUAL : TOK_MOD);
                    break;
                case '&':
                    emit(l, match(l,'&') ? TOK_AND_AND :
                            match(l,'=') ? TOK_AND_EQUAL :
                            TOK_AND);
                    break;
                case '|':
                    emit(l, match(l,'|') ? TOK_PIPE_PIPE :
                            match(l,'=') ? TOK_PIPE_EQUAL :
                            TOK_MINUS);
                    break;
                case '^':
                    emit(l, match(l,'=') ? TOK_CARAT_EQUAL : TOK_CARAT);
                    break;
                case '>':
                    emit(l, match(l,'>') ? (match(l,'=') ? TOK_GT_GT_EQUAL : TOK_GT_GT):
                            match(l,'=') ? TOK_GT_EQUAL :
                            TOK_GT);
                    break;
                case '<':
                    emit(l, match(l,'<') ? (match(l,'=') ? TOK_LT_LT_EQUAL : TOK_LT_LT):
                            match(l,'=') ? TOK_LT_EQUAL :
                            TOK_LT);
                    break;
                case '=':
                    emit(l, match(l,'=') ? TOK_EQUAL_EQUAL : TOK_EQUAL);
                    break;
                case '.':
                    emit(l, match(l,'.') ? TOK_DOT_DOT : TOK_DOT);
                    break;
                case '?':
                    emit(l,TOK_QUESTION);
                    break;
                case ':':
                    emit(l,TOK_COLON);
                    break;
                case '\'':
                    parseChar(l);
                    break;
                case '(':
                    emit(l,TOK_OPEN_PARENS);
                    break;
                case ')':
                    emit(l,TOK_CLOSE_PARENS);
                    break;
                case '[':
                    emit(l,TOK_OPEN_BRACKET);
                    break;
                case ']':
                    emit(l,TOK_CLOSE_BRACKET);
                    break;
                case '{':
                    emit(l,TOK_OPEN_BRACES);
                    break;
                case '}':
                    emit(l,TOK_CLOSE_BRACES);
                    break;
                case ';':
                    emit(l,TOK_TERMINATOR);
                    break;
                case '\"':
                    int tstartl = l->line;
                    int tstartc = l->col;
                    bufferi--;
                    while(peek(l) != EOF && peek(l) != '\"'){
                        buffer[bufferi++] = consume(l);
                    }
                    if(!match(l,'\"')){
                        printf("Error: unclosed string literal starting at Line %d Column %d\n",tstartl,tstartc);
                        panic();
                    }
                    buffer[bufferi] = '\0';

                    char* tokenStr = (char*) malloc(sizeof(char)*bufferi);
                    strcpy(tokenStr, buffer);
                    emit_val(l,TOK_STRING_LIT,tokenStr);
                case ' ':
                case '\t':
                case '\n':
                    break;
                default:
                    printf("Sorry, I don't understand symbol %c at Line %d Column %d\n",c,l->line,l->col);
                    break;
            }
        }
        _loop_end:
    }
    emit(l,TOK_FILE_END);
    
    return l->tokens;
}