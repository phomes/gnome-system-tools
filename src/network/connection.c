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
#include "connection.h"

extern GstTool *tool;

/* helper for getting whether the string or null if it's empty */
static gchar*
get_entry_text (GtkWidget *entry)
{
  gchar *str;

  str = (gchar *) gtk_entry_get_text (GTK_ENTRY (entry));
  return (!str || !*str) ? NULL : str;
}

static void
connection_set_bootproto (GstConnectionDialog *dialog, GstBootProto bootproto)
{
  gtk_combo_box_set_active (GTK_COMBO_BOX (dialog->bootproto_combo),
			    (bootproto == GST_BOOTPROTO_DHCP) ? 1 : 0);
}

static GstBootProto
connection_get_bootproto (GstConnectionDialog *dialog)
{
  gint ret;

  ret = gtk_combo_box_get_active (GTK_COMBO_BOX (dialog->bootproto_combo));
  return (ret == 0) ? GST_BOOTPROTO_STATIC : GST_BOOTPROTO_DHCP;
}

static void
connection_bootproto_init (GstConnectionDialog *dialog)
{
  gtk_combo_box_append_text (GTK_COMBO_BOX (dialog->bootproto_combo), _("Static IP address"));
  gtk_combo_box_append_text (GTK_COMBO_BOX (dialog->bootproto_combo), _("DHCP"));
}

static void
ethernet_dialog_prepare (GstConnectionDialog *dialog)
{
  gchar        *address, *netmask, *gateway;
  GstBootProto  bootproto;
  gboolean      enabled;

  g_object_get (G_OBJECT (dialog->iface),
		"iface-address",   &address,
		"iface-netmask",   &netmask,
		"iface-gateway",   &gateway,
		"iface-bootproto", &bootproto,
		NULL);

  connection_set_bootproto (dialog, bootproto);
  gtk_entry_set_text (GTK_ENTRY (dialog->address), (address) ? address : "");
  gtk_entry_set_text (GTK_ENTRY (dialog->netmask), (netmask) ? netmask : "");
  gtk_entry_set_text (GTK_ENTRY (dialog->gateway), (gateway) ? gateway : "");

  g_free (address);
  g_free (netmask);
  g_free (gateway);
}

static void
ethernet_dialog_save (GstConnectionDialog *dialog)
{
  g_object_set (G_OBJECT (dialog->iface),
		"iface-address",   get_entry_text (dialog->address),
		"iface-netmask",   get_entry_text (dialog->netmask),
		"iface-gateway",   get_entry_text (dialog->gateway),
		"iface-bootproto", connection_get_bootproto (dialog),
		NULL);
}

static void
wireless_dialog_prepare (GstConnectionDialog *dialog)
{
  gchar *essid, *key;

  g_object_get (G_OBJECT (dialog->iface),
		"iface-essid", &essid,
		"iface-wep-key",   &key,
		NULL);

  gtk_entry_set_text (GTK_ENTRY (dialog->essid), (essid) ? essid : "");
  gtk_entry_set_text (GTK_ENTRY (dialog->wep_key), (key) ? key : "");

  g_free (essid);
  g_free (key);
}

static void
wireless_dialog_save (GstConnectionDialog *dialog)
{
  g_object_set (G_OBJECT (dialog->iface),
		"iface-essid",   get_entry_text (dialog->essid),
		"iface-wep-key", get_entry_text (dialog->wep_key),
		NULL);
}

static void
plip_dialog_prepare (GstConnectionDialog *dialog)
{
  gchar *local_address, *remote_address;

  g_object_get (G_OBJECT (dialog->iface),
		"iface-local-address",  &local_address,
		"iface-remote-address", &remote_address,
		NULL);

  gtk_entry_set_text (GTK_ENTRY (dialog->local_address),
		      (local_address) ? local_address : "");
  gtk_entry_set_text (GTK_ENTRY (dialog->remote_address),
		      (remote_address) ? remote_address : "");

  g_free (local_address);
  g_free (remote_address);
}

static void
plip_dialog_save (GstConnectionDialog *dialog)
{
  g_object_set (G_OBJECT (dialog->iface),
		"iface-local-address",  get_entry_text (dialog->local_address),
		"iface-remote-address", get_entry_text (dialog->remote_address),
		NULL);
}

