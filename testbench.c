/* SPDX-License-Identifier: Apache-2.0 */
/**
 * @file 		testbench.c
 *
 * @brief 		Testbench for yaml loader library
 *
 * @copyright 	Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
 *
 * @date 		Feb 2024
 * @author 		Barrett Edwards <code@jrlabs.io>
 * 
 */

/* INCLUDES ==================================================================*/

#include <stdio.h>

#include <stdlib.h>

#include <glib-2.0/glib.h>

#include "yamlloader.h"

/* MACROS ====================================================================*/

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

/* GLOBAL VARIABLES ==========================================================*/

/* PROTOTYPES ================================================================*/

int main(int argc, char *argv[])
{
	GHashTable *ht;

	if (argc < 2 ) 
	{
		printf("Usage: testbench <filename.yaml>\n");
		exit(0);
	}

	ht = yl_load(argv[1]);

	yl_print(ht);

	yl_free(ht);

	return 0;
}

