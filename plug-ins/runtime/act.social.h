#ifndef ACT_SOCIAL_H
#define ACT_SOCIAL_H

struct social_messg {
    int ch_min_pos, ch_max_pos, vict_min_pos, vict_max_pos;
    /* No argument was supplied */
    char *char_no_arg;
    char *others_no_arg;

    /* An argument was there, and a victim was found */
    char *char_found;           /* if NULL, read no further, ignore args */
    char *others_found;
    char *vict_found;

    /* An argument was there, but no victim was found */
    char *not_found;
    //Наречие по умолчанию
    char *nars;
};


struct adverb_list_data {
    char *adverb;
};

struct social_keyword {
    char *keyword;
    int social_message;
};

extern struct social_messg *soc_mess_list;
extern struct social_keyword *soc_keys_list;
extern struct adverb_list_data *adverb_list;

extern int top_of_socialm;
extern int top_of_socialk;
extern int top_of_adverb;

int do_social(struct char_data *ch, char *argument);
int find_action(char *cmd);
void load_socials(FILE * fl);
void load_adverb(void);
void adverb_init();
void adverb_destroy();
void socials_init();
void socials_destroy();

#endif