GstConnectionDialog*
connection_dialog_init (void)
{
  GstConnectionDialog *gcd;

  gcd = g_new0 (GstConnectionDialog, 1);

  gcd->iface  = NULL;
  gcd->dialog = gst_dialog_get_widget (tool->main_dialog, "connection_config_dialog");

  gcd->notebook         = gst_dialog_get_widget (tool->main_dialog, "connection_notebook");
  gcd->general_page     = gst_dialog_get_widget (tool->main_dialog, "connection_general_page");
  gcd->ppp_general_page = gst_dialog_get_widget (tool->main_dialog, "connection_ppp_general_page");
  gcd->account_page     = gst_dialog_get_widget (tool->main_dialog, "connection_account_page");
  gcd->options_page     = gst_dialog_get_widget (tool->main_dialog, "connection_options_page");

  gcd->connection_configured = gst_dialog_get_widget (tool->main_dialog, "connection_device_active");
  gcd->connection_device     = gst_dialog_get_widget (tool->main_dialog, "connection_device");

  gcd->bootproto_combo = gst_dialog_get_widget (tool->main_dialog, "connection_bootproto");
  gcd->address = gst_dialog_get_widget (tool->main_dialog, "connection_address");
  gcd->netmask = gst_dialog_get_widget (tool->main_dialog, "connection_netmask");
  gcd->gateway = gst_dialog_get_widget (tool->main_dialog, "connection_gateway");

  gcd->essid   = gst_dialog_get_widget (tool->main_dialog, "connection_essid");
  gcd->wep_key = gst_dialog_get_widget (tool->main_dialog, "connection_wep_key");

  gcd->local_address  = gst_dialog_get_widget (tool->main_dialog, "connection_local_address");
  gcd->remote_address = gst_dialog_get_widget (tool->main_dialog, "connection_remote_address");

  gcd->wireless_frame = gst_dialog_get_widget (tool->main_dialog, "connection_wireless");
  gcd->ethernet_frame = gst_dialog_get_widget (tool->main_dialog, "connection_ethernet");
  gcd->plip_frame     = gst_dialog_get_widget (tool->main_dialog, "connection_plip");
  gcd->modem_frame    = gst_dialog_get_widget (tool->main_dialog, "connection_modem");

  connection_bootproto_init (gcd);

  return gcd;
}

void
connection_dialog_prepare (GstConnectionDialog *dialog, GstIface *iface)
{
  dialog->iface = iface;
  
  /* FIXME: at this moment there isn't modem connection implemented */

  gtk_widget_hide (dialog->ppp_general_page);
  gtk_widget_hide (dialog->account_page);
  gtk_widget_hide (dialog->options_page);

  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (dialog->notebook), FALSE);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (dialog->notebook), FALSE);

  gtk_container_set_border_width (GTK_CONTAINER (dialog->general_page), 0);
  gtk_container_set_border_width (GTK_CONTAINER (dialog->notebook), 0);

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (dialog->connection_configured),
				gst_iface_is_configured (iface));

  gtk_label_set_text (GTK_LABEL (dialog->connection_device),
		      gst_iface_get_dev (iface));

  if (GST_IS_IFACE_WIRELESS (iface))
    {
      gtk_widget_show (dialog->wireless_frame);
      gtk_widget_show (dialog->ethernet_frame);
      gtk_widget_hide (dialog->plip_frame);

      wireless_dialog_prepare (dialog);
      ethernet_dialog_prepare (dialog);
    }
  else if (GST_IS_IFACE_ETHERNET (iface)
	   || GST_IS_IFACE_IRLAN (iface))
    {
      gtk_widget_hide (dialog->wireless_frame);
      gtk_widget_show (dialog->ethernet_frame);
      gtk_widget_hide (dialog->plip_frame);

      ethernet_dialog_prepare (dialog);
    }
  else if (GST_IS_IFACE_PLIP (iface))
    {
      gtk_widget_hide (dialog->wireless_frame);
      gtk_widget_hide (dialog->ethernet_frame);
      gtk_widget_show (dialog->plip_frame);

      plip_dialog_prepare (dialog);
    }
}

void
connection_save (GstConnectionDialog *dialog)
{
  gst_iface_set_configured (dialog->iface,
			    gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (dialog->connection_configured)));

  if (GST_IS_IFACE_WIRELESS (dialog->iface))
    {
      wireless_dialog_save (dialog);
      ethernet_dialog_save (dialog);
    }
  else if (GST_IS_IFACE_ETHERNET (dialog->iface)
	   || GST_IS_IFACE_IRLAN (dialog->iface))
    {
      ethernet_dialog_save (dialog);
    }
  else if (GST_IS_IFACE_PLIP (dialog->iface))
    {
      plip_dialog_save (dialog);
    }
}
