//
// Created by dragos on 1/11/18.
//

#include "sec_buffers.h" /* for prototypes, also gives stdlib.h */
#include "ptpd.h" /* gives string.h for memcpy, as well as INFO macro  */
#include "ptp_datatypes.h"
/* can't include ptpd.h in sec_buffers.h, otherwise datatypes.h spits "unknown type Buffer" */

/* used by bufferMessage */
int initBufferedMsg(BufferedMsg *bm, char *m, size_t len) {

    bm->msg = malloc(len);
    /* if malloc succeeds, copy message into newly allocated memory */
    if (bm->msg) {
        memcpy(bm->msg, m, len);
    }
    else {
        return 0;
    }

    bm->next = 0;

    return 1;
}

/* called in startup */
int initBuffers(Buffer **buffers, int n) {
    for (int i = 0; i < n; i++) {
        buffers[i] = calloc(1, sizeof(Buffer));
        if (!buffers[i])
            return 0;
    }

    return 1;
}

void freeBuffer(Buffer *b) {

}

void freeBuffers(Buffer **buffers, int n) {

}

/* adds message m of length len to the Buffer b (already initialized) */
int bufferMessage(Buffer *b, char *m, size_t len) {
    /* create a new BufferedMsg and initialize it */
    BufferedMsg *newMsg = calloc(1, sizeof(BufferedMsg));
    if (!newMsg)
        return 0;

    /* returns 0 if memory allocation fails */
    if (!initBufferedMsg(newMsg, m, len))
        return 0;

    /* add the new BufferedMsg to the given Buffer */
    if (b->tail) {
        /* link old tail to new msg */
        b->tail->next = newMsg;
    }
    /* no tail, i.e. empty buffer */
    else {
        b->head = newMsg;
    }

    /* set new tail */
    b->tail = newMsg;
    b->size++;

    return 1;
}

/* dump all BufferedMsgs in given Buffer according to the bufMsgDumper function, which takes a BufferedMsg as param */
void dumpBuffer(Buffer *b, void (*bufMsgDumper)(BufferedMsg *)) {
    BufferedMsg *bm = b->head;
    for (int i = 0; i < b->size; i++) {
        bufMsgDumper(bm);
        bm = bm->next;
    }
}

void dumpBufferedMsg(BufferedMsg *bm) {
    /* unpack buffered msg into local header struct for easy access to fields */
    MsgHeader header;
    msgUnpackHeader(bm->msg, &header);

    /*
     *  header.messageLength (Uint16)
     *  header.messageType (Enum4lower) to get packet length til start of TLV
     */

    char messageTypeString[25];

    UInteger16 packetLength; /* this is also the secTLV offset */

    switch (header.messageType) {
        case ANNOUNCE:
            strcpy(messageTypeString, "announce");
            packetLength = ANNOUNCE_LENGTH;
            break;
        case SYNC:
            strcpy(messageTypeString, "sync");
            packetLength = SYNC_LENGTH;
            break;
        case FOLLOW_UP:
            strcpy(messageTypeString, "followup");
            packetLength = FOLLOW_UP_LENGTH;
            break;
        case PDELAY_REQ:
            strcpy(messageTypeString, "pd_req");
            packetLength = PDELAY_REQ_LENGTH;
            break;
        case PDELAY_RESP:
            strcpy(messageTypeString, "pd_resp");
            packetLength = PDELAY_RESP_LENGTH;
            break;
        case PDELAY_RESP_FOLLOW_UP:
            strcpy(messageTypeString, "pd_respfollowup");
            packetLength = PDELAY_RESP_FOLLOW_UP_LENGTH;
            break;
        default:
            strcpy(messageTypeString, "unknown message type");
            packetLength = 0;
            break;
    }

    char firstICVByte = bm->msg[header.messageLength - GMAC_ICV_LEN]; // debugging.. but GMAC and HMAC have same ICV len
    char lastICVByte = bm->msg[header.messageLength - 1];


    INFO("type: %s (seqid %04x) w/ ICV: %02x...%02x\n",
         messageTypeString, header.sequenceId, firstICVByte, lastICVByte);

}