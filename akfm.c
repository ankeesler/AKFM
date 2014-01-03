//	akfm.c
//	created Tue Dec 31 12:41:13 EST 2013

#include "akfm.h"

/* HASH FUNCTION */

/* akfm_aux_hash */
//hashes the key to return an unsigned int
int akfm_aux_hash(const char *word)
{
	unsigned int n = 0, i = 0;
	while (*word != '\0') {
		switch (i++ % 3) {
			case 0:
				n += (*word++) - 'A';
				break;
			case 1:
				n *= (*word++) - 'A';
				break;
			case 2:
				n -= (*word++) - 'A';
				break;
		}
	}
	return n;
}

#pragma mark - CREATING

/* akfm_create */
//creates a file manager for the save file with that name
//	if the save file is not created, it creates it
//	if the save file is already created, it uses that one
ak_file_manager *akfm_create(const char *filename)
{
	//file manager
	ak_file_manager *akfm = (ak_file_manager *)malloc(sizeof(ak_file_manager));
	akfm->fp = NULL;
	akfm->fn = NULL;
	
	//create realFilename
#define PATH "./akfm/"
#define ENDING ".txt"
	int pl = strlen(PATH), fl = strlen(filename), el = strlen(ENDING);
	akfm->fn = (char *)malloc(sizeof(char) * (pl + fl + el));
	strcpy(akfm->fn, PATH);
	strcat(akfm->fn, filename);
	strcat(akfm->fn, ENDING);
	
	//create directory if necessary
#define PERMISSION S_IRUSR | S_IWUSR | S_IXUSR
	mkdir(PATH, PERMISSION);
	
	//check to see if the file can be opened
	if ((akfm->fp = fopen(akfm->fn, "a")) == NULL) {
		printf("ak_manager_err: cannot open file: %s", akfm->fn);
		akfm_free(akfm);
		akfm = NULL;
	} else {
		fclose(akfm->fp);
	}
	
	return akfm;
}


#pragma mark - WRITING

//ex: write "name" "andrew"
//	14234#6@1$andrew_
//	^^^^^ let's pretend this is the hash value of "name"
//ex: write "age" 21
//	3132#1@4$21_
//	^^^^^ let's pretend this is the hash value of "age"

/* akfm_aux_checkafm */
//makes sure the file manager is not NULL or the file cannot be opened
int akfm_aux_checkafm(ak_file_manager *akfm, const char *m)
{
	//check to see file manager is not null
	if (!akfm) return AKFM_NFM;
	
	//create file pointer to that file
	if ((akfm->fp = fopen(akfm->fn, m)) == NULL) {
		printf("ak_manager_err: cannot open file: %s", akfm->fn);
		return AKFM_NOF;
	}
	
	return 0;
}

/* akfm_aux_adv */
//advances file pointer to next entry
//assumes the file is already open
void akfm_aux_adv(ak_file_manager *akfm)
{
	char c;
	//advance to next @ or $
	while ((c = fgetc(akfm->fp)) != EOF && c != '_')
        ;
}

/* akfm_aux_find */
//advances the file pointer the the # after hash value of key
//returns:
//	1 if it found the entry
//  0 if it did not find the entry
int akfm_aux_find(ak_file_manager *akfm, const char *key)
{
	int n = akfm_aux_hash(key),
        found = 0,
        i;
	while (!found && fscanf(akfm->fp, "%d", &i) == 1) {
		if (i == n)
            found = !found;
		else
            akfm_aux_adv(akfm);
	}
	return found;
}

/* akfm_aux_spcp */
//copies everything except for the repeated entry
void akfm_aux_spcp(ak_file_manager *akfm, const char *key)
{
    FILE *tempf = fopen("temp.txt", "w");
    
    int h = akfm_aux_hash(key);
    int n;
    char c;
    rewind(akfm->fp);
    while (fscanf(akfm->fp, "%d", &n) == 1) {
        if (n == h) {
            //skip the repeated entry
            akfm_aux_adv(akfm);
        } else {
            //copy the entry
            fprintf(tempf, "%d", n);
            while ((c = fgetc(akfm->fp)) != EOF && c != '_')
                fputc(c, tempf);
            
            //copy that _
            if (c != EOF)
                fputc('_', tempf);
        }
    }
    
    //remove the old file and change the name of this one
    fflush(tempf);
    fclose(tempf);
	remove(akfm->fn);
	rename("temp.txt", akfm->fn);
}

/* akfm_write */
//writes data to the file
//connects the key to the value
//user will retrieve the key with the value
int akfm_write_int(ak_file_manager *akfm, const char *key, int value)
{
	int err;
	if ((err = akfm_aux_checkafm(akfm, "r+")))
		return err;
	
	//see if you need to overwrite entry
	int h = akfm_aux_hash(key);
	if (akfm_aux_find(akfm, key)) {
		akfm_aux_spcp(akfm, key);
        akfm->fp = freopen(akfm->fn, "a", akfm->fp);
    }
    fprintf(akfm->fp, "%d#%d@%lu$%d_", h, 1, sizeof(int), value);
	
	//flush and close file
	fflush(akfm->fp);
	fclose(akfm->fp);
	
	return 0;
}

int akfm_write_char(ak_file_manager *akfm, const char *key, char value)
{
	//check environment
	int err;
	if ((err = akfm_aux_checkafm(akfm, "r+")))
		return err;
	
	//see if you need to overwrite entry
	int h = akfm_aux_hash(key);
	if (akfm_aux_find(akfm, key)) {
		akfm_aux_spcp(akfm, key);
        akfm->fp = freopen(akfm->fn, "a", akfm->fp);
    }
    fprintf(akfm->fp, "%d#%d@%lu$%c_", h, 1, sizeof(char), value);
	
	//flush and close file
	fflush(akfm->fp);
	fclose(akfm->fp);
	
	return 0;
}

