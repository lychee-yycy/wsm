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

#ifndef WSM_PASS_H
#define WSM_PASS_H

#include <wlr/util/box.h>
#include <wlr/render/pass.h>

struct wsm_render_pass;

struct wsm_render_rect_options {
	struct wlr_box box;
	struct wlr_render_color color;
	const pixman_region32_t *clip;
	enum wlr_render_blend_mode blend_mode;
};

struct wsm_render_pass_impl {
	void (*add_rectangle)(struct wsm_render_pass *pass,
		const struct wsm_render_rect_options *options);
};

struct wsm_render_pass {
	const struct wsm_render_pass_impl *impl;
};

#endif