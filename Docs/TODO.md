## Brf To-Do List

Add in shim functions for all the other file-related system calls.

Solve the problem with link order of functions so that the Brf
library can be linked in to existing utilities and work with
the "stdio" library.

Improve performance if possible.

Add support for relative filenames, for filenames with ".." in them
and the ability to `cd` into and around the remote filesystem.

Properly map the `errno` values on the server to the 2.11BSD
equivalents.
