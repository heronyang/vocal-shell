#include <pebble.h>
#include "vs_app.h"
#include "vs_const.h"

static Window *window;
static TextLayer *text_layer;

static DictationSession *s_multiple_choice_dictation_session;
static char s_text[VS_TEXT_SIZE];
static bool s_speaking_enabled;

// multiple_choice
char **VS_Multiple_Choices;
void (*VS_Multiple_Choices_Reponsed_Handler)(int);
int VS_Multiple_Choices_N;

/******************************* Interface Handler ********************************/
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
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
  // text_layer_set_text(text_layer, "Press Select to Start");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

/******************************* Helper Function ********************************/

/******************************* Dictation API ********************************/
static void multiple_choice_response_handler(char *transcription) {

  int i;

  // To lower case (prepare for later string comparisions
  for(i = 0; i < VS_TEXT_SIZE; i++) {
    char t = transcription[i];
    if (t >= 'A' && t <= 'Z') {
      transcription[i] += ('a' - 'A');  // to upper case
    }
  }

  // Find out index of the answer
  int result = VS_RETURN_ERROR;
  for(i = 0; i < VS_Multiple_Choices_N; i++) {
    if(strstr(transcription, VS_Multiple_Choices[i])) {
      result = i;
    }
  }

  // Call its reponse handler and pass the answer index in
  (*VS_Multiple_Choices_Reponsed_Handler)(result);

}

static void multiple_choice_dictation_session_callback(DictationSession *session, DictationSessionStatus status,
                                       char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
    // Check this answer
    multiple_choice_response_handler(transcription);
    APP_LOG(APP_LOG_LEVEL_INFO, "Transcription got:%s(size: %d)\n", transcription, strlen(transcription));
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Transcription failed.\n\nError ID:\n%d", (int)status);
  }

}

static void multiple_choice_timer_handler(void *context) {
  // Start dictation
  dictation_session_start(s_multiple_choice_dictation_session);
}

static void show_on_screen_for_seconds_handler(void *context) {

}

void show_on_screen_for_seconds(char *text, int seconds) {
  static char buf[VS_TEXT_SIZE] = {0};
  strcpy(buf, text);

  text_layer_set_text(text_layer, buf);

  // Freeze
  app_timer_register(seconds * 1000, show_on_screen_for_seconds_handler, NULL);
}

void multiple_choice_init(int n_choice, char **choices, void (*responsed_handler)(int)) {

  int i;

  // Invalid input checkings
  if(n_choice > VS_MULTIPLE_CHOICE_MAX) {
    return;
  }

  // Save response handler and choicss into global variables
  VS_Multiple_Choices_Reponsed_Handler = responsed_handler;
  VS_Multiple_Choices = choices;
  VS_Multiple_Choices_N = n_choice;

  // Show on screen
  static char buf[VS_TEXT_SIZE] = {0};
  APP_LOG(APP_LOG_LEVEL_INFO, "size: %d\n", n_choice);
  for(i = 0; i < n_choice; i++) {
    if(i == n_choice - 1) {
      strcat(buf, " or ");
    }
    else if(i != 0) {
      strcat(buf, ", ");
    }
    strcat(buf, choices[i]);
  }
  text_layer_set_text(text_layer, buf);

  // Freeze
  app_timer_register(VS_SLEEP_FOR_READ_SECOND * 1000, multiple_choice_timer_handler, NULL);

}

/******************************* Init / Deinit ********************************/
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
  s_multiple_choice_dictation_session = dictation_session_create(sizeof(s_text),
      multiple_choice_dictation_session_callback, NULL);
  s_speaking_enabled = true;
}

static void deinit(void) {
  window_destroy(window);
}

/******************************* Application ********************************/
void my_multiple_choice_reponse_handler(int selected) {

  // Log
  APP_LOG(APP_LOG_LEVEL_INFO, "Answer Index: %d\n", selected);

  // Put transcription into static char array
  static char buf[VS_TEXT_SIZE];
  snprintf(buf, VS_TEXT_SIZE, "ok, %s (index:%d)", VS_Multiple_Choices[selected], selected);

  // Show on watch
  text_layer_set_text(text_layer, buf);

}

/******************************* Main ********************************/
int main(void) {

  init();

  /* TODO: this part should be in vs_app_body */
  show_on_screen_for_seconds_handler("Hi, welcome to notificaion!\nWhat can I help?", VS_SLEEP_FOR_READ_SECOND);

  char *choices [] = {"apple", "banana", "cat"};
  int n_choice = 3;
  multiple_choice_init(n_choice, choices, &my_multiple_choice_reponse_handler);

  vs_app_body();

  app_event_loop();
  deinit();

  return 0;

}
