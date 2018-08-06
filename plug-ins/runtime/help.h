#ifndef HELP_H
#define HELP_H

struct help_index_data {
    int number;
    char *title;
    char *keyword;
    int type;
    char *entry;
    char *links;
    char *format;
    int duplicate;
    struct help_index_data *next;
};

extern struct help_index_data *help_system;

void load_helpx();

void help_init();
void help_destroy();

#endif
