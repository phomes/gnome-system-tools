/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* e-table.c: this file is part of runlevel-admin, a ximian-setup-tool frontend 
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include "xst.h"
#include <gal/e-table/e-table-scrolled.h>
#include <gal/e-table/e-table-simple.h>
#include <gal/e-table/e-cell-text.h>
#include <gal/e-table/e-cell-toggle.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "e-table.h"
#include "callbacks.h"

#define RUNLEVEL_TABLE_SPEC "runlevel.etspec"

extern XstTool *tool;

GtkWidget *runlevel_table;
GArray *runlevel_array;

const gchar *runlevel_state_advanced = "\
<ETableState> \
  <column source=\"0\"/> \
  <column source=\"1\"/> \
  <column source=\"2\"/> \
  <column source=\"3\"/> \
  <column source=\"4\"/> \
  <column source=\"5\"/> \
  <column source=\"6\"/> \
  <column source=\"7\"/> \
  <grouping> \
  </grouping> \
</ETableState>";

// Images used in table
GdkPixbuf *start_icon;
GdkPixbuf *stop_icon;
GdkPixbuf *do_nothing_icon;

static int
table_col_count (ETableModel *etc,void *data)
{
	return COL_LEVEL6;
}

static int 
table_row_count (ETableModel *etc, void *data)
{
	if (runlevel_array) {
		return runlevel_array->len;
	}
	return 0;
}

static void* 
table_value_service(xmlNodePtr node)
{
	gchar *buf;
	g_return_val_if_fail (node != NULL, NULL);
	buf = xst_xml_get_child_content (node, "name");
	if (!buf)
		buf = xst_xml_get_child_content (node, "script");
	return buf;
}

static void* 
table_value_runlevel (xmlNodePtr node,gint runlevel)
{
	xmlNodePtr runlevels, rl;
	gchar *level, *action;
	g_return_val_if_fail (node != NULL, NULL);
	runlevels = xst_xml_element_find_first (node, "runlevels");
	if (runlevels != NULL){
		for (rl = xst_xml_element_find_first (runlevels, "runlevel"); rl != NULL; rl = xst_xml_element_find_next (rl, "runlevel"))
		{
			level = xst_xml_get_child_content (rl, "number");
			if (runlevel == atoi (level))
			{
				action = xst_xml_get_child_content (rl, "action");
				if (strcmp (action, "start") == 0)
				{
					g_free (level);
					g_free (action);
					// returns start icon;
					return GINT_TO_POINTER (0);
				}
				else
				{
					g_free (level);
					g_free (action);
					// returns stop icon;
					return GINT_TO_POINTER (1);
				}
			}
			g_free (level);
		}
	}
	// returns "do nothing" icon;
	return GINT_TO_POINTER (2);

}

static void*
table_value_at (ETableModel *etc, int col, int row, void *data)
{
	xmlNodePtr node;
	gint runlevel;
	
	node = g_array_index (runlevel_array, xmlNodePtr, row);
	switch (col) {
		case COL_SERVICE:
			return table_value_service (node);
			break;
		case COL_LEVEL0:
		case COL_LEVEL1:
		case COL_LEVEL2:
		case COL_LEVEL3:
		case COL_LEVEL4:
		case COL_LEVEL5:
		case COL_LEVEL6:
			runlevel = col - 1;
			return table_value_runlevel (node, runlevel);
			break;
		default:
			return NULL;
	}
}

static void
table_set_value_at (ETableModel *etc, int col, int row, const void *val, void *data)
{
	xmlNodePtr node,runlevels,rl;
	gchar *level;
	
	if (col != COL_SERVICE){
		node = g_array_index (runlevel_array, xmlNodePtr, row);
		runlevels = xst_xml_element_find_first (node, "runlevels");
		
		if ((int) val == 1) // the state turns to stopped
		{
			for (rl = xst_xml_element_find_first (runlevels, "runlevel"); rl != NULL; rl = xst_xml_element_find_next (rl, "runlevel"))
			{
				level = xst_xml_get_child_content (rl, "number");
				if (col - 1 == atoi (level))
				{
					xst_xml_set_child_content (rl, "action", "stop");
					break;
				}
			}
		}
		else if ((int) val == 0) // the state turns to started
		{
			gchar *buf;

			if (runlevels == NULL){
				runlevels= xst_xml_element_add (node, "runlevels");
			}
			rl = xst_xml_element_add (runlevels, "runlevel");
			xst_xml_element_add (rl, "number");
			xst_xml_element_add (rl, "action");
			buf = g_strdup_printf ("%i", col - 1);
			xst_xml_set_child_content (rl, "number", buf);
			xst_xml_set_child_content (rl, "action", "start");
			g_free (buf);
		}
		else // The state turns to "do nothing"
		{
			for (rl = xst_xml_element_find_first (runlevels, "runlevel"); rl != NULL; rl = xst_xml_element_find_next (rl, "runlevel"))
			{
				level = xst_xml_get_child_content (rl, "number");
				if (col - 1 == atoi (level))
				{
					xst_xml_element_destroy (rl);
					break;
				}
			}
		}
		
		e_table_model_row_changed (etc, row);
		xst_dialog_modify (tool->main_dialog);
	}
}

