/*
 * kmscon - Text Renderer
 *
 * Copyright (c) 2012 David Herrmann <dh.herrmann@googlemail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Text Renderer
 * The Text-Renderer subsystem provides a simple way to draw text into a
 * framebuffer. The system is modular and several different backends are
 * available that can be used.
 * The system is split into:
 *  - Font renderer: The font renderer allows selecting fonts and rendering
 *    single glyphs into memory-buffers
 *  - Text renderer: The text renderer uses the font renderer to draw a whole
 *    console into a framebuffer.
 */

#ifndef KMSCON_TEXT_H
#define KMSCON_TEXT_H

#include <stdlib.h>
#include "font.h"
#include "unicode.h"
#include "uterm.h"

/* fonts */

struct kmscon_font_attr;
struct kmscon_glyph;
struct kmscon_font;
struct kmscon_font_ops;

#define KMSCON_FONT_MAX_NAME 128
#define KMSCON_FONT_DEFAULT_NAME "monospace"
#define KMSCON_FONT_DEFAULT_PPI 72

struct kmscon_font_attr {
	char name[KMSCON_FONT_MAX_NAME];
	unsigned int ppi;
	unsigned int points;
	bool bold;
	bool italic;
	unsigned int height;
	unsigned int width;
};

void kmscon_font_attr_normalize(struct kmscon_font_attr *attr);
bool kmscon_font_attr_match(const struct kmscon_font_attr *a1,
			    const struct kmscon_font_attr *a2);

struct kmscon_glyph {
	struct uterm_video_buffer buf;
	void *data;
};

struct kmscon_font {
	unsigned long ref;
	const struct kmscon_font_ops *ops;
	struct kmscon_font_attr attr;
	unsigned int baseline;
	void *data;
};

struct kmscon_font_ops {
	const char *name;
	int (*init) (struct kmscon_font *out,
		     const struct kmscon_font_attr *attr);
	void (*destroy) (struct kmscon_font *font);
	int (*render) (struct kmscon_font *font, kmscon_symbol_t sym,
		       const struct kmscon_glyph **out);
	int (*render_empty) (struct kmscon_font *font,
			     const struct kmscon_glyph **out);
	int (*render_inval) (struct kmscon_font *font,
			     const struct kmscon_glyph **out);
};

int kmscon_font_register(const struct kmscon_font_ops *ops);
void kmscon_font_unregister(const char *name);

int kmscon_font_find(struct kmscon_font **out,
		     const struct kmscon_font_attr *attr,
		     const char *backend);
void kmscon_font_ref(struct kmscon_font *font);
void kmscon_font_unref(struct kmscon_font *font);

int kmscon_font_render(struct kmscon_font *font, kmscon_symbol_t sym,
		       const struct kmscon_glyph **out);
int kmscon_font_render_empty(struct kmscon_font *font,
			     const struct kmscon_glyph **out);
int kmscon_font_render_inval(struct kmscon_font *font,
			     const struct kmscon_glyph **out);

/* text renderer */

struct kmscon_text;
struct kmscon_text_ops;

struct kmscon_text {
	unsigned long ref;
	const struct kmscon_text_ops *ops;
	struct kmscon_font *font;
	struct uterm_screen *screen;
	uint8_t bg_r;
	uint8_t bg_g;
	uint8_t bg_b;
	unsigned int cols;
	unsigned int rows;
	bool rendering;
	void *data;
};

struct kmscon_text_ops {
	const char *name;
	int (*init) (struct kmscon_text *txt);
	void (*destroy) (struct kmscon_text *txt);
	void (*new_font) (struct kmscon_text *txt);
	void (*new_bgcolor) (struct kmscon_text *txt);
	void (*new_screen) (struct kmscon_text *txt);
	void (*prepare) (struct kmscon_text *txt);
	void (*draw) (struct kmscon_text *txt, kmscon_symbol_t ch,
		      unsigned int posx, unsigned int posy,
		      const struct font_char_attr *attr);
	void (*render) (struct kmscon_text *txt);
};

int kmscon_text_register(const struct kmscon_text_ops *ops);
void kmscon_text_unregister(const char *name);

int kmscon_text_new(struct kmscon_text **out, const char *backend);
void kmscon_text_ref(struct kmscon_text *txt);
void kmscon_text_unref(struct kmscon_text *txt);

void kmscon_text_set_font(struct kmscon_text *txt,
			  struct kmscon_font *font);
void kmscon_text_set_bgcolor(struct kmscon_text *txt,
			     uint8_t r, uint8_t g, uint8_t b);
void kmscon_text_set_screen(struct kmscon_text *txt,
			    struct uterm_screen *screen);
unsigned int kmscon_text_get_cols(struct kmscon_text *txt);
unsigned int kmscon_text_get_rows(struct kmscon_text *txt);

void kmscon_text_prepare(struct kmscon_text *txt);
void kmscon_text_draw(struct kmscon_text *txt, kmscon_symbol_t ch,
		      unsigned int posx, unsigned int posy,
		      const struct font_char_attr *attr);
void kmscon_text_render(struct kmscon_text *txt);

/* modularized backends */

#ifdef KMSCON_HAVE_8X16

int kmscon_font_8x16_load(void);
void kmscon_font_8x16_unload(void);

#else

static inline int kmscon_font_8x16_load(void)
{
	return -EOPNOTSUPP;
}

static inline void kmscon_font_8x16_unload(void)
{
}

#endif

#ifdef KMSCON_HAVE_FREETYPE2

int kmscon_font_freetype2_load(void);
void kmscon_font_freetype2_unload(void);

#else

static inline int kmscon_font_freetype2_load(void)
{
	return -EOPNOTSUPP;
}

static inline void kmscon_font_freetype2_unload(void)
{
}

#endif

#ifdef KMSCON_HAVE_PANGO

int kmscon_font_pango_load(void);
void kmscon_font_pango_unload(void);

#else

static inline int kmscon_font_pango_load(void)
{
	return -EOPNOTSUPP;
}

static inline void kmscon_font_pango_unload(void)
{
}

#endif

int kmscon_text_bblit_load(void);
void kmscon_text_bblit_unload(void);

#endif /* KMSCON_TEXT_H */
