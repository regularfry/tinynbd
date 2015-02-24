tinynbd
=======

A collection of various different NBD servers.

In no way should these be considered production-worthy. About the best
you can say about the source is "well, I guess it works..."

The various servers under src/ are experiments in different ways to
serve NBD.  All the servers are single-threaded, single-client, with
only the most paltry of error-checking.

Nevertheless, they are functional enough to boot a qemu VM, given a
suitable disc image, and they are amenable to benchmarking.


Author
------

Alex Young <alex@blackkettle.org>
