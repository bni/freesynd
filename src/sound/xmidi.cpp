/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
 *   Copyright (C) 2007  Mike Nordell ("proper" IFF parsing)            *
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#include "xmidi.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>

#define XMIDI_CONTROLLER_NEXT_BREAK     117


struct IFFchunk
{
    bool isType(const char szId[4]) const {
        return memcmp(id, szId, 4) == 0;
    }

    // getActualSize takes IFF rounding rules into account,
    // keeping stream sizes 2-byte aligned
    uint32 getActualSize() const {
        return size + (size&1);
    }

    char   id[4];
    uint32 size;
};


static uint32 get_IFF_uint32(const unsigned char* stream)
{
    return (stream[0] << 24) |
           (stream[1] << 16) |
           (stream[2] <<  8) |
           (stream[3]);
}

static void getIFFchunkHeader(IFFchunk& chunk, const unsigned char* stream)
{
    memcpy(&chunk.id, stream, 4);
    chunk.size = get_IFF_uint32(stream + 4);
}


std::vector<XMidi::Midi> XMidi::convertXMidi(uint8* buf, int size)
{
    std::vector<Midi> midi;
    XMidiFile* xmi = loadXMidi(buf, size);
    if (xmi)
    {
    midi.resize(xmi->tracks);
    for(unsigned int i = 0; i < xmi->tracks; ++i)
    {
        midi[i].size_ = retrieveMidi(xmi, i, &(midi[i].data_));
    }
    delete xmi;
    }
    return midi;
}

XMidi::XMidiFile* XMidi::loadXMidi(uint8* buf, int size)
{
    XMidiFile* xmidi = new XMidiFile;

    if (!readFile (xmidi, buf, size))
    {
    delete xmidi;
    xmidi = NULL;
    }
    return xmidi;
}

int XMidi::retrieveMidi(const XMidiFile *xmidi, uint32 track, uint8 **buffer)
{
    if (!xmidi->events)
    {
    fprintf (stderr, "No midi data in loaded\n");
    return 0;
    }

    if (track >= xmidi->tracks)
    {
    fprintf (stderr, "Can't retrieve MIDI data, track out of range\n");
    return 0;
    }

    int len = 14 + convertListToMTrk (NULL, xmidi->events[track]);

    if (len == 14)
    {
    len = 0;
    return 0;
    }

    uint8* buf = new unsigned char[len];
    *buffer = buf;

    if (!buf)
    {
    fprintf (stderr, "Unable to allocate output buffer\n");
    return 0;
    }

    buf[0] = 'M';
    buf[1] = 'T';
    buf[2] = 'h';
    buf[3] = 'd';

    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 6;

    buf[8] = 0;
    buf[9] = 0;

    buf[10] = 0;
    buf[11] = 1;

    buf[12] = (unsigned char) (xmidi->timing[track] >> 8);
    buf[13] = (unsigned char) (xmidi->timing[track] & 0xFF);

    convertListToMTrk (buf+14, xmidi->events[track]);

    return len;
}



XMidi::midi_event* XMidi::allocateEvent(int time)
{
    midi_event *event = new midi_event;
    event->status = 0;
    event->next = NULL;
    event->time = time;
    event->stream = NULL;
    event->len = 0;

    return event;
}

/* Sets current to the new event and updates list */
void XMidi::createNewEvent (XMidiFile *xmidi, int time)
{
    if (!xmidi->list)
    {
        xmidi->list = xmidi->current = allocateEvent(time);
        return;
    }

    if (xmidi->current->time > time)
        xmidi->current = xmidi->list;

    while (xmidi->current->next)
    {
        if (xmidi->current->next->time > time)
        {
            midi_event *event = allocateEvent(time);
            
            event->next = xmidi->current->next;
            xmidi->current->next = event;
            xmidi->current = event;
            return;
        }
        
        xmidi->current = xmidi->current->next;
    }

    xmidi->current->next = allocateEvent(time);
    xmidi->current = xmidi->current->next;
}


//
// Get the MIDI variable-length quantity. A string of 7-bits/byte,
// terminated by a byte not having MSB set..
// Most significant byte first.
// Returns # of bytes extracted from stream.
//
int XMidi::getVLQ (const unsigned char *stream, uint32 *quant)
{
    int i;
    *quant = 0;

    for (i = 0; i < 4; i++)
    {
        *quant <<= 7;
        *quant |= stream[i] & 0x7F;

        if (!(stream[i] & 0x80))
        {
            i++;
            break;
        }
    }

    return i;
}

