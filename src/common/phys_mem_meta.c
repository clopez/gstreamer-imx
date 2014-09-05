/* GStreamer meta data structure for physical memory information
 * Copyright (C) 2013  Carlos Rafael Giani
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "phys_mem_meta.h"


static gboolean gst_imx_phys_mem_meta_init(GstMeta *meta, G_GNUC_UNUSED gpointer params, G_GNUC_UNUSED GstBuffer *buffer)
{
	GstImxPhysMemMeta *imx_phys_mem_meta = (GstImxPhysMemMeta *)meta;
	imx_phys_mem_meta->phys_addr = 0;
	imx_phys_mem_meta->padding = 0;
	imx_phys_mem_meta->parent = NULL;
	return TRUE;
}


GType gst_imx_phys_mem_meta_api_get_type(void)
{
	static volatile GType type;
	static gchar const *tags[] = { "memory", "phys_mem", NULL };

	if (g_once_init_enter(&type))
	{
		GType _type = gst_meta_api_type_register("GstImxPhysMemMetaAPI", tags);
		g_once_init_leave(&type, _type);
	}

	return type;
}

static gboolean gst_imx_phys_meta_transform(GstBuffer *dest, GstMeta *meta, GstBuffer *buffer, GQuark type, gpointer data)
{
	GstImxPhysMemMeta *dmeta, *smeta;

	smeta = (GstImxPhysMemMeta *)meta;

	if (GST_META_TRANSFORM_IS_COPY(type))
	{
		GstMetaTransformCopy *copy = data;
		gboolean do_copy = FALSE;

		/* only copy if both buffers have 1 identical memory */
		if ((gst_buffer_n_memory(dest) == gst_buffer_n_memory(buffer)) && (gst_buffer_n_memory(dest) == 1))
		{
			GstMemory *mem1, *mem2;

			mem1 = gst_buffer_get_memory(dest, 0);
			mem2 = gst_buffer_get_memory(buffer, 0);
			if (mem1 == mem2)
				do_copy = TRUE;
			gst_memory_unref(mem1);
			gst_memory_unref(mem2);
		}

		if (!copy->region && do_copy)
		{
			/* only copy if the complete data is copied as well */
			dmeta = (GstImxPhysMemMeta *)gst_buffer_add_meta(dest, gst_imx_phys_mem_meta_get_info(), NULL);

			if (!dmeta)
				return FALSE;

			GST_DEBUG("copy phys metadata");
			dmeta->phys_addr = smeta->phys_addr;
			dmeta->padding = smeta->padding;
			dmeta->x_padding = smeta->x_padding;
			dmeta->y_padding = smeta->y_padding;
			if (smeta->parent)
				dmeta->parent = gst_buffer_ref(smeta->parent);
			else
				dmeta->parent = gst_buffer_ref(buffer);
		}
	}

	return TRUE;
}


static void gst_imx_phys_meta_free(GstMeta *meta, G_GNUC_UNUSED GstBuffer *buffer)
{
	GstImxPhysMemMeta *smeta = (GstImxPhysMemMeta *)meta;
	gst_buffer_replace(&smeta->parent, NULL);
}


GstMetaInfo const * gst_imx_phys_mem_meta_get_info(void)
{
	static GstMetaInfo const *gst_imx_phys_mem_meta_info = NULL;

	if (g_once_init_enter(&gst_imx_phys_mem_meta_info))
	{
		GstMetaInfo const *meta = gst_meta_register(
			gst_imx_phys_mem_meta_api_get_type(),
			"GstImxPhysMemMeta",
			sizeof(GstImxPhysMemMeta),
			GST_DEBUG_FUNCPTR(gst_imx_phys_mem_meta_init),
			GST_DEBUG_FUNCPTR(gst_imx_phys_meta_free),
			GST_DEBUG_FUNCPTR(gst_imx_phys_meta_transform)
		);
		g_once_init_leave(&gst_imx_phys_mem_meta_info, meta);
	}

	return gst_imx_phys_mem_meta_info;
}

