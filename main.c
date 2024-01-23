#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define BLOCK_SIZE 1024
#define NUM_BLOCKS 65536
#define BLOCK_PER_FILE 1024
#define NUM_FILES 256
#define FIRST_DATA_BLOCK 1001
#define MAX_FILE_SIZE 1048576
#define HIDDEN 0x1
#define READ_ONLY 0x2
#define WHITESPACE " \t\n"     
#define MAX_COMMAND_SIZE 255    // The maximum command-line size
#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports four arguments
#define TODO(cmd) \
    fprintf(stderr, "TODO: implement \033[1;31m\"%s\"\033[0m command at \033[1;33mLine %d\033[0m.\n", cmd, __LINE__) \


//********** FILE STRUCTS ************//

//directory
struct directoryEntry
{
    char filename[64];
    short in_use;
    int32_t inode;
};
struct directoryEntry * directory;

//inode
struct inode 
{
    int32_t blocks[BLOCK_PER_FILE];
    short in_use;
    uint8_t attribute;
    uint32_t file_size;
};
struct inode * inodes;

//********** GLOBAL VARIABLES************//

//data where the entire file system lives
uint8_t data[NUM_BLOCKS][BLOCK_SIZE];

//512 blocks just for free block map
uint8_t * free_blocks;
uint8_t * free_inodes;

FILE *fp;
char image_name[64];
uint8_t image_open;

int32_t findFreeBlock()
{
    int i;
    for (i = 0 ; i < NUM_BLOCKS; i++)
    {
        if (free_blocks[i])
        {
            //return the free block + location of the first data block
            return i + 1001;
        }
    }

    //return -1 if not found
    return -1;
}

int32_t findFreeInode()
{
    int i;
    for (i = 0; i < NUM_FILES; i++)
    {
        if (free_inodes[i])
        {
            return i;
        }
    }

    //return -1 if not found
    return -1;
}

int32_t findFreeInodeBlock(int32_t inode)
{
    int i;
    for (i = 0; i < BLOCK_PER_FILE; i++)
    {
        if (inodes[inode].blocks[i] == -1)
        {
            return i;
        }
    }

    return -1;
}

void init()
{
    //directory starts at 0 and goes to 18
    directory = (struct directoryEntry*)&data[0][0];

    //inodes start at 20 and go to 276
    inodes = (struct inode*) &data[20][0];
    free_blocks = (uint8_t*) &data[1000][0];
    free_inodes = (uint8_t*) &data[19][0];

    memset(image_name, 0, 64);

    //image open is false
    image_open = 0;

    //initialize everything to empty
    int i ;
    for (i = 0; i < NUM_FILES; i++)
    {
        directory[i].in_use = 0;
        directory[i].inode = -1;
        free_inodes[i] = 1;

        memset(directory[i].filename, 0, 64);
        
        //everyblock in the file
        int j;
        for (j = 0; j < NUM_BLOCKS; j++)
        {
            inodes[i].blocks[j] = -1;
            inodes[i].in_use = 0;
            inodes[i].attribute = 0;
            inodes[i].file_size = 0;
        }
    }
    int j;
    for (j = 0; j < NUM_BLOCKS; j++)
    {
        free_blocks[j] = 1;
    }

}

uint32_t df()
{
    //only works when its opened
    int j;
    int count = 0;
    for (j = FIRST_DATA_BLOCK; j < NUM_BLOCKS; j++)
    {
        if (free_blocks[j])
        {
            count++;
        }
    }
    return count * BLOCK_SIZE;
}

