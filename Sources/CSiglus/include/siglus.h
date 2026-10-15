#ifndef SIGLUS_H
#define SIGLUS_H

#include <stddef.h>
#include <stdint.h>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*siglus_native_messagebox_callback_t)(
    void *user_data,
    uint64_t request_id,
    int32_t kind,
    const char *title_utf8,
    const char *message_utf8);

void siglus_string_free(char *ptr);
char *siglus_game_name_from_dir(const char *game_root_utf8);
char *siglus_game_cover_path_from_dir(const char *game_root_utf8);
char *siglus_game_cover_mime_from_dir(const char *game_root_utf8);

/* Shiori: movies the host decodes. Windows Media movies (ASF: WMV 7/8/9 or VC-1 video, WMA
   1/2, Pro or Lossless sound) and MPEG-1/2 video are decoded by the host app (Shiori: its
   LGPL FFmpeg), not by the engine. Register once, before creating a game; without them those
   movies don't open and the game goes on past them. */
#define SIGLUS_HOST_NO_PTS INT64_MIN
typedef struct SiglusHostPicture {
    uint32_t width, height;
    const uint8_t *planes[3];        /* Y, Cb, Cr, 8-bit */
    uint32_t strides[3];             /* bytes per row */
    uint32_t chroma_shift_x, chroma_shift_y; /* 1, 1 for 4:2:0 */
    int64_t pts;                     /* ms (ASF) or 90 kHz (MPEG); SIGLUS_HOST_NO_PTS */
    int32_t key_frame;
} SiglusHostPicture;
typedef struct SiglusHostSound {
    const float *samples;            /* interleaved */
    uint32_t frames, channels, sample_rate;
    int64_t pts_ms;                  /* or SIGLUS_HOST_NO_PTS */
} SiglusHostSound;
/* has_video, has_audio: 1 a stream the host decodes, 0 none, -1 one it can't decode. */
typedef struct SiglusHostMovieInfo {
    uint32_t width, height;
    int64_t duration_ms;             /* -1 unknown */
    int32_t has_video, has_audio;
    uint32_t sample_rate, channels;
} SiglusHostMovieInfo;
/* The movie file, read through the engine: read returns the bytes read (0 at the end,
   negative on error); seek takes SEEK_SET/CUR/END or 0x10000 (the size) and returns the new
   position, negative on error. */
typedef struct SiglusHostIO {
    void *opaque;
    int32_t (*read)(void *opaque, uint8_t *buf, int32_t len);
    int64_t (*seek)(void *opaque, int64_t offset, int32_t whence);
} SiglusHostIO;
/* Pictures and sound stay valid until the next call on the same handle. Each handle is
   used by one thread at a time. */
typedef struct SiglusHostMedia {
    uint32_t version;                /* 1 */
    /* want: 1 video, 2 sound. io stays valid until asf_close. NULL if unreadable. */
    void *(*asf_open)(const SiglusHostIO *io, int32_t want, SiglusHostMovieInfo *info);
    int32_t (*asf_next_picture)(void *movie, SiglusHostPicture *out); /* 1, 0 end, <0 error */
    int32_t (*asf_next_sound)(void *movie, SiglusHostSound *out);     /* 1, 0 end, <0 error */
    void (*asf_close)(void *movie);
    /* MPEG-1/2 video elementary stream, fed in arbitrary pieces; pts_90k is the first
       picture starting in them. NULL data drains at the end. send: 0, <0 fatal error.
       receive: 1 a picture (presentation order), 0 needs more data. */
    void *(*mpeg_video_open)(void);
    int32_t (*mpeg_video_send)(void *decoder, const uint8_t *data, size_t len, int64_t pts_90k);
    int32_t (*mpeg_video_receive)(void *decoder, SiglusHostPicture *out);
    void (*mpeg_video_close)(void *decoder);
} SiglusHostMedia;
/* Copies media; NULL removes it. */
void siglus_set_host_media(const SiglusHostMedia *media);

