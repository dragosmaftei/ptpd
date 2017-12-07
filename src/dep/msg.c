/*-
 * Copyright (c) 2012-2015 Wojciech Owczarek,
 * Copyright (c) 2011-2012 George V. Neville-Neil,
 *                         Steven Kreuzer,
 *                         Martin Burnicki,
 *                         Jan Breuer,
 *                         Gael Mace,
 *                         Alexandre Van Kempen,
 *                         Inaqui Delgado,
 *                         Rick Ratzel,
 *                         National Instruments.
 * Copyright (c) 2009-2010 George V. Neville-Neil,
 *                         Steven Kreuzer,
 *                         Martin Burnicki,
 *                         Jan Breuer,
 *                         Gael Mace,
 *                         Alexandre Van Kempen
 *
 * Copyright (c) 2005-2008 Kendall Correll, Aidan Williams
 *
 * All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   msg.c
 * @author George Neville-Neil <gnn@neville-neil.com>
 * @date   Tue Jul 20 16:17:05 2010
 *
 * @brief  Functions to pack and unpack messages.
 *
 * See spec annex d
 */

#include "../ptpd.h"
#include "../ptp_datatypes.h"
#include "../datatypes.h"

extern RunTimeOpts rtOpts;

#define PACK_SIMPLE( type ) \
void pack##type( void* from, void* to ) \
{ \
	*(type *)to = *(type *)from; \
} \
void unpack##type( void* from, void* to, PtpClock *ptpClock ) \
{ \
	pack##type( from, to ); \
}

#define PACK_ENDIAN( type, size ) \
void pack##type( void* from, void* to ) \
{ \
	*(type *)to = flip##size( *(type *)from ); \
} \
void unpack##type( void* from, void* to, PtpClock *ptpClock ) \
{ \
	pack##type( from, to ); \
}

#define PACK_LOWER_AND_UPPER( type ) \
void pack##type##Lower( void* from, void* to ) \
{ \
        *(char *)to = *(char *)to & 0xF0; \
        *(char *)to = *(char *)to | *(type *)from; \
} \
\
void pack##type##Upper( void* from, void* to ) \
{ \
        *(char *)to = *(char *)to & 0x0F; \
        *(char *)to = *(char *)to | (*(type *)from << 4); \
} \
\
void unpack##type##Lower( void* from, void* to, PtpClock *ptpClock ) \
{ \
        *(type *)to = *(char *)from & 0x0F; \
} \
\
void unpack##type##Upper( void* from, void* to, PtpClock *ptpClock ) \
{ \
        *(type *)to = (*(char *)from >> 4) & 0x0F; \
}

PACK_SIMPLE( Boolean )
PACK_SIMPLE( UInteger8 )
PACK_SIMPLE( Octet )
PACK_SIMPLE( Enumeration8 )
PACK_SIMPLE( Integer8 )

PACK_ENDIAN( Enumeration16, 16 )
PACK_ENDIAN( Integer16, 16 )
PACK_ENDIAN( UInteger16, 16 )
PACK_ENDIAN( Integer32, 32 )
PACK_ENDIAN( UInteger32, 32 )

PACK_LOWER_AND_UPPER( Enumeration4 )
PACK_LOWER_AND_UPPER( UInteger4 )
PACK_LOWER_AND_UPPER( Nibble )

/* The free function is intentionally empty. However, this simplifies
 * the procedure to deallocate complex data types
 */
#define FREE( type ) \
void free##type( void* x) \
{}

FREE ( Boolean )
FREE ( UInteger8 )
FREE ( Octet )
FREE ( Enumeration8 )
FREE ( Integer8 )
FREE ( Enumeration16 )
FREE ( Integer16 )
FREE ( UInteger16 )
FREE ( Integer32 )
FREE ( UInteger32 )
FREE ( Enumeration4 )
FREE ( UInteger4 )
FREE ( Nibble )

static inline int bufGuard(int max, long base, int len, long beginning, int size);

/*
 * check if data we want to read is within the allocated buffer,
 * and if it is within the message length given.
 */
static inline int
bufGuard(int max, long base, int len, long beginning, int size)
{
#ifdef RUNTIME_DEBUG
	int ok = ((beginning - base) < len) && ((beginning + size - base) < len) &&
		((beginning - base) < max) && ((beginning + size - base) < max);
	printf("bufGuard: beginning %ld end %ld: maxlen: %d size %d (%ld %ld %d): %s\n",
		beginning - base, beginning + size - base, len, size, base,
		beginning, size, ok ? "OK" : "!");
#endif
	return(
	    ((beginning - base) < len) && ((beginning + size - base) < len) &&
	    ((beginning - base) < max) && ((beginning + size - base) < max)
	);
}

void
unpackUInteger48( void *buf, void *i, PtpClock *ptpClock)
{

	unpackUInteger16(buf, &((UInteger48*)i)->msb, ptpClock);
	unpackUInteger32(buf + 2, &((UInteger48*)i)->lsb, ptpClock);
}

void
packUInteger48( void *i, void *buf)
{
	packUInteger16(&((UInteger48*)i)->msb, buf);
	packUInteger32(&((UInteger48*)i)->lsb, buf + 2);
}

void
unpackInteger64( void *buf, void *i, PtpClock *ptpClock)
{
	unpackInteger32(buf, &((Integer64*)i)->msb, ptpClock);
	unpackUInteger32(buf + 4, &((Integer64*)i)->lsb, ptpClock);
}

void
packInteger64( void* i, void *buf )
{
	packInteger32(&((Integer64*)i)->msb, buf);
	packUInteger32(&((Integer64*)i)->lsb, buf + 4);
}

/* NOTE: the unpack functions for management messages can probably be refactored into a macro */
int
unpackMMSlaveOnly( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
	int offset = 0;
	XMALLOC(m->tlv->dataField, sizeof(MMSlaveOnly));
	MMSlaveOnly* data = (MMSlaveOnly*)m->tlv->dataField;
	/* see src/def/README for a note on this X-macro */
	#define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
		unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
			      &data->name, ptpClock ); \
		offset = offset + size;
	#include "../def/managementTLV/slaveOnly.def"

	#ifdef PTPD_DBG
	mMSlaveOnly_display(data, ptpClock);
	#endif /* PTPD_DBG */

	return 1;

}

/* NOTE: the pack functions for management messsages can probably be refactored into a macro */
UInteger16
packMMSlaveOnly( MsgManagement* m, Octet *buf)
{
	int offset = 0;
	MMSlaveOnly* data = (MMSlaveOnly*)m->tlv->dataField;
	#define OPERATE( name, size, type ) \
		pack##type( &data->name,\
			    buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
		offset = offset + size;
	#include "../def/managementTLV/slaveOnly.def"

	/* return length */
	return offset;
}

int
unpackMMClockDescription( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
	int offset = 0;
	XMALLOC(m->tlv->dataField, sizeof(MMClockDescription));
	MMClockDescription* data = (MMClockDescription*)m->tlv->dataField;
	memset(data, 0, sizeof(MMClockDescription));
	#define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
		unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
			      &data->name, ptpClock ); \
		offset = offset + size;
	#include "../def/managementTLV/clockDescription.def"

	#ifdef PTPD_DBG
	mMClockDescription_display(data, ptpClock);
	#endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMClockDescription( MsgManagement* m, Octet *buf)
{
	int offset = 0;
	Octet pad = 0;
	MMClockDescription* data = (MMClockDescription*)m->tlv->dataField;
	data->reserved = 0;
	#define OPERATE( name, size, type ) \
		pack##type( &data->name,\
			    buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset); \
		offset = offset + size;
	#include "../def/managementTLV/clockDescription.def"

	/* is the TLV length odd? TLV must be even according to Spec 5.3.8 */
	if(offset % 2) {
		/* add pad of 1 according to Table 41 to make TLV length even */
		packOctet(&pad, buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset);
		offset = offset + 1;
	}

	/* return length */
	return offset;
}

void
freeMMClockDescription( MMClockDescription* data)
{
	#define OPERATE( name, size, type ) \
		free##type( &data->name);
	#include "../def/managementTLV/clockDescription.def"
}

int
unpackMMUserDescription( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
	int offset = 0;
	XMALLOC(m->tlv->dataField, sizeof(MMUserDescription));
	MMUserDescription* data = (MMUserDescription*)m->tlv->dataField;
	memset(data, 0, sizeof(MMUserDescription));
	#define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
		unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
			      &data->name, ptpClock ); \
		offset = offset + size;
	#include "../def/managementTLV/userDescription.def"

	#ifdef PTPD_DBG
	/* mMUserDescription_display(data, ptpClock); */
	#endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMUserDescription( MsgManagement* m, Octet *buf)
{
	int offset = 0;
	Octet pad = 0;
	MMUserDescription* data = (MMUserDescription*)m->tlv->dataField;
	#define OPERATE( name, size, type ) \
		pack##type( &data->name,\
			    buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset); \
		offset = offset + size;
	#include "../def/managementTLV/userDescription.def"

	/* is the TLV length odd? TLV must be even according to Spec 5.3.8 */
	if(offset % 2) {
		/* add pad of 1 according to Table 41 to make TLV length even */
		packOctet(&pad, buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset);
		offset = offset + 1;
	}

	/* return length */
	return offset;
}

void
freeMMUserDescription( MMUserDescription* data)
{
	#define OPERATE( name, size, type ) \
		free##type( &data->name);
	#include "../def/managementTLV/userDescription.def"
}

