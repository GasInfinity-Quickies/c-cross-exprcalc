#include <limits.h>
#include <stdio.h>

#define GASINFINITY_QUICKEXPRCALC_IMPL
#include "quick_gec.h"

int main() {
    char buffer[512];

    printf("Press Ctrl+D to exit\n");
    while (true) {
        printf("> ");    
        
        if(fgets(buffer, 512, stdin) == NULL || feof(stdin)) {
            break;
        }
        
        gec_eval_result result = eval(buffer);

        if(result.success) {
            printf("--> %f\n", result.data.value);
            continue;
        }
        
        if(result.data.error_position <= INT_MAX) {
            printf("X %*s", (int)result.data.error_position, "");
            printf("^\n");
        }
        
        printf("Syntax error\n");
    }

    return 0;
}
