# agent

Apache log analyzer CGI — classifies user-agent strings (OS / browser / bot /
crawler / etc.) from an access log and emits an HTML statistics table.

Vintage 2008 code, ANSI C, kept as a historical artifact.

## Build / run

```sh
make            # builds ./agent (-O2)
make debug      # ./agent-debug, log defaults to ./agent.log
make clean
```

Run against a log file (writes an HTML table to stdout):

```sh
./agent /path/to/access.log > out.html
```

Install as a CGI binary (needs root for the cgi-bin path):

```sh
sudo make install   # strips + copies to /usr/lib/cgi-bin/agent
```

## How it works

`agent.c` is a single translation unit. It compiles a regex table of user-agent
categories (BSD, Linux, Mac, Unix, PDA, Bot, Windows, …), keeps persistent
counters and a byte offset in a binary `agent.state` file so each run only
consumes the new log lines since the previous run, and writes an HTML 3.2-style
table to stdout with a CGI `Content-Type` header. First matching regex wins, so
table order matters; unmatched lines are appended to `/tmp/agent.unknown` for
pattern tuning.

Bot patterns live in `bots` (one fragment per line); `make` flattens that into
`bots.regex`, which is `#include`d into the regex table at build time — edit
`bots`, never the generated `bots.regex`.

## License

GPL-2.0 — see [COPYING](COPYING).
