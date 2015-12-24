#include <pocketsphinx.h>

int main(int argc, char *argv[]) {

    ps_decoder_t *ps;
    cmd_ln_t *config;
    FILE *fh;
    char const *hyp, *uttid;
    int16 buf[512];
    int rv;
    int32 score;

    char audio_filepath[] = "./data/test_set/apple.pcm";

    config = cmd_ln_init(NULL, ps_args(), TRUE,
            "-hmm", MODELDIR "/en-us/en-us",
            "-dict", "./data/test_set/ex.dic",
            "-kws", "./data/test_set/words.txt",
            "-kws_threshold", "1e-6f",
            NULL);

    // Use general English dictionary to match our wordlist
    /*
    config = cmd_ln_init(NULL, ps_args(), TRUE,
            "-hmm", MODELDIR "/en-us/en-us",
            "-lm", MODELDIR "/en-us/en-us.lm.bin",
            "-dict", MODELDIR "/en-us/cmudict-en-us.dict",
            "-kws", "./goforward/words.txt",
            "-kws_threshold", "1e-20f",
            "-logfn", "/dev/null",
            NULL);
            */

    if (config == NULL) {
        fprintf(stderr, "Failed to create config object, see log for details\n");
        return -1;
    }

    ps = ps_init(config);
    if (ps == NULL) {
        fprintf(stderr, "Failed to create recognizer, see log for details\n");
        return -1;
    }

    fh = fopen(audio_filepath, "rb");
    if (fh == NULL) {
        fprintf(stderr, "Unable to open input .raw file\n");
        return -1;
    }

    rv = ps_start_utt(ps);

    while (!feof(fh)) {
        size_t nsamp;
        nsamp = fread(buf, 2, 512, fh);
        rv = ps_process_raw(ps, buf, nsamp, FALSE, FALSE);
    }

    rv = ps_end_utt(ps);
    hyp = ps_get_hyp(ps, &score);
    printf("Recognized: %s\n", hyp);

    fclose(fh);
    ps_free(ps);
    cmd_ln_free_r(config);

    return 0;

}