//
// Instead of treating consecutive delta/interval counts as separate counts,
// just sum them up until we hit a MIDI event.
// Returns # of bytes extracted from stream.
//
int XMidi::getVLQ2 (const unsigned char *stream, uint32 *quant)
{
    int i;
    *quant = 0;

    for (i = 0; i < 4 && !(stream[i] & 0x80); i++)
        *quant += stream[i];

    return i;
}

//
// Write a MIDI variable-length quantity (see getVlc) into 'stream'.
// Returns # of bytes used to store 'value'.
// Note: stream can be NULL (useful to count how much space a value would need)
//
int XMidi::putVLQ(unsigned char *stream, uint32 value)
{
    int buffer;
    int i = 1;
    buffer = value & 0x7F;
    while (value >>= 7)
    {
        buffer <<= 8;
        buffer |= ((value & 0x7F) | 0x80);
        i++;
    }
    if (!stream) return i;
    while (1)
    {
        *stream++ = (buffer & 0xFF);
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
    
    return i;
}

/*
// Converts Events
//
// Pointer to stream is at the status byte
// size 1 is single data byte
// size 2 is dual data byte
// size 3 is XMI Note on
// Returns bytes converted
*/

int XMidi::convertEvent (XMidiFile *xmidi, const int time, const unsigned char status, const unsigned char *stream, const int size)
{
    int i;
    uint32 delta = 0;
    midi_event *prev;

    createNewEvent (xmidi, time);
    xmidi->current->status = status;
    xmidi->current->data[0] = stream[0];

    if (size == 1) return 1;

    xmidi->current->data[1] = stream[1];

    if (size == 2) return 2;

    /* Save old */
    prev = xmidi->current;

    i = getVLQ (stream+2, &delta);

    createNewEvent (xmidi, time+delta*3);

    xmidi->current->status = status;
    xmidi->current->data[0] = stream[0];
    xmidi->current->data[1] = 0;
    
    /* Change the xmidi->current to the prev */
    xmidi->current = prev;

    return i + 2;
}

/* Simple routine to convert system messages */
int XMidi::convertSystemMessage (XMidiFile *xmidi, const int time, const unsigned char *stream)
{
    int i=1;

    createNewEvent (xmidi, time);
    xmidi->current->status = stream[0];

    /* Handling of Meta events */
    if (stream[0] == EV_META)
    {
        xmidi->current->data[0] = stream[1];
        i++;
    }

    i += getVLQ (stream+i, &xmidi->current->len);

    if (!xmidi->current->len) return i;

    xmidi->current->stream = new unsigned char[xmidi->current->len];

    memcpy (xmidi->current->stream, stream+i, xmidi->current->len);

    return i+xmidi->current->len;
}

/*
// XMidiFile to List
// Returns PPQN 
*/
int XMidi::readEventList (XMidiFile *xmidi, const unsigned char *stream)
{
    int    time = 0;
    uint32 delta;
    int    end = 0;
    int    tempo = 500000;
    int    tempo_set = 0;

    int i = 0;

    for (i = 0; !end; )
    {
        switch (stream[i] >> 4)
        {
            /* Note Off */
            case EV_NOTE_OFF >> 4:
            fprintf (stderr, "ERROR: Note off not valid in XMidiFile\n");
            return 0;

            /* Note On */
            case EV_NOTE_ON >> 4:
            i+= 1+convertEvent (xmidi, time, stream[i], stream+i+1, 3);
            break;

            /* 2 byte data */
            /* Aftertouch, Controller and Pitch Wheel */
            case EV_POLY_PRESS >> 4:
            case EV_CONTROL >> 4:
            case EV_PITCH >> 4:
            i+= 1+convertEvent (xmidi, time, stream[i], stream+i+1, 2);
            break;

            /* 1 byte data */
            /* Program Change and Channel Pressure */
            case EV_PROGRAM >> 4:
            case EV_CHAN_PRESS >> 4:
            i+= 1+convertEvent (xmidi, time, stream[i], stream+i+1, 1);
            break;

            /* SysEx */
            case EV_SYSEX >> 4:
            if (stream[i] == EV_META)
            {
                const MetaEvent evt = static_cast<MetaEvent>(stream[i+1]);
                switch (evt)
                {
                    case META_OUTPUT_CABLE:
                    break;
                    case META_TRACK_END: /* End Of Track */
                    end = 1;
                    break;
                    case META_SET_TEMPO:
                    if (!tempo_set) /* Tempo. Need it for PPQN */
                    {
                        tempo = stream[i+5];
                        tempo |= stream[i+4] << 8;
                        tempo |= stream[i+3] << 16;
                        tempo_set = 1;
                    }
                    break;
                    case META_TIME_SIG:
                    {
                        //int num = stream[i+2];        // numerator of the time signature
                        //int denom = 1 << stream[i+2]; // denominator of the time signature
                        //int qnotes = 4 * num / denom; // number of quarter notes per beat
                    }
                    break;
                    case META_KEYSIG:
                    {
                        // value=# of sharps/flats, major/minor
                    }
                    break;
                    default:
                    // log "unknown meta event"?
                    break;
                }
            }
            else
            {
                // log "unknown sysex event"?
            }

            i+= convertSystemMessage (xmidi, time, stream+i);
            break;

            /* Delta T, also known as interval count */
            default:
            {
            const int size = getVLQ2 (stream+i, &delta);
            i+= size;
            delta *= 3;
            time += delta;
            }
            break;
        }
    }

    return (tempo*9)/25000;
}

//
// stream points to the MIDI events.
// size is the size (in bytes) of this event chunk
//
int XMidi::extractEvents(XMidiFile *xmidi, const unsigned char *stream, const uint32 size)
{
    xmidi->list = NULL;

    /* Convert it */
    signed short ppqn = readEventList (xmidi, stream);
    if (!ppqn)
    {
    fprintf (stderr, "Unable to convert data\n");
    return 0;
    }

    xmidi->timing[xmidi->curr_track] = ppqn;
    xmidi->events[xmidi->curr_track] = xmidi->list;

    ++xmidi->curr_track;

    /* Return how many tracks were converted */
    return 1;
}

int XMidi::readFile (XMidiFile *xmidi, const unsigned char *stream, const uint32 streamsize)
{
    // Partial format specifications at
    // http://rewiki.regengedanken.de/wiki/.IFF
    // http://siedler15.ra-doersch.de/formate/single.html
    //
    // See also the specification "IFF - Interchange File Format"
    // as defined by Electronic Arts (EA) 1985, available in
    // e.g. "AMIGA ROM KERNEL REFERENCE MANUAL".

    uint32 read_so_far = 0;
    IFFchunk chunk;

    xmidi->tracks = 1; // default to 1 track, in case there is no XDIR chunk

    do {
    bool result;
    getIFFchunkHeader(chunk, stream);
    stream += 8;
    read_so_far += 8;

    if (chunk.isType("FORM")) {
        result = handleChunkFORM(xmidi, stream, chunk.size);
    } else
    if (chunk.isType("CAT ")) {
        result = handleChunkCAT(xmidi, stream, chunk.size);
    } else {
        return 0; // We don't know how to handle this
    }
    if (!result) {
        return 0; // something failed
    }
    stream      += chunk.getActualSize();
    read_so_far += chunk.getActualSize();
    } while (read_so_far < streamsize);

    return 1; // done
}

/*
// Converts an event list to a MTrk
// Returns bytes of the array
// buf can be NULL
*/
uint32 XMidi::convertListToMTrk (unsigned char *buf, const midi_event *mlist)
{
    int time = 0;
    uint32    delta = 0;
    unsigned char    last_status = 0;
    uint32     i = 8;
    uint32     j;
    int i_start;

    /* This is set true to make the song end when an XMidiFile break is hit. */
    int    sshock_break = 0;

    if (buf)
    {
        buf[0] = 'M';
        buf[1] = 'T';
        buf[2] = 'r';
        buf[3] = 'k';
    }

    for (const midi_event* event = mlist; event; event=event->next)
    {
        i_start = i;

        /* If sshock_break is set, the delta is only 0 */
        delta = sshock_break?0:event->time - time;
        time = event->time;

        if (buf) i += putVLQ (buf+i, delta);
        else i += putVLQ (NULL, delta);

        if ((event->status != last_status) || (event->status >= EV_SYSEX))
        {
            if (buf) buf[i] = event->status;
            i++;
        }

        last_status = event->status;

        switch (event->status >> 4)
        {
            /* 2 bytes data */
            /* Note off, Note on, Aftertouch and Pitch Wheel */
            case EV_NOTE_OFF >> 4: // invalid in XMID
            case EV_NOTE_ON >> 4:
            case EV_POLY_PRESS >> 4:
            case EV_PITCH >> 4:
            if (buf)
            {
                buf[i] = event->data[0];
                buf[i+1] = event->data[1];
            }
            i += 2;
            break;

            /* Controller, we need to catch XMIXI Breaks */
            case EV_CONTROL >> 4:
            if (buf)
            {
                buf[i] = event->data[0];
                buf[i+1] = event->data[1];
            }
            /* XMidiFile Break */
            if (event->data[0] == XMIDI_CONTROLLER_NEXT_BREAK) sshock_break = 1;
            i += 2;
            break;

            /* 1 bytes data */
            /* Program Change and Channel Pressure */
            case EV_PROGRAM >> 4:
            case EV_CHAN_PRESS >> 4:
            if (buf) buf[i] = event->data[0];
            i++;
            break;
            

            /* Variable length */
            /* SysEx */
            case EV_SYSEX >> 4:
            if (event->status == EV_META)
            {
                if (buf) buf[i] = event->data[0];
                i++;
            }
    
            if (buf) i += putVLQ (buf+i, event->len);
            else i += putVLQ (NULL, event->len);
            
            if (event->len)
            {
                for (j = 0; j < event->len; j++)
                {
                    if (buf) buf[i] = event->stream[j]; 
                    i++;
                }
            }

            break;
            

            /* Never occur */
            default:
            fprintf (stdout, "Not supposed to see this\n");
            break;
        }
    }

    if (buf)
    {
        uint32    size = i -8;
        buf[4] = (unsigned char) ((size >> 24) & 0xFF);
        buf[5] = (unsigned char) ((size >> 16) & 0xFF);
        buf[6] = (unsigned char) ((size >> 8) & 0xFF);
        buf[7] = (unsigned char) (size & 0xFF);
    }

    return i;
}

bool XMidi::handleChunkFORM(XMidiFile* xmidi, const unsigned char* stream, uint32 chunksize)
{
    char formType[4];
    memcpy(formType, stream, 4);
    stream += 4;

    if (!memcmp(formType, "XDIR", 4)) {
    return handleChunkXDIR(xmidi, stream, chunksize - 4);
    }

    if (!memcmp(formType, "XMID", 4)) {
    return handleChunkXMID(xmidi, stream, chunksize - 4);
    }

    return false; // We don't know how to handle this
}

// A "CAT " chunk is a conCATenation of other chunks.
bool XMidi::handleChunkCAT(XMidiFile* xmidi, const unsigned char* stream, uint32 chunksize)
{
    IFFchunk chunk;
    uint32 read_so_far = 0;

    do {
    getIFFchunkHeader(chunk, stream);
    // do NOT increment stream by sizeof(chunk) here, as
    // e.g. XMID isn't really a chunk but only a data identifier
    // with no trailing size designator.
    stream += 4;
    read_so_far += 4;

    bool result;

    if (chunk.isType("FORM")) {
        stream += 4;      // adjust for "real" IFF chunk header size
        read_so_far += 4; // ditto
        result = handleChunkFORM(xmidi, stream, chunk.size);
    } else
    if (chunk.isType("XMID")) {
        const uint32 xmid_data_len = chunksize - read_so_far;
        result = handleChunkXMID(xmidi, stream, xmid_data_len);
        chunk.size = xmid_data_len; // "fake" it, for calculation below
    } else {
        return false; // We don't know how to handle this
    }
    if (!result) {
        return false; // something failed
    }
    stream      += chunk.getActualSize();
    read_so_far += chunk.getActualSize();
    } while (read_so_far < chunksize);

    return true;
}

bool XMidi::handleChunkXDIR(XMidiFile* xmidi, const unsigned char* stream, uint32 chunksize)
{
    if (chunksize != 10 /* "INFO" + 32-bit_size + UWORD */) {
    return false;
    }

    IFFchunk chunk;

    getIFFchunkHeader(chunk, stream);
    stream += 8;

    if (!chunk.isType("INFO")) {
    return false; // bugger
    }

    if (chunk.size != 2) {
    return false; // malformed chunk
    }
    // # of tracks (implies # of FORMs) is an IFF abomination, as
    // it's a little-endian 16-bit value.
    xmidi->tracks = READ_LE_UINT16(stream);

    xmidi->allocData();

    return true;
}

//
// The XMID chunk contains the actual midi data
//
bool XMidi::handleChunkXMID(XMidiFile* xmidi, const unsigned char* stream, uint32 chunksize)
{
    IFFchunk chunk;

    uint32 read_so_far = 0;

    do {
    bool result;

    getIFFchunkHeader(chunk, stream);
    stream += 8;
    read_so_far += 8;

    if (chunk.isType("FORM")) {
        result = handleChunkFORM(xmidi, stream, chunk.size);
    } else
        if (chunk.isType("TIMB")) {
        result = handleChunkTIMB(xmidi, stream, chunk.size);
        } else
        if (chunk.isType("EVNT")) {
            result = handleChunkEVNT(xmidi, stream, chunk.size);
        } else {
            return false; // We don't know how to handle this
        }
        if (!result) {
            return 0; // something failed
        }
        stream      += chunk.getActualSize();;
        read_so_far += chunk.getActualSize();
    } while (read_so_far < chunksize);

    return true;
}

//
// Data in the (optional) TIMB chunk is as follow:
//   TIMB<len>
//       UWORD # of timbre list entries, 0-16384
//     { UBYTE patch number 0-127
//       UBYTE timbre bank 0-127 } ...
//
bool XMidi::handleChunkTIMB(XMidiFile* xmidi, const unsigned char* stream, uint32 chunksize)
{
    // The TIMBre chunk contains a list of required timbres.
    // Believe it or not, but the size actually is little-endian
    // (how's that for IFF and compatibility)
    const uint16 timbre_count = READ_LE_UINT16(stream);
    stream += 2;

    // sanity check
    if ((uint32)timbre_count * 2 + 2 != chunksize) {
    // sanity failure. Log "TIMBre chunk insanity"?
    return false;
    }

    typedef XMidiFile::TIMB_ timb_t;
    assert(sizeof(timb_t) == 2);

    timb_t*& t = xmidi->timbres[xmidi->curr_track];

    t = new timb_t[timbre_count];
    memcpy(t, stream, timbre_count * sizeof(*t));
    xmidi->timbre_sizes[xmidi->curr_track] = timbre_count;

    return true;
}

//
// The EVNT chunk. The actual MIDI events.
//
// Data in the EVNT chunk is as follow:
//  EVNT<len>
//    { UBYTE interval count (if < 128) 
//      UBYTE <MIDI event> (if > 127) } ...
//
// <MIDI event> consists of any MIDI Channel Voice, System Exclusive,
// or Meta-event except Note Off.  The MIDI 'running status' convention
// is not used.  Note On events are followed by their duration in intervals,
// expressed as a MIDI variable-length quantity (i.e., a string of 7-bit
// bytes stored most significant byte first, with the high bit set in
// all bytes except the last).
//
bool XMidi::handleChunkEVNT(XMidiFile* xmidi, const unsigned char* stream, uint32 chunksize)
{
    // Set things up and pass the events data to extractEvents

    xmidi->allocData(); // precaution, in case XDIR wasn't found

    int count = extractEvents(xmidi, stream, chunksize);

    if (count != 1)
    {
    fprintf (stderr, "Error: unable to extract track from XMidiFile.\n");
    return false;
    }

    return true;
}

XMidi::XMidiFile::XMidiFile()
:   tracks(1),
    events(0),
    timing(0),
    list(0),
    current(0),
    curr_track(0),
    timbres(0),
    timbre_sizes(0)
{
}


XMidi::XMidiFile::~XMidiFile()
{
    if (events)
    {
        for (int i=0; i < tracks; i++)
        {
            deleteEventList (events[i]);
            delete[] timbres[i];
        }
        delete[] events;
    }
    delete[] timing;
    delete[] timbres;
    delete[] timbre_sizes;
}

void XMidi::XMidiFile::allocData()
{
    if (events) {
    return;
    }

    events = new midi_event*[tracks];
    timing = new int16[tracks];
    timbres = new TIMB_*[tracks];
    timbre_sizes = new uint16[tracks];

    for (int track = 0; track < tracks; ++track)
    {
    timing[track] = 0;
    events[track] = NULL;
    timbres[track] = 0;
    timbre_sizes[track] = 0;
    }
}

void XMidi::XMidiFile::deleteEventList (midi_event *mlist)
{
    midi_event* next = mlist;
    midi_event* event = mlist;

    while ((event = next))
    {
    next = event->next;
    delete [] event->stream;
    delete event;
    }
}

