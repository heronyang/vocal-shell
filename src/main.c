#include <stdio.h>
#include <string.h>

#include <time.h>

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>

#include <pocketsphinx.h>

#include "continuous.h"

#define KWS_THREASHOLD "1e-20f"

#define WORD_SIZE 127
#define CHOICE_SIZE 256
#define CHOICE_N_MAX 5
#define SAY_SIZE_MAX 2048
#define FILEPATH_SIZE_MAX 1024

#define PLAY_SOUND_COMMAND "mpg123 "
#define TTS_COMMAND "say -v \"Kathy\" "

#define SUCCESS 1
#define FAILED -1

/* Prototypes */
int setupPS(const char *vocal_data_folder_path);
void freePS();

int is_valid_phrase_callback(const char *phrase);
void vs_say(const char *sentence);
void vs_init_wait();
void vs_end();
const char *vs_select_app();
int vs_run_app(const char *app_name);
void vs_open_phone_app(const char *phone_app_name);

int app_notification();
int app_time();
int app_music();

//
static ps_decoder_t *ps;
static cmd_ln_t *config;

int setupPS(const char *vocal_data_folder_path) {

    char lm_filepath[FILEPATH_SIZE_MAX],
         dic_filepath[FILEPATH_SIZE_MAX],
         words_filepath[FILEPATH_SIZE_MAX];
    sprintf(lm_filepath, "%s/l.lm", vocal_data_folder_path);
    sprintf(dic_filepath, "%s/d.dic", vocal_data_folder_path);
    sprintf(words_filepath, "%s/words.txt", vocal_data_folder_path);

    config = cmd_ln_init(NULL, ps_args(), TRUE,
            "-hmm", MODELDIR "/en-us/en-us",
            "-lm", lm_filepath,
            "-dict", dic_filepath,
            "-kws", words_filepath,
            "-kws_threshold", KWS_THREASHOLD,
            "-logfn", "./log",
            NULL);

    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return FAILED;
    }

    return SUCCESS;

}

void freePS() {
    ps_free(ps);
    cmd_ln_free_r(config);
}


/* Vocal Shell Functions */
void vs_say(const char *sentence) {
    char command[SAY_SIZE_MAX];
    sprintf(command, TTS_COMMAND "%s", sentence);
    fprintf(stderr, "vs say: %s\n", command);
    system(command);
}

int vs_ask_yes_no(const char *question) {

    /* Say the question to the user */
    vs_say(question);

    /* Setup PS configs */
    if(setupPS("./data/yes_no/") != SUCCESS) {
        return FAILED;
    }

    /* KWS */
    const char *phrase;
    phrase = recognize_from_microphone(ps, config);

    /* Handle response */
    int response = FAILED;
    if(phrase) {
        if(strstr(phrase, "YES")) {
            response = 1;
        } else if(strstr(phrase, "NO")) {
            response = 0;
        }
    }

    freePS();

    return response;
}

int vs_ask_multiple_choice(const char *vocal_data_folder_path) {

    char words_filepath[FILEPATH_SIZE_MAX];
    sprintf(words_filepath, "%s/words.txt", vocal_data_folder_path);

    /* Load choices */
    FILE *fin = fopen(words_filepath, "r");
    char choices[CHOICE_N_MAX][CHOICE_SIZE], choice[CHOICE_SIZE];
    int n = 0;
    while(fgets(choice, CHOICE_SIZE, fin) != NULL) {
        strcpy(choices[n], choice);
        n++;
    }

    /* Say the question to the user */
    int i;
    char hint_sentence[SAY_SIZE_MAX];
    strcpy(hint_sentence, "which one ");
    for( i = 0; i < n; i ++) {
        if (i == (n-1)) {
            strcat(hint_sentence, ", or ");
        } else {
            strcat(hint_sentence, ", ");
        }
        strcat(hint_sentence, strtok(choices[i], "\n"));
    }
    vs_say(hint_sentence);

    /* KWS setups */
    if(setupPS(vocal_data_folder_path) != SUCCESS) {
        return FAILED;
    }

    /* KWS */
    const char *phrase;
    phrase = recognize_from_microphone(ps, config);

    /* Handle response */
    int ans = FAILED;
    if(phrase) {
        for( i = 0; i < n; i++ ) {
            if(strstr(choices[i], phrase)) {
                ans = i;
            }
        }
    }

    freePS();

    return ans;
}

void vs_open_phone_app(const char *phone_app_name) {
    char sentence[SAY_SIZE_MAX];
    sprintf(sentence, "open %s on the phone", phone_app_name);
    vs_say(sentence);
    return;
}

void vs_init_wait() {

    if(setupPS("./data/magic_phrase/") != SUCCESS) {
        return;
    }

    /* KWS */
    const char *phrase;
    phrase = recognize_from_microphone(ps, config);

    /* Handle response */
    if(phrase) {
        fprintf(stderr, "phrase got (finial): %s\n", phrase);
        system(PLAY_SOUND_COMMAND "./sounds/start.mp3");
    }

    freePS();

}

void vs_end() {

    fprintf(stderr, "end\n");
    system(PLAY_SOUND_COMMAND "./sounds/end.mp3");

}

const char *vs_select_app() {

    vs_say("which app");

    if(setupPS("./data/apps/") != SUCCESS) {
        return NULL;
    }

    /* KWS */
    const char *phrase;
    phrase = recognize_from_microphone(ps, config);

    freePS();
    return phrase;

}

int vs_run_app(const char *app_name) {
    if(strstr(app_name, "NOTIFICATION")) {
        return app_notification();
    } else if(strstr(app_name, "TIME")) {
        return app_time();
    } else if(strstr(app_name, "MUSIC")) {
        return app_music();
    } else {
        system(TTS_COMMAND "Wrong App");
        return -1;
    }
}

/* Apps */
int app_notification() {
    int selected_feature_ind = vs_ask_multiple_choice("./data/multiple_choice/notification_features/");

    if(selected_feature_ind == 0) { // READ ALL NOTIFICATIONS
        vs_say("You have 2 notifications, first notification: Facebook, your friend john liked your photo. second notification: CNN, one aircraft had been found in Mexico");
    } else if(selected_feature_ind == 1) { // READ NOTIFICATION OF ONE APP
        vs_say("feature incompleted");
    } else if(selected_feature_ind == 2) {
        vs_say("okay done");
    }

    return 0;
}

int app_time() {

    char result[SAY_SIZE_MAX];
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    sprintf(result, " It is %d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    vs_say(result);

    return 0;
}

int app_music() {

    if(vs_ask_yes_no("Do you want to start playing music randomly?")) {
        vs_open_phone_app("music");
    } else {
        vs_say("okay bye");
    }
    return 0;

}

/* Main */
int main(int argc, char *argv[]) {

    for(;;) {

        vs_init_wait();
        const char *app_name = vs_select_app();
        vs_run_app(app_name);
        vs_end();

    }

    return 0;

}
