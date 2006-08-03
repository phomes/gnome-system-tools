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
 * Authors: Jacob Berkman   <jacob@ximian.com>
 *          Carlos Garnacho <carlosg@gnome.org>
 */

#include <config.h>

#include <gmodule.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>
#include "gst-tool.h"
#include "gst-widget.h"
#include "gst-dialog.h"
#include "gst-conf.h"
#include "gst-marshal.h"

#define GST_DIALOG_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), GST_TYPE_DIALOG, GstDialogPrivate))

typedef struct _GstDialogPrivate GstDialogPrivate;

struct _GstDialogPrivate {
	GstTool *tool;

	gchar   *title;
	gchar   *widget_name;

	GladeXML  *gui;
	GtkWidget *child;

	GtkWidget *lock_button;
	GtkWidget *lock_button_label;

	GSList *gst_widget_list;

	gboolean modified : 1;
	gboolean frozen   : 1;
};

static void gst_dialog_class_init (GstDialogClass *class);
static void gst_dialog_init       (GstDialog      *group);
static void gst_dialog_finalize   (GObject        *object);

static GObject*	gst_dialog_constructor (GType                  type,
					guint                  n_construct_properties,
					GObjectConstructParam *construct_params);

static void gst_dialog_set_property (GObject      *object,
				     guint         prop_id,
				     const GValue *value,
				     GParamSpec   *pspec);

static void gst_dialog_response (GtkDialog *dialog,
				 gint       response);

static void gst_dialog_map      (GtkWidget *widget);

enum {
	PROP_0,
	PROP_TOOL,
	PROP_WIDGET_NAME,
	PROP_TITLE
};

G_DEFINE_TYPE (GstDialog, gst_dialog, GTK_TYPE_DIALOG);

static void
gst_dialog_class_init (GstDialogClass *class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (class);
	GtkDialogClass *dialog_class = GTK_DIALOG_CLASS (class);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);

	object_class->set_property = gst_dialog_set_property;
	object_class->constructor  = gst_dialog_constructor;
	object_class->finalize     = gst_dialog_finalize;

	dialog_class->response     = gst_dialog_response;
	widget_class->map          = gst_dialog_map;

	g_object_class_install_property (object_class,
					 PROP_TOOL,
					 g_param_spec_object ("tool",
							      "tool",
							      "tool",
							      GST_TYPE_TOOL,
							      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class,
					 PROP_WIDGET_NAME,
					 g_param_spec_string ("widget_name",
							      "Widget name",
							      "Widget name",
							      NULL,
							      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (object_class,
					 PROP_TITLE,
					 g_param_spec_string ("title",
							      "title",
							      "title",
							      NULL,
							      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
	g_type_class_add_private (object_class,
				  sizeof (GstDialogPrivate));
}

static void
on_lock_toggled (GtkWidget *widget, gpointer data)
{
	GstDialog *dialog = (GstDialog *) data;
	GstDialogPrivate *priv = GST_DIALOG_GET_PRIVATE (dialog);
	GstTool *tool = gst_dialog_get_tool (dialog);
	gboolean active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
	GtkWidget *d;

	gtk_widget_set_sensitive (GTK_WIDGET (dialog), FALSE);

	if (oobs_session_set_authenticated (tool->session, active)) {
		/* authentication succeeded */
		gst_dialog_apply_widget_policies (dialog);
		gtk_label_set_text_with_mnemonic (priv->lock_button_label,
						  (active) ? _("_Lock") : _("Un_lock"));
	} else {
		g_signal_handlers_block_by_func (widget, on_lock_toggled, data);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), !active);
		g_signal_handlers_unblock_by_func (widget, on_lock_toggled, data);

		d = gtk_message_dialog_new (GTK_WINDOW (dialog),
					    GTK_DIALOG_MODAL,
					    GTK_MESSAGE_ERROR,
					    GTK_BUTTONS_CLOSE,
					    _("Authentication error"));
		gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (d),
							  _("Check that you typed your password correctly and "
							    "that you haven't activated the \"Caps Lock\" key"));
		gtk_dialog_run (GTK_DIALOG (d));
		gtk_widget_destroy (d);
	}

	gtk_widget_set_sensitive (GTK_WIDGET (dialog), TRUE);
}

static GtkWidget *
construct_lock_toggle_button (GstDialog *dialog)
{
	GstDialogPrivate *priv;
	GtkWidget *image, *hbox, *alignment;

	priv = GST_DIALOG_GET_PRIVATE (dialog);

	hbox = gtk_hbox_new (FALSE, 6);
	alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	image = gtk_image_new_from_stock ("gnome-stock-authentication", GTK_ICON_SIZE_BUTTON);
	priv->lock_button_label = gtk_label_new_with_mnemonic (_("Un_lock"));
	gtk_misc_set_alignment (GTK_MISC (priv->lock_button_label), 0.0, 0.5);

	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), priv->lock_button_label, FALSE, FALSE, 0);

	priv->lock_button = gtk_toggle_button_new ();
	gtk_container_add (GTK_CONTAINER (alignment), hbox);
	gtk_container_add (GTK_CONTAINER (priv->lock_button), alignment);

	gtk_widget_show_all (priv->lock_button);
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), priv->lock_button, GTK_RESPONSE_NONE);

	g_signal_connect (priv->lock_button, "toggled",
			  G_CALLBACK (on_lock_toggled), dialog);
}

