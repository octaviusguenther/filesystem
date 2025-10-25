// Copyright (c) 2023 Octavius Guenther
// All rights reserved.

#include "../lib/operations.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//final version 14.07.23, 14:00



enum bool{
	false = 0,
	true = 1
};

// calculates how much tokens a path has
int 
token_counter(char *path){
	char* token;
	char path_cp[90];
	strcpy(path_cp,path);

	int counter = 0;

	token = strtok(path_cp,"/");

    while (token != NULL) {
        

        
        counter++;
       
        
        token = strtok(NULL,"/");
        
    }
	return counter;
}


int
check_valid_path(char *path){

	// falls das erste zeichen nicht "/" ist, dann game over
	if (path[0] != '/' ) {
		
		return -1;
	}
	
	return 0;
}

int 
check_valid_file_path(file_system* fs, char* filename, inode** curr_inode, int* curr_inode_id, int* parent_id){
	int size = fs -> s_block -> num_blocks;
	int token_amount = token_counter(filename);
	int token_number = 1;

	char* folder = strtok(filename,"/");
	

	//Effekt: man kriegt den inode der zum file namen im path gehoert
	//Output: bool wert der sagt ob file path valide ist oder nicht
	while (folder != NULL)
	{

		// ueberpruefe den parent node nach dem namen in den inodes vom direct block

		int is_in = false;

		for (int i = 0; i < size; i++){
			*curr_inode = &(fs -> inodes[i]);
			*curr_inode_id = i;
			
			//der folder ist schon drin
			if ( !strcmp((*curr_inode) -> name,folder) && 
				(*curr_inode) -> n_type == directory &&
				(*curr_inode) -> parent == *parent_id ) {
				
				is_in = true;
				// man befindet sich in der mitte vom path
				if (token_number != token_amount) {
					is_in = true;
					token_number++;
					*parent_id = *curr_inode_id;
					folder = strtok(NULL,"/");
					
				}

			
			} // die file ist drin
			 else if (!strcmp((*curr_inode) -> name,folder) && 
				(*curr_inode) -> n_type == reg_file &&
				(*curr_inode) -> parent == *parent_id){
				
				is_in = true;

				break;

			} 
			// in der iteration wurde nichts gefunden
			else {
				is_in = false;
			}
		
		} 
		//if (token_number != token_amount && is_in == false) {
		if ( is_in == false) {
			printf("folder or file %s doesn't exist\n", folder);
			return -1;
		} 
		if ( token_number == token_amount) {
			
			break;
		}


	}

	return 0;
}


int
check_valid_file_or_folder_path(file_system* fs, char* filename, inode** curr_inode, int* curr_inode_id, int* parent_id){
	int size = fs -> s_block -> num_blocks;
	int token_amount = token_counter(filename);
	int token_number = 1;

	char* folder = strtok(filename,"/");
	

	//Effekt: man kriegt den inode der zum file namen im path gehoert
	//Output: bool wert der sagt ob file path valide ist oder nicht

	if ( folder == NULL){
		*parent_id = -1;
		folder = "/";
		token_amount = 1;
	}
	while (folder != NULL)
	{

		// ueberpruefe den parent node nach dem namen in den inodes vom direct block

		int is_in = false;

		for (int i = 0; i < size; i++){
			*curr_inode = &(fs -> inodes[i]);
			*curr_inode_id = i;
			
			//der folder ist schon drin
			if ( !strcmp((*curr_inode) -> name,folder) && 
				(*curr_inode) -> n_type == directory &&
				(*curr_inode) -> parent == *parent_id ) {
				
				is_in = true;
				// man befindet sich in der mitte vom path
				if (token_number != token_amount) {
					is_in = true;
					token_number++;
					*parent_id = *curr_inode_id;
					folder = strtok(NULL,"/");
					
				} else {
					break;
				}

			
			} // die file ist drin
			 else if (!strcmp((*curr_inode) -> name,folder) && 
				(*curr_inode) -> n_type == reg_file &&
				(*curr_inode) -> parent == *parent_id){
				
				is_in = true;

				break;

			} 
			// in der iteration wurde nichts gefunden
			else {
				is_in = false;
			}
		
		} 
		if (token_number != token_amount && is_in == false) {
		
			printf("folder or file %s doesn't exist\n", folder);
			return -1;
		} 
		if ( token_number == token_amount) {
			
			break;
		}


	}

	return 0;	
}