void createfs(char * filename)
{
    fp = fopen(filename, "w"); 

    strncpy(image_name, filename, strlen(filename));

    if (fp == NULL)
    {
        printf("Error creating file system\n");
        exit(1);
    }
    
    memset(data, 0, NUM_BLOCKS * BLOCK_SIZE);

    //imag open is true now
    image_open = 1;

    //initialize everything to empty
    int i ;
    for (i = 0; i < NUM_FILES; i++)
    {
        directory[i].in_use = 0;
        directory[i].inode = -1;
        free_inodes[i] = 1;

        memset(directory[i].filename, 0, 64);
        
        //everyblock in the file
        int j;
        for (j = 0; j < NUM_BLOCKS; j++)
        {
            inodes[i].blocks[j] = -1;
            inodes[i].in_use = 0;
            inodes[i].attribute = 0;
            inodes[i].file_size = 0;
        }
    }
    int j;
    for (j = 0; j < NUM_BLOCKS; j++)
    {
        free_blocks[j] = 1;
    }
    fclose(fp);

}

void savefs()
{
    //if can't open the image then error
    if (image_open == 0)
    {
        printf("ERROR: Disk Image is not open\n");
    }

    fp = fopen(image_name, "w");

    if (fp == NULL)
    {
        printf("Error creating file system (from savefs)\n");
        exit(1);
    }
    
    fwrite(&data[0][0], BLOCK_SIZE, NUM_BLOCKS, fp);

    // 0 it back out bc we are not using it anymore
    memset(image_name, 0, 64);

    fclose(fp);

}

void openfs(char * filename)
{
    fp = fopen(filename, "r"); 

    if (fp == NULL)
    {
        printf("openfs ERROR: Can not create file system.\n");
        return;
    }

    strncpy(image_name, filename, strlen(filename));

    //we read from the file into our data structure
    fread(&data[0][0], BLOCK_SIZE, NUM_BLOCKS, fp);
    
    image_open = 1;

    fclose(fp);

}

void delete(char * filename)
{
    int i;
    for (i = 0; i < NUM_FILES; i++)
    {
        if (strcmp(directory[i].filename, filename) == 0 && directory[i].in_use == 1)
        {

            // check if the file is read-only
            if (inodes[directory[i].inode].attribute & READ_ONLY)
            {
                printf("ERROR: File is read-only\n");
                return;
            }

            // get the inode number for the file
            int inode_num = directory[i].inode;
            

            // set directory in_use to false
            directory[i].in_use = 0;
            directory[i].inode = -1;
            
            // set inode in_use to false
            free_inodes[inode_num] = 1;
            inodes[inode_num].in_use = 0;
            
            // set the blocks as free | don't zero them out | you keep them but set them to free 
            int j;
            for (j = 0; j < BLOCK_PER_FILE; j++)
            {
                if (inodes[inode_num].blocks[j] != -1)
                {
                    free_blocks[inodes[inode_num].blocks[j] - 1001] = 1;
                    inodes[inode_num].blocks[j] = -1;
                }
            }
            printf("File %s deleted.\n", filename);
            return;
        }
    }
    printf("File %s not found.\n", filename);
}

void undelete(char *filename) {
    // Search the directory for the specified file
    int i;
    for (i = 0; i < NUM_FILES; i++) {
        if (strcmp(directory[i].filename, filename) == 0) {
            break;
        }
    }
    if (i == NUM_FILES) {
        // File not found in the directory
        printf("Undelete: Can not find the file.\n");
        return;
    }

    // File found, get the inode
    int inode_num = directory[i].inode;
    struct inode *inode = &inodes[inode_num];

    if (inode->in_use) {
        // File is not deleted
        printf("Undelete: file '%s' is not deleted\n", filename);
        return;
    }

    // Mark the inode and blocks as used
    inode->in_use = 1;
    int j;
    for (j = 0; j < BLOCK_PER_FILE; j++) {
        int block_num = inode->blocks[j];
        if (block_num != -1) {
            free_blocks[block_num - FIRST_DATA_BLOCK] = 0;
        }
    }

    // Mark the directory entry as used
    directory[i].in_use = 1;

    printf("Undelete: file '%s' has been restored\n", filename);
}