static void
gst_dialog_init (GstDialog *dialog)
{
	GstDialogPrivate *priv;
	GtkWidget *button;

	priv = GST_DIALOG_GET_PRIVATE (dialog);

	priv->tool  = NULL;
	priv->title = NULL;
	priv->gui   = NULL;
	priv->child = NULL;

	priv->modified = FALSE;
	priv->frozen   = FALSE;

	priv->gst_widget_list = NULL;

	construct_lock_toggle_button (dialog);
	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
				GTK_STOCK_HELP, GTK_RESPONSE_HELP,
				GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
				NULL);
}

static GObject*
gst_dialog_constructor (GType                  type,
			guint                  n_construct_properties,
			GObjectConstructParam *construct_params)
{
	GObject *object;
	GstDialog *dialog;
	GstDialogPrivate *priv;

	object = (* G_OBJECT_CLASS (gst_dialog_parent_class)->constructor) (type,
									    n_construct_properties,
									    construct_params);
	dialog = GST_DIALOG (object);
	priv = GST_DIALOG_GET_PRIVATE (dialog);

	if (priv->tool && priv->widget_name) {
		priv->gui   = glade_xml_new (priv->tool->glade_path, NULL, NULL);
		priv->child = gst_dialog_get_widget (dialog, priv->widget_name);

		if (GTK_WIDGET_TOPLEVEL (priv->child)) {
			g_error ("The widget \"%s\" should not be a toplevel widget in the .glade file\n"
				 "You just need to add the widget inside a GtkWindow so that it can be deparented.", priv->widget_name);
		}

		gtk_widget_ref (priv->child);
		gtk_widget_unparent (priv->child);
		gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), priv->child);
	}

	if (priv->title)
		gtk_window_set_title (GTK_WINDOW (dialog), priv->title);

	return object;
}

static void
gst_dialog_set_property (GObject      *object,
			 guint         prop_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
	GstDialogPrivate *priv;

	priv = GST_DIALOG_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_TOOL:
		priv->tool = GST_TOOL (g_value_dup_object (value));
		break;
	case PROP_WIDGET_NAME:
		priv->widget_name = g_value_dup_string (value);
		break;
	case PROP_TITLE:
		priv->title = g_value_dup_string (value);
		break;
	}
}

static void
gst_dialog_finalize (GObject *object)
{
	/* FIXME: need to free stuff */
	(* G_OBJECT_CLASS (gst_dialog_parent_class)->finalize) (object);
}

