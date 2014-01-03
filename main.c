// main.c
// created Thu Jan 2 17:30:33 EST 2014

#include <stdio.h>
#include <ctype.h>

#include "akfm.h"

int main(int argc, char *argv[])
{
    ak_file_manager *manager = akfm_create("chicken");
    
    akfm_write_int(manager, "eggs", 13);
    
    akfm_free(manager);
    
	return 0;
}
