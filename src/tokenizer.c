#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <tokenizer.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

ParallelPID* PPIDHead;
char* output_buffer;

char* trim(char* str) {
    int start, end, len;
    char* trimmed;

    // Find first non-whitespace character
    for (start = 0; str[start] != '\0' && isspace((unsigned char)str[start]); start++);

    // Find last non-whitespace character
    for (end = strlen(str) - 1; end >= 0 && isspace((unsigned char)str[end]); end--);

    len = end - start + 1;

    if (len <= 0) {
        trimmed = malloc(1);
        if (trimmed != NULL) {
            trimmed[0] = '\0'; // Return empty string if all characters were whitespace
        }
    } else {
        trimmed = malloc(len + 1);
        if (trimmed != NULL) {
            strncpy(trimmed, str + start, len);
            trimmed[len] = '\0';
        }
    }

    return trimmed;
}


Token* TokenInit(TYPE type){
    Token* token = (Token*) malloc(sizeof(Token));

    token->type = type;
    token->DATA = (char*)malloc(sizeof(char)*MAX_LINE_LENGTH*11);
    token->next = NULL;
    return token;
}

void TokenPrintRecursive(Token*token){
    if (token == NULL)
    {
        return;
    }

    if (token->next == NULL)
    {
        return;
    }
    TokenPrintRecursive(token->next);
};

void ExpressionReadPipe(Expression*expression){
    ssize_t bytes_read;
    const size_t bufferSize = 1000; // Define your buffer size here
    expression ->output_buffer = (char*)malloc(sizeof(char) * MAX_LINE_LENGTH);

    while ((bytes_read = read(expression->output_pipefd[0], expression ->output_buffer, bufferSize - 1)) > 0) {
        expression ->output_buffer[bytes_read] = '\0'; // Null terminate the string
    }
    close(expression->output_pipefd[0]); // Close read end of the pipe
}

int checkPID(pid_t pid) {
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);

    // Check if waitpid returned because the child process has a status update
    if (result == pid) {
        // Check if the child process terminated normally
        if (WIFEXITED(status)) {
            return 1;  // Process has terminated normally
        }
    }
    return 0;  // Process is still running or waitpid returned an error
}
Expression* ExpressionFindByPID(Expression*expression, pid_t pid){
    if (expression->pid == pid)
    {
        return expression;
    }

    if (expression->next)
    {
        return ExpressionFindByPID(expression->next, pid);
    }
    return NULL;
};

void ParallelPIDListPoll(Expression* expressions_head, DIRECTIVE directive){
    if (!PPIDHead->next)
    {
        return;
    }
    
    ParallelPID* curr = PPIDHead->next;
    while (curr)
    {
        if (checkPID(curr->pid))
        {
            Expression* exp = ExpressionFindByPID(expressions_head, curr->pid);
            if (exp)
            {
                ExpressionReadPipe(exp);
                if (directive == DIRECTIVE_ENDBUFFER || directive == DIRECTIVE_ENDLINE)
                {
                    printf("%s", exp->output_buffer);
                }

                if (directive == DIRECTIVE_PIPE)
                {
                    strcat(output_buffer, exp->output_buffer);
                }
            }
            ParallelPIDRemove(PPIDHead, curr->pid);
            return;
        }
        curr = curr -> next;
    }
    
};

void ParallelPIDListInit(){
    PPIDHead = (ParallelPID*) malloc(sizeof(ParallelPID));
    PPIDHead -> next = NULL;
    PPIDHead -> pid = -1;
}

void ParallelPIDInsert(ParallelPID* PPID, pid_t pid){
    if (PPID->next)
    {
        return ParallelPIDInsert(PPID->next, pid);
    } else {
        PPID->next = (ParallelPID*) malloc(sizeof(ParallelPID));
        PPID = PPID->next;
        PPID->next = NULL;
        PPID -> pid = pid;
        return;
    }
}

void ParallelPIDRemove(ParallelPID* PPID, pid_t pid){
     if (PPID->next)
    {
        ParallelPID *next = PPID->next;

        if (next->pid == pid)
        {
            ParallelPID* middle = PPID->next;
            PPID->next = middle->next;

            free(middle);
            return;
        }

        return ParallelPIDRemove(PPID->next, pid);
    }
}

int ParallelPIDCount(ParallelPID* PPID){
    if (PPID->next != NULL)
    {
        return ParallelPIDCount(PPID->next) + 1;
    }
    return 0;
}

