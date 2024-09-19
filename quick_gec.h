#ifndef GASINFINITY_QUICKIES_C_EXPRCALC_H
#define GASINFINITY_QUICKIES_C_EXPRCALC_H

#include <stddef.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

typedef struct gec_eval_result {
    bool success;

    union {
        double value;
        size_t error_position;
    } data;
} gec_eval_result;

gec_eval_result eval(char *str);

#ifdef GASINFINITY_QUICKEXPRCALC_IMPL
#define GEC_ERROR(result) (!result.success)

typedef enum gec_token_type {
    TK_ERROR,
    TK_NUMBER,
    TK_IDENTIFIER,
    TK_ADD,
    TK_SUB,
    TK_MUL,
    TK_DIV,
    TK_EXP,
    TK_LP,
    TK_RP,
} gec_token_type;

typedef struct gec_token {
    gec_token_type type;

    union {
        double num;
        char* str;
    } data;

   size_t start_index;
   size_t end_index;
} gec_token;

typedef struct gec_lex_ctx {
    const char* str;
    const size_t len;
    size_t idx;
    gec_token curr_tok;
} gec_lex_ctx;

bool advance_tok(gec_lex_ctx *ctx) {
    const char* str = ctx->str;
    const size_t len = ctx->len;
    size_t index = ctx->idx;

    while(str[index] == ' ') {
        ++index;
    }

    if(index >= len)
        return false;
 
    switch (str[index]) {
        case '+': 
        case '-': 
        case '*':
        case '/':
        case '^':
        case '(': 
        case '[':
        case ')': 
        case ']': {
            gec_token_type type = TK_ERROR;

            switch (str[index]) {
                case '+': type = TK_ADD; break;
                case '-': type = TK_SUB; break;
                case '*': type = TK_MUL; break;
                case '/': type = TK_DIV; break;
                case '^': type = TK_EXP; break;
                case '(': type = TK_LP; break;
                case '[': type = TK_LP; break;
                case ')': type = TK_RP; break;
                case ']': type = TK_RP; break;
            }
            
            ctx->curr_tok =  (gec_token){ .type = type, .start_index = index, .end_index = index + 1 };
            ctx->idx = index + 1;
            return true;
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '.':
        case ',': {
            size_t initialIndex = index;
            double result = 0;

            bool isQuotient = str[index] == '.' || str[index] == ',';
            double quotientMagnitude = .1;
            
            if(isQuotient) {
                ++index; 
            } 

            while (index < len) {
                switch (str[index]) {
                    case ',':
                    case '.': {
                        if(isQuotient) {
                            ctx->curr_tok = (gec_token){ .type = TK_ERROR, .start_index = initialIndex, .end_index = index };
                            ctx->idx = index;
                            return true;
                        }

                        ++index;
                        isQuotient = true;
                        break;
                    }
                    case '0':
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9': {
                        size_t digit = str[index++] - '0';

                        if(isQuotient) {
                            result += digit * quotientMagnitude;
                            quotientMagnitude *= .1;
                            break;
                        }

                        result = 10 * result + digit;
                        break;
                    }
                    default: {
                        ctx->curr_tok = (gec_token){ .type = TK_NUMBER, .data = { .num = result }, .start_index = initialIndex, .end_index = index };
                        ctx->idx = index;
                        return true;
                    }
                } 
            }
        }
        default: {
            ctx->curr_tok = (gec_token){ .type = TK_ERROR, .start_index = index, .end_index = index + 1 }; 
            ctx->idx = index + 1;
            return true;
        } 
    }
}

gec_eval_result evalExpr(gec_lex_ctx *ctx);

gec_eval_result evalPrimary(gec_lex_ctx *ctx) {
    switch(ctx->curr_tok.type) {
        case TK_LP: {
            advance_tok(ctx);
            gec_eval_result result = evalExpr(ctx);
            
            if(GEC_ERROR(result))
                return result;

            if(ctx->curr_tok.type != TK_RP) 
                return (gec_eval_result){ .success = false, .data = { .error_position = ctx->curr_tok.start_index } };

            advance_tok(ctx);
            return result;
        }
        case TK_NUMBER: {
            double result = ctx->curr_tok.data.num;
            advance_tok(ctx);
            return (gec_eval_result){ .success = true, .data = { .value = result } };
        }
        case TK_SUB: {
            advance_tok(ctx);
            gec_eval_result result = evalPrimary(ctx);

            if(GEC_ERROR(result))
                return result;

            return (gec_eval_result) { .success = true, .data = { .value = -result.data.value } };
        }
        default:  // ERROR
            return (gec_eval_result) { .success = false, .data = { .error_position = ctx->curr_tok.start_index } };
    }
}

gec_eval_result evalExp(gec_lex_ctx *ctx) {
    gec_eval_result left = evalPrimary(ctx);
    
    if(GEC_ERROR(left))
        return left;

    if(ctx->curr_tok.type == TK_EXP) {
        advance_tok(ctx);
        gec_eval_result right = evalExp(ctx);

        if(GEC_ERROR(right))
            return right;

        double leftResult = left.data.value;
        double rightResult = right.data.value;
        return (gec_eval_result) { .success = true, .data = { .value = pow(leftResult, rightResult) } }; 
    }

    return left;
}

gec_eval_result evalMulDiv(gec_lex_ctx *ctx) {
    gec_eval_result left = evalExp(ctx);
    
    if(GEC_ERROR(left))
        return left;

    gec_token_type type = ctx->curr_tok.type;
    switch(type) {
        case TK_MUL:
        case TK_DIV: {
            advance_tok(ctx);
            gec_eval_result right = evalMulDiv(ctx);

            if(GEC_ERROR(right))
                return right;
            
            double leftResult = left.data.value;
            double rightResult = right.data.value;
            return (gec_eval_result) { .success = true, .data = { .value = type == TK_MUL? leftResult * rightResult : leftResult / rightResult } }; 
        }
        default: return left;
    }
}

gec_eval_result evalAddSub(gec_lex_ctx *ctx) {
    gec_eval_result left = evalMulDiv(ctx);
    
    if(GEC_ERROR(left))
        return left;

    gec_token_type type = ctx->curr_tok.type;
    switch(type) {
        case TK_ADD:
        case TK_SUB: {
            advance_tok(ctx);
            gec_eval_result right = evalAddSub(ctx);

            if(GEC_ERROR(right))
                return right;
            
            double leftResult = left.data.value;
            double rightResult = right.data.value;
            return (gec_eval_result) { .success = true, .data = { .value = type == TK_ADD ? leftResult + rightResult : leftResult - rightResult } }; 
        }
        default: return left;
    }
}

gec_eval_result evalExpr(gec_lex_ctx *ctx) {
    return evalAddSub(ctx);
}

gec_eval_result eval(char *str) {
    gec_lex_ctx ctx = {
        .str = str,
        .len = strlen(str),
        .idx = 0
    }; 

    advance_tok(&ctx);
    return evalExpr(&ctx);
}

#undef GEC_ERROR
#endif
#endif
