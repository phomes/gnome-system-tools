/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 2 -*- */
/* Copyright (C) 2004 Carlos Garnacho Parro.
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
#include "address-list.h"
#include "gst-network-tool.h"

static void
transfer_address_list_to_gui (xmlNodePtr root, gchar *tag, GstAddressList *al)
{
  xmlNodePtr  node;
  gchar      *address;

  for (node = gst_xml_element_find_first (root, tag);
       node; node = gst_xml_element_find_next (node, tag))
    {
      address = gst_xml_element_get_content (node);
      gst_address_list_add_address (al, address);
    }
}

static void
transfer_string_to_entry (xmlNodePtr root, gchar *tag, GtkEntry *entry)
{
  gchar *text;

  if (text = gst_xml_get_child_content (root, tag))
    {
      gtk_entry_set_text (entry, text);
      g_free (text);
    }
}

static void
transfer_gateway_to_combo (xmlNodePtr root)
{
  gchar *text;

  if (text = gst_xml_get_child_content (root, "gatewaydev"))
    {
      gateways_combo_select (text);
      g_free (text);
    }
}

static void
transfer_interfaces_to_list (xmlNodePtr root)
{
  xmlNodePtr node;

  for (node = gst_xml_element_find_first (root, "interface");
       node; node = gst_xml_element_find_next (node, "interface"))
    {
      ifaces_model_add_interface_from_node (node);
    }
}

void
transfer_xml_to_gui (GstTool *tool, gpointer data)
{
  GstNetworkTool *network_tool;
  xmlNode *root;

  network_tool = GST_NETWORK_TOOL (tool);
  root = gst_xml_doc_get_root (tool->config);

  transfer_address_list_to_gui (root, "nameserver", network_tool->dns);
  transfer_address_list_to_gui (root, "search", network_tool->search);

  transfer_string_to_entry (root, "hostname", network_tool->hostname);
  transfer_string_to_entry (root, "domain", network_tool->domain);

  transfer_interfaces_to_list (root);
  transfer_gateway_to_combo (root);
}
