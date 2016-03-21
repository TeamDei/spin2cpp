/*
 * Spin to C/C++ translator
 * Copyright 2016 Total Spectrum Software Inc.
 * 
 * +--------------------------------------------------------------------
 * ¦  TERMS OF USE: MIT License
 * +--------------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * +--------------------------------------------------------------------
 */
#include "spinc.h"
#include "outasm.h"

static int inDat;
static int inCon;
static int didOrg;
static int lmmMode;

static void
doPrintOperand(struct flexbuf *fb, Operand *reg, int useimm)
{
    char temp[128];
    if (!reg) {
        ERROR(NULL, "internal error bad operand");
        flexbuf_addstr(fb, "???");
        return;
    }
    switch (reg->kind) {
    case IMM_INT:
        if (reg->val >= 0 && reg->val < 512) {
            flexbuf_addstr(fb, "#");
            if (reg->name && reg->name[0]) {
                flexbuf_addstr(fb, reg->name);
            } else {
                sprintf(temp, "%d", (int)(int32_t)reg->val);
                flexbuf_addstr(fb, temp);
            }
        } else {
            // the immediate actually got processed as a register
            flexbuf_addstr(fb, reg->name);
        }
        break;
    case BYTE_REF:
    case WORD_REF:
    case LONG_REF:
        ERROR(NULL, "Internal error: tried to use memory directly");
        break;
    case IMM_COG_LABEL:
        if (useimm) {
            flexbuf_addstr(fb, "#");
        }
        /* fall through */
    default:
        flexbuf_addstr(fb, reg->name);
        break;
    }
}

static void
PrintOperandSrc(struct flexbuf *fb, Operand *reg)
{
    doPrintOperand(fb, reg, 1);
}

static void
PrintOperand(struct flexbuf *fb, Operand *reg)
{
    doPrintOperand(fb, reg, 0);
}

void
PrintOperandAsValue(struct flexbuf *fb, Operand *reg)
{
    Operand *indirect;
    
    switch (reg->kind) {
    case IMM_INT:
        flexbuf_printf(fb, "%d", (int)(int32_t)reg->val);
        break;
    case IMM_HUB_LABEL:
    case STRING_DEF:
        flexbuf_addstr(fb, "@@@");
        // fall through
    case IMM_COG_LABEL:
        flexbuf_addstr(fb, reg->name);
        break;
    case IMM_STRING:
        flexbuf_addchar(fb, '"');
        flexbuf_addstr(fb, reg->name);
        flexbuf_addchar(fb, '"');
        break;
    case REG_HUBPTR:
        indirect = (Operand *)reg->val;
        flexbuf_addstr(fb, indirect->name);
        break;
    default:
        PrintOperand(fb, reg);
        break;
    }
}

static void
PrintCond(struct flexbuf *fb, IRCond cond)
{
    switch (cond) {
    case COND_TRUE:
      break;
    case COND_EQ:
      flexbuf_addstr(fb, " if_e");
      break;
    case COND_NE:
      flexbuf_addstr(fb, " if_ne");
      break;
    case COND_LT:
      flexbuf_addstr(fb, " if_b");
      break;
    case COND_GE:
      flexbuf_addstr(fb, " if_ae");
      break;
    case COND_GT:
      flexbuf_addstr(fb, " if_a");
      break;
    case COND_LE:
      flexbuf_addstr(fb, " if_be");
      break;
    case COND_C:
      flexbuf_addstr(fb, " if_c");
      break;
    case COND_NC:
      flexbuf_addstr(fb, " if_nc");
      break;
    default:
      flexbuf_addstr(fb, " if_??");
      break;
    }
    flexbuf_addchar(fb, '\t');
}