static gboolean
table_is_cell_editable (ETableModel *etc, int col, int row, void *data)
{
	if (col == COL_SERVICE)
        return FALSE;
	else
		return TRUE;
}

static void*
table_duplicate_value (ETableModel *etc, int col, const void *value, void *data)
{
        return g_strdup (value);
}

static void
table_free_value (ETableModel *etc, int col, void *value, void *data)
{
        g_free (value);
}

static void*
table_initialize_value (ETableModel *etc, int col, void *data)
{
        return g_strdup ("");
}

static gboolean
table_value_is_empty (ETableModel *etc, int col, const void *value, void *data)
{
	return !(value && *(char *)value);
}

static char*
table_value_to_string (ETableModel *etc, int col, const void *value, void *data)
{
	return g_strdup (value);
}

static ETableExtras*
create_extras (void)
{
	GdkPixbuf *images[3];
	ETableExtras *extras;
	
	images[0] = start_icon;
	images[1] = stop_icon;
	images[2] = do_nothing_icon;
	
	extras = e_table_extras_new ();
	e_table_extras_add_cell (extras, "centered_cell", e_cell_text_new (NULL, GTK_JUSTIFY_LEFT));
	e_table_extras_add_cell (extras, "render_runlevel", e_cell_toggle_new (0,3, images));
	return extras;
}

static void
pixmaps_create (void)
{
	start_icon = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/gnome-light-on.png");
	stop_icon = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/gnome-light-off.png");
	do_nothing_icon = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/stock_remove_24.png");
}

GtkWidget*
table_create (char *widget)
{
	ETableModel *model;
	ETableExtras *extras;
	gchar *spec;
	
	// Creates the pixmaps that are going to be used in the table
	pixmaps_create();
	
	if (runlevel_table)
		return NULL;
	extras = create_extras ();
	
	model = e_table_simple_new (table_col_count,
	                            table_row_count,
	                            NULL,
	                            table_value_at,
			                    table_set_value_at,
			                    table_is_cell_editable,
			                    NULL,
			                    NULL,
			                    table_duplicate_value,
			                    table_free_value,
			                    table_initialize_value,
			                    table_value_is_empty,
			                    table_value_to_string,
			                    NULL);
	spec = xst_conf_get_string (tool, "spec");
	if (!spec) {
		spec = xst_ui_load_etspec (tool->etspecs_common_path, RUNLEVEL_TABLE_SPEC);
		if (!spec)
			g_error ("create table: Couldn't create table.");
		xst_conf_set_string (tool, "spec", spec);
	}
	runlevel_table = e_table_scrolled_new (E_TABLE_MODEL (model), extras, spec, runlevel_state_advanced);
	g_free (spec);
	g_return_val_if_fail (runlevel_table != NULL, NULL);

	return runlevel_table;
}

void 
table_populate (xmlNodePtr root)
{
	xmlNodePtr service,services;
	ETable *table;
	
	g_return_if_fail (root != NULL);
	
	table = e_table_scrolled_get_table (E_TABLE_SCROLLED (runlevel_table));
	runlevel_array = g_array_new (FALSE, FALSE, sizeof (xmlNodePtr));
	
	services = xst_xml_element_find_first (root, "services");
	
	for (service = xst_xml_element_find_first (services, "service"); service != NULL; service = xst_xml_element_find_next (service, "service"))
	{
		g_array_append_val (runlevel_array, service);
	}
	
	e_table_model_changed (table->model);
}

void
table_update_headers (xmlNodePtr root)
{
	ETable *table;
	gchar *state, *buf;
	gint rl;
	XstDialogComplexity complexity;

	g_return_if_fail (runlevel_table != NULL);
	
	table = e_table_scrolled_get_table (E_TABLE_SCROLLED (runlevel_table));
	g_return_if_fail (table != NULL);

	complexity = XST_DIALOG (tool->main_dialog)->complexity;
	
	buf = xst_xml_get_child_content (root, "runlevel");
	rl = buf[0] - '0';
	
	state = g_strdup_printf ("<ETableState> \
							  <column source=\"0\"/> \
							  <column source=\"%i\"/> \
							  <grouping> \
							  </grouping> \
							  </ETableState>", rl+1);
	xst_conf_set_string (tool, "state_basic", state);
	if (complexity == XST_DIALOG_BASIC) {
		e_table_set_state (table, state);
	}
	g_free (state);
}
	
void 
table_update_state(XstDialogComplexity complexity)
{
	ETable *table;
	gchar *state;

	g_return_if_fail (runlevel_table != NULL);
	
	table = e_table_scrolled_get_table (E_TABLE_SCROLLED (runlevel_table));
	g_return_if_fail (table != NULL);
	
	if (complexity == XST_DIALOG_BASIC) {
		state = xst_conf_get_string (tool, "state_basic");
		if (state == NULL) {
			state = g_strdup_printf ("<ETableState> \
									  <column source=\"0\"/> \
									  <grouping> \
									  </grouping> \
									  </ETableState>");
			xst_conf_set_string (tool, "state_basic", state);
		}
	} else {
		state = xst_conf_get_string (tool, "state_adv");
		if (state == NULL) {
			state = g_strdup(runlevel_state_advanced);
			xst_conf_set_string (tool, "state_adv", state);
		}
	}
	e_table_set_state (table, state);
	g_free (state);
}