static void
gst_dialog_response (GtkDialog *dialog,
		     gint       response)
{
	GstDialogPrivate *priv;

	priv = GST_DIALOG_GET_PRIVATE (dialog);

	switch (response) {
	case GTK_RESPONSE_CLOSE:
	case GTK_RESPONSE_DELETE_EVENT:
		gtk_widget_hide (GTK_WIDGET (dialog));
		g_signal_emit_by_name (G_OBJECT (priv->tool), "close");
		break;
	case GTK_RESPONSE_HELP:
		gst_tool_show_help (priv->tool, NULL);
		break;
	}
}

static void
gst_dialog_map (GtkWidget *widget)
{
	GstDialog *dialog = GST_DIALOG (widget);
	GstDialogPrivate *priv;

	(* GTK_WIDGET_CLASS (gst_dialog_parent_class)->map) (widget);

	priv = GST_DIALOG_GET_PRIVATE (dialog);

	gst_dialog_freeze_visible (dialog);
	gst_tool_update_gui (priv->tool);
	gst_dialog_thaw_visible (dialog);

	gst_dialog_apply_widget_policies (dialog);
}

GstDialog*
gst_dialog_new (GstTool *tool, const char *widget, const char *title)
{
	return g_object_new (GST_TYPE_DIALOG,
			     "has-separator", FALSE,
			     "tool", tool,
			     "widget-name", widget,
			     "title", title,
			     NULL);
}

GtkWidget *
gst_dialog_get_widget (GstDialog *dialog, const char *widget)
{
	GstDialogPrivate *priv;
	GtkWidget *w;

	g_return_val_if_fail (dialog != NULL, NULL);
	g_return_val_if_fail (GST_IS_DIALOG (dialog), NULL);

	priv = GST_DIALOG_GET_PRIVATE (dialog);

	g_return_val_if_fail (priv->gui != NULL, NULL);

	w = glade_xml_get_widget (priv->gui, widget);

	if (!w)
		g_error ("Could not find widget: %s", widget);

	return w;
}

void
gst_dialog_apply_widget_policies (GstDialog *dialog)
{
	GstDialogPrivate *priv;
	GSList *list;

	priv = GST_DIALOG_GET_PRIVATE (dialog);

	for (list = priv->gst_widget_list; list; list = g_slist_next (list)) {
		gst_widget_apply_policy (list->data);
	}
}

void
gst_dialog_freeze (GstDialog *dialog)
{
	GstDialogPrivate *priv;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GST_IS_DIALOG (dialog));

	priv = GST_DIALOG_GET_PRIVATE (dialog);
	priv->frozen = TRUE;
}

void 
gst_dialog_thaw (GstDialog *dialog)
{
	GstDialogPrivate *priv;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GST_IS_DIALOG (dialog));

	priv = GST_DIALOG_GET_PRIVATE (dialog);
	priv->frozen = FALSE;
}

void
gst_dialog_freeze_visible (GstDialog *dialog)
{
	GstDialogPrivate *priv;
	GdkCursor *cursor;
	
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GST_IS_DIALOG (dialog));
	g_return_if_fail (dialog->frozen >= 0);

	priv = GST_DIALOG_GET_PRIVATE (dialog);

	if (GTK_WIDGET (dialog)->window) {
		cursor = gdk_cursor_new (GDK_WATCH);
		gdk_window_set_cursor (GTK_WIDGET (dialog)->window, cursor);
		gdk_cursor_unref (cursor);
	}

	if (!priv->frozen)
		gtk_widget_set_sensitive (GTK_WIDGET (dialog), FALSE);
	
	gst_dialog_freeze (dialog);
}

void
gst_dialog_thaw_visible (GstDialog *dialog)
{
	GstDialogPrivate *priv;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GST_IS_DIALOG (dialog));
	g_return_if_fail (dialog->frozen >= 0);

	priv = GST_DIALOG_GET_PRIVATE (dialog);

	if (GTK_WIDGET (dialog)->window)
		gdk_window_set_cursor (GTK_WIDGET (dialog)->window, NULL);

	gst_dialog_thaw (dialog);

	if (!dialog->frozen)
		gtk_widget_set_sensitive (GTK_WIDGET (dialog), TRUE);
}