void closefs()
{
    //make sure the image is open
    if (image_open == 0)
    {
        printf("ERROR: Disk Image is not open\n");
        return;
    }
    fclose(fp);

    image_open = 0;
    memset(image_name, 0, 64);
}

void list()
{
    //loop over the directory and if its in use then it'll print it
    int i;
    int not_found = 1;

    //if there are no file in the directory print list:no files found
    for (i = 0; i < NUM_FILES; i++)
    {
        //TODO\ Add a check to not list if the file is hidden.
        
        if (directory[i].in_use) 
        {
            // it is not printing any files in the directory if one of them is hidden - part of "attrib"
            // if ((inodes[directory[i].inode].attribute & HIDDEN))
            // {
            //     continue;
            // }
            
            not_found = 0;
            
            //null terminate the filename
            char filename[65];
            memset(filename, 0, 65);

            //copy filename from the directory
            strncpy(filename, directory[i].filename, strlen(directory[i].filename));
            printf("%s\n", filename);
            
        }
    }

    //if there are no files in the directory - part of requirement
    if (not_found)
    {
        printf("list ERROR: No files found\n");
    }
}

// The attrib command sets or removes an attribute from the file.
void attrib(char * file_name, char * attribute)
{
    // Search the directory for the specified file
    int i;
    for (i = 0; i < NUM_FILES; i++) {
        if (strcmp(directory[i].filename, file_name) == 0) {
            break;
        }
    }
    if (i == NUM_FILES) {
        // File not found in the directory
        printf("attrib: Can not find the file.\n");
        return;
    }

    // File found, get the inode
    int inode_num = directory[i].inode;
    struct inode *inode = &inodes[inode_num];

    // Set or remove the attribute
    if (strcmp(attribute, "+h") == 0) {
        inode->attribute |= HIDDEN; // it sets the hidden bit
    } else if (strcmp(attribute, "-h") == 0) {
        inode->attribute &= ~HIDDEN; // it clears the hidden bit
    } 
    else if (strcmp(attribute, "+r") == 0) {
        inode->attribute |= READ_ONLY; // it sets the read only bit
    } else if (strcmp(attribute, "-r") == 0) {
        inode->attribute &= ~READ_ONLY; // it clears the read only bit
    }
    else {
        printf("attrib: Invalid attribute '%s'\n", attribute);
        return;
    }

    printf("attrib: Attributes updated successfully\n");
}


