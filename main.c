/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		yamlloader.c
 *
 * @brief 		Code file to Load a .yaml file into a GHashTable
 *
 * @copyright 	Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
 *
 * @date 		Feb 2024
 * @author 		Barrett Edwards <code@jrlabs.io>
 * 
 */

/* INCLUDES ==================================================================*/

#include <stdlib.h>
#include <stdio.h>

#include <glib-2.0/glib.h>
#include <yaml.h> 	// apt install libyaml-dev libyaml-doc

#include "main.h"

/* MACROS ====================================================================*/

#define MAX_STR 256

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

struct yaml_parse_state {
	yaml_parser_t *parser;
	GHashTable *ht;

	int escape;
	char *keyname;
	int loop; 
};

/* GLOBAL VARIABLES ==========================================================*/

/* PROTOTYPES ================================================================*/

int yl_parse(struct yaml_parse_state *y);

/* FUNCTIONS =================================================================*/

/**
 * Free allocated memory
 *
 * @param ht	GHashTable* to walk and free memory from
 */
int yl_free(GHashTable *ht)
{

	return 0;
}

/**
 * Load a .yaml file into a GHashTable
 *
 * @param 	filename 	char* 
 * @return	GHashTable*
 */
GHashTable *yl_load(char *filename)
{
	int rv;
	FILE *fp; 
	GHashTable *ht;
	yaml_parser_t parser;
	struct yaml_parse_state yps;

	/* STEPS:
	 * 1: Validate inputs
	 * 2: Create GHashTable to store the result
	 * 3: Initialize parser
	 * 4: Open config file 
	 * 5: Set YAML parser input file
	 * 6: Parse config File
	 * 7: Cleanup 
	 */

	// STEP 1: Validate inputs
	if(filename == NULL) {
		goto end_ht;
	}

	// STEP 2: Create strdict to store the result
	ht = g_hash_table_new(g_str_hash, g_str_equal);
	if (ht == NULL) {
		return NULL;
	}

	// STEP 3: Initialize parser
	rv = yaml_parser_initialize(&parser);
	if(rv == 0) {
		goto end_ht;
	}

	// STEP 4: Open config file 
	fp = fopen(filename, "r");
	if(fp == 0) {
		goto end_ht;
	}

	// STEP 5: Set YAML parser input file
  	yaml_parser_set_input_file(&parser, fp);

	// STEP 6: Parse config File
	memset(&yps, 0, sizeof(struct yaml_parse_state));
	yps.ht = ht;
	yps.parser = &parser;
	yps.escape = YAML_STREAM_END_TOKEN;

	rv = yl_parse(&yps);
	if(rv != 0) {
		goto end_parser;
	}

	// STEP 7: Cleanup
	yaml_parser_delete(&parser);
	fclose(fp);

	return ht;

end_parser:
	yaml_parser_delete(&parser);
	fclose(fp);
end_ht:
	yl_free(ht);

	return 0;
}

/**
 * Parse function called for each entry in the yaml file
 */
int yl_parse(struct yaml_parse_state *yps)
{
	int rv;
	char *s;
	yl_obj_t *ylo;
	yaml_token_t t;
	GHashTable *newht;
	struct yaml_parse_state newyps;

	do {
		yaml_parser_scan(yps->parser, &t);
		switch(t.type)
		{
			case YAML_STREAM_START_TOKEN: 							break;
			case YAML_STREAM_END_TOKEN:  							break;
			case YAML_KEY_TOKEN: 					yps->loop = 1; 	break;
			case YAML_VALUE_TOKEN: 					yps->loop = 2; 	break;
			case YAML_BLOCK_SEQUENCE_START_TOKEN:					break;
			case YAML_BLOCK_ENTRY_TOKEN: 							break;
			case YAML_BLOCK_END_TOKEN: 								break;
			case YAML_BLOCK_MAPPING_START_TOKEN:	
				if(yps->loop != 2) 					// Ignore the very first blk_map_start in the stream 
					break;
				
				// Create a new GHashTable object and add to the current hashtable 
				ylo = calloc (1, sizeof( yl_obj_t ) );
				ylo->ht = g_hash_table_new(g_str_hash, g_str_equal);
				rv = g_hash_table_insert(yps->ht, yps->keyname, ylo);
				if (rv == FALSE) { // return of 0 is an error
					goto end;
				}

				// Initialize a new yaml+_parse_state 
				memset(&newyps, 0, sizeof(struct yaml_parse_state));
				newyps.ht = ylo->ht;
				newyps.parser = yps->parser;
				newyps.escape = YAML_BLOCK_END_TOKEN;

				// Make recursive parse call with new parse state
				rv = yl_parse(&newyps);
				if (rv != 0) {
					goto end;
				}

				// After returning from the recursivecall to parse, clean up 
				yps->loop = 0;				
				yps->keyname = NULL;
				break;
			case YAML_SCALAR_TOKEN:					
				// Duplicate the string 
				s = strndup(t.data.scalar.value, MAX_STR);

				// store duplicated string value yaml_parse_state as the keyname until we use it later
				if (yps->loop == 1) {
					yps->keyname = s;
				}
				else if (yps->loop == 2) {				
					// Store the duplicated value string in a yl_obj struct and then store that yl_obj in the hash table
					ylo = calloc (1, sizeof( yl_obj_t ) );
					ylo->str = s;

					// We now have the value, store the KV pair in hash table
					rv = g_hash_table_insert(yps->ht, yps->keyname, ylo);
					if (rv ==0) { // 0
						goto end;
					}
					yps->keyname = NULL;
				}
				yps->loop = 0;
				break;
			default: 								yps->loop = 0; 	break;
		}
		if(t.type != yps->escape)
			yaml_token_delete(&t);
	} while(t.type != yps->escape);
	yaml_token_delete(&t);
	return 0;

end:
	yaml_token_delete(&t);

	return -1;
}

/**
 * Print the GHasHTable
 */ 
int yl_print(GHashTable *ht)
{

	if (ht == NULL) {
		return -1;
	}	

	g_hash_table_foreach(ht, _yl_print_entry, 0);	

	return 0;
}

/**
 * Print an individual entry in the GHashTable
 */
void _yl_print_entry(gpointer key, gpointer value, gpointer user_data)
{
	int i;
	__u64 indent;
	yl_obj_t *ylo;

	indent = (__u64) user_data;	
	ylo = (yl_obj_t*) value;

	// Print the indent spaces 
	for ( i = 0 ; i < indent ; i++) 
		printf(" ");

	if (ylo->str != NULL) {
		printf("%s:%s\n", (char*) key, ylo->str);
	}
	else {
		printf("%s:\n", (char*) key);
		g_hash_table_foreach(ylo->ht, _yl_print_entry, (void*)(indent+2));	
	}
}