gboolean
gst_dialog_get_modified (GstDialog *dialog)
{
	GstDialogPrivate *priv;

	g_return_val_if_fail (dialog != NULL, FALSE);
	g_return_val_if_fail (GST_IS_DIALOG (dialog), FALSE);

	priv = GST_DIALOG_GET_PRIVATE (dialog);
	return priv->modified;
}

void
gst_dialog_set_modified (GstDialog *dialog, gboolean state)
{
	GstDialogPrivate *priv;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GST_IS_DIALOG (dialog));

	priv = GST_DIALOG_GET_PRIVATE (dialog);
	priv->modified = state;
}

void
gst_dialog_modify (GstDialog *dialog)
{
	GstDialogPrivate *priv;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GST_IS_DIALOG (dialog));

	priv = GST_DIALOG_GET_PRIVATE (dialog);

	if (priv->frozen || !gst_tool_is_authenticated (priv->tool))
		return;

	gst_dialog_set_modified (dialog, TRUE);
}

void
gst_dialog_modify_cb (GtkWidget *w, gpointer data)
{
	gst_dialog_modify (data);
}

void
gst_dialog_set_widget_policies (GstDialog *dialog, const GstWidgetPolicy *wp)
{
	GstDialogPrivate *priv;
	GstWidget *w;
	int i;

	priv = GST_DIALOG_GET_PRIVATE (dialog);

	for (i = 0; wp [i].widget; i++) {
		w = gst_widget_new (dialog, wp [i]);
		priv->gst_widget_list = g_slist_prepend (priv->gst_widget_list, w);
	}
}

GstWidget *
gst_dialog_get_gst_widget (GstDialog *xd, const gchar *name)
{
	GstDialogPrivate *priv;
	GstWidget *xw = NULL;
	GtkWidget *widget;
	GSList *list;

	g_return_val_if_fail (xd != NULL, NULL);

	priv = GST_DIALOG_GET_PRIVATE (xd);
	widget = gst_dialog_get_widget (xd, name);

	for (list = priv->gst_widget_list; list; list = g_slist_next (list))
	{
		if (((GstWidget *) list->data)->widget == widget)
		{
			xw = list->data;
			break;
		}
	}

	if (xw == NULL)
		g_warning ("Widget %s not found in policy table.", name);
	return (xw);
}

GstTool*
gst_dialog_get_tool (GstDialog *dialog)
{
	GstDialogPrivate *priv;

	priv = GST_DIALOG_GET_PRIVATE (dialog);
	return priv->tool;
}

void
gst_dialog_widget_set_user_sensitive (GstDialog *xd, const gchar *name, gboolean state)
{
	GstWidget *xw;

	g_return_if_fail (xd != NULL);

	xw = gst_dialog_get_gst_widget (xd, name);
	g_return_if_fail (xw != NULL);

	gst_widget_set_user_sensitive (xw, state);
}

static void
dialog_connect_signals (GstDialog *dialog, GstDialogSignal *signals, gboolean connect_after)
{       
	GtkWidget *w;
	guint sig;
	int i;

	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GST_IS_DIALOG (dialog));

	for (i=0; signals[i].widget; i++) {
		w = gst_dialog_get_widget (dialog, signals[i].widget);

		if (connect_after) {
			sig = g_signal_connect_after (G_OBJECT (w),
						      signals[i].signal_name,
						      G_CALLBACK (signals[i].func),
						      dialog);
		} else {
			sig = g_signal_connect (G_OBJECT (w),
						signals[i].signal_name,
						G_CALLBACK (signals[i].func),
						dialog);
		}
		
		if (G_UNLIKELY (!sig))
			g_error ("Error connecting signal `%s' in widget `%s'",
				 signals[i].signal_name, signals[i].widget);
	}
}

void
gst_dialog_connect_signals (GstDialog *dialog, GstDialogSignal *signals)
{
	dialog_connect_signals (dialog, signals, FALSE);
}

void
gst_dialog_connect_signals_after (GstDialog *dialog, GstDialogSignal *signals)
{
	dialog_connect_signals (dialog, signals, TRUE);
}
