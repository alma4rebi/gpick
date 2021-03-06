/*
 * Copyright (c) 2009-2016, Albertas Vyšniauskas
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

#include "Swatch.h"
#include "../Color.h"
#include "../MathUtil.h"
#include <math.h>
#include <boost/math/special_functions/round.hpp>

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GTK_TYPE_SWATCH, GtkSwatchPrivate))
G_DEFINE_TYPE (GtkSwatch, gtk_swatch, GTK_TYPE_DRAWING_AREA);
static gboolean button_release(GtkWidget *swatch, GdkEventButton *event);
static gboolean button_press(GtkWidget *swatch, GdkEventButton *event);
#if GTK_MAJOR_VERSION >= 3
static gboolean draw(GtkWidget *widget, cairo_t *cr);
#else
static gboolean expose(GtkWidget *widget, GdkEventExpose *event);
#endif
enum
{
	ACTIVE_COLOR_CHANGED, COLOR_CHANGED, COLOR_ACTIVATED, CENTER_ACTIVATED, LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = {};
struct GtkSwatchPrivate
{
	Color color[7];
	gint32 current_color;
	gboolean active;
	transformation::Chain *transformation_chain;
#if GTK_MAJOR_VERSION >= 3
	GtkStyleContext *context;
#endif
};
static void finalize(GObject *obj)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(obj);
#if GTK_MAJOR_VERSION >= 3
	g_object_unref(ns->context);
#endif
	G_OBJECT_CLASS(g_type_class_peek_parent(G_OBJECT_CLASS(GTK_SWATCH_GET_CLASS(obj))))->finalize(obj);
}
static void gtk_swatch_class_init(GtkSwatchClass *swatch_class)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(swatch_class);
	obj_class->finalize = finalize;
	g_type_class_add_private(obj_class, sizeof(GtkSwatchPrivate));
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(swatch_class);
	widget_class->button_release_event = button_release;
	widget_class->button_press_event = button_press;
#if GTK_MAJOR_VERSION >= 3
	widget_class->draw = draw;
#else
	widget_class->expose_event = expose;
#endif
	signals[ACTIVE_COLOR_CHANGED] = g_signal_new("active_color_changed", G_OBJECT_CLASS_TYPE(obj_class), G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(GtkSwatchClass, active_color_changed), nullptr, nullptr, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
	signals[COLOR_CHANGED] = g_signal_new("color_changed", G_OBJECT_CLASS_TYPE(obj_class), G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(GtkSwatchClass, color_changed), nullptr, nullptr, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	signals[COLOR_ACTIVATED] = g_signal_new("color_activated", G_OBJECT_CLASS_TYPE(obj_class), G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(GtkSwatchClass, color_activated), nullptr, nullptr, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
	signals[CENTER_ACTIVATED] = g_signal_new("center_activated", G_OBJECT_CLASS_TYPE(obj_class), G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(GtkSwatchClass, center_activated), nullptr, nullptr, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}
#if GTK_MAJOR_VERSION >= 3
static GtkStyleContext* get_style_context(GType type)
{
	GtkWidgetPath* path = gtk_widget_path_new();
	gtk_widget_path_append_type(path, type);
	GtkStyleContext *context = gtk_style_context_new();
	gtk_style_context_set_path(context, path);
	gtk_widget_path_free(path);
	return context;
}
#endif
static void gtk_swatch_init(GtkSwatch *swatch)
{
	gtk_widget_add_events(GTK_WIDGET(swatch), GDK_2BUTTON_PRESS | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_FOCUS_CHANGE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
}
GtkWidget* gtk_swatch_new()
{
	GtkWidget* widget = (GtkWidget*)g_object_new(GTK_TYPE_SWATCH, nullptr);
	GtkSwatchPrivate *ns = GET_PRIVATE(widget);
#if GTK_MAJOR_VERSION >= 3
	gtk_widget_set_size_request(GTK_WIDGET(widget), 150, 136);
#else
	gtk_widget_set_size_request(GTK_WIDGET(widget), 150 + widget->style->xthickness * 2, 136 + widget->style->ythickness * 2);
#endif
	for (gint32 i = 0; i < 7; ++i)
		color_set(&ns->color[i], i / 7.0);
	ns->current_color = 1;
	ns->transformation_chain = 0;
	ns->active = false;
#if GTK_MAJOR_VERSION >= 3
	ns->context = get_style_context(GTK_TYPE_SWATCH);
#endif
	gtk_widget_set_can_focus(widget, true);
	return widget;
}
void gtk_swatch_set_color_to_main(GtkSwatch* swatch)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	color_copy(&ns->color[0], &ns->color[ns->current_color]);
	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}
void gtk_swatch_move_active(GtkSwatch* swatch, gint32 direction)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	if (direction < 0){
		if (ns->current_color == 1){
			ns->current_color = 7 - 1;
		}else{
			ns->current_color--;
		}
	}else{
		ns->current_color++;
		if (ns->current_color >= 7)
			ns->current_color = 1;
	}
}
void gtk_swatch_get_color(GtkSwatch* swatch, guint32 index, Color* color)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	color_copy(&ns->color[index], color);
}
void gtk_swatch_get_main_color(GtkSwatch* swatch, Color* color)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	color_copy(&ns->color[0], color);
}
gint32 gtk_swatch_get_active_index(GtkSwatch* swatch)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	return ns->current_color;
}
void gtk_swatch_get_active_color(GtkSwatch* swatch, Color* color)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	color_copy(&ns->color[ns->current_color], color);
}
void gtk_swatch_set_color(GtkSwatch* swatch, guint32 index, Color* color)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	color_copy(color, &ns->color[index]);
	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}
void gtk_swatch_set_main_color(GtkSwatch* swatch, Color* color)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	color_copy(color, &ns->color[0]);
	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}
void gtk_swatch_set_active_index(GtkSwatch* swatch, guint32 index)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	ns->current_color = index;
	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}
void gtk_swatch_set_active_color(GtkSwatch* swatch, Color* color)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	color_copy(color, &ns->color[ns->current_color]);
	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}
void gtk_swatch_set_main_color(GtkSwatch* swatch, guint index, Color* color)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(swatch);
	color_copy(color, &ns->color[0]);
	gtk_widget_queue_draw(GTK_WIDGET(swatch));
}
static int get_color_by_position(gint x, gint y)
{
	vector2 a, b;
	vector2_set(&a, 1, 0);
	vector2_set(&b, x - 75, y - (75 - 7));
	float distance = vector2_length(&b);
	if (distance < 20){ //center color
		return 0;
	}else if (distance > 70){ //outside
		return -1;
	}else{
		vector2_normalize(&b, &b);
		float angle = acos(vector2_dot(&a, &b));
		if (b.y < 0)
			angle = 2 * PI - angle;
		angle += (PI / 6) * 3;
		if (angle < 0)
			angle += PI * 2;
		if (angle > 2 * PI)
			angle -= PI * 2;
		return 1 + (int)floor(angle / ((PI * 2) / 6));
	}
}
gint gtk_swatch_get_color_at(GtkSwatch* swatch, gint x, gint y)
{
#if GTK_MAJOR_VERSION >= 3
	return get_color_by_position(x, y);
#else
	return get_color_by_position(x - GTK_WIDGET(swatch)->style->xthickness, y - GTK_WIDGET(swatch)->style->ythickness);
#endif
}
static void draw_hexagon(cairo_t *cr, float x, float y, float radius)
{
	cairo_new_sub_path(cr);
	for (int i = 0; i < 6; ++i) {
		cairo_line_to(cr, x + sin(i * PI / 3) * radius, y + cos(i * PI / 3) * radius);
	}
	cairo_close_path(cr);
}
static gboolean draw(GtkWidget *widget, cairo_t *cr)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(widget);
	if (gtk_widget_has_focus(widget) || ns->active){
#if GTK_MAJOR_VERSION >= 3
		gtk_render_focus(ns->context, cr, 0, 0, 150, 136);
#else
		gtk_paint_focus(widget->style, widget->window, GTK_STATE_ACTIVE, nullptr, widget, 0, widget->style->xthickness, widget->style->ythickness, 150, 136);
#endif
		cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
		cairo_arc(cr, 150 - 11.5, 12.5, 6, 0, 2 * M_PI);
		cairo_fill(cr);
		cairo_set_source_rgb(cr, 1, 0, 0);
		cairo_arc(cr, 150 - 12, 12, 6, 0, 2 * M_PI);
		cairo_fill(cr);
	}
	cairo_select_font_face(cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 12);
	cairo_matrix_t matrix;
	cairo_get_matrix(cr, &matrix);
#if GTK_MAJOR_VERSION >= 3
	int padding_x = 0, padding_y = 0;
#else
	int padding_x = widget->style->xthickness, padding_y = widget->style->ythickness;
#endif
	cairo_translate(cr, 75 + padding_x, 75 + padding_y - 7);
	int edges = 6;
	cairo_set_source_rgb(cr, 0, 0, 0);
	float radius_multi = 50 * cos((180 / edges) / (180 / PI));
	float rotation = -(PI/6 * 4);
	//Draw stroke
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_set_line_width(cr, 3);
	for (int i = 1; i < 7; ++i) {
		if (i == ns->current_color)
			continue;
		draw_hexagon(cr, radius_multi * cos(rotation + i * (2 * PI) / edges), radius_multi * sin(rotation + i * (2 * PI) / edges), 27);
	}
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 1, 1, 1);
	draw_hexagon(cr, radius_multi * cos(rotation + (ns->current_color) * (2 * PI) / edges), radius_multi * sin(rotation + (ns->current_color) * (2
			* PI) / edges), 27);
	cairo_stroke(cr);
	Color color;
	//Draw fill
	for (int i = 1; i < 7; ++i) {
		if (i == ns->current_color)
			continue;
		if (ns->transformation_chain){
			ns->transformation_chain->apply(&ns->color[i], &color);
		}else{
			color_copy(&ns->color[i], &color);
		}
		cairo_set_source_rgb(cr, boost::math::round(color.rgb.red * 255.0) / 255.0, boost::math::round(color.rgb.green * 255.0) / 255.0, boost::math::round(color.rgb.blue * 255.0) / 255.0);
		draw_hexagon(cr, radius_multi * cos(rotation + i * (2 * PI) / edges), radius_multi * sin(rotation + i * (2 * PI) / edges), 25.5);
		cairo_fill(cr);
	}
	if (ns->transformation_chain){
		ns->transformation_chain->apply(&ns->color[ns->current_color], &color);
	}else{
		color_copy(&ns->color[ns->current_color], &color);
	}
	cairo_set_source_rgb(cr, boost::math::round(color.rgb.red * 255.0) / 255.0, boost::math::round(color.rgb.green * 255.0) / 255.0, boost::math::round(color.rgb.blue * 255.0) / 255.0);
	draw_hexagon(cr, radius_multi * cos(rotation + (ns->current_color) * (2 * PI) / edges), radius_multi * sin(rotation + (ns->current_color) * (2 * PI) / edges), 25.5);
	cairo_fill(cr);
	//Draw center
	if (ns->transformation_chain){
		ns->transformation_chain->apply(&ns->color[0], &color);
	}else{
		color_copy(&ns->color[0], &color);
	}
	cairo_set_source_rgb(cr, boost::math::round(color.rgb.red * 255.0) / 255.0, boost::math::round(color.rgb.green * 255.0) / 255.0, boost::math::round(color.rgb.blue * 255.0) / 255.0);
	draw_hexagon(cr, 0, 0, 25.5);
	cairo_fill(cr);
	//Draw numbers
	char numb[2] = " ";
	for (int i = 1; i < 7; ++i) {
		Color c;
		if (ns->transformation_chain){
			Color t;
			ns->transformation_chain->apply(&ns->color[i], &t);
			color_get_contrasting(&t, &c);
		}else{
			color_get_contrasting(&ns->color[i], &c);
		}
		cairo_text_extents_t extends;
		numb[0] = '0' + i;
		cairo_text_extents(cr, numb, &extends);
		cairo_set_source_rgb(cr, boost::math::round(c.rgb.red * 255.0) / 255.0, boost::math::round(c.rgb.green * 255.0) / 255.0, boost::math::round(c.rgb.blue * 255.0) / 255.0);
		cairo_move_to(cr, radius_multi * cos(rotation + i * (2 * PI) / edges) - extends.width / 2, radius_multi * sin(rotation + i * (2 * PI) / edges)
				+ extends.height / 2);
		cairo_show_text(cr, numb);
	}
	cairo_set_matrix(cr, &matrix);
	return true;
}
#if GTK_MAJOR_VERSION < 3
static gboolean expose(GtkWidget *widget, GdkEventExpose *event)
{
	cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
	cairo_rectangle(cr, event->area.x, event->area.y, event->area.width, event->area.height);
	cairo_clip(cr);
	cairo_translate(cr, widget->style->xthickness + 0.5, widget->style->ythickness + 0.5);
	gboolean result = draw(widget, cr);
	cairo_destroy(cr);
	return result;
}
#endif
static void offset_xy(GtkWidget *widget, gint &x, gint &y)
{
#if GTK_MAJOR_VERSION < 3
	x = x - widget->style->xthickness;
	y = y - widget->style->ythickness;
#endif
}
static gboolean button_press(GtkWidget *widget, GdkEventButton *event) {
	GtkSwatchPrivate *ns = GET_PRIVATE(widget);
	int x = event->x, y = event->y;
	offset_xy(widget, x, y);
	int new_color = get_color_by_position(x, y);
	gtk_widget_grab_focus(widget);
	if ((event->type == GDK_2BUTTON_PRESS) && (event->button == 1)) {
		if (new_color>0){
			g_signal_emit(widget, signals[COLOR_ACTIVATED], 0);
		}
	}else if ((event->type == GDK_BUTTON_PRESS) && ((event->button == 1) || (event->button == 3))) {
		if (new_color == 0){
			g_signal_emit(widget, signals[CENTER_ACTIVATED], 0);
		}else if (new_color<0){
			g_signal_emit(widget, signals[ACTIVE_COLOR_CHANGED], 0, ns->current_color);
		}else{
			if (new_color != ns->current_color){
				ns->current_color = new_color;
				g_signal_emit(widget, signals[ACTIVE_COLOR_CHANGED], 0, ns->current_color);
				gtk_widget_queue_draw(GTK_WIDGET(widget));
			}
		}
	}
	return FALSE;
}
static gboolean button_release(GtkWidget *widget, GdkEventButton *event) {
	return FALSE;
}
void gtk_swatch_set_transformation_chain(GtkSwatch* widget, transformation::Chain *chain){
	GtkSwatchPrivate *ns = GET_PRIVATE(widget);
	ns->transformation_chain = chain;
	gtk_widget_queue_draw(GTK_WIDGET(widget));
}
void gtk_swatch_set_active(GtkSwatch* widget, gboolean active)
{
	GtkSwatchPrivate *ns = GET_PRIVATE(widget);
	ns->active = active;
	gtk_widget_queue_draw(GTK_WIDGET(widget));
}
