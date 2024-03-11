#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <tokenizer.h>

char* readFile(char* path);
Token* tokenize(char* input);
void* handleTokens(Token* token);
int main(int argc, char **argv)
{
    for (int i = 0; i < argc; ++i)
    {
        printf("argv[%d]: %s\n", i, argv[i]);
    }
    Token* token = tokenize(readFile(argv[1]));

    TokenPrintRecursive(token);
    Expression*expression = ExpressionInit();
    extractExpressions(expression, token);


    handleExpressions(expression);
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