static void
OutputBlob(Flexbuf *fb, Operand *label, Operand *op)
{
    Flexbuf *databuf;
    Flexbuf *relocbuf;
    uint32_t *data;
    int len;
    int addr;
    Reloc *nextreloc;
    int relocs;
    
    if (op->kind != IMM_BINARY) {
        ERROR(NULL, "Internal: bad binary blob");
        return;
    }
    flexbuf_printf(fb, "\tlong\n"); // ensure long alignment
    flexbuf_printf(fb, label->name);
    flexbuf_printf(fb, "\n");
    databuf = (Flexbuf *)op->name;
    relocbuf = (Flexbuf *)op->val;
    if (relocbuf) {
        relocs = flexbuf_curlen(relocbuf) / sizeof(Reloc);
        nextreloc = (Reloc *)flexbuf_peek(relocbuf);
    } else {
        relocs = 0;
        nextreloc = NULL;
    }
    data = (uint32_t *)flexbuf_peek(databuf);
    len = flexbuf_curlen(databuf);
    for (addr = 0; addr < len; addr += 4) {
        flexbuf_printf(fb, "\tlong\t");
        if (relocs > 0) {
            // see if this particular long needs a reloc
            if (nextreloc->addr == addr) {
                int offset = nextreloc->value;
                if (offset == 0) {
                    flexbuf_printf(fb, "@@@%s\n", label->name);
                } else if (offset > 0) {
                    flexbuf_printf(fb, "@@@%s + %d\n", label->name, offset);
                } else {
                    flexbuf_printf(fb, "@@@%s - %d\n", label->name, -offset);
                }
                data++;
                nextreloc++;
                --relocs;
                continue;
            }
        }
        flexbuf_printf(fb, "$%08x\n", data[0]);
        data ++;
    }
    if (addr != len) {
        ERROR(NULL, "binary blob is not a multiple of 4 bytes long");
    }
}

/* find string for opcode */
static const char *
StringFor(IROpcode opc)
{
    switch(opc) {
    case OPC_STRING:
    case OPC_BYTE:
        return "byte";
    case OPC_LONG:
        return "long";
    case OPC_WORD:
        return "word";
    default:
        ERROR(NULL, "internal error, bad StringFor call");
        return "???";
    }
}

bool IsHubDest(Operand *dst)
{
    switch (dst->kind) {
    case IMM_HUB_LABEL:
    case REG_HUBPTR:
        return true;
    default:
        return false;
    }
}

