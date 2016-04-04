#include <iostream>
#include <time.h>
#include <string.h>

#include "random_file.h"
//#include "type_file.h"

// bytes that read in binary the RandomFile
#define nbytes 128

// Limits of the files for test
/* FIXME : Reconocer el tipo de SO que se utiliza para los multiplos de 1000 y 1024 */
#define multSO 1000
#define useGB multSO*multSO*multSO

#define len2gb 2*useGB
#define len4gb 4*useGB
#define len8gb 8*useGB
#define len16gb 16*useGB
#define len32gb 32*useGB
//
#define lenArch 32000000000    // leng32gb, Long file
#define maxLenArch 32000000021 // leng32gb + numBytesMaxFile, Max limit file

// num bytes of the each needle in the RandomFile
#define numByteskey 8
#define numBytesSize 8         
#define numBytesDelete 1
#define numBytesCheckSum32 4

//#define numBytesMaxFile numByteskey + numBytesDelete + numBytesSize + numBytesCheckSum32

void RandomFile::needleError ( FileDB * file ) {
    strcpy (file->keyF,"Error");
    strcpy (file->deleteF, "V");
    file->offsetF = 0;
    file->sizeF = 0;

}


FileDB RandomFile::saveFDB ( const char nameFile [], const char key[]) {
    
    FileDB fdb_;

    std::fstream fichero_in;

    fichero_in.open (nameFile, std::ios::in | std::ios::binary);
    if ( fichero_in.fail() ) {
        needleError(&fdb_);
        std::cout << "Error file (1)\n";
        fichero_in.close();

    } else {

        /* FIXME : Obtener la cantidad de bytes de la BD (RandomFile) */        
        //seekg(0, std::ios::end);
        offsetDB = (uint64_t)tellg();
        seekg(offsetDB);

        if ( offsetDB >= lenArch ) {
            std::cout << "Cannot write in DB\n";
            needleError(&fdb_);
            std::cout << "Error file (2)\n";
            fichero_in.close();

        } else {
        
            // some flags for write
            char is_key[8];         strcpy(is_key, key);
            char is_delete[1];      strcpy(is_delete,"F");
            char is_size[8];
            char is_check_sum[4];   strcpy(is_check_sum,"AAAA");

            /* FIXME : Lee la cantidad de bytes del archivo que se selecciono */
            int nb;
            char bytes_read[nbytes];
            uint64_t size_file_input = 0;
            while (!fichero_in.eof()) {
                fichero_in.read(bytes_read, sizeof(bytes_read));
                
                nb = (int)fichero_in.gcount();
                size_file_input += nb;
            }
            fichero_in.close();
            /**/

            uint64_t num_bytes_file_to_input =  size_file_input + 21; // FIXME : numBytesMaxFile
            if ( (offsetDB + num_bytes_file_to_input) <= maxLenArch ) {
                
                write(is_key, sizeof(is_key));
                write(is_delete, sizeof(is_delete));

                uint64_t aux_sfi = size_file_input;
                unsigned char num_aux = 0;
                for ( int i = 0; i < 8; i++ ) {
                    num_aux = aux_sfi | num_aux;
                    is_size[i] = static_cast<char>( num_aux );
                    aux_sfi = aux_sfi >> 8;
                    num_aux = 0;
                }
                write(is_size, sizeof(is_size));

                
                // FIXME : Puede cambiar a que no se cierre el doc, solo el puntero regrese al inicio del archivo
                fichero_in.open(nameFile, std::ios::in |std::ios::binary);
                while (!fichero_in.eof()) {
                    fichero_in.read(bytes_read, sizeof(bytes_read));
                    
                    int nb = (int)fichero_in.gcount();
                    if ( nb == nbytes )
                        write(bytes_read, sizeof(bytes_read));

                    else {
                        char new_array_bytes[nb];
                        write(new_array_bytes, sizeof(new_array_bytes));
                    }
                }
                fichero_in.close();

                write(is_check_sum, sizeof(is_check_sum));
                
                //
                // Create table to user from a new needle
                //
                strcpy (fdb_.keyF, key);
                strcpy (fdb_.deleteF, "F");
                fdb_.offsetF = offsetDB;
                fdb_.sizeF = size_file_input + 21; // FIXME : numBytesMaxFile
                fichero_in.close();
                
            } else {
                needleError(&fdb_);
                std::cout << "Error file (3)\n";
                fichero_in.close();
            }
        }
    }
    
    return fdb_;
}

bool RandomFile::searchFDB ( const FileDB file ) {

    return false;
}

bool RandomFile::seekFDB ( const FileDB file ) {

    if ( (uint64_t)tellg() >= file.offsetF ) {
        seekg(file.offsetF);

        char read_key[8];
        read(read_key, sizeof(read_key));

        if ( strcmp(read_key, file.keyF) == 0 ) {
            if (recoveryFDB ( file ) == false )
                return false;
        } else {
            if (searchFDB(file) == false )
                return false;
        }

    } else 
        return false;

    return true;
}

bool RandomFile::deleteFDB ( FileDB * file ) {
    seekg(file->offsetF + 8); // FIXME : Cambiar este valor de la lectura del key.

    char read_delete[1];
    read(read_delete, sizeof(read_delete));

    if ( strcmp(read_delete, "F") == 0) {
        strcpy(file->deleteF, "V");
        seekg(file->offsetF + 8);
        strcpy(read_delete, "V");
        write(read_delete, sizeof(read_delete));
    } return
        false;

    return true;
}