//finds the inode index by name
int count_chars(char* path,int size){
    int counter = 0;
    

    for (int i = 0; i < size ; i++){
        if ( path[i] != '\000') {
            counter++;
        }
    }

    return counter;
}



void del_inode(file_system *fs,inode** curr_inode, int *curr_inode_id){ 
	//1. if inode contains a file, just delete all the datablocks

	int curr_data_block_id;
	data_block* curr_data_block;
	if ( (*curr_inode)->n_type == reg_file){
		
		// deleting all the data blocks of the inode
		for ( int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
			
			curr_data_block_id = (*curr_inode) -> direct_blocks[i];
			if (curr_data_block_id != -1) {
				curr_data_block = &(fs ->data_blocks[curr_data_block_id]);

				(*curr_inode) -> direct_blocks[i] = -1;
				//free(curr_data_block);

				//curr_data_block = malloc(sizeof(data_block));
				
				memset(curr_data_block -> block, 0, BLOCK_SIZE);
				curr_data_block -> size = 0;

				//update free list
				fs -> free_list[curr_data_block_id] = 1;


			}
		}


	/* falls inode ein directory ist,
	greife auf directblocks zu und wende del inode auf die kinder an
	*/
	} else if ((*curr_inode) -> n_type == directory ) { 


		inode* child_inode;
		int child_inode_id;
		for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
			if ((*curr_inode) -> direct_blocks[i] != -1){
				child_inode_id = (*curr_inode) -> direct_blocks[i];
				child_inode = &(fs->inodes[child_inode_id]);

				del_inode(fs, &child_inode, &child_inode_id); 
			}
		}
	}
	// reset the parent's directblock
	int parent_id = (*curr_inode) -> parent;
	inode* parent_node = &(fs -> inodes[parent_id]);

	for ( int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
		if (parent_node ->direct_blocks[i] == *curr_inode_id){
			parent_node -> direct_blocks[i] = -1;
			break;
		}
	}
		

	// delete the inode
	(*curr_inode) -> n_type = free_block;
	(*curr_inode) -> size = 0;

		
	(*curr_inode) -> parent = -1;
	memset((*curr_inode) -> name, 0, NAME_MAX_LENGTH);
	memset((*curr_inode) -> direct_blocks, -1, DIRECT_BLOCKS_COUNT);


}
int
fs_mkdir(file_system *fs, char *path)
{	

	if ( path == NULL) {
	return -1;
	}

	if (check_valid_path(path) == -1){
		return -1;
	}
	// angenommen absoluter path sieht so aus "/new_ordner"
	
	// token_count wird benutzt um zu checken ob man am letzten token angekommen ist
	int size = fs -> s_block -> num_blocks;
	int token_amount = token_counter(path);
	int token_number = 1;

	char* folder = strtok(path,"/");

	int parent_id = 0;
	int curr_inode_id;
	int new_inode_id;

	while (folder != NULL)
	{

		// ueberpruefe den parent node nach dem namen in den inodes vom direct block

		int is_in = false;

		for (int i = 0; i < size; i++){
			inode* curr_inode = &(fs -> inodes[i]);
			curr_inode_id = i;
			
			
			if ( !strcmp(curr_inode -> name,folder) && 
				curr_inode -> n_type == directory &&
				curr_inode -> parent == parent_id ) {
				
				is_in = true;
				if (token_number != token_amount) {
					is_in = true;
					token_number++;
					parent_id = curr_inode_id;
					folder = strtok(NULL,"/");
					
				} else {
					printf("folder %s already exists \n", folder);
					return -1;

				}
			
			}else {
				is_in = false;
			}
		
		} 

		if (token_number != token_amount && is_in == false) {
			printf("folder %s doesn't exist\n", folder);
			return -1;
		} 
		if ( token_number == token_amount) {
			
			break;
		}


	}

	// falls weder der folder bereits vorhanden noch der path fehlerhaft ist, lege den inode an
	inode* curr;
		
	// suche kleinst moeglichen freien Inode
	// inode id wird gebraucht fuer direct_blocks zuweisung im parent node
	for ( int i = 0; i < size; i++){
			curr = &(fs -> inodes[i]);
			if ( curr -> n_type == free_block) {
				new_inode_id = i;
				break;
			}
		}

	// ntype wird auf directory gesetzt
	curr -> n_type = directory;

	// path wird in name kopiert
	strncpy(curr->name,folder,NAME_MAX_LENGTH);

	//parent node wurde in for schleife gesetzt
	curr -> parent = parent_id;
		
	// node index wird im parent node im direct_blocks arr im kleinsten eintrag gespeichert
	// speichere root node
	// inodes[0] = root node
	inode* parent = &(fs -> inodes[parent_id]);

	// iteriere direct_blocks array vom parent node
	for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
		if ( parent -> direct_blocks[i] == -1){
			// setze die inode_de vom neu angelegten node im direct_blocks array vom parent node

			parent -> direct_blocks[i] = new_inode_id;
			break;
		}
	}


	// folder is der naechste folder name
	printf("folder %s successfully created\n",folder);

	//increase tk_counter
	return 0;

}

