/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* e-table.h: this file is part of runlevel-admin, a ximian-setup-tool frontend 
 * for run level services administration.
 * 
 * Copyright (C) 2002 Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors: Carlos Garnacho <garparr@teleline.es>.
 */

#ifndef _E_TABLE_H
#define _E_TABLE_H

#include <gnome.h>
#include <gal/e-table/e-table-simple.h>

enum {
	COL_SERVICE,
	COL_LEVEL0,
	COL_LEVEL1,
	COL_LEVEL2,
	COL_LEVEL3,
	COL_LEVEL4,
	COL_LEVEL5,
	COL_LEVEL6,
};

xmlNodePtr	get_selected_node	(gint);
GtkWidget*	table_create		(char*);
/*
void 		table_add		(gint,gchar*,gint,gchar*);
void 		table_delete		(gint);
*/
void 		table_populate		(xmlNodePtr);

#endif /* _E_TABLE_H */
