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

#include <gtk/gtk.h>
#include <gdk/gdkpixbuf.h>
#include "gst.h"
#include "network-iface.h"
#include "callbacks.h"
#include "ifaces-list.h"
#include "gst-network-tool.h"

extern GstTool *tool;

GtkActionEntry popup_menu_items [] = {
  { "Properties",  GTK_STOCK_PROPERTIES, N_("_Properties"), NULL, NULL, G_CALLBACK (on_iface_properties_clicked) },
  { "Activate",    GTK_STOCK_EXECUTE,    N_("_Activate"),   NULL, NULL, G_CALLBACK (on_iface_properties_clicked) },
  { "Deactivate",  GTK_STOCK_STOP,       N_("_Deactivate"), NULL, NULL, G_CALLBACK (on_iface_properties_clicked) }
};

const gchar *ui_description =
  "<ui>"
  "  <popup name='MainMenu'>"
  "    <menuitem action='Properties'/>"
  "    <separator/>"
  "    <menuitem action='Activate'/>"
  "    <menuitem action='Deactivate'/>"
  "  </popup>"
  "</ui>";

GtkTreeModel*
ifaces_model_create (void)
{
  GtkListStore *store;

  store = gtk_list_store_new (COL_LAST,
			      GDK_TYPE_PIXBUF,
			      G_TYPE_STRING,
			      G_TYPE_OBJECT,
			      G_TYPE_STRING,
			      G_TYPE_BOOLEAN,
			      G_TYPE_BOOLEAN,
			      G_TYPE_BOOLEAN);
  return GTK_TREE_MODEL (store);
}

static gboolean
gateways_filter_func (GtkTreeModel *model,
		      GtkTreeIter  *iter,
		      gpointer      data)
{
  gchar *dev;
  gboolean configured, enabled, has_gateway;

  gtk_tree_model_get (model, iter,
		      COL_DEV,         &dev,
		      COL_CONFIGURED,  &configured,
		      COL_ENABLED,     &enabled,
		      COL_HAS_GATEWAY, &has_gateway,
		      -1);

  return configured && enabled && has_gateway;
}


GtkTreeModelFilter*
gateways_filter_model_create (GtkTreeModel *model)
{
  GtkTreeModelFilter *filter_model;

  filter_model = GTK_TREE_MODEL_FILTER (gtk_tree_model_filter_new (model, NULL));

  gtk_tree_model_filter_set_visible_func (filter_model,
					  gateways_filter_func,
					  NULL,
					  NULL);
  return filter_model;
}

static GtkWidget*
popup_menu_create (GtkWidget *widget)
{
  GtkUIManager   *ui_manager;
  GtkActionGroup *action_group;
  GtkWidget      *popup;

  action_group = gtk_action_group_new ("MenuActions");
  gtk_action_group_add_actions (action_group, popup_menu_items, G_N_ELEMENTS (popup_menu_items), widget);

  ui_manager = gtk_ui_manager_new ();
  gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);

  if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, NULL))
    return NULL;

  g_object_set_data (G_OBJECT (widget), "ui-manager", ui_manager);
  popup = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");

  return popup;
}

void
ifaces_list_setup_popup (GtkWidget *table)
{
  GtkUIManager     *ui_manager;
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkTreeIter       iter;
  gboolean          enabled, configured;

  ui_manager = (GtkUIManager *) g_object_get_data (G_OBJECT (table), "ui-manager");
  selection  = gtk_tree_view_get_selection (GTK_TREE_VIEW (table));

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gtk_tree_model_get (model, &iter,
			  COL_CONFIGURED, &configured,
			  COL_ENABLED,    &enabled,
			  -1);
      gtk_widget_set_sensitive (
	gtk_ui_manager_get_widget (ui_manager, "/MainMenu/Activate"), configured && !enabled);
      gtk_widget_set_sensitive (
	gtk_ui_manager_get_widget (ui_manager, "/MainMenu/Deactivate"), configured && enabled);
    }
}

static void
add_list_columns (GtkTreeView *table)
{
  GtkCellRenderer *renderer;

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (table), -1,
					       "Interface",
					       renderer,
					       "pixbuf", COL_IMAGE,
					       "sensitive", COL_CONFIGURED,
					       NULL);

  /* bogus cellrenderer for showing everything unsensitive,
     but still allowing user interaction.
  */
  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (table), -1,
					       "",
					       renderer,
					       NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set (G_OBJECT (renderer), "ellipsize", PANGO_ELLIPSIZE_END, NULL);
  
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (table), -1,
					       "Interface",
					       renderer,
					       "markup", COL_DESC,
					       "sensitive", COL_CONFIGURED,
					       NULL);
}

GtkTreeView*
ifaces_list_create (void)
{
  GtkWidget        *table = gst_dialog_get_widget (tool->main_dialog, "interfaces_list");
  GtkTreeSelection *selection;
  GtkTreeModel     *model;
  GtkWidget        *popup;

  model = GST_NETWORK_TOOL (tool)->interfaces_model;
  gtk_tree_view_set_model (GTK_TREE_VIEW (table), model);
  g_object_unref (G_OBJECT (model));

  add_list_columns (GTK_TREE_VIEW (table));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (table));
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (on_table_selection_changed), NULL);

  popup = popup_menu_create (table);

  g_signal_connect (G_OBJECT (table), "button-press-event",
		    G_CALLBACK (on_table_button_press), (gpointer) popup);
  g_signal_connect (G_OBJECT (table), "popup_menu",
		    G_CALLBACK (on_table_popup_menu), (gpointer) popup);

  return GTK_TREE_VIEW (table);
}

