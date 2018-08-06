#ifndef BAN_H
#define BAN_H

/* don't change these */
#define BAN_NOT  0
#define BAN_NEW  1
#define BAN_SELECT 2
#define BAN_ALL  3
#define BAN_MULTER 4

#define BANNED_SITE_LENGTH    50
#define BANNED_METTER_LENGTH  80
struct ban_list_element {
    char site[BANNED_SITE_LENGTH + 1];
    int type;
    time_t date;
    char name[MAX_NAME_LENGTH + 1];
    char metter[BANNED_METTER_LENGTH + 1];
    struct ban_list_element *next;
};

extern struct ban_list_element *ban_list;

struct proxy_list_element {
    char site[BANNED_SITE_LENGTH + 1];
    int number;
    time_t date;
    char name[MAX_NAME_LENGTH + 1];
    char misc[MAX_STRING_LENGTH + 1];
    struct proxy_list_element *next;
};

extern struct proxy_list_element *proxy_list;


extern char *curses_list[MAX_INVALID_NAMES];
extern int num_curses;
extern int num_invalid;

void load_banned(void);
void load_proxy(void);
void Read_Invalid_List(void);
void Read_Valid_List(void);
void Read_Curses_List(void);
int Valid_Name(char *newname);
int Is_Valid_Name(char *newname);
int Is_Registry_Name(char *newname);
int Is_Valid_Name_no_mob(char *newname);
int Is_Valid_Dc(char *newname);

int is_proxy(char *hostname);
int isbanned(char *hostname);

void ban_destroy();
void ban_init();

#endif
