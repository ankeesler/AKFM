//	akfm.c
//	created Tue Dec 31 12:41:13 EST 2013

#include "akfm.h"

//data types
typedef enum {AK_INT, AK_LONG, AK_FLOAT, AK_DOUBLE, AK_CHAR, AK_STRING} ak_data_t;
int ak_data_toi(ak_data_t t) {return t;}
int ak_data_ctoi(char c) {return (int) c - '0';}

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
//	14234#[AK_STRING]@6$andrew_
//	^^^^^ let's pretend this is the hash value of "name"
//ex: write "age" 21
//	3132#[AK_INT]@1$21_
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
	int n = akfm_aux_hash(key);
	int found = 0;
	int i;
	while (!found && fscanf(akfm->fp, "%d", &i) == 1) {
		if (i == n)
			found = !found;
		else
			akfm_aux_adv(akfm);
	}
	return found;
}

/* akfm_aux_copy */
//copies the file over except for the data at pos
void akfm_aux_copy(ak_file_manager *akfm, fpos_t *pos)
{
	//open new file
	FILE *tempf = fopen("temp.txt", "w");
	
	//copy file except for this entry
	char c;
	int search = 1;
	fpos_t curpos;
	rewind(akfm->fp);
	while ((c = getc(akfm->fp)) != EOF) {
		//see if you are still looking for the repeated key
		if (search)
			fgetpos(tempf, &curpos);
        
		//copy the file unless you are at the same entry
		if (search && curpos == *pos) {
            search = 0;
            akfm_aux_adv(akfm);
        } else {
			fputc(c, tempf);
        }
	}
	
	//remove the old file and change the name of this one
	fflush(tempf);
	remove(akfm->fn);
	rename("temp.txt", akfm->fn);
	
	free(pos);
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
            while ((c = fgetc(akfm->fp)) != EOF && c != '_')
                fputc(c, tempf);
            
            //copy that _
            if (c != EOF)
                fputc('_', tempf);
        }
    }
    
    //remove the old file and change the name of this one
	fflush(tempf);
    fclose(akfm->fp);
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
        reopen(akfm->fn, "r+", akfm->fp);
    }
    fprintf(akfm->fp, "%d#%d@1$%d_", h, AK_INT, value);
	
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
        reopen(akfm->fn, "r+", akfm->fp);
    }
    fprintf(akfm->fp, "%d#%d@1$%c_", h, AK_CHAR, value);
	
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
        reopen(akfm->fn, "r+", akfm->fp);
    }
    fprintf(akfm->fp, "%d#%d@%d$%s_", h, AK_STRING, value);
	
	//flush and close file
	fflush(akfm->fp);
	fclose(akfm->fp);
	
	return 0;		
}


#pragma mark - READING

/* akfm_aux_getv */
//gets the value
//assumes the file pointer is on a $ or @
void *akfm_aux_getv(ak_file_manager *akfm)
{
	//advance file pointer to $ or @
	char c;
	while ((c = fgetc(akfm->fp)) != EOF && (c != '@' && c != '$'))
		;
    
	//get information
	int isArray = c == '@';
	int size = 1;
	ak_data_t type = 0;
	if ((c = fgetc(akfm->fp)) != EOF) {
		type = ak_data_ctoi( c );
		if (isArray) {
			fscanf(akfm->fp, "%d", &size);
		}
	}
	
	//move past the '_' separator
	fseek(akfm->fp, 1, SEEK_CUR);
	
	//makes buffer depending on type
	void *buf;
	size_t s;
	char format[3];
	switch(type) {
		case AK_INT:
			s = sizeof(int);
			strcpy(format,"%d");
			buf = malloc(s * size);
			break;
		case AK_LONG:
			s = sizeof(long);
			strcpy(format,"%ld");
			buf = malloc(s * size);
			break;
		case AK_FLOAT:
			s = sizeof(float);
			strcpy(format, "%f");
			buf = malloc(s * size);
			break;
		case AK_DOUBLE:
			s = sizeof(double);
			strcpy(format, "%lf");
			buf = malloc(s * size);
			break;
		case AK_STRING:
		case AK_CHAR:
			s = sizeof(char);
			strcpy(format, "%c");
			buf = malloc(s * size);
			break;
		default:
			printf("ak_manager_err: cannot read type: %d", (int)type);
	}
	
	//scan data into buffer
	int i;
	for (i = 0; i < size; i ++) {
		fscanf(akfm->fp, format, buf);
		buf += s;
		//move past the ',' separator
		fseek(akfm->fp, 1, SEEK_CUR);
	}
	//reset buffer position
	buf -= (s * size);
	
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
	char *value = (dbuf) ? dbuf : NULL;
	free(dbuf);
	
	return value;
}

#pragma mark - DELETING/ERASING

/* akfm delete */
//deletes the file that akfm is managing
//this will completely erase the file
int akfm_erase(ak_file_manager *akfm)
{
	//check to see file manager is not null
	if (!akfm) return -1;
	
	//remove file
	remove(akfm->fn);
	
	return 0;
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