static inline bool isDirective(char c){
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

Token* tokenize(char* input){
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

    if (token->type != TYPE_DIRECTIVE)
    {
        token->next = TokenInit(TYPE_DIRECTIVE);
        token = token->next;
        token->DATA[0] = DIRECTIVE_ENDBUFFER;
    }
    
    
    return head;
}

Expression* ExpressionInit(){
    Expression* exp = (Expression*) malloc(sizeof(Expression));
    exp->next = NULL;
    return exp;
}

void extractExpressions(Expression* expression, Token*token){
    token->DATA = trim(token->DATA);
    switch (token->type)
    {
    case TYPE_EXECUTABLE:
        expression->executable = token;
        break;

    case TYPE_ARGUMENT:
        expression->argument = token;
        break;
    
    case TYPE_DIRECTIVE:
        expression->directive = token;
        if (token->next)
        {
            expression->next = ExpressionInit();
        }
        break;
    default:
        break;
    }

    if(token->next == NULL){
        return;
    }
    
    extractExpressions(
        (expression->next) ? expression->next:expression,
        token->next
    );
};

void ExpressionPrint(Expression*expression){
    printf("%s ", expression->executable->DATA);
    if (expression->argument)
    {
        printf("%s ", expression->argument->DATA);
    }
    
    switch (expression->directive->DATA[0])
    {
    case DIRECTIVE_PIPE:
        printf(" | ");
        break;
    case DIRECTIVE_PARALLELIZE:
        printf(" ; ");
        break;
    default:
        printf("\n");
    }
};

void ExpressionPrintRecursive(Expression*expression){
    if (expression == NULL)
    {
        return;
    }
    
    ExpressionPrint(expression);

    if (expression->next == NULL)
    {
        return;
    }
    ExpressionPrintRecursive(expression->next);
};

void waitParallelPIDs(Expression* expression,DIRECTIVE directive){    
    while (ParallelPIDCount(PPIDHead) > 0)
    {
        ParallelPIDListPoll(expression, directive);
    }
};

void handleExpressions(Expression*expression){
    output_buffer = (char*)malloc(sizeof(char) * MAX_LINE_LENGTH* 10);
    memset(output_buffer, '\0', MAX_LINE_LENGTH * 10);

    ParallelPIDListInit();

    Expression*current = expression;
    while (current)
    {
        ExpressionPrint(current);

        switch (current->directive->DATA[0])
        {
            case DIRECTIVE_PIPE:
                handleExpression(current);
                memset(output_buffer, '\0', MAX_LINE_LENGTH * 10);
                waitParallelPIDs(expression,DIRECTIVE_PIPE);
                break;

            case DIRECTIVE_PARALLELIZE:
                handleExpression(current);
                break;
            default:
                handleExpression(current);
                memset(output_buffer, '\0', MAX_LINE_LENGTH * 10);
                waitParallelPIDs(expression,DIRECTIVE_ENDLINE);
                break;
        }
        current = current->next;
    }
}
void handleExpression(Expression*expression){
    pid_t pid;
    ssize_t bytes_read;
    const size_t bufferSize = 1024; // Define your buffer size here

    if (pipe(expression->output_pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    if (pipe(expression->input_pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(expression->output_pipefd[0]); // Close unused read end
        dup2(expression->output_pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        close(expression->output_pipefd[1]); // Close write end of the pipe

        close(expression->input_pipefd[1]); // Close unused write end of input pipe
        dup2(expression->input_pipefd[0], STDIN_FILENO); // Redirect stdin to pipe
        close(expression->input_pipefd[0]); // Close read end of the pipe


        char *args_A[] = {expression->executable->DATA, expression->argument->DATA, NULL}; // Replace 'example.txt' with your file
        char *args_B[] = {expression->executable->DATA, NULL};
        
        execvp(expression->executable->DATA, 
                                            strlen(expression->argument->DATA)>0?args_A:args_B);
                                            
        perror("execvp");
        exit(EXIT_FAILURE);

    } else {
        expression->pid = pid;
        close(expression->output_pipefd[1]); // Close unused write end
        close(expression->input_pipefd[0]); // Close unused write end

        write(expression->input_pipefd[1], output_buffer, strlen(output_buffer));
        close(expression->input_pipefd[1]); // Close the write end of the pipe  
        

        expression->output_buffer = (char*) malloc(bufferSize); // Dynamically allocate memory
        if (!expression ->output_buffer) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        ParallelPIDInsert(PPIDHead, expression->pid);
    }
};
