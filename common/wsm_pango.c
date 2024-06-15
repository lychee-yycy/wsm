/*
MIT License

Copyright (c) 2024 YaoBing Xiao

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#define _POSIX_C_SOURCE 200809L
#include "wsm_pango.h"
#include "wsm_log.h"
#include "wsm_common.h"

#include <pango/pangocairo.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t escape_markup_text(const char *src, char *dest) {
    size_t length = 0;
    if (dest) {
        dest[0] = '\0';
    }

    while (src[0]) {
        switch (src[0]) {
        case '&':
            length += 5;
            lenient_strcat(dest, "&amp;");
            break;
        case '<':
            length += 4;
            lenient_strcat(dest, "&lt;");
            break;
        case '>':
            length += 4;
            lenient_strcat(dest, "&gt;");
            break;
        case '\'':
            length += 6;
            lenient_strcat(dest, "&apos;");
            break;
        case '"':
            length += 6;
            lenient_strcat(dest, "&quot;");
            break;
        default:
            if (dest) {
                dest[length] = *src;
                dest[length + 1] = '\0';
            }
            length += 1;
        }
        src++;
    }
    return length;
}

PangoLayout *get_pango_layout(cairo_t *cairo, const PangoFontDescription *desc,
                              const char *text, double scale, bool markup) {
    PangoLayout *layout = pango_cairo_create_layout(cairo);
    PangoAttrList *attrs;
    if (markup) {
        char *buf;
        GError *error = NULL;
        if (pango_parse_markup(text, -1, 0, &attrs, &buf, NULL, &error)) {
            pango_layout_set_text(layout, buf, -1);
            free(buf);
        } else {
            wsm_log(WSM_ERROR, "pango_parse_markup '%s' -> error %s", text,
                     error->message);
            g_error_free(error);
            markup = false; // fallback to plain text
        }
    }
    if (!markup) {
        attrs = pango_attr_list_new();
        pango_layout_set_text(layout, text, -1);
    }

    pango_attr_list_insert(attrs, pango_attr_scale_new(scale));
    pango_layout_set_font_description(layout, desc);
    pango_layout_set_single_paragraph_mode(layout, 1);
    pango_layout_set_attributes(layout, attrs);
    pango_attr_list_unref(attrs);
    return layout;
}

void get_text_size(cairo_t *cairo, const PangoFontDescription *desc, int *width, int *height,
                   int *baseline, double scale, bool markup, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *buf = vformat_str(fmt, args);
    va_end(args);
    if (buf == NULL) {
        return;
    }

    PangoLayout *layout = get_pango_layout(cairo, desc, buf, scale, markup);
    pango_cairo_update_layout(cairo, layout);
    pango_layout_get_pixel_size(layout, width, height);
    if (baseline) {
        *baseline = pango_layout_get_baseline(layout) / PANGO_SCALE;
    }
    g_object_unref(layout);

    free(buf);
}

void get_text_metrics(const PangoFontDescription *description, int *height, int *baseline) {
    cairo_t *cairo = cairo_create(NULL);
    PangoContext *pango = pango_cairo_create_context(cairo);
    // When passing NULL as a language, pango uses the current locale.
    PangoFontMetrics *metrics = pango_context_get_metrics(pango, description, NULL);

    *baseline = pango_font_metrics_get_ascent(metrics) / PANGO_SCALE;
    *height = *baseline + pango_font_metrics_get_descent(metrics) / PANGO_SCALE;

    pango_font_metrics_unref(metrics);
    g_object_unref(pango);
    cairo_destroy(cairo);
}

void render_text(cairo_t *cairo, PangoFontDescription *desc,
                 double scale, bool markup, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *buf = vformat_str(fmt, args);
    va_end(args);
    if (buf == NULL) {
        return;
    }

    PangoLayout *layout = get_pango_layout(cairo, desc, buf, scale, markup);
    cairo_font_options_t *fo = cairo_font_options_create();
    cairo_get_font_options(cairo, fo);
    pango_cairo_context_set_font_options(pango_layout_get_context(layout), fo);
    cairo_font_options_destroy(fo);
    pango_cairo_update_layout(cairo, layout);
    pango_cairo_show_layout(cairo, layout);
    g_object_unref(layout);

    free(buf);
}