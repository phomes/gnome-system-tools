/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* Copyright (C) 2004 Carlos Garnacho
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
 * Authors: Carlos Garnacho Parro  <carlosg@gnome.org>
 */

#include "gst.h"
#include "gst-network-tool.h"
#include "network-iface.h"
#include "ifaces-list.h"

extern GstTool *tool;

void
on_table_selection_changed (GtkTreeSelection *selection, gpointer data)
{
  GtkWidget    *properties, *activate, *deactivate;
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GstIface     *iface;
  gboolean      enabled, configured;
  gboolean      state_properties, state_activate, state_deactivate;

  properties = gst_dialog_get_widget (tool->main_dialog, "properties_button");
  activate   = gst_dialog_get_widget (tool->main_dialog, "activate_button");
  deactivate = gst_dialog_get_widget (tool->main_dialog, "deactivate_button");

  state_properties = state_activate = state_deactivate = FALSE;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter,
			  COL_OBJECT, &iface,
			  COL_CONFIGURED, &configured,
			  -1);
      enabled = gst_iface_get_enabled (iface);
      g_object_unref (iface);

      state_properties = TRUE;
      state_activate = configured && !enabled;
      state_deactivate = configured && enabled;
    }

  gtk_widget_set_sensitive (properties, state_properties);
  gtk_widget_set_sensitive (activate,   state_activate);
  gtk_widget_set_sensitive (deactivate, state_deactivate);
}

void
on_iface_properties_clicked (GtkWidget *widget, gpointer data)
{
  GstConnectionDialog *dialog;
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GstIface *iface;

  selection = gtk_tree_view_get_selection (GST_NETWORK_TOOL (tool)->interfaces_list);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter,
			  COL_OBJECT, &iface,
			  -1);

      dialog = GST_NETWORK_TOOL (tool)->dialog;
      connection_dialog_prepare (dialog, iface);
      g_object_unref (iface);

      gtk_window_set_transient_for (GTK_WINDOW (dialog->dialog), GTK_WINDOW (tool->main_dialog));
      gtk_widget_show (dialog->dialog);
    }
}

void
on_iface_active_changed (GtkWidget *widget, gpointer data)
{
  GstConnectionDialog *dialog;
  gboolean active;

  active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
  dialog = GST_NETWORK_TOOL (tool)->dialog;

  gtk_widget_set_sensitive (dialog->wireless_frame, active);
  gtk_widget_set_sensitive (dialog->ethernet_frame, active);
  gtk_widget_set_sensitive (dialog->plip_frame, active);
  gtk_widget_set_sensitive (dialog->modem_frame, active);
  gtk_widget_set_sensitive (dialog->account_page, active);
  gtk_widget_set_sensitive (dialog->options_page, active);
}

void
on_bootproto_changed (GtkWidget *widget, gpointer data)
{
  GstConnectionDialog *dialog;
  gboolean enabled;
  gint     pos;

  dialog = GST_NETWORK_TOOL (tool)->dialog;
  pos = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
  enabled = (pos == 0);

  gtk_widget_set_sensitive (dialog->address, enabled);
  gtk_widget_set_sensitive (dialog->netmask, enabled);
  gtk_widget_set_sensitive (dialog->gateway, enabled);
}

void
on_connection_cancel_clicked (GtkWidget *widget, gpointer data)
{
  GstConnectionDialog *dialog;

  dialog = GST_NETWORK_TOOL (tool)->dialog;
  g_object_unref (dialog->iface);
  gtk_widget_hide (dialog->dialog);
}

void
on_connection_ok_clicked (GtkWidget *widget, gpointer data)
{
  GstConnectionDialog *dialog;
  GtkTreeSelection    *selection;
  GtkTreeModel        *model;
  GtkTreeIter          iter;

  dialog = GST_NETWORK_TOOL (tool)->dialog;

  /* FIXME: add checks to the dialog */
  if (TRUE)
    {
      connection_save (dialog);
      g_object_unref (dialog->iface);
      gtk_widget_hide (dialog->dialog);

      selection = gtk_tree_view_get_selection (GST_NETWORK_TOOL (tool)->interfaces_list);

      if (gtk_tree_selection_get_selected (selection, &model, &iter))
	ifaces_model_modify_interface_at_iter (&iter);

      gst_dialog_modify (tool->main_dialog);
    }
}

static void
do_popup_menu (GtkWidget *table, GtkWidget *popup, GdkEventButton *event)
{
  gint button, event_time;

  if (!popup)
    return;

  if (event)
    {
      button     = event->button;
      event_time = event->time;
    }
  else
    {
      button     = 0;
      event_time = gtk_get_current_event_time ();
    }

  ifaces_list_setup_popup (table);

  gtk_menu_popup (GTK_MENU (popup), NULL, NULL, NULL, NULL,
		  button, event_time);
}

gboolean
on_table_button_press (GtkWidget *table, GdkEventButton *event, gpointer data)
{
  GtkTreePath      *path;
  GtkWidget        *popup;
  GtkTreeSelection *selection;

  popup = (GtkWidget *) data;
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (table));

  if (event->button == 3)
    {
      gtk_widget_grab_focus (table);

      if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (table),
					 event->x, event->y,
					 &path, NULL, NULL, NULL))
        {
	  gtk_tree_selection_unselect_all (selection);
	  gtk_tree_selection_select_path (selection, path);

	  do_popup_menu (table, popup, event);
	}

      return TRUE;
    }

  if (event->type == GDK_2BUTTON_PRESS || event->type == GDK_3BUTTON_PRESS) {
    on_iface_properties_clicked (NULL, NULL);
    return TRUE;
  }

  return FALSE;
}

gboolean
on_table_popup_menu (GtkWidget *widget, GtkWidget *popup)
{
  do_popup_menu (widget, popup, NULL);
  return TRUE;
}

void
on_activate_button_clicked (GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy (GST_NETWORK_TOOL (tool)->interfaces_list);
}

void
on_deactivate_button_clicked (GtkWidget *widget, gpointer data)
{
  GtkTreeSelection *selection;
  GtkTreeModel *model;
  GtkTreeIter   iter;
  GstIface     *iface;

  selection = gtk_tree_view_get_selection (GST_NETWORK_TOOL (tool)->interfaces_list);

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter,
			  COL_OBJECT, &iface,
			  -1);

      gst_iface_disable (iface);
      ifaces_model_modify_interface_at_iter (&iter);
      g_object_unref (iface);

      g_signal_emit_by_name (G_OBJECT (selection), "changed");
    }
}