int akfm_write_string(ak_file_manager *akfm, const char *key, const char *value)
{
	//check environment
	int err;
	if ((err = akfm_aux_checkafm(akfm, "r+")))
		return err;
    
	//see if you need to overwrite entry
	int h = akfm_aux_hash(key);
	if (akfm_aux_find(akfm, key)) {
		akfm_aux_spcp(akfm, key);
        akfm->fp = freopen(akfm->fn, "a", akfm->fp);
    }
    fprintf(akfm->fp, "%d#%d@%lu$%s_", h, (int)strlen(value), sizeof(char), value);
	
	//flush and close file
	fflush(akfm->fp);
	fclose(akfm->fp);
	
	return 0;		
}


#pragma mark - READING

/* akfm_aux_getv */
//gets the value
//assumes the file pointer is on #
void *akfm_aux_getv(ak_file_manager *akfm)
{
    char c;
    
	//advance file pointer past #
	while ((c = fgetc(akfm->fp)) != EOF && c != '#')
		;
    
	//get number
	int num;
    fscanf(akfm->fp, "%d", &num);
    
    //advance file pointer past @
    while ((c = fgetc(akfm->fp)) != EOF && c != '@')
		;
    
    //get size
    int size;
    fscanf(akfm->fp, "%d", &size);
    
    //advance file pointer past $
    while ((c = fgetc(akfm->fp)) != EOF && c != '$')
		;
	
	//makes buffer depending on type
	void *buf;
	char format[3];
	switch(size) {
		case 1:
			strcpy(format,"%c");
			buf = malloc(num * size);
			break;
		case 4:
			strcpy(format,"%d");
			buf = malloc(num * size);
			break;
		default:
			printf("ak_manager_err: cannot read size: %d", size);
	}
	
	//scan data into buffer
	int i;
	for (i = 0; i < num; i ++) {
		fscanf(akfm->fp, format, buf);
		buf += size;
	}
	//reset buffer position
	buf -= (size * num);
	
	//return buffer
	return buf;
	
}

/* akfm_read */
//reads the data from the file with key
//returns the value
//returns:
//	value, if the key exists
//	0, else
int akfm_read_int(ak_file_manager *akfm, const char *key)
{
	//check environment
	int err;
	if ((err = akfm_aux_checkafm(akfm, "r")))
		return err;
    
	//find the key, if it is there
	int *dbuf = NULL;
	if (akfm_aux_find(akfm, key))
		dbuf = (int *)akfm_aux_getv(akfm);

	//flush and close file
	fflush(akfm->fp);
	fclose(akfm->fp);
	
	//copy value from buffer
	int value = (dbuf) ? *dbuf : 0;
	free(dbuf);
	
	return value;
}

char akfm_read_char(ak_file_manager *akfm, const char *key)
{
	//check environment
	int err;
	if ((err = akfm_aux_checkafm(akfm, "r")))
		return err;
	
	//find the key, if it is there
	char *dbuf = NULL;
	if (akfm_aux_find(akfm, key))
		dbuf = (char *)akfm_aux_getv(akfm);
	
	
	//flush and close file
	fflush(akfm->fp);
	fclose(akfm->fp);
	
	//copy value from buffer
	char value = (dbuf) ? *dbuf : 0;
	free(dbuf);
	
	return value;
}

char *akfm_read_string(ak_file_manager *akfm, const char *key)
{
	//check environment
	int err;
	if ((err = akfm_aux_checkafm(akfm, "r")))
		return NULL;
	
	//find the key, if it is there
	char *dbuf = NULL;
	if (akfm_aux_find(akfm, key))
		dbuf = (char *)akfm_aux_getv(akfm);
	
	//flush and close file
	fflush(akfm->fp);
	fclose(akfm->fp);
	
	//copy value from buffer
	char *value = (dbuf) ? dbuf : 0;
	free(dbuf);
	
	return value;
}

#pragma mark - DELETING/ERASING

/* akfm delete */
//deletes the key, if it exists
//returns:
//  0 if something was deleted
//  1 if nothing was deleted
int akfm_delete(ak_file_manager *akfm, const char *key)
{
    //check environment
	int err;
	if ((err = akfm_aux_checkafm(akfm, "r+")))
		return err;
	
	//see if entry exists
    int rval = 1;
	if (akfm_aux_find(akfm, key)) {
		akfm_aux_spcp(akfm, key);
        rval = 0;
    }
	
	//flush and close file
	fflush(akfm->fp);
	fclose(akfm->fp);

	return rval;
}

/* akfm_erase */
//erases the whole file
//returns:
//  0 if succesfully erased
//  error otherwise
int akfm_erase(ak_file_manager *akfm)
{
	//check to see file manager is not null
	if (!akfm) return AKFM_NFM;
	
	//remove file
	return remove(akfm->fn);
}


/* akfm free */
//this frees up all the pointers that the file manager has
//call this instead of free(akfm)
void akfm_free(ak_file_manager *akfm)
{
	
	if (akfm) {
		if (akfm->fn) {
			//flush and close file
			fflush(akfm->fp);
			fclose(akfm->fp);
			
			//free members of struct
			free(akfm->fn);
		}
		
		//free actual file manager
		free(akfm);
	}
}