int unpackMMInitialize( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMInitialize));
        MMInitialize* data = (MMInitialize*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/initialize.def"

        #ifdef PTPD_DBG
        mMInitialize_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMInitialize( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMInitialize* data = (MMInitialize*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/initialize.def"

        /* return length*/
        return offset;
}

int unpackMMDefaultDataSet( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMDefaultDataSet));
        MMDefaultDataSet* data = (MMDefaultDataSet*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/defaultDataSet.def"

        #ifdef PTPD_DBG
        mMDefaultDataSet_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMDefaultDataSet( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMDefaultDataSet* data = (MMDefaultDataSet*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/defaultDataSet.def"

        /* return length*/
        return offset;
}

int unpackMMCurrentDataSet( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMCurrentDataSet));
        MMCurrentDataSet* data = (MMCurrentDataSet*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/currentDataSet.def"

        #ifdef PTPD_DBG
        mMCurrentDataSet_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMCurrentDataSet( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMCurrentDataSet* data = (MMCurrentDataSet*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/currentDataSet.def"

        /* return length*/
        return offset;
}

int unpackMMParentDataSet( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMParentDataSet));
        MMParentDataSet* data = (MMParentDataSet*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/parentDataSet.def"

        #ifdef PTPD_DBG
        mMParentDataSet_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMParentDataSet( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMParentDataSet* data = (MMParentDataSet*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/parentDataSet.def"

        /* return length*/
        return offset;
}

int unpackMMTimePropertiesDataSet( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMTimePropertiesDataSet));
        MMTimePropertiesDataSet* data = (MMTimePropertiesDataSet*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/timePropertiesDataSet.def"

        #ifdef PTPD_DBG
        mMTimePropertiesDataSet_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMTimePropertiesDataSet( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMTimePropertiesDataSet* data = (MMTimePropertiesDataSet*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/timePropertiesDataSet.def"

        /* return length*/
        return offset;
}

int unpackMMPortDataSet( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMPortDataSet));
        MMPortDataSet* data = (MMPortDataSet*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/portDataSet.def"

        #ifdef PTPD_DBG
        mMPortDataSet_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMPortDataSet( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMPortDataSet* data = (MMPortDataSet*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/portDataSet.def"

        /* return length*/
        return offset;
}

int unpackMMPriority1( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMPriority1));
        MMPriority1* data = (MMPriority1*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/priority1.def"

        #ifdef PTPD_DBG
        mMPriority1_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMPriority1( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMPriority1* data = (MMPriority1*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/priority1.def"

        /* return length*/
        return offset;
}

int unpackMMPriority2( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMPriority2));
        MMPriority2* data = (MMPriority2*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/priority2.def"

        #ifdef PTPD_DBG
        mMPriority2_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMPriority2( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMPriority2* data = (MMPriority2*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/priority2.def"

        /* return length*/
        return offset;
}

int unpackMMDomain( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMDomain));
        MMDomain* data = (MMDomain*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/domain.def"

        #ifdef PTPD_DBG
        mMDomain_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMDomain( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMDomain* data = (MMDomain*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/domain.def"

        /* return length*/
        return offset;
}

int unpackMMLogAnnounceInterval( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMLogAnnounceInterval));
        MMLogAnnounceInterval* data = (MMLogAnnounceInterval*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/logAnnounceInterval.def"

        #ifdef PTPD_DBG
        mMLogAnnounceInterval_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMLogAnnounceInterval( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMLogAnnounceInterval* data = (MMLogAnnounceInterval*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/logAnnounceInterval.def"

        /* return length*/
        return offset;
}

int unpackMMAnnounceReceiptTimeout( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField,sizeof(MMAnnounceReceiptTimeout));
        MMAnnounceReceiptTimeout* data = (MMAnnounceReceiptTimeout*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/announceReceiptTimeout.def"

        #ifdef PTPD_DBG
        mMAnnounceReceiptTimeout_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMAnnounceReceiptTimeout( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMAnnounceReceiptTimeout* data = (MMAnnounceReceiptTimeout*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/announceReceiptTimeout.def"

        /* return length*/
        return offset;
}

int unpackMMLogSyncInterval( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMLogSyncInterval));
        MMLogSyncInterval* data = (MMLogSyncInterval*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/logSyncInterval.def"

        #ifdef PTPD_DBG
        mMLogSyncInterval_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMLogSyncInterval( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMLogSyncInterval* data = (MMLogSyncInterval*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/logSyncInterval.def"

        /* return length*/
        return offset;
}

int unpackMMVersionNumber( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMVersionNumber));
        MMVersionNumber* data = (MMVersionNumber*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/versionNumber.def"

        #ifdef PTPD_DBG
        mMVersionNumber_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMVersionNumber( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMVersionNumber* data = (MMVersionNumber*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/versionNumber.def"

        /* return length*/
        return offset;
}

int unpackMMTime( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMTime));
        MMTime* data = (MMTime*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/time.def"

        #ifdef PTPD_DBG
        mMTime_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMTime( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMTime* data = (MMTime*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/time.def"

        /* return length*/
        return offset;
}

int unpackMMClockAccuracy( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMClockAccuracy));
        MMClockAccuracy* data = (MMClockAccuracy*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/clockAccuracy.def"

        #ifdef PTPD_DBG
        mMClockAccuracy_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMClockAccuracy( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMClockAccuracy* data = (MMClockAccuracy*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/clockAccuracy.def"

        /* return length*/
        return offset;
}

int unpackMMUtcProperties( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMUtcProperties));
        MMUtcProperties* data = (MMUtcProperties*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/utcProperties.def"

        #ifdef PTPD_DBG
        mMUtcProperties_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMUtcProperties( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMUtcProperties* data = (MMUtcProperties*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/utcProperties.def"

        /* return length*/
        return offset;
}

int unpackMMTraceabilityProperties( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMTraceabilityProperties));
        MMTraceabilityProperties* data = (MMTraceabilityProperties*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/traceabilityProperties.def"

        #ifdef PTPD_DBG
        mMTraceabilityProperties_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMTraceabilityProperties( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMTraceabilityProperties* data = (MMTraceabilityProperties*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/traceabilityProperties.def"

        /* return length*/
        return offset;
}

int unpackMMTimescaleProperties( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMTimescaleProperties));
        MMTimescaleProperties* data = (MMTimescaleProperties*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/timescaleProperties.def"
	data->ptp = (data->ptp & 0x08) >> 3;
        #ifdef PTPD_DBG
        mMTimescaleProperties_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMTimescaleProperties( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMTimescaleProperties* data = (MMTimescaleProperties*)m->tlv->dataField;
	data->ptp = (data->ptp << 3) & 0x08;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/timescaleProperties.def"

        /* return length*/
        return offset;
}

int unpackMMUnicastNegotiationEnable( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMUnicastNegotiationEnable));
        MMUnicastNegotiationEnable* data = (MMUnicastNegotiationEnable*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/unicastNegotiationEnable.def"

        #ifdef PTPD_DBG
        mMUnicastNegotiationEnable_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMUnicastNegotiationEnable( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMUnicastNegotiationEnable* data = (MMUnicastNegotiationEnable*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/unicastNegotiationEnable.def"

        /* return length*/
        return offset;
}


int unpackMMDelayMechanism( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMDelayMechanism));
        MMDelayMechanism* data = (MMDelayMechanism*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/delayMechanism.def"

        #ifdef PTPD_DBG
        mMDelayMechanism_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMDelayMechanism( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMDelayMechanism* data = (MMDelayMechanism*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf +  MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/delayMechanism.def"

        /* return length*/
        return offset;
}

int unpackMMLogMinPdelayReqInterval( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMLogMinPdelayReqInterval));
        MMLogMinPdelayReqInterval* data = (MMLogMinPdelayReqInterval*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/logMinPdelayReqInterval.def"

        #ifdef PTPD_DBG
        mMLogMinPdelayReqInterval_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

UInteger16
packMMLogMinPdelayReqInterval( MsgManagement* m, Octet *buf)
{
        int offset = 0;
        MMLogMinPdelayReqInterval* data = (MMLogMinPdelayReqInterval*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/logMinPdelayReqInterval.def"

        /* return length*/
        return offset;
}

int unpackMMErrorStatus( Octet *buf, int baseOffset, MsgManagement* m, PtpClock* ptpClock)
{
        int offset = 0;
        XMALLOC(m->tlv->dataField, sizeof(MMErrorStatus));
        MMErrorStatus* data = (MMErrorStatus*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
		if(!bufGuard(PACKET_SIZE, (long)buf, m->header.messageLength, (long)(buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset), size)) return 0;\
                unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + TLV_LENGTH + offset,\
                              &data->name, ptpClock ); \
                offset = offset + size;
        #include "../def/managementTLV/errorStatus.def"

        #ifdef PTPD_DBG
        mMErrorStatus_display(data, ptpClock);
        #endif /* PTPD_DBG */

	return 1;

}

void
freeMMErrorStatus( MMErrorStatus* data)
{
	#define OPERATE( name, size, type ) \
		free##type( &data->name);
	#include "../def/managementTLV/errorStatus.def"
}

UInteger16
packMMErrorStatus( MsgManagement* m, Octet *buf)
{
        int offset = 0;
	Octet pad = 0;
        MMErrorStatus* data = (MMErrorStatus*)m->tlv->dataField;
        #define OPERATE( name, size, type ) \
                pack##type( &data->name,\
                            buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset ); \
                offset = offset + size;
        #include "../def/managementTLV/errorStatus.def"

	/* is the TLV length odd? TLV must be even according to Spec 5.3.8 */
	if(offset % 2) {
		/* add pad of 1 according to Table 41 to make TLV length even */
		packOctet(&pad, buf + MANAGEMENT_LENGTH + TLV_LENGTH + offset);
		offset = offset + 1;
	}

	/* return length*/
	return offset;
}

void
unpackSMRequestUnicastTransmission( Octet *buf, MsgSignaling* m, PtpClock* ptpClock)
{
	int offset = 0;
	XMALLOC(m->tlv->valueField, sizeof(SMRequestUnicastTransmission));
	SMRequestUnicastTransmission* data = (SMRequestUnicastTransmission*)m->tlv->valueField;
	/* see src/def/README for a note on this X-macro */
	#define OPERATE( name, size, type ) \
		unpack##type( buf + SIGNALING_LENGTH + TL_LENGTH + offset,\
			      &data->name, ptpClock ); \
		offset = offset + size;
	#include "../def/signalingTLV/requestUnicastTransmission.def"

	#ifdef PTPD_DBG
	sMRequestUnicastTransmission_display(data, ptpClock);
	#endif /* PTPD_DBG */


}

UInteger16
packSMRequestUnicastTransmission( MsgSignaling* m, Octet *buf)
{
	int offset = 0;
	SMRequestUnicastTransmission* data = (SMRequestUnicastTransmission*)m->tlv->valueField;
	#define OPERATE( name, size, type ) \
		pack##type( &data->name,\
			    buf + SIGNALING_LENGTH + TL_LENGTH + offset ); \
		offset = offset + size;
	#include "../def/signalingTLV/requestUnicastTransmission.def"

	/* return length */
	return offset;
}

void
unpackSMGrantUnicastTransmission( Octet *buf, MsgSignaling* m, PtpClock* ptpClock)
{
	int offset = 0;
	XMALLOC(m->tlv->valueField, sizeof(SMGrantUnicastTransmission));
	SMGrantUnicastTransmission* data = (SMGrantUnicastTransmission*)m->tlv->valueField;

	/* see src/def/README for a note on this X-macro */
	#define OPERATE( name, size, type ) \
		unpack##type( buf + SIGNALING_LENGTH + TL_LENGTH + offset,\
			      &data->name, ptpClock ); \
		offset = offset + size;
	#include "../def/signalingTLV/requestUnicastTransmission.def"

	#ifdef PTPD_DBG
	sMGrantUnicastTransmission_display(data, ptpClock);
	#endif /* PTPD_DBG */
}

UInteger16
packSMGrantUnicastTransmission( MsgSignaling* m, Octet *buf)
{
	int offset = 0;
	SMGrantUnicastTransmission* data = (SMGrantUnicastTransmission*)m->tlv->valueField;
	#define OPERATE( name, size, type ) \
		pack##type( &data->name,\
			    buf + SIGNALING_LENGTH + TL_LENGTH + offset ); \
		offset = offset + size;
	#include "../def/signalingTLV/grantUnicastTransmission.def"

	/* return length */
	return offset;
}

void
unpackSMCancelUnicastTransmission( Octet *buf, MsgSignaling* m, PtpClock* ptpClock)
{
	int offset = 0;
	XMALLOC(m->tlv->valueField, sizeof(SMCancelUnicastTransmission));
	SMCancelUnicastTransmission* data = (SMCancelUnicastTransmission*)m->tlv->valueField;
	/* see src/def/README for a note on this X-macro */
	#define OPERATE( name, size, type ) \
		unpack##type( buf + SIGNALING_LENGTH + TL_LENGTH + offset,\
			      &data->name, ptpClock ); \
		offset = offset + size;
	#include "../def/signalingTLV/cancelUnicastTransmission.def"

	#ifdef PTPD_DBG
	sMCancelUnicastTransmission_display(data, ptpClock);
	#endif /* PTPD_DBG */

}

UInteger16
packSMCancelUnicastTransmission( MsgSignaling* m, Octet *buf)
{
	int offset = 0;
	SMCancelUnicastTransmission* data = (SMCancelUnicastTransmission*)m->tlv->valueField;
	#define OPERATE( name, size, type ) \
		pack##type( &data->name,\
			    buf + SIGNALING_LENGTH + TL_LENGTH + offset ); \
		offset = offset + size;
	#include "../def/signalingTLV/cancelUnicastTransmission.def"

	/* return length */
	return offset;
}

void
unpackSMAcknowledgeCancelUnicastTransmission( Octet *buf, MsgSignaling* m, PtpClock* ptpClock)
{
	int offset = 0;
	XMALLOC(m->tlv->valueField, sizeof(SMAcknowledgeCancelUnicastTransmission));
	SMAcknowledgeCancelUnicastTransmission* data = (SMAcknowledgeCancelUnicastTransmission*)m->tlv->valueField;
	/* see src/def/README for a note on this X-macro */
	#define OPERATE( name, size, type ) \
		unpack##type( buf + SIGNALING_LENGTH + TL_LENGTH + offset,\
			      &data->name, ptpClock ); \
		offset = offset + size;
	#include "../def/signalingTLV/acknowledgeCancelUnicastTransmission.def"

	#ifdef PTPD_DBG
	sMAcknowledgeCancelUnicastTransmission_display(data, ptpClock);
	#endif /* PTPD_DBG */


}

UInteger16
packSMAcknowledgeCancelUnicastTransmission( MsgSignaling* m, Octet *buf)
{
	int offset = 0;
	SMAcknowledgeCancelUnicastTransmission* data = (SMAcknowledgeCancelUnicastTransmission*)m->tlv->valueField;
	#define OPERATE( name, size, type ) \
		pack##type( &data->name,\
			    buf + SIGNALING_LENGTH + TL_LENGTH + offset ); \
		offset = offset + size;
	#include "../def/signalingTLV/acknowledgeCancelUnicastTransmission.def"

	/* return length */
	return offset;
}

void
unpackClockIdentity( Octet *buf, ClockIdentity *c, PtpClock *ptpClock)
{
	int i;
	for(i = 0; i < CLOCK_IDENTITY_LENGTH; i++) {
		unpackOctet((buf+i),&((*c)[i]), ptpClock);
	}
}

void packClockIdentity( ClockIdentity *c, Octet *buf)
{
	int i;
	for(i = 0; i < CLOCK_IDENTITY_LENGTH; i++) {
		packOctet(&((*c)[i]),(buf+i));
	}
}

void
freeClockIdentity( ClockIdentity *c) {
	/* nothing to free */
}

void
unpackClockQuality( Octet *buf, ClockQuality *c, PtpClock *ptpClock)
{
	int offset = 0;
	ClockQuality* data = c;
	#define OPERATE( name, size, type) \
		unpack##type (buf + offset, &data->name, ptpClock); \
		offset = offset + size;
	#include "../def/derivedData/clockQuality.def"
}

void
packClockQuality( ClockQuality *c, Octet *buf)
{
	int offset = 0;
	ClockQuality *data = c;
	#define OPERATE( name, size, type) \
		pack##type (&data->name, buf + offset); \
		offset = offset + size;
	#include "../def/derivedData/clockQuality.def"
}

void
freeClockQuality( ClockQuality *c)
{
	/* nothing to free */
}

void
unpackTimeInterval( Octet *buf, TimeInterval *t, PtpClock *ptpClock)
{
        int offset = 0;
        TimeInterval* data = t;
        #define OPERATE( name, size, type) \
                unpack##type (buf + offset, &data->name, ptpClock); \
                offset = offset + size;
        #include "../def/derivedData/timeInterval.def"
}

void
packTimeInterval( TimeInterval *t, Octet *buf)
{
        int offset = 0;
        TimeInterval *data = t;
        #define OPERATE( name, size, type) \
                pack##type (&data->name, buf + offset); \
                offset = offset + size;
        #include "../def/derivedData/timeInterval.def"
}

void
freeTimeInterval( TimeInterval *t)
{
	/* nothing to free */
}

void
unpackTimestamp( Octet *buf, Timestamp *t, PtpClock *ptpClock)
{
        int offset = 0;
        Timestamp* data = t;
        #define OPERATE( name, size, type) \
                unpack##type (buf + offset, &data->name, ptpClock); \
                offset = offset + size;
        #include "../def/derivedData/timestamp.def"
}

void
packTimestamp( Timestamp *t, Octet *buf)
{
        int offset = 0;
        Timestamp *data = t;
        #define OPERATE( name, size, type) \
                pack##type (&data->name, buf + offset); \
                offset = offset + size;
        #include "../def/derivedData/timestamp.def"
}

void
freeTimestamp( Timestamp *t)
{
        /* nothing to free */
}


void
unpackPortIdentity( Octet *buf, PortIdentity *p, PtpClock *ptpClock)
{
	int offset = 0;
	PortIdentity* data = p;
	#define OPERATE( name, size, type) \
		unpack##type (buf + offset, &data->name, ptpClock); \
		offset = offset + size;
	#include "../def/derivedData/portIdentity.def"
}

void
packPortIdentity( PortIdentity *p, Octet *buf)
{
	int offset = 0;
	PortIdentity *data = p;
	#define OPERATE( name, size, type) \
		pack##type (&data->name, buf + offset); \
		offset = offset + size;
	#include "../def/derivedData/portIdentity.def"
}

void
freePortIdentity( PortIdentity *p)
{
	/* nothing to free */
}

void
unpackPortAddress( Octet *buf, PortAddress *p, PtpClock *ptpClock)
{
	unpackEnumeration16( buf, &p->networkProtocol, ptpClock);
	unpackUInteger16( buf+2, &p->addressLength, ptpClock);
	if(p->addressLength) {
		XMALLOC(p->addressField, p->addressLength);
		memcpy( p->addressField, buf+4, p->addressLength);
	} else {
		p->addressField = NULL;
	}
}

void
packPortAddress(PortAddress *p, Octet *buf)
{
	packEnumeration16(&p->networkProtocol, buf);
	packUInteger16(&p->addressLength, buf+2);
	if(p->addressLength) {
		memcpy( buf+4, p->addressField, p->addressLength);
	}
}

void
freePortAddress(PortAddress *p)
{
	if(p->addressField) {
		free(p->addressField);
		p->addressField = NULL;
	}
}

void
unpackPTPText( Octet *buf, PTPText *s, PtpClock *ptpClock)
{
	unpackUInteger8( buf, &s->lengthField, ptpClock);
	if(s->lengthField) {
		XMALLOC(s->textField, s->lengthField);
		memcpy( s->textField, buf+1, s->lengthField);
	} else {
		s->textField = NULL;
	}
}

void
packPTPText(PTPText *s, Octet *buf)
{
	packUInteger8(&s->lengthField, buf);
	if(s->lengthField) {
		memcpy( buf+1, s->textField, s->lengthField);
	}
}

void
freePTPText(PTPText *s)
{
	if(s->textField) {
		free(s->textField);
		s->textField = NULL;
	}
}

void
unpackPhysicalAddress( Octet *buf, PhysicalAddress *p, PtpClock *ptpClock)
{
	unpackUInteger16( buf, &p->addressLength, ptpClock);
	if(p->addressLength) {
		XMALLOC(p->addressField, p->addressLength);
		memcpy( p->addressField, buf+2, p->addressLength);
	} else {
		p->addressField = NULL;
	}
}

void
packPhysicalAddress(PhysicalAddress *p, Octet *buf)
{
	packUInteger16(&p->addressLength, buf);
	if(p->addressLength) {
		memcpy( buf+2, p->addressField, p->addressLength);
	}
}

void
freePhysicalAddress(PhysicalAddress *p)
{
	if(p->addressField) {
		free(p->addressField);
		p->addressField = NULL;
	}
}

void
copyClockIdentity( ClockIdentity dest, ClockIdentity src)
{
	memcpy(dest, src, CLOCK_IDENTITY_LENGTH);
}

void
copyPortIdentity( PortIdentity *dest, PortIdentity *src)
{
	copyClockIdentity(dest->clockIdentity, src->clockIdentity);
	dest->portNumber = src->portNumber;
}

void
unpackMsgHeader(Octet *buf, MsgHeader *header, PtpClock *ptpClock)
{
	int offset = 0;
	MsgHeader* data = header;
	#define OPERATE( name, size, type) \
		unpack##type (buf + offset, &data->name, ptpClock); \
		offset = offset + size;
	#include "../def/message/header.def"
}

void
packMsgHeader(MsgHeader *h, Octet *buf)
{
	int offset = 0;

	/* set uninitalized bytes to zero */
	h->reserved0 = 0;
	h->reserved1 = 0;
	h->reserved2 = 0;

	#define OPERATE( name, size, type ) \
		pack##type( &h->name, buf + offset ); \
		offset = offset + size;
	#include "../def/message/header.def"
}

void
unpackManagementTLV(Octet *buf, int baseOffset, MsgManagement *m, PtpClock* ptpClock)
{
	int offset = 0;
	XMALLOC(m->tlv, sizeof(ManagementTLV));
	/* read the management TLV */
	#define OPERATE( name, size, type ) \
		unpack##type( buf + baseOffset + MANAGEMENT_LENGTH + offset, &m->tlv->name, ptpClock ); \
		offset = offset + size;
	#include "../def/managementTLV/managementTLV.def"
}

void
packManagementTLV(ManagementTLV *tlv, Octet *buf)
{
	int offset = 0;
	#define OPERATE( name, size, type ) \
		pack##type( &tlv->name, buf + MANAGEMENT_LENGTH + offset ); \
		offset = offset + size;
	#include "../def/managementTLV/managementTLV.def"
}

void
freeManagementTLV(MsgManagement *m)
{
        /* cleanup outgoing managementTLV */
        if(m->tlv) {
                if(m->tlv->dataField) {
                        if(m->tlv->tlvType == TLV_MANAGEMENT) {
                                freeMMTLV(m->tlv);
                        } else if(m->tlv->tlvType == TLV_MANAGEMENT_ERROR_STATUS) {
                                freeMMErrorStatusTLV(m->tlv);
                        }
                        free(m->tlv->dataField);
			m->tlv->dataField = NULL;
                }
                free(m->tlv);
		m->tlv = NULL;
        }
}

void
packMsgManagement(MsgManagement *m, Octet *buf)
{
	int offset = 0;
	MsgManagement *data = m;

	/* set unitialized bytes to zero */
	m->reserved0 = 0;
	m->reserved1 = 0;

	#define OPERATE( name, size, type) \
		pack##type (&data->name, buf + offset); \
		offset = offset + size;
	#include "../def/message/management.def"

}

void unpackMsgManagement(Octet *buf, MsgManagement *m, PtpClock *ptpClock)
{
	int offset = 0;
	MsgManagement* data = m;
	#define OPERATE( name, size, type) \
		unpack##type (buf + offset, &data->name, ptpClock); \
		offset = offset + size;
	#include "../def/message/management.def"

	#ifdef PTPD_DBG
	msgManagement_display(data);
	#endif /* PTPD_DBG */
}

void
unpackSignalingTLV(Octet *buf, MsgSignaling *m, PtpClock* ptpClock)
{
	int offset = 0;
	XMALLOC(m->tlv, sizeof(SignalingTLV));
	/* read the signaling TLV */
	#define OPERATE( name, size, type ) \
		unpack##type( buf + SIGNALING_LENGTH + offset, &m->tlv->name, ptpClock ); \
		offset = offset + size;
	#include "../def/signalingTLV/signalingTLV.def"
}

void
packSignalingTLV(SignalingTLV *tlv, Octet *buf)
{
	int offset = 0;
	#define OPERATE( name, size, type ) \
		pack##type( &tlv->name, buf + SIGNALING_LENGTH + offset ); \
		offset = offset + size;
	#include "../def/signalingTLV/signalingTLV.def"
}

void
freeSignalingTLV(MsgSignaling *m)
{
        /* cleanup outgoing signaling TLV */
        if(m->tlv) {
                if(m->tlv->valueField) {
                        free(m->tlv->valueField);
			m->tlv->valueField = NULL;
                }
                free(m->tlv);
		m->tlv = NULL;
        }
}

void
packMsgSignaling(MsgSignaling *m, Octet *buf)
{
	int offset = 0;
	MsgSignaling *data = m;

	#define OPERATE( name, size, type) \
		pack##type (&data->name, buf + offset); \
		offset = offset + size;
/*
    OPERATE( header, 34, MsgHeader)
    OPERATE( targetPortIdentity, 10, PortIdentity)

    packMsgHeader(&data->header, buf + offset);
    offset = offset + 34;

    packPortIdentity(&data->targetPortIdentity, buf + offset);
    offset = offset + 10;
 */
	#include "../def/message/signaling.def"

}

void
unpackMsgSignaling(Octet *buf, MsgSignaling *m, PtpClock *ptpClock)
{
	int offset = 0;
	MsgSignaling* data = m;
	#define OPERATE( name, size, type) \
		unpack##type (buf + offset, &data->name, ptpClock); \
		offset = offset + size;
	#include "../def/message/signaling.def"

	#ifdef PTPD_DBG
	msgSignaling_display(data);
	#endif /* PTPD_DBG */
}

/*Unpack Header from IN buffer to msgTmpHeader field */
void
msgUnpackHeader(Octet * buf, MsgHeader * header)
{
	header->transportSpecific = (*(Nibble *) (buf + 0)) >> 4;
	header->messageType = (*(Enumeration4 *) (buf + 0)) & 0x0F;
	header->versionPTP = (*(UInteger4 *) (buf + 1)) & 0x0F;
	/* force reserved bit to zero if not */
	header->messageLength = flip16(*(UInteger16 *) (buf + 2));
	header->domainNumber = (*(UInteger8 *) (buf + 4));
	header->flagField0 = (*(Octet *) (buf + 6));
	header->flagField1 = (*(Octet *) (buf + 7));
	memcpy(&header->correctionField.msb, (buf + 8), 4);
	memcpy(&header->correctionField.lsb, (buf + 12), 4);
	header->correctionField.msb = flip32(header->correctionField.msb);
	header->correctionField.lsb = flip32(header->correctionField.lsb);
	copyClockIdentity(header->sourcePortIdentity.clockIdentity, (buf + 20));
	header->sourcePortIdentity.portNumber =
		flip16(*(UInteger16 *) (buf + 28));
	header->sequenceId = flip16(*(UInteger16 *) (buf + 30));
	header->controlField = (*(UInteger8 *) (buf + 32));
	header->logMessageInterval = (*(Integer8 *) (buf + 33));

#ifdef PTPD_DBG
	msgHeader_display(header);
#endif /* PTPD_DBG */
}

/*Pack header message into OUT buffer of ptpClock*/
void
msgPackHeader(Octet * buf, PtpClock * ptpClock)
{

	/* (spec annex D) */
	*(UInteger8 *) (buf + 0) = ptpClock->portDS.transportSpecific << 4;
	*(UInteger4 *) (buf + 1) = ptpClock->portDS.versionNumber;
	*(UInteger8 *) (buf + 4) = ptpClock->defaultDS.domainNumber;
	/* clear flag field - message packing functions should populate it */
	memset((buf + 6), 0, 2);

	memset((buf + 8), 0, 8);
	copyClockIdentity((buf + 20), ptpClock->portDS.portIdentity.clockIdentity);
	*(UInteger16 *) (buf + 28) = flip16(ptpClock->portDS.portIdentity.portNumber);
	/* LogMessageInterval defaults to 0x7F, will be set to another value if needed as per table 24*/
	*(UInteger8 *) (buf + 33) = 0x7F;
}

#ifdef PTPD_SECURITY

/* these only pack and unpack constant length fields of the security TLV, see securityTLV.def */
void
msgUnpackSecurityTLV(Octet * buf, SecurityTLV *data, PtpClock *ptpClock)
{
    int offset = 0;

    #define OPERATE( name, size, type) \
	    	    unpack##type (buf + offset, &data->name, ptpClock); \
		        offset = offset + size;

    #include "../def/securityTLV/securityTLV.def"
}

void msgPackSecurityTLV(SecurityTLV *data, Octet *buf)
{
    int offset = 0;

    #define OPERATE( name, size, type) \
	    	pack##type (&data->name, buf + offset); \
		    offset = offset + size;

    #include "../def/securityTLV/securityTLV.def"
}

/*
 * buf is the output buffer (PTP header start)
 * this should be called after the buffer has been packed (including header) for the type of message
 */
UInteger16 addSecurityTLV(Octet *buf, const RunTimeOpts *rtOpts)
{
    /*
     * we've "used policy limiting fields to query the SPD" (i.e. in this emulation, just checked 'securityEnabled'),
     * and the query "returned an SPP to query SAD to obtain the relevant SA which contains other security paramaters",
     * (i.e. in this emulation, the parameters were read in from config file)... these parameters include:
     * imm/delayed, keyID, key, keyLen, algType (and thus ICV length), total TLV length (variable based on ICV length)
     * NOT including optional fields, and for delayed, a key disclosure delay;
     * thus to get the actual total TLV length, we just pull down the length stored in the SA / securityOpts (constant
     * fields + ICV length), and account for the disclosed key if applicable (signalled by the key disclosure delay)
     */

    /*
     * only this is pulled down into a local variable (as opposed to being accessed through rtOpts->securityOpts) since
     * it might get adjusted for optional fields on a per message basis depending on the key disclosure delay
     */
    UInteger16 secTLVLen = rtOpts->securityOpts.secTLVLen;
    /*
     * if delayed processing, check key disclosure delay to see if we need to include a disclosed key
     * emulated here very simply for proof of concept as a boolean, but in reality some other mechanism would apply
     */
    if (rtOpts->securityOpts.delayed && rtOpts->securityOpts.disclosureDelay) {
        /* adjust length accordingly (add key length) */
        secTLVLen += rtOpts->securityOpts.keyLen;
    } /* else, either not delayed, or delayed, but don't need to disclose key in this TLV, so don't adjust len */


    *(UInteger8 *) (buf + 6) |= PTP_SECURITY; /* adding security flag to the header */

    UInteger16  msg_len = flip16(*(UInteger16 *) (buf + 2)); /* get message length out from the header */
    /* adjusting the header's message length field to account for sec TLV */
    *(UInteger16 *) (buf + 2) = flip16(msg_len + secTLVLen);

    /* make TLV struct and populate it before packing the buffer */
    SecurityTLV sec_tlv;
    memset(&sec_tlv, 0, SEC_TLV_CONSTANT_LEN);
    sec_tlv.tlvType = SECURITY;
    sec_tlv.lengthField = secTLVLen - 4; /* -4 to discount TYPE and LENGTH (2 bytes each) */
    sec_tlv.SPP = rtOpts->securityOpts.SPP;
    sec_tlv.keyID = rtOpts->securityOpts.keyID;
    /* if disclosed key needs to be included, this must be indicated in the secParamIndicator's disclosedKey bit */
    if (rtOpts->securityOpts.delayed && rtOpts->securityOpts.disclosureDelay) {
        sec_tlv.secParamIndicator |= SPI_DISCLOSED_KEY;
    } /* else this flag will remain 0, as the whole sec_tlv was initialized to 0 */

    /*
     * pack the buffer with what we have so far for the TLV; avoiding the padding inherent in structs
     * start at end of message (i.e. buf + msg_len)
     */
    msgPackSecurityTLV(&sec_tlv, buf + msg_len);

    /* pack the disclosed key if necessary */
    if (rtOpts->securityOpts.delayed && rtOpts->securityOpts.disclosureDelay) {
        /* add disclosed key at 10th byte offset... just 0xdds for testing */
        memset(buf + msg_len + SEC_TLV_CONSTANT_LEN, 0xdd, rtOpts->securityOpts.keyLen);
    }

    /*
     * if delayed processing, save correction field and zero it out before calculating ICV
     * fill it back in the buffer later
     */
    Integer64 correctionFieldTmp;

    if (rtOpts->securityOpts.delayed ||
        (!rtOpts->securityOpts.delayed && rtOpts->securityOpts.immIgnoreCorrection)) {
        memcpy(&correctionFieldTmp.msb, (buf + 8), 4);
        memcpy(&correctionFieldTmp.lsb, (buf + 12), 4);
        /* don't need to flip the values copied into correctionFieldTmp since they won't be interpreted/used */
        memset(buf + 8, 0, 8); // zero out the correction field
    }

// CALCULATE AND PACK ICV need to pass in buffer, rtOpts->securityOpts, msg_len, secTLVLen



    calculateAndPackICV(&rtOpts->securityOpts, (unsigned char *)buf,
                        msg_len + secTLVLen - rtOpts->securityOpts.icvLength);



//
//    /* calculate ICV from buffer, then pack it directly in the buffer */
//    unsigned char *static_digest;
//
//    /*
//	 * want from header all the way up to ICV, so:
//	 * msg_len + total TLV length (variable but already calculated) - ICV length (variable)
//	 * calculating it this way accounts for different alg types, as well as for the disclosed key, if present,
//	 * if doing delayed processing
//	 */
//    static_digest = dm_HMAC(dm_EVP_sha256(), rtOpts->securityOpts.key, rtOpts->securityOpts.keyLen,
//                            (unsigned char *) buf,
//                            msg_len + secTLVLen - rtOpts->securityOpts.icvLength,
//                            NULL, NULL);
//
//    /* truncate the digest to 128 bits and pack it by copying just 16 bytes directly into the buffer */
//    memcpy(buf + msg_len + secTLVLen - rtOpts->securityOpts.icvLength,
//           static_digest, rtOpts->securityOpts.icvLength);
//



    /*
     * for delayed (or if imm but ignoring correction field),
     * restore correctionField to its previous value before it was zeroed out
     */
    if (rtOpts->securityOpts.delayed ||
        (!rtOpts->securityOpts.delayed && rtOpts->securityOpts.immIgnoreCorrection)) {
        memcpy((buf + 8), &correctionFieldTmp.msb, 4);
        memcpy((buf + 12), &correctionFieldTmp.lsb, 4);
    }

    return secTLVLen;
}

/*
 *
 * buf is the start of the PTP header
 */
void calculateAndPackICV(const SecurityOpts *secOpts, unsigned char *buf, UInteger16 icvOffset) {

    IntegrityAlgTyp algTyp = secOpts->integrityAlgTyp;

    switch (algTyp) {
        case HMAC_SHA256:
            INFO("SEND doing HMAC\n");
            /* the result of the ICV calculation will be stored here */
            unsigned char *static_digest;

            /*
             * want from header all the way up to ICV, so:
             * msg_len + total TLV length (variable but already calculated) - ICV length (variable)
             * calculating it this way accounts for different alg types, as well as for the disclosed key, if present,
             * if doing delayed processing
             */
            static_digest = dm_HMAC(dm_EVP_sha256(), secOpts->key, secOpts->keyLen,
                                    buf, icvOffset, NULL, NULL);

            /* truncate the digest to 128 bits and pack it by copying just 16 bytes directly into the buffer */
            memcpy(buf + icvOffset, static_digest, secOpts->icvLength);

            break;
        case GMAC_AES256:
            INFO("SEND doing GMAC\n");
            // DM: TODO ERROR MESSAGES INSTEAD OF PERROR
            /* get random bytes for IV */
            unsigned char iv[IV_LEN];
            memset(iv, 0, sizeof(iv));
            FILE *fd;
            if (!(fd = fopen("/dev/urandom", "r")))
                perror("error opening");
            /* read random bytes from /dev/urandom */
            int bytes_read = fread(iv, 1, sizeof(iv), fd);
            if (bytes_read != sizeof(iv))
                printf("error... only read %d of %lu bytes\n", bytes_read, sizeof(iv));
            if (fclose(fd))
                perror("error closing");

            /* call dm_GMAC passing in key, iv, ivlen, data (start of ptp header),
             * data len (icvOffset minus IV len, don't want to include IV in the integrity calculation)
             * icv start / where to place the calculated icv, and icv len
             */
            if (!dm_GMAC(secOpts->key, iv, sizeof(iv), buf, icvOffset - sizeof(iv),
                         buf + icvOffset, secOpts->icvLength)) {
                perror("error calculating GMAC");
            }
            /* pack IV in buffer before ICV so receiver will be able to use same IV to verify ICV */
            memcpy(buf + icvOffset - sizeof(iv), iv, sizeof(iv));

            if (DM_MSGS)
                INFO("IV[0]: %01x, ICV[0]:%01x\n", iv[0], *(buf + icvOffset));

            break;
        default:
            INFO("error, unknown algTyp");
            break;
    }
}

/*
 * buf is the start of PTP header
 */
Boolean calculateAndVerifyICV(const SecurityOpts *secOpts, unsigned char *buf, UInteger16 icvOffset) {
    IntegrityAlgTyp algTyp = secOpts->integrityAlgTyp;

    switch (algTyp) {
        case HMAC_SHA256:
            INFO("REC doing HMAC\n");

            unsigned char *static_digest;

            /*
             * want from header all the way up to ICV, so:
             * packetlength + total TLV length (variable but already calculated) - ICV length (variable)
             * calculating it this way accounts for different alg types, as well as for the disclosed key, if present,
             * if doing delayed processing
             */
            static_digest = dm_HMAC(dm_EVP_sha256(), secOpts->key, secOpts->keyLen,
                                    buf, icvOffset, NULL, NULL);

            /*
             * ICV gets truncated to 128 bits, so compare only 16 bytes
             * msgIbuf + packetLength is the start of the secTLV, + total TLV len - ICV len = start of ICV
             */
            if (memcmp(static_digest, buf + icvOffset, secOpts->icvLength)) {
                return FALSE;
            }
            break;
        case GMAC_AES256: {
            INFO("REC doing GMAC\n");
            /* create local buffer to store calculated icv */
            unsigned char localICV[secOpts->icvLength];
            memset(localICV, 0, sizeof(localICV));

            /* call dm_GMAC passing in key, iv from received msg, ivlen,
             * data (start of ptp header), data len (not including ICV OR IV)
             * output (local buffer created to store this calculation) and icv len
             */
            if (!dm_GMAC(secOpts->key, buf + icvOffset - IV_LEN, IV_LEN,
                         buf, icvOffset - IV_LEN,
                         localICV, secOpts->icvLength)) {
                perror("error calculating GMAC in ICV verification");
            }

            /* cmp locally calculated ICV with ICV in rec buf */
            if (memcmp(localICV, buf + icvOffset, secOpts->icvLength)) {
                return FALSE;
            }
            break;
        }
        default:
            INFO("error, unknown algTyp");
            return FALSE;
    }
    return TRUE;
}

#endif /* PTPD_SECURITY */

#ifndef PTPD_SLAVE_ONLY
/*Pack SYNC message into OUT buffer of ptpClock*/
void
msgPackSync(Octet * buf, UInteger16 sequenceId, Timestamp * originTimestamp, PtpClock * ptpClock)
{
	msgPackHeader(buf, ptpClock);

	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x00;
	/* Two step flag - table 20: Sync and PdelayResp only */
	if (ptpClock->defaultDS.twoStepFlag)
		*(UInteger8 *) (buf + 6) |= PTP_TWO_STEP;

    /* Table 19 */
    *(UInteger16 *) (buf + 2) = flip16(SYNC_LENGTH);
	*(UInteger16 *) (buf + 30) = flip16(sequenceId);
	*(UInteger8 *) (buf + 32) = 0x00;

	 /* Table 24 - unless it's multicast, logMessageInterval remains    0x7F */
	 if(rtOpts.transport == IEEE_802_3 || rtOpts.ipMode != IPMODE_UNICAST )
		*(Integer8 *) (buf + 33) = ptpClock->portDS.logSyncInterval;
	memset((buf + 8), 0, 8);

	/* Sync message */
	*(UInteger16 *) (buf + 34) = flip16(originTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(originTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(originTimestamp->nanosecondsField);

}
#endif /* PTPD_SLAVE_ONLY */

/*Unpack Sync message from IN buffer */
void
msgUnpackSync(Octet * buf, MsgSync * sync)
{
	sync->originTimestamp.secondsField.msb =
		flip16(*(UInteger16 *) (buf + 34));
	sync->originTimestamp.secondsField.lsb =
		flip32(*(UInteger32 *) (buf + 36));
	sync->originTimestamp.nanosecondsField =
		flip32(*(UInteger32 *) (buf + 40));

#ifdef PTPD_DBG
	msgSync_display(sync);
#endif /* PTPD_DBG */
}


/* When building slave only, this code does not get compiled */
#ifndef PTPD_SLAVE_ONLY
/*Pack Announce message into OUT buffer of ptpClock*/
void
msgPackAnnounce(Octet * buf, UInteger16 sequenceId, Timestamp * originTimestamp, PtpClock * ptpClock)
{
	UInteger16 stepsRemoved;
	
	msgPackHeader(buf, ptpClock);

	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x0B;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(ANNOUNCE_LENGTH);
	*(UInteger16 *) (buf + 30) = flip16(sequenceId);
	*(UInteger8 *) (buf + 32) = 0x05;
	/* Table 24: for Announce, logMessageInterval is never 0x7F */
	*(Integer8 *) (buf + 33) = ptpClock->portDS.logAnnounceInterval;

	/* Announce message */
	*(UInteger16 *) (buf + 34) = flip16(originTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(originTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(originTimestamp->nanosecondsField);

	*(Integer16 *) (buf + 44) = flip16(ptpClock->timePropertiesDS.currentUtcOffset);
	*(UInteger8 *) (buf + 47) = ptpClock->parentDS.grandmasterPriority1;
	*(UInteger8 *) (buf + 48) = ptpClock->defaultDS.clockQuality.clockClass;
	*(Enumeration8 *) (buf + 49) = ptpClock->defaultDS.clockQuality.clockAccuracy;
	*(UInteger16 *) (buf + 50) =
		flip16(ptpClock->defaultDS.clockQuality.offsetScaledLogVariance);
	*(UInteger8 *) (buf + 52) = ptpClock->parentDS.grandmasterPriority2;
	copyClockIdentity((buf + 53), ptpClock->parentDS.grandmasterIdentity);
	/* resolve bugs #37 and #40 - alignment errors on ARMv5 */
	stepsRemoved = flip16(ptpClock->currentDS.stepsRemoved);
	memcpy(buf + 61, &stepsRemoved, sizeof(UInteger16));
	*(Enumeration8 *) (buf + 63) = ptpClock->timePropertiesDS.timeSource;

	/*
	 * TimePropertiesDS in FlagField, 2nd octet - spec 13.3.2.6 table 20
	 * Could / should have used constants here PTP_LI_61 etc, but this is clean
	 */
	*(UInteger8*) (buf + 7) = ptpClock->timePropertiesDS.leap61			<< 0;
	*(UInteger8*) (buf + 7) |= (ptpClock->timePropertiesDS.leap59)			<< 1;
	*(UInteger8*) (buf + 7) |= (ptpClock->timePropertiesDS.currentUtcOffsetValid)	<< 2;
	*(UInteger8*) (buf + 7) |= (ptpClock->timePropertiesDS.ptpTimescale)		<< 3;
	*(UInteger8*) (buf + 7) |= (ptpClock->timePropertiesDS.timeTraceable)		<< 4;
	*(UInteger8*) (buf + 7) |= (ptpClock->timePropertiesDS.frequencyTraceable)	<< 5;
}
#endif /* PTPD_SLAVE_ONLY */

/*Unpack Announce message from IN buffer of ptpClock to msgtmp.Announce*/
void
msgUnpackAnnounce(Octet * buf, MsgAnnounce * announce)
{
	UInteger16 stepsRemoved;
	
	announce->originTimestamp.secondsField.msb =
		flip16(*(UInteger16 *) (buf + 34));
	announce->originTimestamp.secondsField.lsb =
		flip32(*(UInteger32 *) (buf + 36));
	announce->originTimestamp.nanosecondsField =
		flip32(*(UInteger32 *) (buf + 40));
	announce->currentUtcOffset = flip16(*(UInteger16 *) (buf + 44));
	announce->grandmasterPriority1 = *(UInteger8 *) (buf + 47);
	announce->grandmasterClockQuality.clockClass =
		*(UInteger8 *) (buf + 48);
	announce->grandmasterClockQuality.clockAccuracy =
		*(Enumeration8 *) (buf + 49);
	announce->grandmasterClockQuality.offsetScaledLogVariance =
		flip16(*(UInteger16 *) (buf + 50));
	announce->grandmasterPriority2 = *(UInteger8 *) (buf + 52);
	copyClockIdentity(announce->grandmasterIdentity, (buf + 53));
	/* resolve bugs #37 and #40 - alignment errors on ARMv5 */
	memcpy(&stepsRemoved, buf + 61, sizeof(UInteger16));
	announce->stepsRemoved = flip16(stepsRemoved);
	announce->timeSource = *(Enumeration8 *) (buf + 63);

	#ifdef PTPD_DBG
	msgAnnounce_display(announce);
	#endif /* PTPD_DBG */
}

#ifndef PTPD_SLAVE_ONLY /* does not get compiled when building slave only */
/*pack Follow_up message into OUT buffer of ptpClock*/
void
msgPackFollowUp(Octet * buf, Timestamp * preciseOriginTimestamp, PtpClock * ptpClock, const UInteger16 sequenceId)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x08;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(FOLLOW_UP_LENGTH);
	*(UInteger16 *) (buf + 30) = flip16(sequenceId);
	*(UInteger8 *) (buf + 32) = 0x02;

	 /* Table 24 - unless it's multicast, logMessageInterval remains    0x7F */
	 if(rtOpts.transport == IEEE_802_3 || rtOpts.ipMode != IPMODE_UNICAST)
		*(Integer8 *) (buf + 33) = ptpClock->portDS.logSyncInterval;

	/* Follow_up message */
	*(UInteger16 *) (buf + 34) =
		flip16(preciseOriginTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) =
		flip32(preciseOriginTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) =
		flip32(preciseOriginTimestamp->nanosecondsField);
}
#endif /* PTPD_SLAVE_ONLY */

/*Unpack Follow_up message from IN buffer of ptpClock to msgtmp.follow*/
void
msgUnpackFollowUp(Octet * buf, MsgFollowUp * follow)
{
	follow->preciseOriginTimestamp.secondsField.msb =
		flip16(*(UInteger16 *) (buf + 34));
	follow->preciseOriginTimestamp.secondsField.lsb =
		flip32(*(UInteger32 *) (buf + 36));
	follow->preciseOriginTimestamp.nanosecondsField =
		flip32(*(UInteger32 *) (buf + 40));

	#ifdef PTPD_DBG
	msgFollowUp_display(follow);
	#endif /* PTPD_DBG */
}


/*pack PdelayReq message into OUT buffer of ptpClock*/
void
msgPackPdelayReq(Octet * buf, Timestamp * originTimestamp, PtpClock * ptpClock)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x02;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(PDELAY_REQ_LENGTH);
	*(UInteger16 *) (buf + 30) = flip16(ptpClock->sentPdelayReqSequenceId);
	*(UInteger8 *) (buf + 32) = 0x05;
	/* Table 23 */
	*(Integer8 *) (buf + 33) = 0x7F;
	/* Table 24 */
	memset((buf + 8), 0, 8);

	/* Pdelay_req message */
	*(UInteger16 *) (buf + 34) = flip16(originTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(originTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(originTimestamp->nanosecondsField);

	memset((buf + 44), 0, 10);
	/* RAZ reserved octets */
}

/*pack delayReq message into OUT buffer of ptpClock*/
void
msgPackDelayReq(Octet * buf, Timestamp * originTimestamp, PtpClock * ptpClock)
{
	msgPackHeader(buf, ptpClock);

	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x01;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(DELAY_REQ_LENGTH);

	/* -- PTP_UNICAST flag will be set in netsend* if needed */

	*(UInteger16 *) (buf + 30) = flip16(ptpClock->sentDelayReqSequenceId);
	*(UInteger8 *) (buf + 32) = 0x01;
	/* Table 23 */
	*(Integer8 *) (buf + 33) = 0x7F;
	/* Table 24 */
	memset((buf + 8), 0, 8);

	/* Pdelay_req message */
	*(UInteger16 *) (buf + 34) = flip16(originTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(originTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(originTimestamp->nanosecondsField);
}

/*pack delayResp message into OUT buffer of ptpClock*/
void
msgPackDelayResp(Octet * buf, MsgHeader * header, Timestamp * receiveTimestamp, PtpClock * ptpClock)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x09;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(DELAY_RESP_LENGTH);
	*(UInteger8 *) (buf + 4) = header->domainNumber;

	/* -- PTP_UNICAST flag will be set in netsend* if needed */

	memset((buf + 8), 0, 8);

	/* Copy correctionField of PdelayReqMessage */
	*(Integer32 *) (buf + 8) = flip32(header->correctionField.msb);
	*(Integer32 *) (buf + 12) = flip32(header->correctionField.lsb);

	*(UInteger16 *) (buf + 30) = flip16(header->sequenceId);

	*(UInteger8 *) (buf + 32) = 0x03;

	 /* Table 24 - unless it's multicast, logMessageInterval remains    0x7F */
	 /* really tempting to cheat here, at least for hybrid, but standard is a standard */
	if ((header->flagField0 & PTP_UNICAST) != PTP_UNICAST) {
	    *(Integer8 *) (buf + 33) = ptpClock->portDS.logMinDelayReqInterval;
	}

	/* Pdelay_resp message */
	*(UInteger16 *) (buf + 34) =
		flip16(receiveTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(receiveTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(receiveTimestamp->nanosecondsField);
	copyClockIdentity((buf + 44), header->sourcePortIdentity.clockIdentity);
	*(UInteger16 *) (buf + 52) =
		flip16(header->sourcePortIdentity.portNumber);
}

/*pack PdelayResp message into OUT buffer of ptpClock*/
void
msgPackPdelayResp(Octet * buf, MsgHeader * header, Timestamp * requestReceiptTimestamp, PtpClock * ptpClock)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x03;
	/* Two step flag - table 20: Sync and PdelayResp only */
	if (ptpClock->defaultDS.twoStepFlag)
		*(UInteger8 *) (buf + 6) |= PTP_TWO_STEP;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(PDELAY_RESP_LENGTH);
	*(UInteger8 *) (buf + 4) = header->domainNumber;
	memset((buf + 8), 0, 8);


	*(UInteger16 *) (buf + 30) = flip16(header->sequenceId);

	*(UInteger8 *) (buf + 32) = 0x05;
	/* Table 23 */
	*(Integer8 *) (buf + 33) = 0x7F;
	/* Table 24 */

	/* Pdelay_resp message */
	*(UInteger16 *) (buf + 34) = flip16(requestReceiptTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(requestReceiptTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(requestReceiptTimestamp->nanosecondsField);
	copyClockIdentity((buf + 44), header->sourcePortIdentity.clockIdentity);
	*(UInteger16 *) (buf + 52) = flip16(header->sourcePortIdentity.portNumber);

}


/*Unpack delayReq message from IN buffer of ptpClock to msgtmp.req*/
void
msgUnpackDelayReq(Octet * buf, MsgDelayReq * delayreq)
{
	delayreq->originTimestamp.secondsField.msb =
		flip16(*(UInteger16 *) (buf + 34));
	delayreq->originTimestamp.secondsField.lsb =
		flip32(*(UInteger32 *) (buf + 36));
	delayreq->originTimestamp.nanosecondsField =
		flip32(*(UInteger32 *) (buf + 40));

	#ifdef PTPD_DBG
	msgDelayReq_display(delayreq);
	#endif /* PTPD_DBG */

}


/*Unpack PdelayReq message from IN buffer of ptpClock to msgtmp.req*/
void
msgUnpackPdelayReq(Octet * buf, MsgPdelayReq * pdelayreq)
{
	pdelayreq->originTimestamp.secondsField.msb =
		flip16(*(UInteger16 *) (buf + 34));
	pdelayreq->originTimestamp.secondsField.lsb =
		flip32(*(UInteger32 *) (buf + 36));
	pdelayreq->originTimestamp.nanosecondsField =
		flip32(*(UInteger32 *) (buf + 40));

	#ifdef PTPD_DBG
	msgPdelayReq_display(pdelayreq);
	#endif /* PTPD_DBG */

}


/*Unpack delayResp message from IN buffer of ptpClock to msgtmp.presp*/
void
msgUnpackDelayResp(Octet * buf, MsgDelayResp * resp)
{
	resp->receiveTimestamp.secondsField.msb =
		flip16(*(UInteger16 *) (buf + 34));
	resp->receiveTimestamp.secondsField.lsb =
		flip32(*(UInteger32 *) (buf + 36));
	resp->receiveTimestamp.nanosecondsField =
		flip32(*(UInteger32 *) (buf + 40));
	copyClockIdentity(resp->requestingPortIdentity.clockIdentity,
	       (buf + 44));
	resp->requestingPortIdentity.portNumber =
		flip16(*(UInteger16 *) (buf + 52));

	#ifdef PTPD_DBG
	msgDelayResp_display(resp);
	#endif /* PTPD_DBG */
}


/*Unpack PdelayResp message from IN buffer of ptpClock to msgtmp.presp*/
void
msgUnpackPdelayResp(Octet * buf, MsgPdelayResp * presp)
{
	presp->requestReceiptTimestamp.secondsField.msb =
		flip16(*(UInteger16 *) (buf + 34));
	presp->requestReceiptTimestamp.secondsField.lsb =
		flip32(*(UInteger32 *) (buf + 36));
	presp->requestReceiptTimestamp.nanosecondsField =
		flip32(*(UInteger32 *) (buf + 40));
	copyClockIdentity(presp->requestingPortIdentity.clockIdentity,
	       (buf + 44));
	presp->requestingPortIdentity.portNumber =
		flip16(*(UInteger16 *) (buf + 52));

	#ifdef PTPD_DBG
	msgPdelayResp_display(presp);
	#endif /* PTPD_DBG */
}

/*pack PdelayRespfollowup message into OUT buffer of ptpClock*/
void
msgPackPdelayRespFollowUp(Octet * buf, MsgHeader * header, Timestamp * responseOriginTimestamp, PtpClock * ptpClock, const UInteger16 sequenceId)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x0A;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(PDELAY_RESP_FOLLOW_UP_LENGTH);
	*(UInteger16 *) (buf + 30) = flip16(sequenceId);
	*(UInteger8 *) (buf + 32) = 0x05;
	/* Table 23 */
	*(Integer8 *) (buf + 33) = 0x7F;
	/* Table 24 */

	/* Copy correctionField of PdelayReqMessage */
	*(Integer32 *) (buf + 8) = flip32(header->correctionField.msb);
	*(Integer32 *) (buf + 12) = flip32(header->correctionField.lsb);

	/* Pdelay_resp_follow_up message */
	*(UInteger16 *) (buf + 34) =
		flip16(responseOriginTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) =
		flip32(responseOriginTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) =
		flip32(responseOriginTimestamp->nanosecondsField);
	copyClockIdentity((buf + 44), header->sourcePortIdentity.clockIdentity);
	*(UInteger16 *) (buf + 52) =
		flip16(header->sourcePortIdentity.portNumber);
}

/*Unpack PdelayResp message from IN buffer of ptpClock to msgtmp.presp*/
void
msgUnpackPdelayRespFollowUp(Octet * buf, MsgPdelayRespFollowUp * prespfollow)
{
	prespfollow->responseOriginTimestamp.secondsField.msb =
		flip16(*(UInteger16 *) (buf + 34));
	prespfollow->responseOriginTimestamp.secondsField.lsb =
		flip32(*(UInteger32 *) (buf + 36));
	prespfollow->responseOriginTimestamp.nanosecondsField =
		flip32(*(UInteger32 *) (buf + 40));
	copyClockIdentity(prespfollow->requestingPortIdentity.clockIdentity,
	       (buf + 44));
	prespfollow->requestingPortIdentity.portNumber =
		flip16(*(UInteger16 *) (buf + 52));

#ifdef PTPD_DBG
        msgPdelayRespFollowUp_display(prespfollow);
#endif /* PTPD_DBG */
}

/* Pack Management message into OUT buffer */
void
msgPackManagementTLV(Octet *buf, MsgManagement *outgoing, PtpClock *ptpClock)
{
        DBGV("packing ManagementTLV message \n");

	UInteger16 dataLength = 0;

	switch(outgoing->tlv->managementId)
	{
	case MM_NULL_MANAGEMENT:
	case MM_SAVE_IN_NON_VOLATILE_STORAGE:
	case MM_RESET_NON_VOLATILE_STORAGE:
	case MM_ENABLE_PORT:
	case MM_DISABLE_PORT:
		dataLength = 0;
		break;
	case MM_CLOCK_DESCRIPTION:
		dataLength = packMMClockDescription(outgoing, buf);
		#ifdef PTPD_DBG
		mMClockDescription_display(
				(MMClockDescription*)outgoing->tlv->dataField, ptpClock);
		#endif /* PTPD_DBG */
		break;
        case MM_USER_DESCRIPTION:
                dataLength = packMMUserDescription(outgoing, buf);
                #ifdef PTPD_DBG
                mMUserDescription_display(
                                (MMUserDescription*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_INITIALIZE:
                dataLength = packMMInitialize(outgoing, buf);
                #ifdef PTPD_DBG
                mMInitialize_display(
                                (MMInitialize*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_DEFAULT_DATA_SET:
                dataLength = packMMDefaultDataSet(outgoing, buf);
                #ifdef PTPD_DBG
                mMDefaultDataSet_display(
                                (MMDefaultDataSet*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_CURRENT_DATA_SET:
                dataLength = packMMCurrentDataSet(outgoing, buf);
                #ifdef PTPD_DBG
                mMCurrentDataSet_display(
                                (MMCurrentDataSet*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_PARENT_DATA_SET:
                dataLength = packMMParentDataSet(outgoing, buf);
                #ifdef PTPD_DBG
                mMParentDataSet_display(
                                (MMParentDataSet*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_TIME_PROPERTIES_DATA_SET:
                dataLength = packMMTimePropertiesDataSet(outgoing, buf);
                #ifdef PTPD_DBG
                mMTimePropertiesDataSet_display(
                                (MMTimePropertiesDataSet*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_PORT_DATA_SET:
                dataLength = packMMPortDataSet(outgoing, buf);
                #ifdef PTPD_DBG
                mMPortDataSet_display(
                                (MMPortDataSet*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_PRIORITY1:
                dataLength = packMMPriority1(outgoing, buf);
                #ifdef PTPD_DBG
                mMPriority1_display(
                                (MMPriority1*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_PRIORITY2:
                dataLength = packMMPriority2(outgoing, buf);
                #ifdef PTPD_DBG
                mMPriority2_display(
                                (MMPriority2*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_DOMAIN:
                dataLength = packMMDomain(outgoing, buf);
                #ifdef PTPD_DBG
                mMDomain_display(
                                (MMDomain*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
	case MM_SLAVE_ONLY:
		dataLength = packMMSlaveOnly(outgoing, buf);
		#ifdef PTPD_DBG
		mMSlaveOnly_display(
				(MMSlaveOnly*)outgoing->tlv->dataField, ptpClock);
		#endif /* PTPD_DBG */
		break;
        case MM_LOG_ANNOUNCE_INTERVAL:
                dataLength = packMMLogAnnounceInterval(outgoing, buf);
                #ifdef PTPD_DBG
                mMLogAnnounceInterval_display(
                                (MMLogAnnounceInterval*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_ANNOUNCE_RECEIPT_TIMEOUT:
                dataLength = packMMAnnounceReceiptTimeout(outgoing, buf);
                #ifdef PTPD_DBG
                mMAnnounceReceiptTimeout_display(
                                (MMAnnounceReceiptTimeout*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_LOG_SYNC_INTERVAL:
                dataLength = packMMLogSyncInterval(outgoing, buf);
                #ifdef PTPD_DBG
                mMLogSyncInterval_display((MMLogSyncInterval*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_VERSION_NUMBER:
                dataLength = packMMVersionNumber(outgoing, buf);
                #ifdef PTPD_DBG
                mMVersionNumber_display(
                                (MMVersionNumber*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_TIME:
                dataLength = packMMTime(outgoing, buf);
                #ifdef PTPD_DBG
                mMTime_display(
                                (MMTime*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_CLOCK_ACCURACY:
                dataLength = packMMClockAccuracy(outgoing, buf);
                #ifdef PTPD_DBG
                mMClockAccuracy_display(
                                (MMClockAccuracy*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_UTC_PROPERTIES:
                dataLength = packMMUtcProperties(outgoing, buf);
                #ifdef PTPD_DBG
                mMUtcProperties_display(
                                (MMUtcProperties*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_TRACEABILITY_PROPERTIES:
                dataLength = packMMTraceabilityProperties(outgoing, buf);
                #ifdef PTPD_DBG
                mMTraceabilityProperties_display(
                                (MMTraceabilityProperties*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_UNICAST_NEGOTIATION_ENABLE:
                dataLength = packMMUnicastNegotiationEnable(outgoing, buf);
                #ifdef PTPD_DBG
                mMUnicastNegotiationEnable_display(
                                (MMUnicastNegotiationEnable*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_DELAY_MECHANISM:
                dataLength = packMMDelayMechanism(outgoing, buf);
                #ifdef PTPD_DBG
                mMDelayMechanism_display(
                                (MMDelayMechanism*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
        case MM_LOG_MIN_PDELAY_REQ_INTERVAL:
                dataLength = packMMLogMinPdelayReqInterval(outgoing, buf);
                #ifdef PTPD_DBG
                mMLogMinPdelayReqInterval_display(
                                (MMLogMinPdelayReqInterval*)outgoing->tlv->dataField, ptpClock);
                #endif /* PTPD_DBG */
                break;
	default:
		DBGV("packing management msg: unsupported id \n");
	}

	/* set the outgoing tlv lengthField to 2 + N where 2 is the managementId field
         * and N is dataLength, the length of the management tlv dataField field.
	 * See Table 39 of the spec.
	 */
	outgoing->tlv->lengthField = 2 + dataLength;

	packManagementTLV((ManagementTLV*)outgoing->tlv, buf);
}

/* Pack ManagementErrorStatusTLV message into OUT buffer */
void
msgPackManagementErrorStatusTLV(Octet *buf, MsgManagement *outgoing,
				PtpClock *ptpClock)
{
        DBGV("packing ManagementErrorStatusTLV message \n");

	UInteger16 dataLength = 0;

	dataLength = packMMErrorStatus(outgoing, buf);
	#ifdef PTPD_DBG
	mMErrorStatus_display((MMErrorStatus*)outgoing->tlv->dataField, ptpClock);
	#endif /* PTPD_DBG */

	/* set the outgoing tlv lengthField to 2 + (6 + N) where 2 is the
 	 * managementErrorId field and (6 + N) is dataLength, where 6 is
 	 * the managementId and reserved field and N is the displayData field
 	 * and optional pad field. See Table 71 of the spec.
	 */
	outgoing->tlv->lengthField = 2 + dataLength;

	packManagementTLV((ManagementTLV*)outgoing->tlv, buf);
}

/* Pack  singaling message into OUT buffer */
void
msgPackSignalingTLV(Octet *buf, MsgSignaling *outgoing, PtpClock *ptpClock)
{
        DBGV("packing SignalingTLV message \n");

	UInteger16 dataLength = 0;

	switch(outgoing->tlv->tlvType)
	{
	case TLV_REQUEST_UNICAST_TRANSMISSION:
		dataLength = packSMRequestUnicastTransmission(outgoing, buf);
		break;
	case TLV_GRANT_UNICAST_TRANSMISSION:
		dataLength = packSMGrantUnicastTransmission(outgoing, buf);
		break;
	case TLV_CANCEL_UNICAST_TRANSMISSION:
		dataLength = packSMCancelUnicastTransmission(outgoing, buf);
		break;
	case TLV_ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION:
		dataLength = packSMAcknowledgeCancelUnicastTransmission(outgoing, buf);
		break;
	default:
		DBGV("packing signaling msg: unsupported tlv type \n");
	};
	/* set the outgoing tlv lengthField to 2 + N where 2 is the managementId field
         * and N is dataLength, the length of the management tlv dataField field.
	 * See Table 39 of the spec.
	 */
	outgoing->tlv->lengthField = dataLength;
	packSignalingTLV((SignalingTLV*)outgoing->tlv, buf);
}

void
freeMMTLV(ManagementTLV* tlv) {
	DBGV("cleanup managementTLV data\n");
	switch(tlv->managementId)
	{
	case MM_CLOCK_DESCRIPTION:
		DBGV("cleanup clock description \n");
		freeMMClockDescription((MMClockDescription*)tlv->dataField);
		break;
	case MM_USER_DESCRIPTION:
		DBGV("cleanup user description \n");
		freeMMUserDescription((MMUserDescription*)tlv->dataField);
		break;
	case MM_NULL_MANAGEMENT:
	case MM_SAVE_IN_NON_VOLATILE_STORAGE:
	case MM_RESET_NON_VOLATILE_STORAGE:
	case MM_INITIALIZE:
	case MM_DEFAULT_DATA_SET:
	case MM_CURRENT_DATA_SET:
	case MM_PARENT_DATA_SET:
	case MM_TIME_PROPERTIES_DATA_SET:
	case MM_PORT_DATA_SET:
	case MM_PRIORITY1:
	case MM_PRIORITY2:
	case MM_DOMAIN:
	case MM_SLAVE_ONLY:
	case MM_LOG_ANNOUNCE_INTERVAL:
	case MM_ANNOUNCE_RECEIPT_TIMEOUT:
	case MM_LOG_SYNC_INTERVAL:
	case MM_VERSION_NUMBER:
	case MM_ENABLE_PORT:
	case MM_DISABLE_PORT:
	case MM_TIME:
	case MM_CLOCK_ACCURACY:
	case MM_UTC_PROPERTIES:
	case MM_TRACEABILITY_PROPERTIES:
	case MM_UNICAST_NEGOTIATION_ENABLE:
	case MM_DELAY_MECHANISM:
	case MM_LOG_MIN_PDELAY_REQ_INTERVAL:
	default:
		DBGV("no managementTLV data to cleanup \n");
	}
}

void
freeMMErrorStatusTLV(ManagementTLV *tlv) {
	DBGV("cleanup managementErrorStatusTLV data \n");
	freeMMErrorStatus((MMErrorStatus*)tlv->dataField);
}

void
msgPackManagement(Octet *buf, MsgManagement *outgoing, PtpClock *ptpClock)
{
	DBGV("packing management message \n");
	packMsgManagement(outgoing, buf);

}

/*
 * Unpack Management message from IN buffer of ptpClock to msgtmp.manage
 * return TRUE if there are more packed TLVs left in the message
 */
Boolean
msgUnpackManagement(Octet *buf, MsgManagement * manage, MsgHeader * header, PtpClock *ptpClock, const int tlvOffset)
{
	unpackMsgManagement(buf, manage, ptpClock);

	if ( manage->header.messageLength >= (MANAGEMENT_LENGTH + tlvOffset + TL_LENGTH) )
	{
		unpackManagementTLV(buf, tlvOffset, manage, ptpClock);

		/* at this point, we know what managementTLV we have, so return and
		 * let someone else handle the data */
		manage->tlv->dataField = NULL;
		return TRUE;
	}
	else /* no (more) TLV attached to this message */
	{
		manage->tlv = NULL;
		return FALSE;
		
	}

}

void
msgPackSignaling(Octet *buf, MsgSignaling *outgoing, PtpClock *ptpClock)
{
	DBGV("packing signaling message \n");
	packMsgSignaling(outgoing, buf);
}

/*
 * Unpack Signaling message from IN buffer of ptpClock to msgtmp.signaling
 * return TRUE if there are more packed TLVs left in the message.
 */
Boolean
msgUnpackSignaling(Octet *buf, MsgSignaling * signaling, MsgHeader * header, PtpClock *ptpClock, const int tlvOffset)
{
	unpackMsgSignaling(buf, signaling, ptpClock);

	if ( signaling->header.messageLength >= (SIGNALING_LENGTH + tlvOffset + TL_LENGTH) )
	{
		unpackSignalingTLV(buf + tlvOffset, signaling, ptpClock);

		DBGV("Signaling seq %d: Found TLV type 0x%04x: %d bytes left\n", header->sequenceId,
			signaling->tlv->tlvType,signaling->header.messageLength - SIGNALING_LENGTH - tlvOffset);

		/* at this point, we know what managementTLV we have, so return and
		 * let someone else handle the data */
		signaling->tlv->valueField = NULL;
		return TRUE;
	}
	else /* no (more) TLVs attached to this message */
	{
		signaling->tlv = NULL;
		DBGV("Signaling seq %d: No more TLVs in message\n", header->sequenceId);
		return FALSE;
	}

}

/**
 * Dump the most recent packet in the daemon
 *
 * @param ptpClock The central clock structure
 */
void msgDump(PtpClock *ptpClock)
{

#if defined(freebsd)
	static int dumped = 0;
#endif /* FreeBSD */

	msgDebugHeader(&ptpClock->msgTmpHeader);
	switch (ptpClock->msgTmpHeader.messageType) {
	case SYNC:
		msgDebugSync(&ptpClock->msgTmp.sync);
		break;
   
	case ANNOUNCE:
		msgDebugAnnounce(&ptpClock->msgTmp.announce);
		break;
   
	case FOLLOW_UP:
		msgDebugFollowUp(&ptpClock->msgTmp.follow);
		break;
   
	case DELAY_REQ:
		msgDebugDelayReq(&ptpClock->msgTmp.req);
		break;
   
	case DELAY_RESP:
		msgDebugDelayResp(&ptpClock->msgTmp.resp);
		break;
   
	case MANAGEMENT:
		msgDebugManagement(&ptpClock->msgTmp.manage);
		break;

	case SIGNALING:
		NOTIFY("* msgDebugSignaling not implemented *\n");
		/* TODO: IMPLEMENT ME */
		/* msgDebugSignaling(&ptpClock->msgTmp.signaling); */
		break;
   
	default:
		NOTIFY("msgDump:unrecognized message\n");
		break;
	}

#if defined(freebsd)
	/* Only dump the first time, after that just do a message. */
	if (dumped != 0)
		return;

	dumped++;
	NOTIFY("msgDump: core file created.\n");   

	switch(rfork(RFFDG|RFPROC|RFNOWAIT)) {
	case -1:
		NOTIFY("could not fork to core dump! errno: %s",
		       strerror(errno));
		break;
	case 0:
		abort(); /* Generate a core dump */
	default:
		/* This default intentionally left blank. */
		break;
	}
#endif /* FreeBSD */
}

/**
 * Dump a PTP message header
 *
 * @param header a pre-filled msg header structure
 */

void msgDebugHeader(MsgHeader *header)
{
	NOTIFY("msgDebugHeader: messageType %d\n", header->messageType);
	NOTIFY("msgDebugHeader: versionPTP %d\n", header->versionPTP);
	NOTIFY("msgDebugHeader: messageLength %d\n", header->messageLength);
	NOTIFY("msgDebugHeader: domainNumber %d\n", header->domainNumber);
	NOTIFY("msgDebugHeader: flags %02hhx %02hhx\n",
	       header->flagField0, header->flagField1);
	NOTIFY("msgDebugHeader: correctionfield %d\n", header->correctionField);
	NOTIFY("msgDebugHeader: sourcePortIdentity.clockIdentity "
	       "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx%02hhx:%02hhx\n",
	       header->sourcePortIdentity.clockIdentity[0],
	       header->sourcePortIdentity.clockIdentity[1],
	       header->sourcePortIdentity.clockIdentity[2],
	       header->sourcePortIdentity.clockIdentity[3],
	       header->sourcePortIdentity.clockIdentity[4],
	       header->sourcePortIdentity.clockIdentity[5],
	       header->sourcePortIdentity.clockIdentity[6],
	       header->sourcePortIdentity.clockIdentity[7]);
	NOTIFY("msgDebugHeader: sourcePortIdentity.portNumber %d\n",
	       header->sourcePortIdentity.portNumber);
	NOTIFY("msgDebugHeader: sequenceId %d\n", header->sequenceId);
	NOTIFY("msgDebugHeader: controlField %d\n", header->controlField);
	NOTIFY("msgDebugHeader: logMessageIntervale %d\n",
	       header->logMessageInterval);

}

/**
 * Dump the contents of a sync packet
 *
 * @param sync A pre-filled MsgSync structure
 */

void msgDebugSync(MsgSync *sync)
{
	NOTIFY("msgDebugSync: originTimestamp.seconds %u\n",
	       sync->originTimestamp.secondsField);
	NOTIFY("msgDebugSync: originTimestamp.nanoseconds %d\n",
	       sync->originTimestamp.nanosecondsField);
}

/**
 * Dump the contents of a announce packet
 *
 * @param sync A pre-filled MsgAnnounce structure
 */

void msgDebugAnnounce(MsgAnnounce *announce)
{
	NOTIFY("msgDebugAnnounce: originTimestamp.seconds %u\n",
	       announce->originTimestamp.secondsField);
	NOTIFY("msgDebugAnnounce: originTimestamp.nanoseconds %d\n",
	       announce->originTimestamp.nanosecondsField);
	NOTIFY("msgDebugAnnounce: currentUTCOffset %d\n",
	       announce->currentUtcOffset);
	NOTIFY("msgDebugAnnounce: grandmasterPriority1 %d\n",
	       announce->grandmasterPriority1);
	NOTIFY("msgDebugAnnounce: grandmasterClockQuality.clockClass %d\n",
	       announce->grandmasterClockQuality.clockClass);
	NOTIFY("msgDebugAnnounce: grandmasterClockQuality.clockAccuracy %d\n",
	       announce->grandmasterClockQuality.clockAccuracy);
	NOTIFY("msgDebugAnnounce: "
	       "grandmasterClockQuality.offsetScaledLogVariance %d\n",
	       announce->grandmasterClockQuality.offsetScaledLogVariance);
	NOTIFY("msgDebugAnnounce: grandmasterPriority2 %d\n",
	       announce->grandmasterPriority2);
	NOTIFY("msgDebugAnnounce: grandmasterClockIdentity "
	       "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx%02hhx:%02hhx\n",
	       announce->grandmasterIdentity[0],
	       announce->grandmasterIdentity[1],
	       announce->grandmasterIdentity[2],
	       announce->grandmasterIdentity[3],
	       announce->grandmasterIdentity[4],
	       announce->grandmasterIdentity[5],
	       announce->grandmasterIdentity[6],
	       announce->grandmasterIdentity[7]);
	NOTIFY("msgDebugAnnounce: stepsRemoved %d\n",
	       announce->stepsRemoved);
	NOTIFY("msgDebugAnnounce: timeSource %d\n",
	       announce->timeSource);
}

/**
 * NOT IMPLEMENTED
 *
 * @param req
 */
void msgDebugDelayReq(MsgDelayReq *req) {}

/**
 * Dump the contents of a followup packet
 *
 * @param follow A pre-fille MsgFollowUp structure
 */
void msgDebugFollowUp(MsgFollowUp *follow)
{
	NOTIFY("msgDebugFollowUp: preciseOriginTimestamp.seconds %u\n",
	       follow->preciseOriginTimestamp.secondsField);
	NOTIFY("msgDebugFollowUp: preciseOriginTimestamp.nanoseconds %d\n",
	       follow->preciseOriginTimestamp.nanosecondsField);
}

/**
 * Dump the contents of a delay response packet
 *
 * @param resp a pre-filled MsgDelayResp structure
 */
void msgDebugDelayResp(MsgDelayResp *resp)
{
	NOTIFY("msgDebugDelayResp: delayReceiptTimestamp.seconds %u\n",
	       resp->receiveTimestamp.secondsField);
	NOTIFY("msgDebugDelayResp: delayReceiptTimestamp.nanoseconds %d\n",
	       resp->receiveTimestamp.nanosecondsField);
	NOTIFY("msgDebugDelayResp: requestingPortIdentity.clockIdentity "
	       "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx%02hhx:%02hhx\n",
	       resp->requestingPortIdentity.clockIdentity[0],
	       resp->requestingPortIdentity.clockIdentity[1],
	       resp->requestingPortIdentity.clockIdentity[2],
	       resp->requestingPortIdentity.clockIdentity[3],
	       resp->requestingPortIdentity.clockIdentity[4],
	       resp->requestingPortIdentity.clockIdentity[5],
	       resp->requestingPortIdentity.clockIdentity[6],
	       resp->requestingPortIdentity.clockIdentity[7]);
	NOTIFY("msgDebugDelayResp: requestingPortIdentity.portNumber %d\n",
	       resp->requestingPortIdentity.portNumber);
}

/**
 * Dump the contents of management packet
 *
 * @param manage a pre-filled MsgManagement structure
 */

void msgDebugManagement(MsgManagement *manage)
{
	NOTIFY("msgDebugDelayManage: targetPortIdentity.clockIdentity "
	       "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
	       manage->targetPortIdentity.clockIdentity[0],
	       manage->targetPortIdentity.clockIdentity[1],
	       manage->targetPortIdentity.clockIdentity[2],
	       manage->targetPortIdentity.clockIdentity[3],
	       manage->targetPortIdentity.clockIdentity[4],
	       manage->targetPortIdentity.clockIdentity[5],
	       manage->targetPortIdentity.clockIdentity[6],
	       manage->targetPortIdentity.clockIdentity[7]);
	NOTIFY("msgDebugDelayManage: targetPortIdentity.portNumber %d\n",
	       manage->targetPortIdentity.portNumber);
	NOTIFY("msgDebugManagement: startingBoundaryHops %d\n",
	       manage->startingBoundaryHops);
	NOTIFY("msgDebugManagement: boundaryHops %d\n", manage->boundaryHops);
	NOTIFY("msgDebugManagement: actionField %d\n", manage->actionField);
	NOTIFY("msgDebugManagement: tvl %s\n", manage->tlv);
}
