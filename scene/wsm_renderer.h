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

#ifndef WSM_RENDERER_H
#define WSM_RENDERER_H

struct wlr_buffer;
struct wlr_renderer;
struct wlr_buffer_pass_options;

struct wsm_render_pass;

struct wsm_renderer_impl {
	struct wsm_render_pass *(*begin_buffer_pass)(struct wlr_renderer *renderer,
		struct wlr_buffer *buffer, const struct wlr_buffer_pass_options *options);
};

struct wsm_renderer {
	struct wlr_renderer *wlr_renderer;
	const struct wsm_renderer_impl *impl;
};

struct wsm_renderer* wsm_renderer_create();
// void wsm_vulkan_renderer_init();
// void wsm_pixman_renderer_init();

#endif
