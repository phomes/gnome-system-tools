/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * Copyright (C) 2001 Ximian, Inc.
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
 * Authors: Hans Petter Jansson <hpj@ximian.com>
 */

#include <config.h>

#include "gst-widget.h"
#include "gst-dialog.h"


void
gst_widget_apply_policy (GstWidget *xw)
{
	GstDialogComplexity complexity;
	gboolean have_access;
	GstTool *tool;

	g_return_if_fail (xw != NULL);

	tool = gst_dialog_get_tool (xw->dialog);
	have_access = gst_tool_is_authenticated (tool);

	/*
	if (!xw->user_sensitive)
		gtk_widget_set_sensitive (xw->widget, FALSE);
	else {
		if (xw->need_root_access)
			gtk_widget_set_sensitive (xw->widget, have_access);
		else
			gtk_widget_set_sensitive (xw->widget, TRUE);
	}
	*/


	gtk_widget_set_sensitive (xw->widget,
				  ((!xw->need_root_access || have_access) && xw->user_sensitive));
}


GstWidget *
gst_widget_new_full (GtkWidget *w, GstDialog *d, gboolean need_root_access, gboolean user_sensitive)
{
	GstWidget *xw;

	g_return_val_if_fail (w != NULL, NULL);
	g_return_val_if_fail (d != NULL, NULL);

	xw = g_new0 (GstWidget, 1);

	xw->widget = w;
	xw->dialog = d;
	xw->need_root_access = need_root_access;
	xw->user_sensitive = user_sensitive;

	return xw;
}

GstWidget *
gst_widget_new (GstDialog *dialog, GstWidgetPolicy policy)
{
	return gst_widget_new_full (gst_dialog_get_widget (dialog, policy.widget),
				    dialog, policy.need_root_access, policy.user_sensitive);
}

/* Backwards compatibility function. Will be removed as soon as all references to
 * it are cleaned out. */
void
gst_widget_set_user_sensitive (GstWidget *xw, gboolean user_sensitive)
{
	xw->user_sensitive = user_sensitive;
	gst_widget_apply_policy (xw);
}
