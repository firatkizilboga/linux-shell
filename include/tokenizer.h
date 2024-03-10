#define MAX_LINE_LENGTH 250
#define SEPERATOR ' '

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
    int pipefd[2];         
    pid_t pid;
    bool DONE;
    struct Expression* next; 
} Expression;

typedef struct {
    pid_t pid;
    struct ParallelPID* next;
} ParallelPID;

Token* TokenInit(TYPE type);
void TokenPrintRecursive(Token*token);
Token* tokenize(char* input);
Expression* ExpressionInit();
void extractExpressions(Expression* expression, Token*token);
void ExpressionPrint(Expression*expression);
void ExpressionPrintRecursive(Expression*expression);
void handleExpression(Expression*expression);
void handleExpressions(Expression*expression);