#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	/* gethostname() */
#include <sys/types.h>
#include <regex.h>

#ifdef DEBUG
#define LOG "agent.log"
#else
#define LOG "/var/log/apache/agent.log"
#endif

#define STATE "agent.state"

struct os
{
  int count;
  char *name;
  char *regex;
  regex_t *preg;
};

struct os table[] = {
  {0, "BSD", "bsd|fetch", 0},
  {0, "Linux", "linux|konq|gnome", 0},
  {0, "*nix", "X11|Lynx|sunos|aix|perl|wget|contype", 0},
  {0, "All *nix", "fjdkaslvjhsaljdvklsavjds", 0},
  {0, "Mac", "mac", 0},
  {0, "PDA", "gulliver", 0},
  {0, "Windows", "win|msie|frontpage|microsoft|aol|gozilla", 0},
  {0, "Bot (spider)",
   "bot|spider|arach|crawl|harvest|slurp|griffon|walker|scooter|archiver|asterias|search|spyder|hubater|letscape|titan|Mozilla/3.01 (compatible;)",
   0},
  {0, "WinWorm", "^-\n$", 0},	/* erm, why is \n needed? */
  {0, "Other", ".*", 0},
  {0, NULL, NULL, 0}
};

int
main (int argc, char **argv)
{
  int i = 0, total = 0;
  long offset = 0;
  FILE *fd[2];
  char buf[BUFSIZ];
  regmatch_t pmatch[1000];

  gethostname (buf, BUFSIZ);
  printf
    ("Content-Type: text/html\n\n<HTML><BODY><CENTER>OS Statistics for %s<BR><BR>\n",
     buf);

  while (table[i].regex)
    {
			table[i].preg = (regex_t *)malloc(sizeof(regex_t));
      regcomp (table[i].preg, table[i].regex,
	       REG_EXTENDED | REG_ICASE | REG_NOSUB);
      i++;
    }

  fd[0] = fopen (LOG, "r");
  if (fd[0] == NULL)
    {
      printf ("Unable to open log file (%s)\n", LOG);
      perror ("agent: ");
      return -1;
    }
  fd[1] = fopen (STATE, "r+");
  if (fd[1])
    {
      fread (&offset, sizeof (long), 1, fd[1]);
      fread (&total, sizeof (long), 1, fd[1]);
      i = 0;
      while (table[i].name)
	{
	  fread (&table[i].count, sizeof (int), 1, fd[1]);
	  ++i;
	}
      fseek (fd[1], 0, SEEK_SET);
    }
  else
    fd[1] = fopen (STATE, "w");
  fseek (fd[0], offset, SEEK_SET);

  while (fgets (buf, 1024, fd[0]))
    {
      i = 0;
      while (table[i].regex)
	{
	  if (regexec (table[i].preg, buf, strlen (buf), pmatch, 0) == 0)
	    {
	      table[i].count++;
	      break;
	    }
	  ++i;
	}
      ++total;
    }
  offset = ftell (fd[0]);
  fclose (fd[0]);
  fwrite (&offset, sizeof (long), 1, fd[1]);
  fwrite (&total, sizeof (int), 1, fd[1]);
  i = 0;
  while (table[i].name)
    {
      fwrite (&table[i].count, sizeof (int), 1, fd[1]);
      ++i;
    }
  fclose (fd[1]);

  table[3].count = table[0].count + table[1].count + table[2].count;

  i = 0;
  printf ("<TABLE><TR><TH>OS</TH><TH>Count</TH><TH>Percent</TH></TR>\n");
  while (table[i].name)
    {
      printf ("<TR><TD>%s</TD><TD>%d</TD><TD>(% .2f %%)</TD></TR>\n",
	      table[i].name, table[i].count,
	      100.0 * (float) table[i].count / (float) total);
      ++i;
    }
  printf ("</TABLE></CENTER></BODY></HTML>\n");

	exit (EXIT_SUCCESS);
}
