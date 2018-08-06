/* $Id$
 *
 * ruffina, 2009
 */
#include "logstream.h"
#include "planescape.h"

#include "sysdep.h"
#include "structs.h"
#include "comm-decl.h"

/*
 * set malloc options to abort program execution on any error
 */
static void set_strict_malloc()
{
#ifdef __FreeBSD__
    extern const char* _malloc_options;
    // all warnings become fatal: abort() is called
    static char new_options[] = "A";
    _malloc_options = new_options; 
#endif
#ifdef __linux__
    // print diagnostic message to stderr and abort;
    // overrides previous definition of this env.var
    setenv("MALLOC_CHECK_", "3", 1);
#endif
}

/*
 * print usage summary
 */
static void usage(const char *progname)
{
   /* Do NOT use -C, this is the copyover mode and without
    * the proper copyover.dat file, the game will go nuts!
    * -spl 
    */
    printf
	("Usage: %s [-c] [-m] [-q] [-r] [-s] [-d pathname] [-p portnum]\n"
	 "  -d <directory> Specify library directory (defaults to 'share').\n"
	 "  -h		Print this command line argument help.\n"
	 "  -m		Start in mini-MUD mode.\n"
	 "  -o <file>	Write log to <file> instead of stderr.\n"
	 "  -q		Disable extended load messages.\n"
	 "  -r		Restrict MUD -- no new players allowed.\n"
	 "  -s		Suppress special procedure assignments.\n"
	 "  -p		Specify listen port.\n", progname);
}

/*
 * parse command line options with getopt(), 
 * these options override the ones specified in planescape.xml config file
 */
static void setopts(int argc, char **argv)
{
    int ch;
    const char *argv0 = argv[0];

    while ((ch = getopt(argc, argv, "C:d:hmo:qrsp:")) != -1) {
	switch (ch) {
	case 'C':
	    /* -C <socket number> - recover from copyover, this is the control socket */
	    mud->modCopyOver.override(true);
	    mother_desc = atoi(optarg);
	    break;

	case 'd':
	    mud->shareDir.override(optarg);
	    notice("Using '%s' as shared directory.", optarg);
	    break;

	case 'm':
	    mud->modMini.override(true);
	    notice("Running in minimized mode & with no rent check.");
	    break;

	case 'o':
	    mud->logPattern.override(optarg);
	    notice("Will write logs to %s.", optarg);
	    break;

	case 'q':
	    mud->modQuietLoad.override(true);
	    notice("Extended load messages disabled.");
	    break;

	case 'r':
	    mud->modRestrict.override(1);
	    notice("Restricting game -- no new players allowed.");
	    break;

	case 's':
	    mud->modNoSpecials.override(true);
	    notice("Suppressing assignment of special routines.");
	    break;

	case 'p':
	    mud->port.override(atoi(optarg));

	    if (mud->port <= 1024) {
		bug("SYSERR: Illegal port number '%s'.", optarg);
		exit(1);
	    }
            break;

	case 'h':
	    usage(argv0);
	    exit(0);

	default:
	    usage(argv0);
	    exit(1);
	}
    }

    argc -= optind;
    argv += optind;

    if (argc > 1) {
    	bug("SYSERR: Illegal argument string specified.");
    	usage(argv0);
    	exit(1);
    }

    if (argc > 0) {
    	mud->setConfigFilePath(*argv);
    }
    
    mud->execFileName = argv0;
}

/*
 * main 
 */
int main(int argc, char **argv)
{
    set_strict_malloc();

    try {
		new PlaneScape();

		setopts(argc, argv);

		mud->load();

		try {
			mud->startup();
			mud->loop();
			mud->shutdown();
		} catch(const Exception & rex) {
			rex.printStackTrace(LogStream::sendFatal());
		}

		mud->save();

    } catch(const Exception & ex) {
		LogStream::sendFatal() << "Caught Exception in main() " << endl;
		ex.printStackTrace(LogStream::sendFatal());
		return 1;

    }
    catch(const std::exception & stdex) {
		LogStream::sendFatal() << "Caught std::exception in main() "
							   << stdex.what() << endl;
		return 1;

    }
    catch(...) {
		LogStream::sendFatal() << "Caught unknown exception in main()"
							   << endl;
		return 1;
    }

    return 52;                 /* what's so great about HHGTTG, anyhow? */
}
