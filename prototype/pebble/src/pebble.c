#include <pebble.h>

#define TEXT_SIZE 256

static Window *window;
static TextLayer *text_layer;

static DictationSession *s_dictation_session;
static char s_text[TEXT_SIZE];
static bool s_speaking_enabled;

/******************************* Dictation API ********************************/

static void show_transcription(char *transcription) {
  // put transcription into static char array
  static char buf[TEXT_SIZE];
  snprintf(buf, TEXT_SIZE, "%s", transcription);

  // show on watch
  text_layer_set_text(text_layer, buf);
}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status,
                                       char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
    // Check this answer
    show_transcription(transcription);
    APP_LOG(APP_LOG_LEVEL_INFO, "Transcription got:%s(size: %d)\n", transcription, strlen(transcription));
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Transcription failed.\n\nError ID:\n%d", (int)status);
  }
}

/******************************* Interface Handler ********************************/
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Start");

  // Start voice dictation UI
  s_speaking_enabled = false;
  dictation_session_start(s_dictation_session);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Down");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 80 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press Select to Start");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
      });
  const bool animated = true;
  window_stack_push(window, animated);

  // Create new dictation session
  s_dictation_session = dictation_session_create(sizeof(s_text),
      dictation_session_callback, NULL);
  s_speaking_enabled = true;
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
