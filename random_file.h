#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <inttypes.h> 
#include <stdlib.h>

/* Struct needle to user
 * - key    8 byte
 * - delete 1 byte
 * - offset 8 byte
 * - size   8 byte
 */

/* Struct needle to DB
 * - key    8 byte
 * - delete 1 byte
 * - size   8 byte
 * - data   - byte
 * - crc32  4 byte
 */

struct FileDB {
    char keyF[8];
    char deleteF[1];
    uint64_t offsetF;
    uint64_t sizeF;
};


class RandomFile : public std::fstream {
    public:
    
        uint64_t offsetDB;  // Usar un constructor y destructor
        //char nameDB [] = "DB.bin";

        RandomFile() : std::fstream("DB.bin", std::ios::in | std::ios::out | std::ios::binary) {
            if(!good()) {
                open("DB.bin", std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
            }
        }
        
        void needleError ( FileDB * file );
        FileDB saveFDB ( const char nameFile [], const char key[] );
        bool seekFDB ( const FileDB file );
        bool searchFDB ( const FileDB file );
        bool deleteFDB ( FileDB * file );
        bool recoveryFDB ( const FileDB file );
        bool compactDB ();

};