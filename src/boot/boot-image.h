/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* boot-image.h: this file is part of boot-admin, a ximian-setup-tool frontend 
 * for boot administration.
 * 
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
 * Authors: Tambet Ingo <tambet@ximian.com>.
 */

#ifndef BOOT_IMAGE_H
#define BOOT_IMAGE_H

#include <gtk/gtk.h>
#include "gst.h"

#define MAX_IMAGES 16

typedef enum {
	TYPE_UNKNOWN,
	TYPE_WINNT,
	TYPE_WIN9X,
	TYPE_DOS,
	TYPE_LINUX,
        TYPE_LINSWAP,
	TYPE_HURD
} GstBootImageType;

typedef struct {
	const gchar *label;
	GstBootImageType type;
} GstBootImageTypeTable;

typedef struct {
	xmlNodePtr node;
	gboolean new;
	GstBootImageType type;
	gboolean is_default;
	
	gchar *label;
	gchar *image;
	gchar *root;
	gchar *append;
	gchar *initrd;
	gchar *module;
	gchar *password;
} BootImage;

BootImage        *boot_image_new          (void);
BootImage        *boot_image_get_by_node  (xmlNodePtr node);
void              boot_image_save         (BootImage *image);
void              boot_image_destroy      (BootImage *image);

gchar            *boot_image_valid_label  (BootImage *image);
gchar            *boot_image_valid_device (BootImage *image);
gchar            *boot_image_valid_root   (BootImage *image);
gchar            *boot_image_valid_initrd (BootImage *image);
gchar            *boot_image_valid_module (BootImage *image);
gchar            *boot_image_check        (BootImage *image);

/* Helpers */
gint              boot_image_count        (xmlNodePtr root);
gchar            *type_to_label           (GstBootImageType type);
GstBootImageType  label_to_type           (const gchar *label);
GList            *type_labels_list        (void);

#endif /* BOOT_IMAGE_H */