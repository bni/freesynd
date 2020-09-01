///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Copyright � NetworkDLS 2002, All rights reserved
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//
// Modified by Bohdan Stelmakh for use in Freesynd
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _CCRC32_H
#define _CCRC32_H
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CCRC32{

    public:
        CCRC32(void);
        ~CCRC32(void);

        void Initialize(void);

        bool FileCRC(const char *sFileName, unsigned int *ulOutCRC, size_t ulBufferSize);

        unsigned int FullCRC(const unsigned char *sData, size_t ulDataLength);
        void FullCRC(const unsigned char *sData, size_t ulLength, unsigned int *ulOutCRC);

        void PartialCRC(unsigned int *ulCRC, const unsigned char *sData, size_t ulDataLength);

    private:
        unsigned int Reflect(unsigned int ulReflect, const char cChar);
        unsigned int ulTable[256]; // CRC lookup table array.
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
