#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <tokenizer.h>
#include <interpreter.h>

extern Expression*expression_head;
char* readFile(char* path);
Token* tokenize(char* input);
void* handleTokens(Token* token);
int main(int argc, char **argv)
{
    ParallelPIDListInit();

    if (argc < 2)
    {
        interpreter();
    }

    expression_head = NULL;
    char * input = readFile(argv[1]);
    Token* token = tokenize(input);
    free(input);
    
    expression_head = ExpressionInit();
    extractExpressions(expression_head, token);
    handleExpressions(expression_head, true);
    gracefulExit(EXIT_SUCCESS);
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

