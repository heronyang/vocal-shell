#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>

#include <pocketsphinx.h>

#include <stdio.h>
#include <string.h>

#include <time.h>

#include "continuous.h"

#define KWS_THREASHOLD "1e-20f"
#define KWS_THREASHOLD_2 "1e-16f"

#define WORD_SIZE 127
#define CHOICE_SIZE 256
#define CHOICE_N_MAX 5
#define SAY_SIZE_MAX 2048

#define PLAY_SOUND_COMMAND "mpg123 "
#define TTS_COMMAND "say -v \"Kathy\" "

/* Prototypes */
int app_notification();
int app_time();
int app_music();

int is_valid_phrase_callback(const char *phrase);
void vs_say(const char *sentence);
void vs_init_wait();
void vs_end();
const char *vs_select_app();
int vs_run_app(const char *app_name);
void vs_open_phone_app(const char *phone_app_name);


/* Helper Functions */
int is_valid_phrase_callback(const char *phrase) {
    if(strlen(phrase) <= 1) {
        return 0;
    }
    return 1;
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

    /* KWS setups */
    ps_decoder_t *ps;
    cmd_ln_t *config;

    config = cmd_ln_init(NULL, ps_args(), TRUE,
            "-hmm", MODELDIR "/en-us/en-us",
            "-dict", "./data/yes_no/d.dic",
            "-kws", "./data/yes_no/words.txt",
            "-kws_threshold", KWS_THREASHOLD_2,
            "-logfn", "./log",
            NULL);

    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return -1;
    }

    /* KWS */
    const char *phrase;
    phrase = recognize_from_microphone(ps, config, NULL, is_valid_phrase_callback);

    /* Handle response */
    int response = -1;
    if(phrase) {
        if(strstr(phrase, "YES")) {
            response = 1;
        } else if(strstr(phrase, "NO")) {
            response = 0;
        }
    }

    ps_free(ps);
    cmd_ln_free_r(config);

    return response;
}

int vs_ask_multiple_choice(char *dict_file_path, char *question_list_file_path) {

    /* Load choices */
    FILE *fin = fopen(question_list_file_path, "r");
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
    ps_decoder_t *ps;
    cmd_ln_t *config;

    config = cmd_ln_init(NULL, ps_args(), TRUE,
            "-hmm", MODELDIR "/en-us/en-us",
            "-dict", dict_file_path,
            "-kws", question_list_file_path,
            "-kws_threshold", KWS_THREASHOLD_2,
            "-logfn", "./log",
            NULL);

    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return -1;
    }

    /* KWS */
    const char *phrase;
    phrase = recognize_from_microphone(ps, config, NULL, is_valid_phrase_callback);

    /* Handle response */
    int ans = -1;
    if(phrase) {
        for( i = 0; i < n; i++ ) {
            if(strstr(choices[i], phrase)) {
                ans = i;
            }
        }
    }

    ps_free(ps);
    cmd_ln_free_r(config);

    return ans;
}

void vs_open_phone_app(const char *phone_app_name) {
    char sentence[SAY_SIZE_MAX];
    sprintf(sentence, "open %s on the phone", phone_app_name);
    vs_say(sentence);
    return;
}

void vs_init_wait() {

    ps_decoder_t *ps;
    cmd_ln_t *config;

    /* KWS setups */
    config = cmd_ln_init(NULL, ps_args(), TRUE,
            "-hmm", MODELDIR "/en-us/en-us",
            "-dict", "./data/magic_phrase/d.dic",
            "-kws", "./data/magic_phrase/words.txt",
            "-kws_threshold", KWS_THREASHOLD,
            "-logfn", "./log",
            NULL);

    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return;
    }

    /* KWS */
    const char *phrase;
    phrase = recognize_from_microphone(ps, config, NULL, is_valid_phrase_callback);

    /* Handle response */
    if(phrase) {
        fprintf(stderr, "phrase got (finial): %s\n", phrase);
        system(PLAY_SOUND_COMMAND "./sounds/start.mp3");
    }

    ps_free(ps);
    cmd_ln_free_r(config);

}

void vs_end() {
    fprintf(stderr, "end\n");
    system(PLAY_SOUND_COMMAND "./sounds/end.mp3");
}

const char *vs_select_app() {

    vs_say("which app");

    ps_decoder_t *ps;
    cmd_ln_t *config;

    /* KWS setups */
    config = cmd_ln_init(NULL, ps_args(), TRUE,
            "-hmm", MODELDIR "/en-us/en-us",
            "-dict", "./data/apps/d.dic",
            "-kws", "./data/apps/words.txt",
            "-kws_threshold", KWS_THREASHOLD_2,
            "-logfn", "./log",
            NULL);

    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return NULL;
    }

    /* KWS */
    const char *phrase;
    phrase = recognize_from_microphone(ps, config, NULL, is_valid_phrase_callback);

    /* Handle response */
    return (phrase)? phrase: NULL;

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
    int selected_feature_ind = vs_ask_multiple_choice("./data/multiple_choice/notification_features/d.dic",
            "./data/multiple_choice/notification_features/words.txt");

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
    sprintf(result, " Current local time and date is %s", asctime (timeinfo));
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
        if(app_name)    vs_run_app(app_name);
        vs_end();

    }

    return 0;

}
