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

#include "network-iface-modem.h"
#include "gst.h"

struct _GstIfaceModemPriv
{
  GdkPixbuf *pixbuf;
};

static void gst_iface_modem_class_init (GstIfaceModemClass *class);
static void gst_iface_modem_init       (GstIfaceModem      *iface);
static void gst_iface_modem_finalize   (GObject            *object);

static const GdkPixbuf* gst_iface_modem_get_pixbuf   (GstIface *iface);
static gchar*           gst_iface_modem_get_desc     (GstIface *iface);
static gboolean         gst_iface_modem_has_gateway  (GstIface *iface);

static gpointer parent_class;

GType
gst_iface_modem_get_type (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo iface_modem_info =
        {
          sizeof (GstIfaceModemClass),
          NULL,		/* base_init */
          NULL,		/* base_finalize */
          (GClassInitFunc) gst_iface_modem_class_init,
          NULL,		/* class_finalize */
          NULL,		/* class_data */
          sizeof (GstIfaceModem),
          0,		/* n_preallocs */
          (GInstanceInitFunc) gst_iface_modem_init,
        };

      type = g_type_register_static (GST_TYPE_IFACE, "GstIfaceModem",
                                     &iface_modem_info, 0);
    }

  return type;
}

static void
gst_iface_modem_class_init (GstIfaceModemClass *class)
{
  GObjectClass  *object_class = G_OBJECT_CLASS (class);
  GstIfaceClass *iface_class  = GST_IFACE_CLASS (class);

  parent_class = g_type_class_peek_parent (class);

  object_class->finalize = gst_iface_modem_finalize;

  iface_class->get_iface_pixbuf = gst_iface_modem_get_pixbuf;
  iface_class->get_iface_desc   = gst_iface_modem_get_desc;
  iface_class->has_gateway      = gst_iface_modem_has_gateway;
}

static void
gst_iface_modem_init (GstIfaceModem *iface)
{
  g_return_if_fail (GST_IS_IFACE_MODEM (iface));

  iface->_priv = g_new0 (GstIfaceModemPriv, 1);
  iface->_priv->pixbuf = gdk_pixbuf_new_from_file (PIXMAPS_DIR "/ppp.png", NULL);
}

static void
gst_iface_modem_finalize (GObject *object)
{
  GstIfaceModem *iface = GST_IFACE_MODEM (object);

  g_return_if_fail (GST_IS_IFACE_MODEM (object));

  if (G_OBJECT_CLASS (parent_class)->finalize)
    (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static const GdkPixbuf*
gst_iface_modem_get_pixbuf (GstIface *iface)
{
  return GST_IFACE_MODEM (iface)->_priv->pixbuf;
}

static gchar*
gst_iface_modem_get_desc (GstIface *iface)
{
  return _("Modem connection");
}

static gboolean
gst_iface_modem_has_gateway (GstIface *iface)
{
  return FALSE;
}

void
gst_iface_modem_set_config_from_xml (GstIfaceModem *iface,
				     xmlNodePtr     node)
{
  /* config the parent class data */
  gst_iface_set_config_from_xml (GST_IFACE (iface), node);

}

GstIfaceModem*
gst_iface_modem_new_from_xml (xmlNodePtr node)
{
  GstIfaceModem *iface;

  iface = g_object_new (GST_TYPE_IFACE_MODEM, NULL);
  gst_iface_modem_set_config_from_xml (iface, node);

  return iface;
}
