//	akfm.h
//	created Tue Dec 31 12:41:13 EST 2013

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

//errors
#define AKFM_NFM -1 //the file manager is NULL
#define AKFM_NOF -2 //can't open the file

typedef struct {
	FILE *fp;
	char *fn;
} ak_file_manager;

/* akfm_create */
//creates a file manager for the save file with that name
//	if the save file is not created, it creates it
//	if the save file is already created, it uses that one
ak_file_manager *akfm_create(const char *filename);


/* akfm_write */
//writes data to the file
//connects the key to the value
//user will retrieve the key with the value
//returns:
//	0 on success
//	AKFM_NFM if file manager is NULL
//	AKFM_NOF if can't open file
int akfm_write_int(ak_file_manager *akfm,
                   const char *key,
                   int value);
int akfm_write_char(ak_file_manager *akfm,
                    const char *key,
                    char value);

int akfm_write_string(ak_file_manager *akfm,
                      const char *key, 
                      const char *value);				



/* akfm_read */
//reads the data from the file with key
//returns the value
//returns:
//	value if it is in the file
//	0 if it is not in the file
int akfm_read_int(ak_file_manager *akfm, const char *key);
char akfm_read_char(ak_file_manager *akfm, const char *key);
char *akfm_read_string(ak_file_manager *akfm, const char *key);

/* akfm delete */
//deletes the file that akfm is managing
//this will completely erase the file
//returns:
//	0 on success
//	-1 if file manager is NULL
int akfm_erase(ak_file_manager *akfm);


/* akfm free */
//this frees up all the pointers that the file manager has
//call this instead of free(akfm)
void akfm_free(ak_file_manager *akfm);