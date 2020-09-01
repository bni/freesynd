/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2006  Tarjei Knapstad <tarjei.knapstad@gmail.com>    *
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
#ifndef XMIDI_H
#define XMIDI_H

#include "common.h"
#include <vector>

/*!
 * XMidi class.
 */
class XMidi {
  public:
    struct Midi {
      uint8* data_;
      int size_;
    };

    // Reads an .xmi file and returns a vector with individual MIDI tracks
    std::vector<Midi> convertXMidi(uint8 *buf, int size);

protected:
    // The code below and corresponding original implementation was taken
    // from the The System Shock Hack Project (BSD license),
    // http://tsshp.sourceforge.net/, Copyright (c) 2001 Ryan Nunn
    // Function names have been changed to protect the innocent (and match our
    // coding standards...)
    struct midi_event {
      int              time;
      uint8            status;

      uint8            data[2];

      uint32           len;
      uint8            *stream;

      midi_event       *next;
    };

    struct XMidiFile {
      uint16          tracks;

      midi_event      **events;
      int16           *timing;

      midi_event      *list;
      midi_event      *current;

      XMidiFile();
      ~XMidiFile();
      void allocData();
      void freeData();
      static void deleteEventList(midi_event *mlist);

      int curr_track; // used during load of multi-track XMI's (e.g. syngame.xmi)

      //
      // Data from the TIMB (timbre) chunk.
      //
      struct TIMB_
      {
        unsigned char patch_number;
        unsigned char timbre_bank; // -1 =
      };
      TIMB_         **timbres;
      uint16        *timbre_sizes;
    };

    /*
     * Loads an XMidi file.
     * Returns NULL if failed
     */
    XMidiFile *loadXMidi(uint8* buf, int size);

    /* Frees an XMidi returned by XMidi_Load */
    void freeXMidi(XMidiFile *xmidi);

    /*
     * Creates a buffer that contains a MIDI File.
     * Buffer size is returned.
     * If size is 0, an error was encountered
     */
    int retrieveMidi(const XMidiFile *xmidi, uint32 track, uint8 **buffer);

private:
    static midi_event *allocateEvent(int time);
    static void createNewEvent(XMidiFile *xmidi, int time);
    static int getVLQ(const unsigned char *stream, uint32 *quant);
    static int getVLQ2(const unsigned char *stream, uint32 *quant);
    int putVLQ(unsigned char *stream, uint32 value);
    static int convertEvent(XMidiFile *xmidi, const int time,
            const unsigned char status, const unsigned char *stream,
            const int size);
    static int convertSystemMessage(XMidiFile *xmidi, const int time,
            const unsigned char *stream);
    static int readEventList(XMidiFile *xmidi, const unsigned char *stream);
    static int extractEvents(XMidiFile *xmidi, const unsigned char *stream,
            const uint32 size);
    int readFile (XMidiFile *xmidi, const unsigned char *stream,
            uint32 streamsize);
    uint32 convertListToMTrk (unsigned char *buf, const midi_event *mlist);

    static bool handleChunkFORM(XMidiFile* xmidi, const unsigned char* stream,
            uint32 chunksize);
    static bool handleChunkCAT(XMidiFile* xmidi, const unsigned char* stream,
            uint32 chunksize);
    static bool handleChunkXDIR(XMidiFile* xmidi, const unsigned char* stream,
            uint32 chunksize);
    static bool handleChunkXMID(XMidiFile* xmidi, const unsigned char* stream,
            uint32 chunksize);
    static bool handleChunkTIMB(XMidiFile* xmidi, const unsigned char* stream,
            uint32 chunksize);
    static bool handleChunkEVNT(XMidiFile* xmidi, const unsigned char* stream,
            uint32 chunksize);

    enum MidiEvent {
        EV_NOTE_OFF   = 0x80,
        EV_NOTE_ON    = 0x90,
        EV_POLY_PRESS = 0xa0,
        EV_CONTROL    = 0xb0,
        EV_PROGRAM    = 0xc0,
        EV_CHAN_PRESS = 0xd0,
        EV_PITCH      = 0xe0,
        EV_SYSEX      = 0xf0,
        EV_ESC        = 0xf7, // SysEx event continuation
        EV_META       = 0xff  // MetaEvent
    };

    enum MetaEvent {
        META_OUTPUT_CABLE = 0x21,
        META_EOT          = 0x2f, META_TRACK_END = META_EOT,
        META_TEMPO        = 0x51, META_SET_TEMPO = META_TEMPO,
        META_TIME_SIG     = 0x58,
        META_KEYSIG       = 0x59
    };
};

#endif
