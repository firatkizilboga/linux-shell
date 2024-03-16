#pragma once
#define MAX_LINE_LENGTH 250
#define SEPERATOR ' '
#include <unistd.h>
#include <sys/types.h>


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

typedef struct {
    char * DATA;
    TYPE type;
    struct Token* next;
} Token;


typedef struct {
    Token* executable;     
    Token* argument;       
    Token* directive;      
    char* output_buffer;
    int output_pipefd[2];
    int input_pipefd[2];
    pid_t pid;
    struct Expression* next; 
} Expression;

typedef struct {
    pid_t pid;
    struct ParallelPID* next;
} ParallelPID;

Token* TokenInit(TYPE type);
Token* tokenize(char* input);

Expression* ExpressionInit();

void expressionsAppend(Expression* expression_tail);
void extractExpressions(Expression* expression, Token*token);
void ExpressionPrint(Expression*expression);
void ExpressionPrintRecursive(Expression*expression);
void handleExpressions(Expression*expression, bool print_prompt);
void handleExpression(Expression*expression);
void tokenzieRecursively(char* input, Token * token, int token_i);
void ParallelPIDRemove(ParallelPID* PPID, pid_t pid);
void handleQuit(Expression*expression);
void ParallelPIDListInit();

__attribute__((noreturn)) void gracefulExit(int status);