int
fs_mkfile(file_system *fs, char *path_and_name)
{
	
	if ( path_and_name == NULL) {
	return -1;
	}

	if (check_valid_path(path_and_name) == -1){
		return -1;
	}
	// angenommen absoluter path sieht so aus "/new_ordner"
	
	// token_count wird benutzt um zu checken ob man am letzten token angekommen ist
	int size = fs -> s_block -> num_blocks;
	int token_amount = token_counter(path_and_name);
	int token_number = 1;

	char* folder = strtok(path_and_name,"/");

	int parent_id = 0;
	int curr_inode_id;
	int new_inode_id;

	while (folder != NULL)
	{

		// ueberpruefe den parent node nach dem namen in den inodes vom direct block

		int is_in = false;

		for (int i = 0; i < size; i++){
			inode* curr_inode = &(fs -> inodes[i]);
			curr_inode_id = i;
			
			//der folder ist schon drin
			if ( !strcmp(curr_inode -> name,folder) && 
				curr_inode -> n_type == directory &&
				curr_inode -> parent == parent_id ) {
				
				is_in = true;
				if (token_number != token_amount) {
					is_in = true;
					token_number++;
					parent_id = curr_inode_id;
					folder = strtok(NULL,"/");
					
				}

			
			}
			 else if (!strcmp(curr_inode -> name,folder) && 
				curr_inode -> n_type == reg_file &&
				curr_inode -> parent == parent_id){

					printf("file %s already exists \n", folder);
					return -1;
			} 
			
			else {
				is_in = false;
			}
		
		} 

		if (token_number != token_amount && is_in == false) {
			printf("folder %s doesn't exist\n", folder);
			return -1;
		} 
		if ( token_number == token_amount) {
			
			break;
		}


	}

	// falls weder der folder bereits vorhanden noch der path fehlerhaft ist, lege den inode an
	inode* curr;
		
	// suche kleinst moeglichen freien Inode
	// inode id wird gebraucht fuer direct_blocks zuweisung im parent node
	for ( int i = 0; i < size; i++){
			curr = &(fs -> inodes[i]);
			if ( curr -> n_type == free_block) {
				new_inode_id = i;
				break;
			}
		}

	// ntype wird auf reg_file gesetzt
	curr -> n_type = reg_file;

	// path wird in name kopiert
	strncpy(curr->name,folder,NAME_MAX_LENGTH);

	//parent node wurde in for schleife gesetzt
	curr -> parent = parent_id;
		
	// node index wird im parent node im direct_blocks arr im kleinsten eintrag gespeichert
	// speichere root node
	// inodes[0] = root node
	inode* parent = &(fs -> inodes[parent_id]);

	// iteriere direct_blocks array vom parent node
	for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
		if ( parent -> direct_blocks[i] == -1){
			// setze die inode_de vom neu angelegten node im direct_blocks array vom parent node

			parent -> direct_blocks[i] = new_inode_id;
			break;
		}
	}


	// folder is der naechste folder name
	printf("file %s successfully created\n",folder);

	//increase tk_counter
	return 0;
	
}