void insert(char * filename)
{
    //verify that the filename isn't null
    if (filename == NULL)
    {
        printf("ERROR: Filename is NULL.\n");
        return;
    }

    //verify that the file exists
    struct stat buf;
    int ret = stat(filename, &buf);

    if (ret == -1)
    {
        printf("ERROR: File does not exist.\n");
        return;
    }


    //verify the file isn't too big
    if (buf.st_size > MAX_FILE_SIZE)
    {
        printf("ERROR: File is too large.\n");
        return;
    }

    //verify that there is enough space
    if(buf.st_size > df())
    {
        printf("ERROR: Not enough free disk space\n");
        return;
    }

    //find an empty directory entry
    int i;
    int directory_entry = -1;
    for (i = 0; i < NUM_FILES; i++)
    {
        if (directory[i].in_use == 0)
        {
            directory_entry = i;
            break;
        }
    }

    if (directory_entry == -1)
    {
        printf("ERROR: Could not find a free directory entry\n");
        return;
    }

    // find free indoes and place the file in there

    // Open the input file read-only 
    FILE *ifp = fopen ( filename, "r" ); 
    printf("Reading %d bytes from %s\n", (int) buf . st_size,filename );
 
    // Save off the size of the input file since we'll use it in a couple of places and 
    // also initialize our index variables to zero. 
    int32_t copy_size   = buf . st_size;

    // We want to copy and write in chunks of BLOCK_SIZE. So to do this 
    // we are going to use fseek to move along our file stream in chunks of BLOCK_SIZE.
    // We will copy bytes, increment our file pointer by BLOCK_SIZE and repeat.
    int32_t offset      = 0;               

    // We are going to copy and store our file in BLOCK_SIZE chunks instead of one big 
    // memory pool. Why? We are simulating the way the file system stores file data in
    // blocks of space on the disk. block_index will keep us pointing to the area of
    // the area that we will read from or write to.
    int32_t block_index = -1;
    

    //find a free inode
    int32_t inode_index = findFreeInode();
    if (inode_index == -1)
    {
    printf("ERROR: Can not find a free inode.\n");
    return;
    }

    
    //place the  file into the directory
    directory[directory_entry].in_use = 1;
    directory[directory_entry].inode= inode_index;
    strncpy(directory[directory_entry].filename, filename, strlen(filename));

    // copy_size is initialized to the size of the input file so each loop iteration we
    // will copy BLOCK_SIZE bytes from the file then reduce our copy_size counter by
    // BLOCK_SIZE number of bytes. When copy_size is less than or equal to zero we know
    // we have copied all the data from the input file.
    while( copy_size > 0 )
    {

      // Index into the input file by offset number of bytes.  Initially offset is set to
      // zero so we copy BLOCK_SIZE number of bytes from the front of the file.  We 
      // then increase the offset by BLOCK_SIZE and continue the process.  This will
      // make us copy from offsets 0, BLOCK_SIZE, 2*BLOCK_SIZE, 3*BLOCK_SIZE, etc.


      //moves your fp along anywhere inside the file
      //hey this fp move this many bytes into the file
      fseek( ifp, offset, SEEK_SET );
 
      // Read BLOCK_SIZE number of bytes from the input file and store them in our
      // data array. 


      //find a free block
      block_index = findFreeBlock();

      if (block_index == -1)
      {
        printf("ERROR: Can not find a free block.\n");
        return;
      }

      inodes[inode_index].file_size = buf.st_size;
      int32_t bytes  = fread( data[block_index], BLOCK_SIZE, 1, ifp );

      //save the block in the inode
      int32_t inode_block = findFreeInodeBlock(inode_index);
      inodes[inode_index].blocks[inode_block] = block_index;

      // If bytes == 0 and we haven't reached the end of the file then something is 
      // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
      // It means we've reached the end of our input file.
      if( bytes == 0 && !feof( ifp ) )
      {
        printf("An error occured reading from the input file.\n");
        return ;
      }

      // Clear the EOF file flag.
      clearerr( ifp );

      // Reduce copy_size by the BLOCK_SIZE bytes.
      copy_size -= BLOCK_SIZE;
      
      // Increase the offset into our input file by BLOCK_SIZE.  This will allow
      // the fseek at the top of the loop to position us to the correct spot.
      offset    += BLOCK_SIZE;

      // Increment the index into the block array 
      // DO NOT just increment block index in your file system
      block_index = findFreeBlock();
    }

    // We are done copying from the input file so close it out.
    fclose( ifp );
}

// ENCRYPT FILE
//the encrypt function takes a filename and a 1 byte cipher key and performs an xor encryption
//on the file, writing the encrypted file back to the disk

void xor_encrypt(char *filename, char * key)
{
    //reading binary bc we need to go byte by byte
    fp = fopen(filename, "rb");

    //if the file does not exist
    if (fp == NULL)
    {
        printf("ERROR: File does not exist.\n");
        return;
    }

    //output file - original name with .enc appended
    char *output_filename = (char*) malloc( strlen(filename) + 5 );
    strcpy(output_filename, filename);
    strcat(output_filename, ".enc");

    //open the output file
    FILE *ofp = fopen(output_filename, "wb");

    //if the file does not exist
    if (ofp == NULL)
    {
        printf("ERROR: File does not exist.\n");
        return;
    }

    //get the length of the key
    size_t keylen = strlen(key);
    size_t keyidx = 0;

    //buffer to hold the bytes read from the file
    char buffer[1];

    //number of bytes read from the file
    int bytes_read;

    while (!feof(fp))
    {
        bytes_read = fread(&buffer, 1, 1, fp);

        if (bytes_read > 0)
        {
            buffer[0] = buffer[0] ^ key[keyidx];
            keyidx = (keyidx + 1) % keylen;
            fwrite(buffer, 1, bytes_read, ofp);
        }
    }

    printf("%s File encrypted successfully.\n", filename);

    //close the files
    fclose(fp);
    fclose(ofp);
}


