/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
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

#include "dernc.h"

namespace RNC_INTERNAL {
    struct BitStream {
        uint32 bit_buffer;      // Holds between 16 and 32 bits
        int bit_count;          // How many bits does bitbuf hold?
    };

    struct HuffmanTable {
        int node_count;         // Number of nodes in the tree
        struct {
            uint32 code;
            int code_length;
            int value;
        } table[32];
    };

    static uint16 crc_table[256];
    static bool is_crc_setup = false;

    void setupCRCTable() {
        uint16 temp;

        for (int i = 0; i < 256; ++i) {
            temp = i;

            for (int j = 0; j < 8; ++j)
                temp = (temp & 1 ? (temp >> 1) ^ 0xA001 : temp >> 1);

            crc_table[i] = temp;
        } is_crc_setup = true;
    }

    uint32 bitPeek(BitStream &bit_stream, uint32 mask) {
        return bit_stream.bit_buffer &mask;
    }

    static void bitAdvance(BitStream &bit_stream, int count,
            uint8 *&packed_data) {
        bit_stream.bit_buffer >>= count;
        bit_stream.bit_count -= count;
        if (bit_stream.bit_count < 16) {
            packed_data += 2;
            bit_stream.bit_buffer |=
                (READ_LE_UINT16(packed_data) << bit_stream.bit_count);
            bit_stream.bit_count += 16;
        }
    }

    static void bitAdvance8(BitStream &bit_stream, int count,
            uint8 *&packed_data, uint8 *packed_data_end) {
        bit_stream.bit_buffer >>= count;
        bit_stream.bit_count -= count;
        if (bit_stream.bit_count < 16) {
            packed_data += 2;
            if (packed_data < packed_data_end) {
                bit_stream.bit_buffer |=
                    ((uint32)(*packed_data) << bit_stream.bit_count);
                bit_stream.bit_count += 16;
            }
        }
    }

    uint32 bitRead(BitStream &bit_stream, uint32 mask, int count,
            uint8 *&packed_data) {
        uint32 result = bitPeek(bit_stream, mask);
        bitAdvance(bit_stream, count, packed_data);
        return result;
    }

    uint32 bitRead8(BitStream &bit_stream, uint32 mask, int count,
            uint8 *&packed_data, uint8 *packed_data_end) {
        uint32 result = bitPeek(bit_stream, mask);
        bitAdvance8(bit_stream, count, packed_data, packed_data_end);
        return result;
    }

    void readHuffmanTable(HuffmanTable &huffman_table,
            BitStream &bit_stream, uint8 *&packed_data) {
        int count = bitRead(bit_stream, 0x1f, 5, packed_data);
        if (!count)
            return;

        int leaf_max = 1;
        int leaf_length[32];
        for (int i = 0; i < count; ++i) {
            leaf_length[i] = bitRead(bit_stream, 0x0f, 4, packed_data);
            if (leaf_max < leaf_length[i])
                leaf_max = leaf_length[i];
        }

        uint32 code_b = 0;
        int node_count = 0;
        for (int i = 1; i <= leaf_max; ++i) {
            for (int j = 0; j < count; ++j)
                if (leaf_length[j] == i) {
                    huffman_table.table[node_count].code =
                        mirror(code_b, i);
                    huffman_table.table[node_count].code_length = i;
                    huffman_table.table[node_count].value = j;
                    ++code_b;
                    ++node_count;
                }
            code_b <<= 1;
        }

        huffman_table.node_count = node_count;
    }

    int readHuffmanData(HuffmanTable &huffman_table,
            BitStream &bit_stream, uint8 *&packed_data,
            uint8 *packed_data_end) {
        int i;
        uint32 mask;

        for (i = 0; i < huffman_table.node_count; ++i) {
            mask = (1 << huffman_table.table[i].code_length) - 1;
            if (bitPeek(bit_stream, mask) == huffman_table.table[i].code)
                break;
        }

        if (i == huffman_table.node_count)
            return -1;
        if ((packed_data + 2) < packed_data_end)
            bitAdvance(bit_stream, huffman_table.table[i].code_length,
                   packed_data);
        else
            bitAdvance8(bit_stream, huffman_table.table[i].code_length,
                   packed_data, packed_data_end);

        uint32 result = huffman_table.table[i].value;

        if (result >= 2) {
            result = 1 << (result - 1);
            if ((packed_data + 2) < packed_data_end)
                result |= bitRead(bit_stream, result - 1,
                        huffman_table.table[i].value - 1, packed_data);
            else
                result |= bitRead8(bit_stream, result - 1,
                        huffman_table.table[i].value - 1, packed_data,
                        packed_data_end);
        }

        return result;
    }

    void bitReadInit(BitStream &bit_stream, uint8 *&packed_data) {
        bit_stream.bit_buffer = READ_LE_UINT16(packed_data);
        bit_stream.bit_count = 16;
    }

