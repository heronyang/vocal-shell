const char *recognize_from_microphone(ps_decoder_t *ps, cmd_ln_t *config, void (*response_callback)(const char *phrase), int (*legal_check_callback)(const char *phrase));

extern const arg_t cont_args_def[];