void
ifaces_model_add_interface_from_node (xmlNodePtr node)
{
  gchar    *type;
  GstIface *iface = NULL;

  type = gst_xml_element_get_attribute (node, "type");
  g_return_if_fail (type != NULL);

  if (strcmp (type, "ethernet") == 0)
    iface = GST_IFACE (gst_iface_ethernet_new_from_xml (node));
  else if (strcmp (type, "wireless") == 0)
    iface = GST_IFACE (gst_iface_wireless_new_from_xml (node));
  else if (strcmp (type, "irlan") == 0)
    iface = GST_IFACE (gst_iface_irlan_new_from_xml (node));
  else if (strcmp (type, "plip") == 0)
    iface = GST_IFACE (gst_iface_plip_new_from_xml (node));
  else if (strcmp (type, "modem") == 0)
    iface = GST_IFACE (gst_iface_modem_new_from_xml (node));

  if (iface)
    {
      ifaces_model_add_interface (iface);
      g_object_unref (iface);
      g_free (type);
    }
}

static void
update_gateways_combo (void)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gint          count = 0;
  gboolean      valid;

  /* refilter the gateways model */
  gtk_tree_model_filter_refilter (GST_NETWORK_TOOL (tool)->gateways_model);

  /* put sensitive/unsensitive the combo depending on the items number */
  model = GTK_TREE_MODEL (GST_NETWORK_TOOL (tool)->gateways_model);
  valid = gtk_tree_model_get_iter_first (model, &iter);

  while (valid)
    {
      count++;
      valid = gtk_tree_model_iter_next (model, &iter);
    }

  gtk_widget_set_sensitive (GTK_WIDGET (GST_NETWORK_TOOL (tool)->gateways_list), count > 0);
  gtk_widget_set_sensitive (gst_dialog_get_widget (tool->main_dialog, "gateways_combo_label"),
			    count > 0);
}

void
ifaces_model_modify_interface_at_iter (GtkTreeIter *iter)
{
  GtkTreeModel *model;
  GstIface     *iface;

  model = GST_NETWORK_TOOL (tool)->interfaces_model;

  gtk_tree_model_get (model, iter, COL_OBJECT, &iface, -1);

  gtk_list_store_set (GTK_LIST_STORE (model), iter,
		      COL_IMAGE, gst_iface_get_pixbuf (GST_IFACE (iface)),
		      COL_DESC, gst_iface_get_desc (GST_IFACE (iface)),
		      COL_DEV, gst_iface_get_dev (GST_IFACE (iface)),
		      COL_CONFIGURED, gst_iface_is_configured (GST_IFACE (iface)),
		      COL_ENABLED, gst_iface_get_enabled (GST_IFACE (iface)),
		      COL_HAS_GATEWAY, gst_iface_has_gateway (GST_IFACE (iface)),
		      -1);
  g_object_unref (iface);

  update_gateways_combo ();
}

void
ifaces_model_add_interface (GstIface *iface)
{
  GtkTreeModel *model;
  GtkTreeIter   iter;

  g_return_if_fail (iface != NULL);

  model = GST_NETWORK_TOOL (tool)->interfaces_model;

  gtk_list_store_append (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      COL_OBJECT, iface,
		      -1);
  ifaces_model_modify_interface_at_iter (&iter);
}

static void
add_combo_layout (GtkComboBox *combo)
{
  GtkCellRenderer *renderer;

  gtk_cell_layout_clear (GTK_CELL_LAYOUT (combo));
/*
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo),
			      renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo),
				  renderer,
				  "pixbuf", COL_IMAGE,
				  "visible", COL_HAS_GATEWAY,
				  NULL);
  g_object_unref (renderer);
*/
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo),
			      renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo),
				  renderer,
				  "markup", COL_DEV,
				  "visible", COL_HAS_GATEWAY,
				  NULL);
  g_object_unref (renderer);
}

GtkComboBox*
gateways_combo_create (void)
{
  GtkWidget    *combo = gst_dialog_get_widget (tool->main_dialog, "gateways_combo");
  GtkTreeModel *model;

  model = GTK_TREE_MODEL (GST_NETWORK_TOOL (tool)->gateways_model);
  gtk_combo_box_set_model (GTK_COMBO_BOX (combo), model);
  g_object_unref (model);

  add_combo_layout (GTK_COMBO_BOX (combo));

  return GTK_COMBO_BOX (combo);
}

void
gateways_combo_select (gchar *dev)
{
  GtkComboBox  *combo = GST_NETWORK_TOOL (tool)->gateways_list;
  GtkTreeModel *model;
  GtkTreeIter   iter;
  gboolean      valid;
  gchar        *iter_dev;

  g_return_if_fail (dev != NULL);

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
  valid = gtk_tree_model_get_iter_first (model, &iter);

  while (valid)
    {
      gtk_tree_model_get (model, &iter,
			  COL_DEV, &iter_dev,
			  -1);

      if (iter_dev && (strcmp (dev, iter_dev) == 0))
        {
	  gtk_combo_box_set_active_iter (GTK_COMBO_BOX (combo), &iter);
	  valid = FALSE; /* just exit */
	}
      else
	valid = gtk_tree_model_iter_next (model, &iter);

      g_free (iter_dev);
    }
}
