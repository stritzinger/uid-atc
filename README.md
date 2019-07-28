# UID-ATC

To be portable this should be built in `/opt/grisp/grisp-software/uid-atc/$GITHASH` with `$GITHASH` being the first 10 letters of the revision hash.

## Build

```
$ git submodule update --init rtems-source-builder
$ make all
```
