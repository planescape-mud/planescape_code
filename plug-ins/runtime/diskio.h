/*
 diskio.h Fast file buffering

 Version 1.2b This is beta software.  Use at your own risk.
*/

#ifndef _DISKIO_H_
#define _DISKIO_H_

#define FB_READ  (1 << 0)       /* read from disk */
#define FB_WRITE (1 << 1)       /* write to disk */
#define FB_APPEND (1 << 2)      /* write with append */

#define FB_STARTSIZE 4192       /* 4k starting buffer for writes */

#ifndef IS_SET
#define IS_SET(flag, bits) ((flag) & (bits))
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct {
    char *buf;                  /* start of buffer   */
    char *ptr;                  /* current location pointer  */
    int size;                   /* size in bytes of buffer  */
    int flags;                  /* read/write/append, future expansion */
    char *name;                 /* filename (for delayed writing) */
} FBFILE;

void tag_argument(char *argument, char *tag);
int fbgetline(FBFILE * fbfl, char *line);
FBFILE *fbopen(const char *fname, int mode);
int fbclose(FBFILE * fbfl);
int fbprintf(FBFILE * fbfl, const char *format, ...);
void fbrewind(FBFILE * fbfl);
int fbcat(char *fromfilename, FBFILE * tofile);
char *fbgetstring(FBFILE * fl);

#endif
