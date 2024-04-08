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

#ifndef _YAMLLOADER_H
#define _YAMLLOADER_H

/* INCLUDES ==================================================================*/

#include <linux/types.h>

#include <glib-2.0/glib.h>

/* MACROS ====================================================================*/

/* ENUMERATIONS ==============================================================*/

/* STRUCTS ===================================================================*/

typedef struct yl_obj 
{
	char *str;
	GHashTable *ht;
} yl_obj_t; 

/* GLOBAL VARIABLES ==========================================================*/

/* PROTOTYPES ================================================================*/

GHashTable *yl_load(char *filename);
int yl_free(GHashTable *ht);
int yl_print(GHashTable *ht);
void _yl_print_entry(gpointer key, gpointer value, gpointer user_data);

#endif //ifndef _YAMLLOADER_H