char *
fs_list(file_system *fs, char *path)
{

	if (path == NULL) {
		return NULL;
	}


	// path abarbeiten

	int size = fs -> s_block -> num_blocks;

	char* string = malloc(90);
	*string = 0;
	int token_amount = token_counter(path);
	char* folder = strtok(path,"/");

	int tk_number = 1;
	
	int parent_id = 0;

	inode* curr_inode;
	int curr_inode_id;

	// Fall wir sind im  root verzeichnis
	if ( folder == NULL) {
		parent_id = -1;
		folder = "/";
	} else {

		int is_in = false;
		while (folder != NULL){
		
		//
			for (int i = 0; i < size; i++){
				curr_inode = &(fs -> inodes[i]);
				curr_inode_id = i;

				//der name ist gleich
				if (!strcmp(curr_inode -> name, folder) &&
					curr_inode -> n_type == directory &&
					curr_inode -> parent == parent_id ) {
					
					is_in = true;
					// es ist der letzte order im path
					if ( tk_number == token_amount) {
						// unterbreche die schleife
						break;
					} else {
						parent_id = curr_inode_id;
						tk_number++;
						folder = strtok(NULL,"/");
						break;
					}

				}

			}
			if (is_in == false){
					printf("folder %s is missing\n",folder);
					return NULL;
			} else {
				break;
			}
		}

	}

	//wird sind im root verzeichnis


	
	int* curr_direct_blocks;


		
	// angenommen path = /home
	// finde inode zugehoerig zu path
	// greife auf direct_blocks array zu, wo die untergeordneten Verzeichnisse und Files existieren
	for (int i = 0; i < size; i++) {

		curr_inode = &(fs -> inodes[i]);
		
		// falls inode mit dem gleichen namen gefunden wurde
		if (!strcmp(curr_inode -> name, folder) &&
		curr_inode -> n_type == directory &&
		curr_inode -> parent == parent_id) {
		//greife auf direct blocks array zu 
			curr_direct_blocks = curr_inode -> direct_blocks;
			break;
		}

	}

	//iteriere direct blocks array 
	int inode_id;
	for (int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
			inode_id = curr_direct_blocks[i];
			if (inode_id != -1) {

				curr_inode = &(fs -> inodes[inode_id]);

				if (curr_inode->n_type == directory) {
					strcat(string,"DIR ");
				} else {
					strcat(string,"FIL ");
				}
				strcat(string, curr_inode -> name);
				strcat(string,"\n");
				

			}
	}




	
	

	return string;
}

