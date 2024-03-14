#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <tokenizer.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
extern Expression*expression_head;

void interpreter(){
    

    while (true)
    {
        char str[MAX_LINE_LENGTH];
        Expression* expression_tail = ExpressionInit();
        printf(">>> ");
        fgets(str, 250, stdin);
        //check if EOF

        if (feof(stdin))
        {
            handleQuit(NULL);
        }
        
        str[strlen(str)-1] = '\0';

        Token* token = tokenize(str);
        extractExpressions(expression_tail, token);
        expressionsAppend(expression_tail);
        handleExpressions(expression_tail, false);
    }
};