//
// Created by dragos on 1/11/18.
//

#include "sec_buffers.h" /* for prototypes, also gives stdlib.h */
#include "ptpd.h" /* gives string.h for memcpy, as well as INFO macro  */
//#include "ptp_datatypes.h"
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

    bm->len = len;
    bm->icvFailed = FALSE;
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
    if (!b) {
        return;
    }
    /* first free the BufferedMsgs */
    BufferedMsg *cur = b->head;
    while (cur) {
        BufferedMsg *next = cur->next;
        /* free the BufferedMsg */
        free(cur->msg);
        free(cur);
        cur = next;
    }

    /* finally, free the Buffer */
    free(b);
}

void freeBuffers(Buffer **buffers, int n) {
    /* free each Buffer first */
    for (int i = 0; i < n; i++) {
        freeBuffer(buffers[i]);
    }

    /* finally, free the n Buffer pointers */
    free(buffers);
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

    UInteger16 packetLength; /* this is also the authTLV offset */

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

    /* unsigned char for cleaner printing */
    unsigned char firstICVByte = (unsigned char) bm->msg[header.messageLength - GMAC_ICV_LEN]; // debugging.. but GMAC and HMAC have same ICV len
    unsigned char lastICVByte = (unsigned char) bm->msg[header.messageLength - 1];

    INFO("SEC: \tmessage type: %s (seqid %04x), correction field (lsb): ...%08x, ICV: %02x...%02x verified? %d\n",
         messageTypeString, header.sequenceId, header.correctionField.lsb, firstICVByte, lastICVByte, !bm->icvFailed);

}