
/*
 * $Id: agent.c,v 1.18 2003/08/21 17:38:55 erik Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>		/* gethostname() */
#include <sys/types.h>
#include <regex.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>


#ifdef DEBUG
#define LOG "agent.log"
#else
#define LOG "/var/log/apache/agent.log"
#endif

#define STATE "agent.state"

struct os
{
    int count;
    int mortal;
    char *name;
    char *regex;
    regex_t *preg;
};

struct os table[] = {
    {0, 1, "BSD", "bsd|fetch", 0},
    {0, 1, "Linux", "gnome|konq|linux|webdownloader", 0},
    {0, 1, "Unix",
     "aix|contype|java|lwp::|lynx|osix|perl|solaris|sunos|unix|wget|x11", 0},
    {0, 1, "Mac", "mac|apple", 0},
    {0, 1, "PDA", "gulliver", 0},
    {0, 0, "Bot (spider)",
     "aport|appie|arach|archiver|assort|asterias|augurfind|bot|bumblebee|crawl|docomo|efp@gmx\\.net|googlebot|griffon|harvest|hubater|indy library|infoseek|inktomi|intelliseek|internetseer|jack\n$|jeeves|lachesis|larbin|letscape|mail ?sweeper|marvin|mercator|moget|mozilla/3.0[1]? \\(compatible[;]?\\)|mozilla/5\\.0\n$|muscat|netcraft|offline explorer|pita|pompos|quepasacreep|scooter|search|slurp|spider|spyder|teleport pro|teoma|titan|validator|walker|webcollage|webcopier|webfountain|webreaper|webseek|webstripper|wfarc|zao|zeus|zyborg",
     0},
    {0, 0, "WinWorm", "^-\n$", 0},	/* erm, why is \n needed? */
    {0, 0, "bad guy tool", "simpsons cgi scanner|bordermanager", 0},
    {0, 1, "Windows", "win|msie|frontpage|microsoft|aol|gozilla", 0},
    {0, 0, "Other", ".*", 0},
    {0, 0, NULL, NULL, 0}
};

int
countcmp (const void *_a, const void *_b)
{
    struct os *a, *b;

    a = (struct os *) _a;
    b = (struct os *) _b;
    return -(a->count - b->count);
}

int
main (int argc, char **argv)
{
    int i = 0, total = 0, nixcount = 0, statefile, otherlog;
    unsigned long offset = 0;
    FILE *logfile;
    char buf[BUFSIZ];
    regmatch_t pmatch[1000];

    gethostname (buf, BUFSIZ);
    printf
	("Content-Type: text/html\n\n<HTML><BODY><CENTER><H2>OS Statistics for %s</H2><BR><BR>\n",
	 buf);

    while (table[i].regex)
      {
	  table[i].preg = (regex_t *) malloc (sizeof (regex_t));
	  regcomp (table[i].preg, table[i].regex,
		   REG_EXTENDED | REG_ICASE | REG_NOSUB);
	  i++;
      }

    logfile = fopen (LOG, "r");
    if (logfile == NULL)
      {
	  printf ("Unable to open log file (%s)\n", LOG);
	  perror ("agent: ");
	  return -1;
      }

    /* perhaps we should stat and do better reporting? */
    statefile = open (STATE, O_RDWR);
    if (statefile != -1)
      {
	  flock (statefile, LOCK_EX);
	  read (statefile, &offset, sizeof (long));
	  read (statefile, &total, sizeof (long));
	  i = 0;
	  while (table[i].name)
	    {
		read (statefile, &table[i].count, sizeof (int));
		++i;
	    }
	  lseek (statefile, 0, SEEK_SET);
      }
    else
	statefile = open (STATE, O_WRONLY | O_CREAT, 0660);

    otherlog = open ("/tmp/agent.unknown", O_WRONLY | O_CREAT, 0664);

    fseek (logfile, offset, SEEK_SET);

    while (fgets (buf, 1024, logfile))
      {
	  i = 0;
	  while (table[i].regex)
	    {
		if (regexec (table[i].preg, buf, strlen (buf), pmatch, 0) ==
		    0)
		  {
		      table[i].count++;
		      if (!strncmp (table[i].name, "Other", 5))
			  write (otherlog, buf, strlen (buf));
		      break;
		  }
		++i;
	    }
	  ++total;
      }
    close (otherlog);
    offset = ftell (logfile);
    fclose (logfile);
    write (statefile, &offset, sizeof (long));
    write (statefile, &total, sizeof (int));
    i = 0;
    while (table[i].name)
      {
	  write (statefile, &table[i].count, sizeof (int));
	  ++i;
      }
    flock (statefile, LOCK_UN);
    close (statefile);

    nixcount = table[0].count + table[1].count + table[2].count + table[3].count;

    qsort (table, 9, sizeof (struct os), countcmp);

    i = 0;
    printf
	("<TABLE CELLSPACING=0 CELLPADDING=0><TR><TH>OS</TH><TH>Count</TH><TH>Percent</TH></TR>\n");
    while (table[i].name)
      {
	  if (table[i].count)
	      printf ("<TR><TD>%s</TD><TD>%d</TD><TD>(% .2f %%)</TD></TR>\n",
		      table[i].name, table[i].count,
		      100.0 * (float) table[i].count / (float) total);
	  ++i;
      }
    printf ("<TR><TD><HR></TD><TD><HR></TD><TD><HR></TD></TR>\n");
    printf ("<TR><TD>*nix</TD><TD>%d</TD><TD>(% .2f %%)</TD></TR>\n",
	    nixcount, 100.0 * nixcount / (float) total);
    printf ("<TR><TD>Total</TD><TD>%d</TD><TD>(% .2f %%)</TD></TR>\n", total,
	    100.0 * total / (float) total);
    printf ("</TABLE>\n");

#ifndef CLEAN
    printf ("<BR><H2>Mortal Connectors</H2><BR>");
    i = 0;
    total = 0;
    while (table[i].name)
      {
	  if (table[i].mortal)
	      total += table[i].count;
	  ++i;
      }
    i = 0;
    printf
	("<TABLE CELLSPACING=0 CELLPADDING=0><TR><TH>OS</TH><TH>Count</TH><TH>Percent</TH></TR>\n");
    while (table[i].name)
      {
	  if (table[i].mortal && table[i].count)
	      printf ("<TR><TD>%s</TD><TD>%d</TD><TD>(% .2f %%)</TD></TR>\n",
		      table[i].name, table[i].count,
		      100.0 * (float) table[i].count / (float) total);
	  ++i;
      }
    printf ("<TR><TD><HR></TD><TD><HR></TD><TD><HR></TD></TR>\n");
    printf ("<TR><TD>*nix</TD><TD>%d</TD><TD>(% .2f %%)</TD></TR>\n",
	    nixcount, 100.0 * nixcount / (float) total);
    printf ("<TR><TD>Total</TD><TD>%d</TD><TD>(% .2f %%)</TD></TR>\n", total,
	    100.0 * total / (float) total);
    printf ("</TABLE>\n");
#endif


    printf ("</CENTER></BODY></HTML>\n");

    exit (EXIT_SUCCESS);
}
