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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <glade/glade.h>

#include "gst.h"
#include "gst-network-tool.h"
#include "transfer.h"
#include "ifaces-list.h"
#include "callbacks.h"

GstTool *tool;

static GstDialogSignal signals[] = {
  /* connections tab */
  { "properties_button",            "clicked", G_CALLBACK (on_iface_properties_clicked) },
  { "activate_button",              "clicked", G_CALLBACK (on_activate_button_clicked) },
  { "deactivate_button",            "clicked", G_CALLBACK (on_deactivate_button_clicked) },
  /* general tab */
  { "hostname",                     "changed", G_CALLBACK (gst_dialog_modify_cb) },
  { "domain",                       "changed", G_CALLBACK (gst_dialog_modify_cb) },
  /* host aliases tab */
  { "host_aliases_add",             "clicked", G_CALLBACK (on_host_aliases_add_clicked) },
  { "host_aliases_properties",      "clicked", G_CALLBACK (on_host_aliases_properties_clicked) },
  { "host_aliases_delete",          "clicked", G_CALLBACK (on_host_aliases_delete_clicked) },
  /* connection dialog */
  { "connection_device_active",     "clicked", G_CALLBACK (on_iface_active_changed) },
  { "connection_ppp_device_active", "clicked", G_CALLBACK (on_iface_active_changed) },
  { "connection_bootproto",         "changed", G_CALLBACK (on_bootproto_changed) },
  { "connection_cancel",            "clicked", G_CALLBACK (on_connection_cancel_clicked) },
  { "connection_ok",                "clicked", G_CALLBACK (on_connection_ok_clicked) }
};

int
main (int argc, gchar *argv[])
{
  gst_init ("network-admin", argc, argv, NULL);

  tool = gst_network_tool_new ();
  gst_network_tool_construct (GST_NETWORK_TOOL (tool), "network", _("Network settings"));

  gst_tool_set_xml_funcs (tool, transfer_xml_to_gui, transfer_gui_to_xml, NULL);
  gst_dialog_connect_signals (tool->main_dialog, signals);
  gst_tool_main (tool, FALSE);

  return 0;
}