#if defined(__APPLE__) && TARGET_OS_IPHONE
void *siglus_ios_create(
    void *ui_view,
    uint32_t surface_width,
    uint32_t surface_height,
    double native_scale_factor,
    const char *game_root_utf8);
void siglus_ios_resize_viewport(
    void *handle,
    uint32_t surface_width,
    uint32_t surface_height,
    uint32_t viewport_x,
    uint32_t viewport_y,
    uint32_t viewport_width,
    uint32_t viewport_height);
void siglus_ios_logical_size(void *handle, uint32_t *width_out, uint32_t *height_out);
void siglus_ios_set_native_messagebox_callback(
    void *handle,
    siglus_native_messagebox_callback_t callback,
    void *user_data);
void siglus_ios_submit_messagebox_result(void *handle, uint64_t request_id, int64_t value);
int32_t siglus_ios_step(void *handle, uint32_t dt_ms);
void siglus_ios_resize(void *handle, uint32_t surface_width, uint32_t surface_height);
void siglus_ios_touch(void *handle, int32_t phase, double x_points, double y_points);
void siglus_ios_text_input(void *handle, const char *text_utf8);
/* Shiori: button 0 left, 1 right, 2 middle; wheel steps (up positive); Windows VK codes. */
void siglus_ios_mouse_button(void *handle, int32_t button, int32_t down);
void siglus_ios_mouse_wheel(void *handle, int32_t delta_y);
void siglus_ios_key(void *handle, int32_t code, int32_t down);
/* Shiori: a trackpad or mouse (on) or fingers drive the game; with a pointer the game's own
   cursor sprite is drawn. has_custom_cursor is 1 while the game draws one. */
void siglus_ios_set_pointer_mode(void *handle, int32_t on);
int32_t siglus_ios_has_custom_cursor(void *handle);
/* Shiori: rebuilds the audio output (iOS stops it on an interruption or a media services
   reset) on the current route, keeping what plays. Call when an interruption ends, the route
   changes or media services are reset. */
void siglus_ios_restart_audio(void);
/* Shiori: saves the game's place in its own format (the save point where the message on
   screen began) into normal slot index, or the first empty slot when index is
   negative. Returns the slot, -1 when the game can't save now, -2 when no slot is free, -3
   on failure; dry_run nonzero only answers which. Call between steps. */
int32_t siglus_ios_save_place(void *handle, int32_t index, int32_t dry_run);
void siglus_ios_ime_preedit(void *handle, const char *text_utf8, int32_t cursor_start, int32_t cursor_end);
void siglus_ios_destroy(void *handle);
#endif

#if defined(__ANDROID__)
void siglus_android_init_context(void *java_vm_ptr, void *context_ptr);
void *siglus_android_create(
    void *native_window_ptr,
    uint32_t surface_width_px,
    uint32_t surface_height_px,
    double native_scale_factor,
    const char *game_dir_utf8);
void siglus_android_set_native_messagebox_callback(
    void *handle,
    siglus_native_messagebox_callback_t callback,
    void *user_data);
void siglus_android_submit_messagebox_result(void *handle, uint64_t request_id, int64_t value);
int32_t siglus_android_step(void *handle, uint32_t dt_ms);
void siglus_android_resize(void *handle, uint32_t surface_width_px, uint32_t surface_height_px);
void siglus_android_set_surface(
    void *handle,
    void *native_window_ptr,
    uint32_t surface_width_px,
    uint32_t surface_height_px);
void siglus_android_touch(void *handle, int32_t phase, double x_px, double y_px);
void siglus_android_text_input(void *handle, const char *text_utf8);
void siglus_android_ime_preedit(void *handle, const char *text_utf8, int32_t cursor_start, int32_t cursor_end);
void siglus_android_destroy(void *handle);
#endif

