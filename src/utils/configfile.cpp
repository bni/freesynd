// Richard J. Wagner  v2.1  24 May 2004  wagnerr@umich.edu
// Modified by Joey Parrish, June 2011 joey.parrish@gmail.com

// Copyright (c) 2004 Richard J. Wagner
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "configfile.h"

using std::string;

ConfigFile::ConfigFile( string filename, string delimiter,
                        string comment, string sentry )
    : myDelimiter(delimiter), myComment(comment), mySentry(sentry)
{
    // Construct a ConfigFile, getting keys and values from given file
    
    std::ifstream in( filename.c_str() );
    
    if( !in ) throw file_not_found( filename ); 
    
    in >> (*this);
}


ConfigFile::ConfigFile()
    : myDelimiter( string(1,'=') ), myComment( string(1,'#') )
{
    // Construct a ConfigFile without a file; empty
}


void ConfigFile::remove( const string& key )
{
    // Remove key and its value
    myContents.erase( key );
    if (myLineNumbers.find( key ) != myLineNumbers.end()) {
        myLines[myLineNumbers[key]].erase();
        myLineNumbers.erase( key );
    }
    return;
}


bool ConfigFile::keyExists( const string& key ) const
{
    // Indicate whether key is found
    mapci p = myContents.find( key );
    return ( p != myContents.end() );
}


/* static */
void ConfigFile::trim( string& s )
{
    // Remove leading and trailing whitespace
    static const char whitespace[] = " \n\t\v\r\f";
    s.erase( 0, s.find_first_not_of(whitespace) );
    s.erase( s.find_last_not_of(whitespace) + 1U );
}


std::ostream& operator<<( std::ostream& os, const ConfigFile& cf )
{
    // Save a ConfigFile to os
    for( size_t i = 0;
         i != cf.myLines.size();
         ++i )
    {
        os << cf.myLines[i] << std::endl;
    }
    if ( cf.mySentry != "" )
    {
        os << cf.mySentry << std::endl;
    }
    return os;
}


std::istream& operator>>( std::istream& is, ConfigFile& cf )
{
    // Load a ConfigFile from is
    // Read in keys and values, keeping internal whitespace
    typedef string::size_type pos;
    const string& delim  = cf.myDelimiter;  // separator
    const string& comm   = cf.myComment;    // comment
    const string& sentry = cf.mySentry;     // end of file sentry
    const pos skip = delim.length();        // length of separator
    
    string nextline = "";  // might need to read ahead to see where value ends
    
    while( is || nextline.length() > 0 )
    {
        // Read an entire line at a time
        string line;
        if( nextline.length() > 0 )
        {
            line = nextline;  // we read ahead; use it now
            nextline = "";
        }
        else
        {
            std::getline( is, line );
        }
        int line_number = cf.myLines.size();
        cf.myLines.push_back(line);
        
        // Ignore comments
        // TODO: we will loose commented line here, this is bad
        // if it will be modified
        line = line.substr( 0, line.find(comm) );
        
        // Check for end of file sentry
        if( sentry != "" && line.find(sentry) != string::npos ) return is;
        
        // Parse the line if it contains a delimiter
        pos delimPos = line.find( delim );
        if( delimPos != string::npos )
        {
            // Extract the key
            string key = line.substr( 0, delimPos );
            line.replace( 0, delimPos+skip, "" );
            
            // See if value continues on the next line
            // Stop at blank line, next line with a key, end of stream,
            // or end of file sentry
            while( is )
            {
                std::getline( is, nextline );
                
                if ( is.eof() )
                {
                    // don't add an extra blank line to the list.
                    break;
                }
                
                string nlcopy = nextline;
                ConfigFile::trim(nlcopy);
                if( nlcopy == "" )
                {
                    cf.myLines.push_back(nextline);
                    break;
                }
                
                if (nextline.find(comm) != 0) {
                    // TODO: we will loose commented line here, this is bad
                    nextline = nextline.substr( 0, nextline.find(comm) );
                } else {
                    // Comment will be processed later
                    break;
                }
                if( nextline.find(delim) != string::npos )
                {
                    // we will process it at next run correctly
                    break;
                }
                if( sentry != "" && nextline.find(sentry) != string::npos )
                {
                    cf.myLines.push_back(nextline);
                    break;
                }
                
                nlcopy = nextline;
                ConfigFile::trim(nlcopy);
                if( nlcopy != "" )
                {
                    cf.myLines[line_number] += "\n";
                    line += "\n";
                }
                line += nlcopy;
                cf.myLines[line_number] += nlcopy;
            }
            
            // Store key and value
            ConfigFile::trim(key);
            ConfigFile::trim(line);
            cf.myContents[key] = line;  // overwrites if key is repeated
            cf.myLineNumbers[key] = line_number;
        }
    }
    
    return is;
}
