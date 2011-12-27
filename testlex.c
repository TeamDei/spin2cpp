//
// Microtests for lexical analyzer
//
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "spinc.h"

static void EXPECTEQfn(long x, long val, int line) {
    if (x != val) {
        fprintf(stderr, "test failed at line %d of %s: expected %ld got %ld\n",
                line, __FILE__, val, x);
        abort();
    }
}

#define EXPECTEQ(x, y) EXPECTEQfn((x), (y), __LINE__)

static void
testNumber(const char *str, uint32_t val)
{
    AST *ast;
    LexStream L;
    int t;
    int c;
    printf("testing number[%s]...", str);
    strToLex(&L, str);
    t = getToken(&L, &ast);
    EXPECTEQ(t, T_NUM);
    c = lexgetc(&L);
    EXPECTEQ(c, T_EOF);
    assert(ast != NULL);
    assert(ast->kind == AST_INTEGER);
    EXPECTEQ(ast->d.ival, val);
    printf("passed\n");
}

static void
testIdentifier(const char *str, const char *expect)
{
    LexStream L;
    AST *ast;
    int t;

    strToLex(&L, str);
    t = getToken(&L, &ast);
    EXPECTEQ(t, T_IDENTIFIER);
    assert(ast != NULL);
    assert(ast->kind == AST_IDENTIFIER);
    EXPECTEQ(0, strcmp(ast->d.string, expect));
    printf("from [%s] read identifier [%s]\n", str, ast->d.string);
}

static void
testTokenStream(const char *str, int *tokens, int numtokens)
{
    int i;
    LexStream L;
    AST *ast;
    int t;

    printf("testing tokens [%s]...", str); fflush(stdout);
    strToLex(&L, str);
    for (i = 0; i < numtokens; i++) {
        t = getToken(&L, &ast);
        EXPECTEQ(t, tokens[i]);
    }
    printf("passed\n");
}

void
ERROR(const char *msg, ...)
{
    va_list args;

    fprintf(stderr, "Error: ");
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);
    fprintf(stderr, "\n");
}

#define N_ELEM(x) (sizeof(x)/sizeof(x[0]))
static int tokens0[] = { T_NUM, '+', T_NUM, T_EOF };
static int tokens1[] = { T_IDENTIFIER, '-', T_NUM, '+', T_IDENTIFIER, T_EOF };
static int tokens2[] = { T_CON, T_CON, T_IDENTIFIER, T_CON, T_NUM, T_EOF };
static int tokens3[] = { '<', T_LE, T_GE, '>', T_EQ, '=', '+', '<' };

static const char *token4test = "pub \r\n  if\n    foo\n  bar\n";
static int tokens4[] = { T_PUB, T_EOLN, T_INDENT, T_IF, T_EOLN, T_INDENT, T_IDENTIFIER, T_EOLN, T_OUTDENT, T_IDENTIFIER, T_EOLN };

static const char *token5test = "pub\n  if\n    foo\n      bar\n  else";
static int tokens5[] = { T_PUB, T_EOLN, T_INDENT, T_IF, T_EOLN, T_INDENT, T_IDENTIFIER, T_EOLN, T_INDENT, T_IDENTIFIER, T_EOLN, T_OUTDENT, T_OUTDENT, T_ELSE};

static const char *token6test = "dat\nlabel if_z add x,#1 wc";
static int tokens6[] = { T_DAT, T_EOLN, T_IDENTIFIER, T_INSTRMODIFIER, T_INSTR,
                         T_IDENTIFIER, ',', '#', T_NUM, T_INSTRMODIFIER};

int
main()
{
    initLexer();

    testTokenStream("1 + 1", tokens0, N_ELEM(tokens0));
    testTokenStream("{a comment line} $1 + 1", tokens0, N_ELEM(tokens0));
    testTokenStream("{{a nested comment}} $1 + 1", tokens0, N_ELEM(tokens0));
    testTokenStream("x-1+y", tokens1, N_ELEM(tokens1));
    testTokenStream("_x0{some comment 1} - 1 + y_99", tokens1, N_ELEM(tokens1));
    testTokenStream("con CON con99 Con 99", tokens2, N_ELEM(tokens2));
    testTokenStream("< =< => > == = +<", tokens3, N_ELEM(tokens3));
    testTokenStream(token4test, tokens4, N_ELEM(tokens4));
    testTokenStream(token5test, tokens5, N_ELEM(tokens5));
    testTokenStream(token6test, tokens6, N_ELEM(tokens6));

    testNumber("0", 0);
    testNumber("00", 0);
    testNumber("007", 7);
    testNumber("008", 8);
    testNumber("\t \t 123", 123);
    testNumber("65535", 65535);
    testNumber("  $41", 65);
    testNumber("$01_ff", 511);
    testNumber("$A5", 165);
    testNumber("%101", 5);
    testNumber("%11", 3);
    testNumber("%%31", 13);
    testNumber("80_000_000", 80000000);

    testIdentifier("x99+8", "x99");
    testIdentifier("_a_b", "_a_b");
    printf("all tests passed\n");
    return 0;
}