#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
typedef struct SiglusPumpHandle SiglusPumpHandle;
SiglusPumpHandle *siglus_pump_create(const char *game_root_utf8);
void siglus_pump_set_native_messagebox_callback(
    SiglusPumpHandle *handle,
    siglus_native_messagebox_callback_t callback,
    void *user_data);
void siglus_pump_submit_messagebox_result(SiglusPumpHandle *handle, uint64_t request_id, int64_t value);
void siglus_pump_text_input(SiglusPumpHandle *handle, const char *text_utf8);
void siglus_pump_ime_preedit(SiglusPumpHandle *handle, const char *text_utf8, int32_t cursor_start, int32_t cursor_end);
void siglus_pump_key_down(SiglusPumpHandle *handle, int32_t key_code);
void siglus_pump_key_up(SiglusPumpHandle *handle, int32_t key_code);
int32_t siglus_pump_step(SiglusPumpHandle *handle, uint32_t timeout_ms);
void siglus_pump_destroy(SiglusPumpHandle *handle);
int32_t siglus_run_entry(const char *game_root_utf8);
#endif

/* ---------------------------------------------------------------------------
 * Multi-engine launcher API (game_launcher): SiglusEngine, RealLive, AVG32
 * and UK2.  Returned strings are UTF-8 JSON or text; free them with
 * game_string_free.  RealLive/AVG32/UK2 games are hosted frame by frame
 * through game_fb_*; SiglusEngine games keep using the siglus_* host API.
 * ------------------------------------------------------------------------- */
void game_string_free(char *ptr);
/* One game folder: {"id","root","engine","engine_name","supported",
 * "unsupported_reason","title","cover","cover_kind","nls","nls_options",
 * "evidence"}.  nls and cover_cache_dir may be NULL. */
char *game_probe_json(const char *root_utf8, const char *nls, const char *cover_cache_dir_utf8);
/* Every game at or below path (depth folder levels), as a JSON array. */
char *game_scan_json(const char *path_utf8, int32_t depth, const char *cover_cache_dir_utf8);
/* Registers a font file for game text (e.g. the system CJK font). 0 = ok. */
int32_t game_add_font_file(const char *path_utf8);

typedef struct GameFbHandle GameFbHandle;
/* engine: "reallive", "avg32", "uk2" or NULL to detect; nls: "sjis", "gbk",
 * "big5", "utf8", "western", "korean", "auto" or NULL (Shift-JIS). */
GameFbHandle *game_fb_open(const char *root_utf8, const char *engine, const char *nls, char **error_out);
/* 0 while running, 1 once the game has ended. */
int32_t game_fb_step(GameFbHandle *game, uint32_t dt_ms);
/* Tightly packed RGBA8 frame, valid until the next call on the handle. */
const uint8_t *game_fb_frame(GameFbHandle *game, uint32_t *width, uint32_t *height);
void game_fb_pointer_move(GameFbHandle *game, int32_t x, int32_t y);
/* button: 0 left, 1 right. */
void game_fb_pointer_button(GameFbHandle *game, int32_t button, int32_t pressed);
void game_fb_wheel(GameFbHandle *game, int32_t up);
/* 1 Enter, 2 Escape, 3 Space, 4 Up, 5 Down, 6 Left, 7 Right, 8 PageUp,
 * 9 PageDown, 10 Home, 11 End, 12 Backspace, 13 Tab, 14 Ctrl, 15 Shift,
 * 0x100+n F-n, 0x10000+c Unicode character c. */
void game_fb_key(GameFbHandle *game, uint32_t code, int32_t pressed);
void game_fb_text(GameFbHandle *game, const char *text_utf8);
int32_t game_fb_cursor_visible(GameFbHandle *game);
const char *game_fb_title(GameFbHandle *game);
void game_fb_close(GameFbHandle *game);

#if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
/* Runs any supported game in its own window until it ends (main thread). */
int32_t game_run_entry(const char *root_utf8, const char *nls);
#endif

#ifdef __cplusplus
}
#endif

#endif
