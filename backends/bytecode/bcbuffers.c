//
// Commented byte buffer for spin2cpp
//
// Copyright 2021 Ada Gottensträter and Total Spectrum Software Inc.
// see the file COPYING for conditions of redistribution
//

#include "bcbuffers.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

OutputSpan *BOB_Push(ByteOutputBuffer *buf,uint8_t *data,int data_size,const char *comment) {
    OutputSpan *newSpan = calloc(sizeof(OutputSpan)+data_size,1);
    if (!newSpan) {
        ERROR(NULL,"Out of memory (while allocating OutputSpan)");
        exit(1);
    }

    if (data&&data_size) memcpy(newSpan->data,data,data_size);
    newSpan->size = data_size;
    newSpan->comment = comment;

    if (buf->tail != NULL) buf->tail->next = newSpan;
    buf->tail = newSpan;
    if (buf->head == NULL) buf->head = newSpan;

    buf->total_size += data_size;

    return newSpan;
}

void BOB_Replace(OutputSpan *span,uint8_t *data,int data_size,const char *comment) {
    if (!span) return;
    if (data && span->size != data_size) {
        ERROR(NULL,"Error replacing span data: data size(%d) doesn't match span size(%d)",data_size,span->size);
        return;
    }
    if (data) memcpy(span->data,data,data_size);
    if(comment) span->comment = comment;
}

OutputSpan *BOB_PushByte(ByteOutputBuffer *buf,uint8_t data,const char *comment) {
    return BOB_Push(buf,&data,1,comment);
}
OutputSpan *BOB_PushWord(ByteOutputBuffer *buf,uint16_t data,const char *comment) {
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        data = __builtin_bswap16(data);
    #endif
    return BOB_Push(buf,(uint8_t*)&data,2,comment);
}
OutputSpan *BOB_PushLong(ByteOutputBuffer *buf,uint32_t data,const char *comment) {
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        data = __builtin_bswap32(data);
    #endif
    return BOB_Push(buf,(uint8_t*)&data,4,comment);
}

void BOB_ReplaceByte(OutputSpan *span,uint8_t data,const char *comment) {
    return BOB_Replace(span,&data,1,comment);
}
void BOB_ReplaceWord(OutputSpan *span,uint16_t data,const char *comment) {
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        data = __builtin_bswap16(data);
    #endif
    return BOB_Replace(span,(uint8_t*)&data,2,comment);
}
void BOB_ReplaceLong(OutputSpan *span,uint32_t data,const char *comment) {
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        data = __builtin_bswap32(data);
    #endif
    return BOB_Replace(span,(uint8_t*)&data,4,comment);
}



int BOB_Align(ByteOutputBuffer *buf,int alignment) {
    int pad = alignment - (buf->total_size%alignment);
    if (pad != alignment) {
        BOB_Push(buf,NULL,pad,"(padding)");
        return pad;
    } else return 0;
}

// Printf that auto-allocates some space (and never frees it, lol)
char *auto_printf(size_t max,const char *format,...) {
    char *buffer = malloc(max);
    va_list args;
    va_start(args, format);
    vsnprintf(buffer,max,format,args);
    va_end(args);
    return buffer;
}
