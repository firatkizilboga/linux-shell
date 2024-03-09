#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE_LENGH 250

typedef enum {
    TYPE_EXECUTABLE, //any
    TYPE_ARGUMENT,  //any
    TYPE_DIRECTIVE //DIRECTIVE
}TYPE;

typedef enum {
    DIRECTIVE_PIPE = '|',
    DIRECTIVE_PARALLELIZE = ';',
    DIRECTIVE_ENDLINE = '\n',
    DIRECTIVE_ENDBUFFER = '\0'
}DIRECTIVE;

const char SEPERATOR = ' ';

typedef struct {
    char * DATA;
    TYPE type;
    struct Token* next;
} Token;

Token* TokenInit(TYPE type){
    Token* token = (Token*) malloc(sizeof(Token));

    token->type = type;
    token->DATA = (char*)malloc(sizeof(char)*MAX_LINE_LENGH);
    token->next = NULL;
    return token;
}

void TokenPrintRecursive(Token*token){
    if (token == NULL)
    {
        return;
    }
    
    printf("TYPE: %d\n", token->type);
    printf("DATA: %s\n", token->DATA);

    if (token->next == NULL)
    {
        return;
    }
    TokenPrintRecursive(token->next);
};

char* readFile(char* path);
Token* tokenzize(char* input);
void* handleTokens(Token* token);
int main(int argc, char **argv)
{
    for (int i = 0; i < argc; ++i)
    {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    Token* token = tokenzize(readFile(argv[1]));

    TokenPrintRecursive(token);

}
void* handleTokens(Token* token){}

bool isDirective(char c){
    switch (c)
    {
    case DIRECTIVE_PIPE:
        return true;
    
    case DIRECTIVE_PARALLELIZE:
        return true;
    
    case DIRECTIVE_ENDLINE:
        return true;

    case DIRECTIVE_ENDBUFFER:
        return true;
    
    default:
        return false;
    }
    return false;
};

Token* tokenzize(char* input){
    Token* token = TokenInit(TYPE_EXECUTABLE);
    Token* head = token;


    int i = 0;
    int token_i = 0;
    while (input[i] != '\0')
    {
        char c = input[i];
        switch (token->type)
        {
        case TYPE_EXECUTABLE:
            if (c==SEPERATOR)
            {
                token->DATA[token_i] = '\0';
                token->next = TokenInit(TYPE_ARGUMENT);
                token = token->next;
                token_i = 0;
                i++;
                continue;
            }
            break;
        case TYPE_ARGUMENT:
            if (isDirective(c)){
                token->DATA[token_i] = '\0';
                token->next = TokenInit(TYPE_DIRECTIVE);
                token = token->next;
                token_i = 0;

                continue;
            }
            break;
        case TYPE_DIRECTIVE:
            token->DATA[token_i] = c;
            token->DATA[token_i+1] = '\0';
            token->next = TokenInit(TYPE_EXECUTABLE);
            token = token->next;
            token_i = 0;
            i++;
            continue;
            break;
        default:
            break;
        }

        token->DATA[token_i] = c;
        i++;
        token_i++;
    }
    if (token_i != 0 && token->DATA[token_i] != DIRECTIVE_ENDBUFFER){
        token->DATA[token_i] = DIRECTIVE_ENDBUFFER;
    }
    return head;
}

char* readFile(char* path){
    //RETURNS char* BUFFER ENDING WITH '\0'

    FILE *infile;
    char* buffer;
    infile = fopen(path, "rb");

    if (infile == NULL) {
        fprintf(stderr, "Cannot open file for reading: %s\n", path);
        abort();
    }

    fseek( infile , 0L , SEEK_END);
    long lSize = ftell( infile );
    rewind( infile );
    
    buffer = (char*)malloc(sizeof(char)* (lSize+1));

    int i = 0, c;
    while ((c = fgetc(infile)) != EOF)
    {
        buffer[i] = c;
        i++;
    }
    
    fclose(infile);
    buffer[i] = '\0';

    return buffer;
}