/* convert IR list into p1 assembly language */
void
P1AssembleIR(struct flexbuf *fb, IR *ir)
{
    if (ir->opc == OPC_CONST) {
        // handle const declaration
        if (!inCon) {
            flexbuf_addstr(fb, "CON\n");
            inCon = 1;
            inDat = 0;
        }
        flexbuf_addstr(fb, "\t");
        PrintOperand(fb, ir->dst);
        flexbuf_addstr(fb, " = ");
        PrintOperandAsValue(fb, ir->src);
        flexbuf_addstr(fb, "\n");
        return;
    }
    if (!inDat) {
        flexbuf_addstr(fb, "DAT\n");
        inCon = 0;
        inDat = 1;
        if (!didOrg) {
            flexbuf_addstr(fb, "\torg\t0\n");
            didOrg = 1;
        }
    }
    if (1) {
        // handle certain instructions specially
        switch (ir->opc) {
        case OPC_CALL:
            if (IsHubDest(ir->dst)) {
                if (!lmmMode) {
                    // call of hub function from COG
                    PrintCond(fb, ir->cond);
                    flexbuf_addstr(fb, "mov\tpc, $+2\n");
                    PrintCond(fb, ir->cond);
                    flexbuf_addstr(fb, "call\t#LMM_CALL_FROM_COG\n");
                } else {
                    PrintCond(fb, ir->cond);
                    flexbuf_addstr(fb, "jmp\t#LMM_CALL\n");
                }
                flexbuf_addstr(fb, "\tlong\t");
                if (ir->dst->kind != IMM_HUB_LABEL) {
                    ERROR(NULL, "internal error: non-hub label in LMM jump");
                }
                PrintOperandAsValue(fb, ir->dst);
                flexbuf_addstr(fb, "\n");
                return;
            }
            break;
        case OPC_DJNZ:
            if (IsHubDest(ir->src)) {
                PrintCond(fb, ir->cond);
                flexbuf_addstr(fb, "djnz\t");
                PrintOperand(fb, ir->dst);
                flexbuf_addstr(fb, ", #LMM_JUMP\n");
                flexbuf_addstr(fb, "\tlong\t");
                if (ir->src->kind != IMM_HUB_LABEL) {
                    ERROR(NULL, "internal error: non-hub label in LMM jump");
                }
                PrintOperandAsValue(fb, ir->src);
                flexbuf_addstr(fb, "\n");
                return;
            }
            break;
        case OPC_JUMP:
            if (IsHubDest(ir->dst)) {
                PrintCond(fb, ir->cond);
                flexbuf_addstr(fb, "rdlong\tpc,pc\n");
                flexbuf_addstr(fb, "\tlong\t");
                if (ir->dst->kind != IMM_HUB_LABEL) {
                    ERROR(NULL, "internal error: non-hub label in LMM jump");
                }
                PrintOperandAsValue(fb, ir->dst);
                flexbuf_addstr(fb, "\n");
                return;
            }
            break;
        case OPC_RET:
            if (lmmMode) {
                PrintCond(fb, ir->cond);
                flexbuf_addstr(fb, "sub\tsp, #4\n");
                PrintCond(fb, ir->cond);
                flexbuf_addstr(fb, "rdlong\tpc, sp\n");
                return;
            }
        default:
            break;
        }
    }
    
    if (ir->instr) {
        int ccset;
        
        PrintCond(fb, ir->cond);
        flexbuf_addstr(fb, ir->instr->name);
        switch (ir->instr->ops) {
        case NO_OPERANDS:
            break;
        case SRC_OPERAND_ONLY:
        case DST_OPERAND_ONLY:
        case CALL_OPERAND:
            flexbuf_addstr(fb, "\t");
            PrintOperandSrc(fb, ir->dst);
            break;
        default:
            flexbuf_addstr(fb, "\t");
            PrintOperand(fb, ir->dst);
            flexbuf_addstr(fb, ", ");
            PrintOperandSrc(fb, ir->src);
            break;
        }
        ccset = ir->flags & (FLAG_WC|FLAG_WZ|FLAG_NR|FLAG_WR);
        if (ccset) {
            const char *sepstring = " ";
            if (ccset & FLAG_WC) {
                flexbuf_printf(fb, "%swc", sepstring);
                sepstring = ",";
            }
            if (ccset & FLAG_WZ) {
                flexbuf_printf(fb, "%swz", sepstring);
                sepstring = ",";
            }
            if (ccset & FLAG_NR) {
                flexbuf_printf(fb, "%snr", sepstring);
            } else if (ccset & FLAG_WR) {
                flexbuf_printf(fb, "%swr", sepstring);
            }
        }
        flexbuf_addstr(fb, "\n");
        return;
    }
    
    switch(ir->opc) {
    case OPC_DUMMY:
        break;
    case OPC_DEAD:
        /* no code necessary, internal opcode */
        flexbuf_addstr(fb, "\t.dead\t");
        flexbuf_addstr(fb, ir->dst->name);
        flexbuf_addstr(fb, "\n");
        break;
    case OPC_COMMENT:
        PrintOperand(fb, ir->dst);
	break;
    case OPC_LABEL:
        flexbuf_addstr(fb, ir->dst->name);
        flexbuf_addstr(fb, "\n");
        break;
    case OPC_RET:
        flexbuf_addchar(fb, '\t');
        flexbuf_addstr(fb, "ret\n");
        break;
    case OPC_BYTE:
    case OPC_WORD:
    case OPC_LONG:
    case OPC_STRING:
        flexbuf_addchar(fb, '\t');
	flexbuf_addstr(fb, StringFor(ir->opc));
	flexbuf_addstr(fb, "\t");
	PrintOperandAsValue(fb, ir->dst);
        flexbuf_addstr(fb, "\n");
	break;
    case OPC_LABELED_BLOB:
        // output a binary blob
        // dst has a label
        // data is in a string in src
        OutputBlob(fb, ir->dst, ir->src);
        break;
    case OPC_ORGH:
        flexbuf_addstr(fb, "\tfit\t496\n");
        lmmMode = 1;
        break;
    default:
        ERROR(NULL, "Internal error: unable to process IR\n");
        break;
    }
}

/* assemble an IR list */
char *
IRAssemble(IRList *list)
{
    IR *ir;
    struct flexbuf fb;
    char *ret;
    
    inDat = 0;
    inCon = 0;
    didOrg = 0;
    lmmMode = 0;
    
    flexbuf_init(&fb, 512);
    for (ir = list->head; ir; ir = ir->next) {
        P1AssembleIR(&fb, ir);
    }
    flexbuf_addchar(&fb, 0);
    ret = flexbuf_get(&fb);
    flexbuf_delete(&fb);
    return ret;
}
