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

#ifndef __CONNECTION_H
#define __CONNECTION_H

#include "network-iface.h"

typedef struct _GstConnectionDialog GstConnectionDialog;

struct _GstConnectionDialog {
  GstIface  *iface;
  GtkWidget *dialog;

  /* notebook pages */
  GtkWidget *notebook;
  GtkWidget *general_page;
  GtkWidget *ppp_general_page;
  GtkWidget *account_page;
  GtkWidget *options_page;

  GtkWidget *connection_configured;
  GtkWidget *connection_device;

  /* ethernet */
  GtkWidget *bootproto_combo;
  GtkWidget *address;
  GtkWidget *netmask;
  GtkWidget *gateway;

  /* wireless */
  GtkWidget *essid;
  GtkWidget *wep_key;

  /* plip */
  GtkWidget *local_address;
  GtkWidget *remote_address;

  /* frames */
  GtkWidget *wireless_frame;
  GtkWidget *ethernet_frame;
  GtkWidget *plip_frame;
  GtkWidget *modem_frame;
};

GstConnectionDialog *connection_dialog_init (void);
void connection_dialog_prepare    (GstConnectionDialog*, GstIface*);
void connection_save (GstConnectionDialog*);

#endif /* __CONNECTION_H */