    void bitReadFix(BitStream &bit_stream, uint8 *&packed_data) {
        bit_stream.bit_count -= 16;
        // Remove the top 16 bits
        bit_stream.bit_buffer &= (1 << bit_stream.bit_count) - 1;
        // Replace with the data at the current input position
        bit_stream.bit_buffer |=
            (READ_LE_UINT16(packed_data) << bit_stream.bit_count);
        bit_stream.bit_count += 16;
    }

    void bitReadFix8(BitStream &bit_stream, uint8 *&packed_data) {
        bit_stream.bit_count -= 16;
        // Remove the top 16 bits
        bit_stream.bit_buffer &= (1 << bit_stream.bit_count) - 1;
        // Replace with the data at the current input position
        bit_stream.bit_buffer |=
            ((uint32)(*packed_data) << bit_stream.bit_count);
        bit_stream.bit_count += 16;
    }

}

const char *const rnc::errorString(int error_code) {
    using namespace RNC_INTERNAL;

    static const char *const errors[] = {
        "No error",
        "File is not RNC-1 format",
        "Huffman decode error",
        "File size mismatch",
        "CRC error in packed data",
        "CRC error in unpacked data",
        "Unknown error"
    };
    static const int maxError = sizeof(errors) / sizeof(*errors) - 1;

    error_code = -error_code;
    return errors[error_code <
                  0 ? 0 : (error_code > maxError ? maxError : error_code)];
}

int rnc::unpackedLength(uint8 *packed_data) {
    using namespace RNC_INTERNAL;

    if (READ_BE_UINT32(packed_data) != RNC_SIGNATURE)
        return FILE_IS_NOT_RNC;

    return READ_BE_UINT32(packed_data + 4);
}

uint16 rnc::crc(uint8 *data, int data_length) {
    using namespace RNC_INTERNAL;
    if (!is_crc_setup)
        setupCRCTable();

    uint16 result = 0;
    do {
        result ^= *data++;
        result = (result >> 8) ^ crc_table[result & 0xff];
    } while (--data_length);

    return result;
}

int rnc::unpack(uint8 *packed_data, uint8 *unpacked_data) {
    using namespace RNC_INTERNAL;

    if (READ_BE_UINT32(packed_data) != RNC_SIGNATURE)
        return FILE_IS_NOT_RNC;

    int output_length = READ_BE_UINT32(packed_data + 4);
    int input_length = READ_BE_UINT32(packed_data + 8);

    uint16 unpacked_crc = READ_BE_UINT16(packed_data + 12);
    uint16 packed_crc = READ_BE_UINT16(packed_data + 14);

    uint8 *input = packed_data + 18;    // Skip the header
    uint8 *output = unpacked_data;

    uint8 *input_end = input + input_length;
    uint8 *output_end = output + output_length;

    // Check the packed data's CRC
    if (crc(input, input_end - input) != packed_crc)
        return PACKED_CRC_ERROR;

    BitStream bit_stream;

    bitReadInit(bit_stream, input);
    bitAdvance(bit_stream, 2, input);   // Discard first two bits

    // Process compressed chunks
    HuffmanTable raw_huff_tbl, dist_huff_tbl, len_huff_tbl;
    int length, position;
    uint32 ch_count;
    while (output < output_end) {
        readHuffmanTable(raw_huff_tbl, bit_stream, input);
        readHuffmanTable(dist_huff_tbl, bit_stream, input);
        readHuffmanTable(len_huff_tbl, bit_stream, input);

        ch_count = bitRead(bit_stream, 0xffff, 16, input);

        while (1) {
            length = readHuffmanData(raw_huff_tbl, bit_stream, input,
                input_end);
            if (length == -1)
                return HUF_DECODE_ERROR;

            if (length) {
                while (length--)
                    *output++ = *input++;
                if ((input + 1) < input_end)
                    bitReadFix(bit_stream, input);
                else
                    bitReadFix8(bit_stream, input);
            }

            if (--ch_count <= 0)
                break;

            position = readHuffmanData(dist_huff_tbl, bit_stream, input,
                input_end);
            if (position == -1)
                return HUF_DECODE_ERROR;

            length = readHuffmanData(len_huff_tbl, bit_stream, input,
                input_end);
            if (length == -1)
                return HUF_DECODE_ERROR;

            position += 1;
            length += 2;

            while (length--) {
                *output = output[-position];
                output++;
            }
        }
    }

    // Check to see if the unpacked data is the correct length
    if (output != output_end)
        return FILE_SIZE_MISMATCH;

    // Finally check our unpacked data's CRC
    if (crc(output_end - output_length, output_length) != unpacked_crc)
        return UNPACKED_CRC_ERROR;

    return output_length;
}