int
fs_writef_for_import(file_system *fs, char *filename, char *text,size_t n_bytes)
{
	int size = fs -> s_block -> num_blocks;

	size_t text_length = n_bytes;
	
	int text_written = 0;
	
	int token_amount = token_counter(filename);
	int token_number = 1;

	char* folder = strtok(filename,"/");

	int parent_id = 0;
	int curr_inode_id;
	inode* curr_inode;

	while (folder != NULL)
	{

		// ueberpruefe den parent node nach dem namen in den inodes vom direct block

		int is_in = false;

		for (int i = 0; i < size; i++){
			curr_inode = &(fs -> inodes[i]);
			curr_inode_id = i;
			
			//der folder ist schon drin
			if ( !strcmp(curr_inode -> name,folder) && 
				curr_inode -> n_type == directory &&
				curr_inode -> parent == parent_id ) {
				
				is_in = true;
				if (token_number != token_amount) {
					is_in = true;
					token_number++;
					parent_id = curr_inode_id;
					folder = strtok(NULL,"/");
					
				}

			
			}
			 else if (!strcmp(curr_inode -> name,folder) && 
				curr_inode -> n_type == reg_file &&
				curr_inode -> parent == parent_id){
				
				is_in = true;

				break;

			} 
			
			else {
				is_in = false;
			}
		
		} 
		//if (token_number != token_amount && is_in == false) {
		if ( is_in == false) {
			printf("folder or file %s doesn't exist\n", folder);
			return -1;
		} 
		if ( token_number == token_amount) {
			
			break;
		}


	}
	
	/* file path wurde gecheckt und parent_node wurde festgelegt,
	 jetzt wird geschrieben 
	*/
	/* -----------------------------------------------------------------------------------------------------------------*/
	// 1. Falls die Datei leer ist, finde den naechsten freien datablock


	data_block* curr_datablock;
	int curr_datablock_id;

	int remaining_space;

	

	while ( text != NULL) {

		if ( curr_inode -> size == 0){

			// 1. Falls die Datei leer ist, finde den naechsten freien datablock
			for ( int i = 0; i < size; i++) {
				// finde leeren datenblock
				curr_datablock = &(fs -> data_blocks[i]);
				curr_datablock_id = i;
				// hier fehlt anscheinend ob der datenblock frei ist
				if (curr_datablock -> size == 0){
					break;
				}
			}

			//1.1 ist text size groesser als Block size (1024)?
			// Ja? schreibe so viel wie geht in den freien Block rein
			// Nein? schreibe alles und setze text auf null

			
			if (text_length > BLOCK_SIZE) {

				memcpy((char*)curr_datablock -> block,text,BLOCK_SIZE);
				text_written = text_written + BLOCK_SIZE;

				text = &text[BLOCK_SIZE];

				//nachdem schreiben muss text_length verkleinert werden
				text_length = text_length - BLOCK_SIZE;

				int curr_block_size = BLOCK_SIZE;

				curr_datablock -> size = curr_block_size;

				curr_inode -> size = curr_inode -> size + curr_block_size;

			} else {
				memcpy((char*)curr_datablock -> block, text, text_length);
				text_written = text_length;
				text = NULL;

				int curr_block_size = text_length;
				
				curr_datablock -> size = curr_block_size;

				curr_inode -> size = curr_inode -> size + curr_block_size;

			}

			// 3. size vom daten block und zugehoerigem inode festlegen

			// zaehlen von allen bytes ausser /000
				// int curr_block_size = count_chars( (char*)curr_datablock -> block, BLOCK_SIZE);

				// curr_datablock -> size = curr_block_size;

				// //inode size ist die summe der datenbloecke
				// curr_inode -> size = curr_inode -> size + curr_block_size;

			// 4. DIRECT_BLOCK setzen vom file Inode
			for ( int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
		
				if (curr_inode -> direct_blocks[i] == -1) {
				curr_inode -> direct_blocks[i] = curr_datablock_id;
				break;
					}
				}

			// 6. benutzte bloecke sollen in der freiliste als nicht frei markiert werden
			fs -> free_list[curr_datablock_id] = 0;


		//1.1 Falls die datei nicht leer ist
		// curr_inode -> size != 0
		} else {
			/* datei ist nicht leer
			finde also den zuletzt beschriebenen data_block
			TO DO: Falls es keinen beschriebenen block gibt, der noch platz hat, finde neuen freien block*/
			int capable_block_found = true;	
			// a capable block ist derjenige, der nicht leer aber auch nicht voll ist	
			for ( int i = 0; i < size; i++){
				
				if (curr_inode -> direct_blocks[i] != -1 ) {
					//huge bug
					curr_datablock_id = curr_inode -> direct_blocks[i];
					curr_datablock = &(fs -> data_blocks[curr_datablock_id]);
					
					
					remaining_space = BLOCK_SIZE - curr_datablock -> size;
					
					
					// falls block voll ist suche weiter
					if (remaining_space == 0 ) {
						capable_block_found = false;
						continue;
						
					// falls platz im block ist, breche ab
					} else {
						break;
					}

				} 
			} // datablock wurde gefunden 

			// falls kein capable gefunden wurde, suche einen freien
			if (!capable_block_found) {
				for ( int i = 0; i < size; i++) {
				// finde leeren datenblock
				curr_datablock = &(fs -> data_blocks[i]);
				curr_datablock_id = i;

				remaining_space = BLOCK_SIZE - curr_datablock -> size;

				if (curr_datablock -> size == 0){
					break;
				}
			} 
			//direct blocksetzen
			for ( int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
		
				if (curr_inode -> direct_blocks[i] == -1) {
				curr_inode -> direct_blocks[i] = curr_datablock_id;
				break;
					}
				}	
			fs -> free_list[curr_datablock_id] = 0;		
			}
			
		//text_length = count_chars(text,size * BLOCK_SIZE);
		char* offset = (char*)curr_datablock -> block;
		//schreiben von daten in den block
		// falls platz nicht ausreicht, schreibe soviel rein wie es geht
		if ( text_length > remaining_space){
			memcpy(&offset[curr_datablock->size],text,remaining_space);

			curr_datablock -> size = curr_datablock -> size + remaining_space;

			// inode size ist die sie summe der datenbloecke
			curr_inode -> size = curr_inode -> size + remaining_space;

			text_written = text_written + remaining_space;

			text = &text[remaining_space];

			text_length = text_length - remaining_space;
			
			} else { //falls platz ausreicht, schreibe alles
				memcpy(&offset[curr_datablock->size],text,text_length);
				// size updaten

				curr_datablock -> size = curr_datablock -> size + text_length;

				//inode size ist die summe der datenbloecke
				curr_inode -> size = curr_inode -> size + text_length;

				text_written = text_written + text_length;

				text = NULL;

				text_length = 0;
			}
		} 
		

	}
	

	return text_written;
}

