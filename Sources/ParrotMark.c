#include <kinc/display.h>
#include <kinc/graphics2/graphics.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/indexbuffer.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/texture.h>
#include <kinc/graphics4/vertexbuffer.h>
#include <kinc/image.h>
#include <kinc/input/mouse.h>
#include <kinc/io/filereader.h>
#include <kinc/log.h>
#include <kinc/math/random.h>
#include <kinc/system.h>
#include <kinc/window.h>

#include <assert.h>
#include <stdlib.h>

// static kinc_g4_texture_t texture;

typedef struct Parrot {
	float x, y;
	float speed_x, speed_y;
	float rotation, rotation_speed;
} Parrot_t;

static Parrot_t *parrots = NULL;
static size_t num_parrots = 0;

static float gravity = 0.5f;
static float maxX, maxY, minX, minY;

static kinc_image_t image;

#define HEAP_SIZE 1024 * 1024
static uint8_t *heap = NULL;
static size_t heap_top = 0;

static void *allocate(size_t size) {
	size_t old_top = heap_top;
	heap_top += size;
	assert(heap_top <= HEAP_SIZE);
	return &heap[old_top];
}

uint64_t frame = 0;

static void update_simple(void) {
	if (frame % 100 == 0) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Parrots: %i", num_parrots);
	}
	++frame;

	for (int i = 0; i < num_parrots; ++i) {
		Parrot_t *parrot = &parrots[i];

		parrot->x += parrot->speed_x;
		parrot->y += parrot->speed_y;
		parrot->speed_y += gravity;

		if (parrot->x > maxX) {
			parrot->speed_x *= -1;
			parrot->x = maxX;
		}
		else if (parrot->x < minX) {
			parrot->speed_x *= -1;
			parrot->x = minX;
		}

		if (parrot->y > maxY) {
			parrot->speed_y *= -0.8f;
			parrot->y = maxY;
			if (kinc_random_get_in(0, 1) == 0) {
				parrot->speed_y -= 3 + kinc_random_get_in(0, 4);
			}
		}
		else if (parrot->y < minY) {
			parrot->speed_y = 0;
			parrot->y = minY;
		}
	}

	kinc_g2_begin();

	kinc_g2_clear(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < num_parrots; ++i) {
		kinc_g2_draw_image(&image, parrots[i].x, parrots[i].y);
	}

	kinc_g2_end();
}

static void update_rotating(void) {
	if (frame % 100 == 0) {
		kinc_log(KINC_LOG_LEVEL_INFO, "Parrots: %i", num_parrots);
	}
	++frame;

	for (int i = 0; i < num_parrots; ++i) {
		Parrot_t *parrot = &parrots[i];

		parrot->x += parrot->speed_x;
		parrot->y += parrot->speed_y;
		parrot->speed_y += gravity;
		parrot->rotation += parrot->rotation_speed;

		if (parrot->x > maxX) {
			parrot->speed_x *= -1;
			parrot->x = maxX;
		}
		else if (parrot->x < minX) {
			parrot->speed_x *= -1;
			parrot->x = minX;
		}

		if (parrot->y > maxY) {
			parrot->speed_y *= -0.8f;
			parrot->y = maxY;
			if (kinc_random_get_in(0, 1) == 0) {
				parrot->speed_y -= 3 + kinc_random_get_in(0, 4);
			}
		}
		else if (parrot->y < minY) {
			parrot->speed_y = 0;
			parrot->y = minY;
		}
	}

	kinc_g2_begin();

	kinc_g2_clear(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < num_parrots; ++i) {
		kinc_g2_set_rotation(parrots[i].rotation, parrots[i].x + image.width / 2.0f, parrots[i].y + image.height / 2.0f);
		kinc_g2_draw_image(&image, parrots[i].x, parrots[i].y);
	}

	kinc_g2_end();
}

static void create_parrots(int count) {
	for (int i = 0; i < count; ++i) {
		++num_parrots;
		parrots[num_parrots - 1].x = 0;
		parrots[num_parrots - 1].y = 0;
		parrots[num_parrots - 1].speed_x = kinc_random_get_in(0, 500) / 100.0f;
		parrots[num_parrots - 1].speed_y = kinc_random_get_in(0, 100) / 10.0f / 2.0f - 2.5f;
		parrots[num_parrots - 1].rotation = 0.0f;
		parrots[num_parrots - 1].rotation_speed = kinc_random_get_in(1, 100) / 100.0f;
	}
}

void mouse_press(int window, int button, int x, int y) {
	create_parrots(button == 1 ? 1000 : 10000);
}

int kickstart(int argc, char **argv) {
	int screen_width = 1024;
	int screen_height = 768;

	kinc_display_init();

	kinc_window_options_t win;
	kinc_window_options_set_defaults(&win);
	win.display_index = 2;

	kinc_init("ParrotMark", screen_width, screen_height, &win, NULL);
	// kinc_set_update_callback(update_simple);
	kinc_set_update_callback(update_rotating);

	heap = (uint8_t *)malloc(HEAP_SIZE);
	assert(heap != NULL);

	kinc_random_init((int)(kinc_time() * 1000.0));

	parrots = (Parrot_t *)malloc(sizeof(Parrot_t) * 10 * 1000 * 1000);

	void *image_mem = allocate(250 * 250 * 4);
	kinc_image_init_from_file(&image, image_mem, "small_parrot.png");
	// kinc_g4_texture_init_from_image(&texture, &image);
	// kinc_image_destroy(&image);

	minX = 0.0f;
	maxX = (float)(screen_width - image.width);
	minY = 0.0f;
	maxY = (float)(screen_height - image.height);

	create_parrots(100);

	kinc_mouse_press_callback = mouse_press;

	kinc_g2_init(screen_width, screen_height);

	kinc_start();

	return 0;
}
