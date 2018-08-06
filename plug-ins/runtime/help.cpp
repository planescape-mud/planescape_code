
#include "sysdep.h"
#include "help.h"
#include "xhelp.h"
#include "parser_id.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "planescape.h"
#include "mudfilereader.h"

struct help_index_data *help_system = 0;        /* óÉÓÔÅÍÁ ÐÏÍÏÝÉ */

void load_helpx()
{
    CHelp Help;
    struct help_index_data *add;

    if (!Help.Initialization())
        exit(1);

    MudFileReader reader(mud->helpDir);

    while (reader.hasNext())
        if (!Help.ReadConfig(reader.next().getCPath()))
            exit(1);

    int number = Help.GetNumberItem();

    for (int i = 0; i < number; i++) {
        CItem *help = Help.GetItem(i);
        CREATE(add, struct help_index_data, 1);

        if (!mud->modQuietLoad)
            log("úáçòõúëá: ÓÐÒÁ×ËÁ \"%s\"", help->GetItem(HELP_TITLE)->GetString());

        add->number = help->GetItem(HELP_NUMBER)->GetInt();

        CREATE(add->title, char, strlen(help->GetItem(HELP_TITLE)->GetString()) + 1);

        strcpy(add->title, help->GetItem(HELP_TITLE)->GetString());

        CREATE(add->keyword, char, strlen(help->GetItem(HELP_ALIAS)->GetString()) + 1);

        strcpy(add->keyword, help->GetItem(HELP_ALIAS)->GetString());

        CREATE(add->entry, char, strlen(help->GetItem(HELP_CONTENT)->GetString()) + 1);

        strcpy(add->entry, help->GetItem(HELP_CONTENT)->GetString());

        if (help->GetItem(HELP_FORMAT)->GetString()) {
            CREATE(add->format, char, strlen(help->GetItem(HELP_FORMAT)->GetString()) + 1);

            strcpy(add->format, help->GetItem(HELP_FORMAT)->GetString());
        }

        if (help->GetItem(HELP_LINKS)->GetString()) {
            CREATE(add->links, char, strlen(help->GetItem(HELP_LINKS)->GetString()) + 1);

            strcpy(add->links, help->GetItem(HELP_LINKS)->GetString());
        }

        add->type = help->GetItem(HELP_TYPE)->GetInt();

        add->next = help_system;
        help_system = add;
    }
}

void help_init()
{
    /* before world loading */
    log("úÁÇÒÕÖÁÀ ÆÁÊÌÙ ÐÏÍÏÝÉ × xformatÅ");
    load_helpx();
}

void help_destroy()
{
    struct help_index_data *h;

    for (h = help_system; h; h = h->next) {
        FREEPTR(h->title);
        FREEPTR(h->keyword);
        FREEPTR(h->entry);
        FREEPTR(h->format);
        FREEPTR(h->links);
    }

    DESTROY_LIST(help_system, next, h);
}