int
fs_writef(file_system *fs, char *filename, char *text)
{
	int size = fs -> s_block -> num_blocks;
	int text_length = strlen(text);
	int text_written = 0;
	
	int token_amount = token_counter(filename);
	int token_number = 1;

	char* folder = strtok(filename,"/");

	int parent_id = 0;
	int curr_inode_id;
	inode* curr_inode;

	while (folder != NULL)
	{

		// ueberpruefe den parent node nach dem namen in den inodes vom direct block

		int is_in = false;

		for (int i = 0; i < size; i++){
			curr_inode = &(fs -> inodes[i]);
			curr_inode_id = i;
			
			//der folder ist schon drin
			if ( !strcmp(curr_inode -> name,folder) && 
				curr_inode -> n_type == directory &&
				curr_inode -> parent == parent_id ) {
				
				is_in = true;
				if (token_number != token_amount) {
					is_in = true;
					token_number++;
					parent_id = curr_inode_id;
					folder = strtok(NULL,"/");
					
				}

			
			}
			 else if (!strcmp(curr_inode -> name,folder) && 
				curr_inode -> n_type == reg_file &&
				curr_inode -> parent == parent_id){
				
				is_in = true;

				break;

			} 
			
			else {
				is_in = false;
			}
		
		} 
		//if (token_number != token_amount && is_in == false) {
		if ( is_in == false) {
			printf("folder or file %s doesn't exist\n", folder);
			return -1;
		} 
		if ( token_number == token_amount) {
			
			break;
		}


	}
	
	/* file path wurde gecheckt und parent_node wurde festgelegt,
	 jetzt wird geschrieben 
	*/
	/* -----------------------------------------------------------------------------------------------------------------*/
	// 1. Falls die Datei leer ist, finde den naechsten freien datablock


	data_block* curr_datablock;
	int curr_datablock_id;

	int remaining_space;

	

	while ( text != NULL) {

		if ( curr_inode -> size == 0){

			// 1. Falls die Datei leer ist, finde den naechsten freien datablock
			for ( int i = 0; i < size; i++) {
				// finde leeren datenblock
				curr_datablock = &(fs -> data_blocks[i]);
				curr_datablock_id = i;

				if (curr_datablock -> size == 0){
					break;
				}
			}

			//1.1 ist text size groesser als Block size (1024)?
			// Ja? schreibe so viel wie geht in den freien Block rein
			// Nein? schreibe alles und setze text auf null
			text_length = strlen(text);
			if (text_length > BLOCK_SIZE) {

				memcpy((char*)curr_datablock -> block,text,BLOCK_SIZE);
				text_written = text_written + BLOCK_SIZE;

				text = &text[BLOCK_SIZE];



			} else {
				memcpy((char*)curr_datablock -> block, text, text_length);
				text_written = text_length;
				text = NULL;
			}

			// 3. size vom daten block und zugehoerigem inode festlegen

			// zaehlen von allen bytes ausser /000
				int curr_block_size = count_chars( (char*)curr_datablock -> block, BLOCK_SIZE);

				curr_datablock -> size = curr_block_size;

				//inode size ist die summe der datenbloecke
				curr_inode -> size = curr_inode -> size + curr_block_size;

			// 4. DIRECT_BLOCK setzen vom file Inode
			for ( int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
		
				if (curr_inode -> direct_blocks[i] == -1) {
				curr_inode -> direct_blocks[i] = curr_datablock_id;
				break;
					}
				}

			// 6. benutzte bloecke sollen in der freiliste als nicht frei markiert werden
			fs -> free_list[curr_datablock_id] = 0;


		//1.1 Falls die datei nicht leer ist
		// curr_inode -> size != 0
		} else {
			/* datei ist nicht leer
			finde also den zuletzt beschriebenen data_block
			TO DO: Falls es keinen beschriebenen block gibt, der noch platz hat, finde neuen freien block*/
			int capable_block_found = true;	
			// a capable block ist derjenige, der nicht leer aber auch nicht voll ist	
			for ( int i = 0; i < size; i++){
				
				if (curr_inode -> direct_blocks[i] != -1 ) {
					curr_datablock_id = i;
					curr_datablock = &(fs -> data_blocks[curr_datablock_id]);
					
					
					remaining_space = BLOCK_SIZE - curr_datablock -> size;
					
					
					// falls block voll ist suche weiter
					if (remaining_space == 0 ) {
						capable_block_found = false;
						continue;
						
					// falls platz im block ist, breche ab
					} else {
						break;
					}

				} 
			} // datablock wurde gefunden 

			// falls kein capable gefunden wurde, suche einen freien
			if (!capable_block_found) {
				for ( int i = 0; i < size; i++) {
				// finde leeren datenblock
				curr_datablock = &(fs -> data_blocks[i]);
				curr_datablock_id = i;

				remaining_space = BLOCK_SIZE - curr_datablock -> size;

				if (curr_datablock -> size == 0){
					break;
				}
			} 
			//direct blocksetzen
			for ( int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
		
				if (curr_inode -> direct_blocks[i] == -1) {
				curr_inode -> direct_blocks[i] = curr_datablock_id;
				break;
					}
				}	
			fs -> free_list[curr_datablock_id] = 0;		
			}
			
		//text_length = count_chars(text,size * BLOCK_SIZE);

		text_length = strlen(text);

		char* offset = (char*)curr_datablock -> block; 
		//schreiben von daten in den block
		// falls platz nicht ausreicht, schreibe soviel rein wie es geht
		if ( text_length > remaining_space){
			memcpy(&offset[curr_datablock -> size],text,remaining_space);

			curr_datablock -> size = curr_datablock -> size + remaining_space;

			// inode size ist die sie summe der datenbloecke
			curr_inode -> size = curr_inode -> size + remaining_space;

			text_written = text_written + remaining_space;

			text = &text[remaining_space];
			
			} else { //falls platz ausreicht, schreibe alles
				memcpy(&offset[curr_datablock -> size],text,text_length);
				// size updaten

				curr_datablock -> size = curr_datablock -> size + strlen(text);

				//inode size ist die summe der datenbloecke
				curr_inode -> size = curr_inode -> size + strlen(text);

				text_written = text_written + text_length;

				text = NULL;
			}
		} 
		

	}
	

	return text_written;
}

