/*
 * Copyright © 2024 YaoBing Xiao
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef WSM_OUTPUT_MANAGER_CONFIG_H
#define WSM_OUTPUT_MANAGER_CONFIG_H

#include <stdbool.h>

// #include <libxml/parser.h>
// #include <libxml/tree.h>
// #include <libxml/xmlwriter.h>

struct wsm_list;
struct wsm_output_manager;

struct wsm_output_manager_config {
    struct wsm_output_manager *output_manager;
    // xmlDocPtr doc;
    // xmlTextWriterPtr writer;
    struct wsm_list *configs;
};

struct wsm_output_manager_config *wsm_output_manager_config_create(struct wsm_output_manager *output_manager);
void wwsm_output_manager_config_destory(struct wsm_output_manager_config *config);
void load_default_output_config();

#endif