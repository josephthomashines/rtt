#include "rtc.h"

void camera_init(camera *c, const u32 hsize, const u32 vsize, const f64 fov)
{
  c->hsize = hsize;
  c->vsize = vsize;
  c->fov = fov;
  
  camera_set_transform(c, IDENTITY);

  {
    f64 half_view = (f64)tan(fov / 2.0);
    f64 aspect = (f64)hsize / (f64)vsize;

    if (aspect >= 1) {
      c->half_width = half_view;
      c->half_height = half_view / aspect;
    } else {
      c->half_width = half_view * aspect;
      c->half_height = half_view;
    }

    c->pixel_size = (c->half_width * 2.0) / (f64)hsize;
  }

  c->antialias = false;
}

void camera_set_transform(camera *c, const m4 T)
{
  memcpy(c->transform, T, sizeof(m4));
  m4_inverse(c->transform, c->inverse_transform);
}

void camera_ray_for_pixel(const camera *c, const u32 x, const u32 y, ray *out)
{
  f64 x_offset = ((f64)x + 0.5) * c->pixel_size;
  f64 y_offset = ((f64)y + 0.5) * c->pixel_size;

  f64 world_x = c->half_width - x_offset;
  f64 world_y = c->half_height - y_offset;

  v4 pixel = {0};
  m4_mulv(c->inverse_transform, point(world_x, world_y, -1), pixel);

  m4_mulv(c->inverse_transform, point(0, 0, 0), out->origin);

  v4_sub(pixel, out->origin, out->direction);
  v4_norm(out->direction, out->direction);
}

// apply jittering to implement antialiasing
void camera_raw_ray_for_pixel(const camera *c, const f64 x_offset, const f64 y_offset, ray *out)
{
  f64 world_x = c->half_width - x_offset;
  f64 world_y = c->half_height - y_offset;

  v4 pixel = {0};
  m4_mulv(c->inverse_transform, point(world_x, world_y, -1), pixel);

  m4_mulv(c->inverse_transform, point(0, 0, 0), out->origin);

  v4_sub(pixel, out->origin, out->direction);
  v4_norm(out->direction, out->direction);
}