uint8_t *
fs_readf(file_system *fs, char *filename, int *file_size)
{
	

	// first check the path to the filename
	int parent_id = 0;
	int curr_inode_id;
	inode* curr_inode = NULL;

	if ( check_valid_file_path(fs,filename, &curr_inode, &curr_inode_id, &parent_id) == -1 ){
		return NULL;
	}

	*file_size = curr_inode -> size;

	if (*file_size == 0) {
		return NULL;
	}
	/* inode wurde gefunden
	jetzt gebe die directs blocks nach einanander aus */

	uint8_t* all_blocks = malloc((*file_size));
	uint8_t* all_blocks_pointer = all_blocks;
	data_block* curr_datablock = NULL;
	int datablock_id;
	int curr_datablock_size = 0;
	for ( int i = 0; i < DIRECT_BLOCKS_COUNT; i++){
		datablock_id = curr_inode -> direct_blocks[i];
		if (datablock_id != -1){
		curr_datablock = &(fs -> data_blocks[datablock_id]);
		curr_datablock_size = curr_datablock -> size;


		memcpy(all_blocks_pointer, curr_datablock ->block, curr_datablock_size );
		all_blocks_pointer = &all_blocks_pointer[curr_datablock_size];
		} 

		
		

		

	}
	
	return all_blocks;	
}