void xor_decrypt(char *filename, char * key)
{
    //reading binary bc we need to go byte by byte
    fp = fopen(filename, "rb");

    //if the file does not exist
    if (fp == NULL)
    {
        printf("ERROR: File does not exist.\n");
        return;
    }

    //output file - original name with .enc removed
    char *output_filename = (char*) malloc( strlen(filename) - 4 );
    strncpy(output_filename, filename, strlen(filename) - 4);

    //open the output file
    FILE *ofp = fopen(output_filename, "wb");

    //if the file does not exist
    if (ofp == NULL)
    {
        printf("ERROR: File does not exist.\n");
        return;
    }

    //get the length of the key
    int keylen = strlen(key);
    int keyidx = 0;

    //buffer to hold the bytes read from the file
    char buffer[1];

    //number of bytes read from the file
    int bytes_read;

    while (!feof(fp))
    {
        bytes_read = fread(&buffer, 1, 1, fp);

        if (bytes_read > 0)
        {
            buffer[0] = buffer[0] ^ key[keyidx];
            keyidx = (keyidx + 1) % keylen;
            fwrite(buffer, 1, bytes_read, ofp);
        }
    }

    printf("%s File decrypted successfully.\n", filename);

    //close the files
    fclose(fp);
    fclose(ofp);
}
//read function to implement the read command
void read_file(char* filename, int start_byte, int num_bytes)
{
    FILE* fileFP = fopen(filename, "r");
    if(fileFP ==NULL)
    {
        printf("Error: File cannot be opened. \n");
        return;
    }

    fseek(fileFP, start_byte, SEEK_SET);
    
    for(int i=1; i<num_bytes; i++)
    {
        unsigned char byte;
        if(fread(&byte, sizeof(unsigned char), 1, fileFP)!= 1)
        {
            printf("ERROR: File cannot be opened. \n");
            fclose(fileFP);
            return;
        }
        printf("%02X ", byte);
    }

    fclose(fileFP);
}