bool RandomFile::recoveryFDB ( const FileDB file ) {

    // Check if exist the file in the  DB (RandomFile)
    if ( strcmp(file.keyF, "Error") == 0) {
        std::cout << "E(1)\n";
        return false;
    }

    else  if ( strcmp(file.deleteF, "V") == 0 && file.offsetF == 0 && file.sizeF == 0 ) {
        std::cout << "E(2)\n";
        return false;
    }

    else {
        char n_read[nbytes];

        std::ofstream fs(file.keyF);    
    
        uint64_t n_data_read = 0;
        uint64_t n_data = file.sizeF - 21;          // FIXME : No se puede recuperar el valor de numBytesMaxFile
        uint64_t offset_data = file.offsetF + 17;   // FIXME : .numByteskey(8) + .numBytesDelete(1) + numBytesSize(8)

        seekg(offset_data);

        // Get the Size of file in the DB
        while ( n_data_read < n_data ) {
            int aux_data_read = n_data - n_data_read;

            if ( aux_data_read >= nbytes ) {
                read(n_read, sizeof(n_read));
                fs.write(n_read, sizeof(n_read));
                n_data_read+=nbytes;
            } else {
                char aux_[aux_data_read];
                read(aux_, sizeof(aux_));
                fs.write(aux_, sizeof(aux_));
                n_data_read+=sizeof(aux_);
            }
         }
    
        fs.close();
        return true;
    }
}

bool RandomFile::compactDB () {

    /*
    uint64_t length_DB = (uint64_t)tellg();
    if ( (lenArch * 0.75) > length_DB )
        return false;
    else {
    */
        std::cout << "=== Struct in the needle === \n\n";

        unsigned aux_max_file = (uint64_t)tellg();
        unsigned aux_file = 0;

        char read_key[8];
        char read_delete[1];
        char read_size[8];

        seekg(0);
        while ( true ) {            
            
            read(read_key, sizeof(read_key));
            read(read_delete, sizeof(read_delete));
            read(read_size, sizeof(read_size));

            uint64_t n_size = 0;
            unsigned char char_aux;
            for ( int i = 7; i > -1; i-- ) {
                char_aux = read_size[i];
                n_size = n_size | char_aux;
                if ( i > 0 )
                    n_size = n_size << 8;
            }

            std::cout << "key : " << read_key << "\n";
            std::cout << "Delete : " << read_delete << "\n";
            std::cout << "size : " << read_size << ", n_size : " << n_size << "\n\n";

            // Go to next needle
            // FIXME : numBytesMaxFile
            aux_file += n_size + 21;
            if ( aux_file < aux_max_file ) {
                seekg(aux_file);
                
            } else
                break;

        }
    //}
}

//
// Main Principal
//
int main (void) {

    RandomFile file_;
    file_.offsetDB = 0;
    //strcpy(file_.nameDB,"DataBaseExample.bin");

    // Files
    char file0[] = "filesin/ima1.jpg";
    char file1[] = "filesin/ima2.jpg";
    char file2[] = "filesin/ima3.jpg";
    char file3[] = "filesin/song1.mp3";
    char file4[] = "filesin/song2.mp3";
    char file5[] = "filesin/song3.mp3";

    // User Needle
    FileDB * user_needle0 = (FileDB *) malloc(sizeof(FileDB));
    FileDB * user_needle1 = (FileDB *) malloc(sizeof(FileDB));
    FileDB * user_needle2 = (FileDB *) malloc(sizeof(FileDB));
    FileDB * user_needle3 = (FileDB *) malloc(sizeof(FileDB));
    FileDB * user_needle4 = (FileDB *) malloc(sizeof(FileDB));
    FileDB * user_needle5 = (FileDB *) malloc(sizeof(FileDB));

    // Write
    *user_needle0 = file_.saveFDB ( file0, "1.jpg");
    *user_needle1 = file_.saveFDB ( file1, "2.jpg");
    *user_needle2 = file_.saveFDB ( file2, "3.jpg");
    *user_needle3 = file_.saveFDB ( file3, "1.mp3");
    *user_needle4 = file_.saveFDB ( file4, "2.mp3");
    *user_needle5 = file_.saveFDB ( file5, "3.mp3");

    // Compact
    file_.compactDB();

    return 0;
}

//RandomFile file_;
//std::cout << file_.charToint ( "ABC" ) << "\n";
//int a = 223;
//char b = a;

//std::cout << " dig  : " << (int)num_aux << "\n";
//std::cout << " dig  : " <<  static_cast<char>( num_aux ) << "\n";
//std::cout << ( char )a << std::endl;
//std::cout << static_cast<char>( a ) << std::endl;
//std::cout << b << std::endl;


/*
    // Configure the random file ....
    // Read
    file_.recoveryFDB ( *user_needle0 );
    file_.recoveryFDB ( *user_needle1 );
    file_.recoveryFDB ( *user_needle2 );
    file_.recoveryFDB ( *user_needle3 );
    file_.recoveryFDB ( *user_needle4 );
    file_.recoveryFDB ( *user_needle5 );

*/