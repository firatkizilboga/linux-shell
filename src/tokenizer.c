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
char cwd[PATH_MAX];
Expression* expression_head;

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
    memset(token->DATA, '\0', MAX_LINE_LENGTH*11);
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
        if (WIFSIGNALED(status)){
            return 1;
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
    tokenzieRecursively(input, token, 0);
    return token;
}


void tokenzieRecursively(char* input, Token * token, int token_i){
    if (*input == '\0')
    {
        if (token->type != TYPE_DIRECTIVE)
        {
            token->next = TokenInit(TYPE_DIRECTIVE);
            Token*t = token->next;
            t->DATA[0] = '\0';
        }

        token->DATA[token_i] = '\0';
        return;
    }

    switch (token->type)
    {
        case TYPE_EXECUTABLE:
            if (*input == SEPERATOR | isDirective(*input))
            {
                while (*input == SEPERATOR)
                {
                    input++;
                }
                TYPE type = isDirective(*input) ? TYPE_DIRECTIVE : TYPE_ARGUMENT;
                token->next = TokenInit(type);
                return tokenzieRecursively(input, token->next, 0);
            }
            token->DATA[token_i] = *input;
            return tokenzieRecursively(input+1, token, token_i+1);
            break;
        case TYPE_ARGUMENT:
            if (isDirective(*input))
            {
                token->next = TokenInit(TYPE_DIRECTIVE);
                return tokenzieRecursively(input, token->next, 0);
            }

            token->DATA[token_i] = *input;
            return tokenzieRecursively(input+1, token, token_i+1);
            break;
        case TYPE_DIRECTIVE:
            token->DATA[token_i] = *input;
            token->next = TokenInit(TYPE_EXECUTABLE);
            input++;
            while (*input == SEPERATOR)
            {
                input++;
            }
            return tokenzieRecursively(input, token->next, 0);
            break;
        default:
            break;
    }
    
};


Expression* ExpressionInit(){
    Expression* exp = (Expression*) malloc(sizeof(Expression));
    exp->argument = NULL;
    exp->executable = NULL;
    exp->directive = NULL;
    exp->next = NULL;
    return exp;
}

void extractExpressions(Expression* expression, Token*token){
    
    char * trimmed = trim(token->DATA);
    if (token->type != TYPE_DIRECTIVE)
    {
        if (token->DATA[0] != '\n')
        {
            token->DATA = trimmed;
        }
        
    }
    
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
void expressionsAppend(Expression* expression_tail){
    if (!expression_head)
    {
        expression_head = expression_tail;
        return;
    }

    Expression*expression_head_cpy = expression_head;
    
    while (expression_head_cpy->next)
    {
        expression_head_cpy = expression_head_cpy->next;
    }
    expression_head_cpy -> next = expression_tail;
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

int handleCD(Expression*expression){
    if (expression->argument && strlen(expression->argument->DATA) > 0)
    {
        return chdir(expression->argument->DATA);
    } else {
        return chdir(getenv("HOME"));
    }
}

void handleHistory(Expression*expression){
    Expression* expression_head_cpy = expression_head;
    
    bool state = true;
    int i = 1;
    while (expression_head_cpy)
    {
        if (state)
        {
            printf("%d ", i);
            i++;
        }
        ExpressionPrint(expression_head_cpy);
        //fflush(stdout);
        if (expression_head_cpy->directive->DATA[0] == DIRECTIVE_PIPE || expression_head_cpy->directive->DATA[0] == DIRECTIVE_PARALLELIZE)
        {
            state = false;
        } else {
            state = true;
        }
        expression_head_cpy = expression_head_cpy->next;
        if (expression_head_cpy == expression)
        {
            if (state)
            {
                printf("%d ", i);
                i++;
            }
            ExpressionPrint(expression_head_cpy);
            break;
        }
        
    }
};

void handleExpressions(Expression*expression, bool print_prompt){
    output_buffer = (char*)malloc(sizeof(char) * MAX_LINE_LENGTH* 10);
    memset(output_buffer, '\0', MAX_LINE_LENGTH * 10);
    ParallelPIDListInit();

    
    Expression*current = expression;
    while (current)
    {
        if (print_prompt)
        {
            ExpressionPrint(current);
        }
        
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
                waitParallelPIDs(expression, DIRECTIVE_ENDLINE);
                break;
        }
        current = current->next;
    }
}

void handleExpression(Expression*expression){
    pid_t pid;
    ssize_t bytes_read;
    const size_t bufferSize = 1024; // Define your buffer size here
    if (strcmp(expression->executable->DATA, "cd") == 0){
        if (handleCD(expression) == 0){
            return;
        }else{
            printf("%s: No such file or directory\n", expression->argument->DATA);
            return;
        }
    };
    if (strcmp(expression->executable->DATA, "history") == 0){
        handleHistory(expression);
        return;
    };


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
        close(expression->input_pipefd[0]); // Close read end of the pip

        if (expression == NULL || expression->executable == NULL || expression->executable->DATA == NULL) {
            fprintf(stderr, "Error: Invalid 'expression' structure.\n");
            exit(EXIT_FAILURE);
        }

        char *args_A[3] = {expression->executable->DATA, NULL, NULL};
        char *args_B[2] = {expression->executable->DATA, NULL};

        if (expression->argument && expression->argument->DATA && strlen(expression->argument->DATA)) {
            args_A[1] = expression->argument->DATA; // Ensure DATA is not NULL
        }

        // Using ternary operator to choose between args_A and args_B
        char **exec_args = expression->argument && expression->argument->DATA ? args_A : args_B;

        if (expression->executable && expression->executable->DATA && strlen(expression->executable->DATA))
        {
            execvp(exec_args[0], exec_args);
        } else{
            exit(EXIT_FAILURE);
        }
        
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