int
fs_rm(file_system *fs, char *path)
{	
	// 1. check wieder ob es der path valid ist
	// first check the path to the filename
	int parent_id = 0;
	int curr_inode_id;
	inode* curr_inode = NULL;

	if ( check_valid_file_or_folder_path(fs,path, &curr_inode, &curr_inode_id, &parent_id) == -1 ){
		return -1;
	}	

	//2. jetzt schreibe rekursive hilfsfunktion
	del_inode(fs, &curr_inode, &curr_inode_id);

	return 0;
}

int
fs_import(file_system *fs, char *int_path, char *ext_path)
{	
    FILE *fp = fopen(ext_path,"r");
    if (!fp) {
        perror("File opening failed");
        return -1;
    }
	int n = fs -> s_block ->num_blocks;
	size_t size = n * BLOCK_SIZE * sizeof(uint8_t);
	char* text = malloc(size);

	memset(text,0,size);

	// was macht fread wenn size > laenge vom string ?
	size_t const written_bytes = fread(text, sizeof(text[0]),size,fp);
	// funktioniert nicht bei binary daten
	

	fclose(fp);
	

	// hier wird der path gecheckt

	int value = fs_writef_for_import(fs,int_path,text, written_bytes );

	free(text);

	

	if (value == -1){
		return -1;
	}

	return 0;
	
}


int
fs_export(file_system *fs, char *int_path, char *ext_path)
{
	

	int file_size;
	uint8_t* output = fs_readf(fs,int_path,&file_size);

	if (output == NULL){
		return -1;
	}

	FILE *fp = fopen(ext_path,"w");
	// 
	fwrite(output,sizeof(uint8_t),file_size,fp);


	memset(output,0,file_size);

	free(output);

	fclose(fp);



	return 0;
}
