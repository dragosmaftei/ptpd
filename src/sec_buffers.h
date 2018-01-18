//
// Created by dragos on 1/11/18.
//

#ifndef SEC_BUFFERS_H_
#define SEC_BUFFERS_H_

#include <stdlib.h> /* for size_t */

/*
 * for delayed security processing, we need to buffer messages for later verification
 * a BufferedMsg is a linked list style node that holds a PTP message
 */
typedef struct BufferedMsg {
    char *msg;
    struct BufferedMsg *next;
} BufferedMsg;

/*
 * a Buffer is a linked list of BufferedMsgs representing all buffered messages for a given time interval
 */
typedef struct Buffer {
    BufferedMsg *head;
    BufferedMsg *tail;
    int size;
} Buffer;

/* inits and bufferMessage return 0 if memory allocation fails, 1 otherwise */
int initBufferedMsg(BufferedMsg *bm, char *m, size_t len);
int initBuffers(Buffer **buffers, int n);
void freeBuffer(Buffer *b);
void freeBuffers(Buffer **buffers, int n);
int bufferMessage(Buffer *b, char *m, size_t len);

/* dump all BufferedMsgs in given Buffer according to the bufMsgDumper function, which takes a BufferedMsg as param */
void dumpBuffer(Buffer *b, void (*bufMsgDumper)(BufferedMsg *bm));
void dumpBufferedMsg(BufferedMsg *bm);

#endif /* SEC_BUFFERS_H_ */