int main()
{
    fp = NULL;
    init();

    char * command_string = (char*) malloc( MAX_COMMAND_SIZE );

    while( 1 )
    {
        // Print out the msh prompt
        printf ("mfs> ");

        // Read the command from the commandline.
        while( !fgets (command_string, MAX_COMMAND_SIZE, stdin) );

        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];

        for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
        {
        token[i] = NULL;
        }

        int token_count = 0;                                 
                                                           
        // Pointer to point to the token
        // parsed by strsep
        char *argument_ptr = NULL;                                         
                                                           
        char *working_string  = strdup( command_string );                

        // we are going to move the working_string pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *head_ptr = working_string;

        // Tokenize the input strings with whitespace used as the delimiter
        while ( ( (argument_ptr = strsep(&working_string, WHITESPACE ) ) != NULL) && 
                (token_count<MAX_NUM_ARGUMENTS))
        {
        token[token_count] = strndup( argument_ptr, MAX_COMMAND_SIZE );
        if( strlen( token[token_count] ) == 0 )
        {
            token[token_count] = NULL;
        }
            token_count++;
        }

        /*
        *  COMMANDS SECTION 
        */

        // Check if the user entered a command
        if (token[0] != NULL) {

            if (!strcmp(token[0], "quit") || !strcmp(token[0], "exit"))
            {
                exit(0);
            }
            else if (!strcmp(token[0], "insert"))
            {
                if (!image_open)
                {
                    printf("ERROR: Disk Image is not opened.\n");
                    continue;
                }
                if (token[1] == NULL)
                {
                    printf("ERROR: No filename specified\n");
                    continue;
                }

                //if the strlen is longer than 64 bytes (max filename length)
                if (strlen(token[1]) > 64)
                {
                    printf("insert ERROR: Filename too long\n");
                    continue;
                }

                insert(token[1]);
            }
            else if (!strcmp(token[0], "retrieve"))
            {
                TODO("retrieve");
            }
            else if (!strcmp(token[0], "read"))
            {
                //implementing the read section
                if(token_count < 5)
                {
                    printf("ERROR: Invalid number of arguments\n"); 
                    continue;
                }

                char* filename = token[1];
                int start_byte= atoi(token[2]);
                int num_bytes = atoi(token[3]);
                read_file(filename, start_byte, num_bytes);
            }
            else if (!strcmp(token[0], "delete"))
            {
                delete(token[1]);
            }
            else if (!strcmp(token[0], "undel"))
            {
                undelete(token[1]);
            }
            else if (!strcmp(token[0], "list"))
            {
                if (!image_open)
                {
                    printf("ERROR: Disk Image is not open\n");
                    continue;
                }
                list();
            }
            else if (!strcmp(token[0], "df"))
            {
                if (!image_open)
                {
                    printf("ERROR: Disk Image is not open\n");
                    continue;
                }

                printf("%d bytes free\n", df());
            }
            else if (!strcmp(token[0], "openfs"))
            {
                if (token[1] == NULL)
                {
                    printf("ERROR: No file name specified\n");
                    continue;
                }
                openfs(token[1]);
            }
            else if (!strcmp(token[0], "closefs"))
            {
                closefs();
            }
            else if (!strcmp(token[0], "createfs"))
            {
                if (token[1] == NULL)
                {
                    printf("ERROR: No file name specified\n");
                    continue;
                }
                createfs(token[1]);
            }
            else if (!strcmp(token[0], "savefs"))
            {
                savefs();
            }
            else if (!strcmp(token[0], "attrib"))
            {
                // Check if the user entered a filename
                if (token[1] == NULL)
                {
                    printf("ERROR: No filename specified\n");
                    continue;
                }
                // check if the user entered a valid attribute
                else if (token[2] == NULL)
                {
                    printf("ERROR: No attribute specified\n");
                    continue;
                }
                else
                {
                    attrib(token[1], token[2]);
                }

            }
            else if (!strcmp(token[0], "encrypt"))
            {
                //encrpyt filename cipher key
                if (token[1] == NULL)
                {
                    printf("ERROR: No filename specified\n");
                    continue;
                }
                //make sure the cipher value is one byte
                if (token[2] == NULL)
                {
                    printf("ERROR: No cipher specified\n");
                    continue;
                }
                xor_encrypt(token[1], token[2]);
            }
            else if (!strcmp(token[0], "decrypt"))
            {
                if (token[1] == NULL)
                {
                    printf("ERROR: No filename specified\n");
                    continue;
                }
                //make sure the cipher value is one byte
                if (token[2] == NULL)
                {
                    printf("ERROR: No cipher specified\n");
                    continue;
                }
                xor_decrypt(token[1], token[2]);
            }
            else 
            {
                printf("Command not found\n");
            }
        }
   

      // Now print the tokenized input as a debug check
      // \TODO Remove this for loop and replace with your shell functionality

      //int token_index  = 0;
      //for( token_index = 0; token_index < token_count; token_index ++ ) 
      //{
        //printf("token[%d] = %s\n", token_index, token[token_index] );  
      //}


      // Cleanup allocated memory
      for( int i = 0; i < MAX_NUM_ARGUMENTS; i++ )
      {
        if( token[i] != NULL )
        {
          free( token[i] );
        }
      }

      free( head_ptr );

    }

    free( command_string );

    return 0;
}
