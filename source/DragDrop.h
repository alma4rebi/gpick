/*
 * Copyright (c) 2009, Albertas Vyšniauskas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of the software author nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DRAGDROP_H_
#define DRAGDROP_H_

#include <gtk/gtk.h>
#include "ColorObject.h"

enum DragDropFlags{
	DRAGDROP_SOURCE = 1<<1,
	DRAGDROP_DESTINATION = 1<<2,
};

struct DragDrop{
	GtkWidget* widget;
	void* userdata;
	
	struct ColorObject* (*get_color_object)(struct DragDrop* dd);
	int (*set_color_object_at)(struct DragDrop* dd, struct ColorObject* colorobject, int x, int y);
	bool (*test_at)(struct DragDrop* dd, int x, int y);
	bool (*data_received)(struct DragDrop* dd, GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint target_type, guint time);
	bool (*data_get)(struct DragDrop* dd, GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint target_type, guint time);
	bool (*data_delete)(struct DragDrop* dd, GtkWidget *widget, GdkDragContext *context);
		
	struct dynvHandlerMap* handler_map;
};

int dragdrop_init(struct DragDrop* dd);
int dragdrop_widget_attach(GtkWidget* widget, DragDropFlags flags, struct DragDrop *dd);


#endif /* DRAGDROP_H_ */