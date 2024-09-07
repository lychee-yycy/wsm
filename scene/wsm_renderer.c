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

#include "wsm_log.h"
#include "wsm_server.h"
#include "wsm_renderer.h"
#include "gles2/wsm_gles2_renderer.h"

#include <stdlib.h>

#include <wlr/render/wlr_renderer.h>
#include <wlr/render/gles2.h>

struct wsm_renderer* wsm_renderer_create() {
	struct wsm_renderer *renderer = calloc(1, sizeof(struct wsm_renderer));
	if (!wsm_assert(renderer, "Could not create wsm_renderer: allocation failed!")) {
		return NULL;
	}
	renderer->wlr_renderer = wlr_renderer_autocreate(global_server.backend);
	if (!renderer->wlr_renderer) {
		wsm_log(WSM_ERROR, "Failed to create wlr_renderer");
		free(renderer);
		return NULL;
	}

	if (wlr_renderer_is_gles2(renderer->wlr_renderer)) {
		wsm_gles2_renderer_init();
	}

	return renderer;
}