canvas *camera_render(const camera *v, const world *w, render_stats *s)
{

  canvas *c = canvas_alloc(v->hsize, v->vsize);

  if (s != NULL) {
    s->width = v->hsize;
    s->height = v->vsize;
    s->start = prof_read_cpu_timer();
  }

  if (v->antialias) {
    /*
    // Fixed
    for (u32 y = 0; y < v->vsize; y++) {
      for (u32 x = 0; x < v->hsize; x++) {
        v3 color_at = {0};
        ray r = {0};

        {
          // TL
          f64 x_offset = ((f64)x + 0.0) * v->pixel_size;
          f64 y_offset = ((f64)y + 0.0) * v->pixel_size;

          v3 this_color = {0};
          camera_raw_ray_for_pixel(v, x_offset, y_offset, &r);
          world_color_at(w, &r, MAX_DEPTH, this_color);
          v3_add(this_color, color_at, color_at);
        }

        {
          // TM
          f64 x_offset = ((f64)x + 0.5) * v->pixel_size;
          f64 y_offset = ((f64)y + 0.0) * v->pixel_size;

          v3 this_color = {0};
          camera_raw_ray_for_pixel(v, x_offset, y_offset, &r);
          world_color_at(w, &r, MAX_DEPTH, this_color);
          v3_add(this_color, color_at, color_at);
        }

        {
          // TR
          f64 x_offset = ((f64)x + 1.0) * v->pixel_size;
          f64 y_offset = ((f64)y + 0.0) * v->pixel_size;

          v3 this_color = {0};
          camera_raw_ray_for_pixel(v, x_offset, y_offset, &r);
          world_color_at(w, &r, MAX_DEPTH, this_color);
          v3_add(this_color, color_at, color_at);
        }

        {
          // ML
          f64 x_offset = ((f64)x + 0.0) * v->pixel_size;
          f64 y_offset = ((f64)y + 0.5) * v->pixel_size;

          v3 this_color = {0};
          camera_raw_ray_for_pixel(v, x_offset, y_offset, &r);
          world_color_at(w, &r, MAX_DEPTH, this_color);
          v3_add(this_color, color_at, color_at);
        }

        {
          // MM
          f64 x_offset = ((f64)x + 0.5) * v->pixel_size;
          f64 y_offset = ((f64)y + 0.5) * v->pixel_size;

          v3 this_color = {0};
          camera_raw_ray_for_pixel(v, x_offset, y_offset, &r);
          world_color_at(w, &r, MAX_DEPTH, this_color);
          v3_add(this_color, color_at, color_at);
        }

        {
          // MR
          f64 x_offset = ((f64)x + 1.0) * v->pixel_size;
          f64 y_offset = ((f64)y + 0.5) * v->pixel_size;

          v3 this_color = {0};
          camera_raw_ray_for_pixel(v, x_offset, y_offset, &r);
          world_color_at(w, &r, MAX_DEPTH, this_color);
          v3_add(this_color, color_at, color_at);
        }

        {
          // BL
          f64 x_offset = ((f64)x + 0.0) * v->pixel_size;
          f64 y_offset = ((f64)y + 1.0) * v->pixel_size;

          v3 this_color = {0};
          camera_raw_ray_for_pixel(v, x_offset, y_offset, &r);
          world_color_at(w, &r, MAX_DEPTH, this_color);
          v3_add(this_color, color_at, color_at);
        }

        {
          // BM
          f64 x_offset = ((f64)x + 0.5) * v->pixel_size;
          f64 y_offset = ((f64)y + 1.0) * v->pixel_size;

          v3 this_color = {0};
          camera_raw_ray_for_pixel(v, x_offset, y_offset, &r);
          world_color_at(w, &r, MAX_DEPTH, this_color);
          v3_add(this_color, color_at, color_at);
        }

        {
          // BR
          f64 x_offset = ((f64)x + 1.0) * v->pixel_size;
          f64 y_offset = ((f64)y + 1.0) * v->pixel_size;

          v3 this_color = {0};
          camera_raw_ray_for_pixel(v, x_offset, y_offset, &r);
          world_color_at(w, &r, MAX_DEPTH, this_color);
          v3_add(this_color, color_at, color_at);
        }

        v3_scale(color_at, 1.0 / 9, color_at);
        canvas_write(c, x, y, color_at);
      }
    } 
  */
    // Noise
    for (u32 y = 0; y < v->vsize; y++) {
      for (u32 x = 0; x < v->hsize; x++) {
        v3 color_at = {0};
        ray r = {0};

        u32 N = 32;
        for (u32 i = 0; i < N; i++) {
          f64 jitter_x = (random_uniform() - 0.5);
          f64 jitter_y = (random_uniform() - 0.5);
          f64 x_offset = ((f64)x + jitter_x + 0.5) * v->pixel_size;
          f64 y_offset = ((f64)y + jitter_y + 0.5) * v->pixel_size;

          v3 this_color = {0};
          camera_raw_ray_for_pixel(v, x_offset, y_offset, &r);
          world_color_at(w, &r, MAX_DEPTH, this_color);
          v3_add(this_color, color_at, color_at);
        }

        v3_scale(color_at, 1.0 / (f64)N, color_at);
        canvas_write(c, x, y, color_at);
      }
    } 
  } else {
    for (u32 y = 0; y < v->vsize; y++) {
      for (u32 x = 0; x < v->hsize; x++) {
        ray r = {0};
        camera_ray_for_pixel(v, x, y, &r);

        v3 color_at = {0};
        world_color_at(w, &r, MAX_DEPTH, color_at);

        canvas_write(c, x, y, color_at);
      }
    }
  }



  if (s != NULL) {
    s->end = prof_read_cpu_timer();
  }

  return c;
}

void render_stats_print(const render_stats *s)
{
  if (s->start == 0) {
    printf("\n");
    return;
  }

  u64 cpu_freq = prof_estimate_cpu_freq(100);
  u64 duration = s->end - s->start;

  u64 total_pixels = s->width * s->height;

  f64 total_pixels_million = (f64)total_pixels / 1000000;
  f64 duration_s = ((f64)duration / (f64)cpu_freq);
  f64 ns_per_px = (f64)duration / (f64)total_pixels;

  printf("[%llux%llupx (%0.2fm px) in %0.4fs (%0.2fns/px)]\n", 
      s->width, s->height, total_pixels_million, duration_s, ns_per_px);
}

