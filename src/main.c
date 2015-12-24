#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>

#include <pocketsphinx.h>

#include <stdio.h>
#include <string.h>

#include "continuous.h"

static ps_decoder_t *ps;
static cmd_ln_t *config;

int is_valid_phrase_callback(const char *phrase) {
    if(strlen(phrase) <= 1) {
        return 0;
    }
    return 1;
}

void recognized_callback(const char *phrase) {
    fprintf(stderr, "Recognized (callback): %s\n", phrase);
}

void kws_main(int argc, char *argv[]) {

    // char const *cfg;
    // config = cmd_ln_parse_r(NULL, cont_args_def, argc, argv, TRUE);
    config = cmd_ln_init(NULL, ps_args(), TRUE,
            "-hmm", MODELDIR "/en-us/en-us",
            "-dict", "./data/magic_phrase/d.dic",
            "-kws", "./data/magic_phrase/words.txt",
            "-kws_threshold", "1e-6f",
            NULL);

    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return;
    }

    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);

    recognize_from_microphone(ps, config, recognized_callback, is_valid_phrase_callback);
    fprintf(stderr, "further content is here\n");

    ps_free(ps);
    cmd_ln_free_r(config);

}


int main(int argc, char *argv[]) {

    kws_main(argc, argv);

    return 0;

}
