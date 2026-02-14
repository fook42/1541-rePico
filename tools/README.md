# conv_x64 #

small tool to convert d64/g64 in d64/g64 image format

## main purpose ##

testing of read/write functions from 1541-rePico

## usage ##

```conv_x64 <file_in> <file_out>```

will determine extension of file_in (.d64 or .g64)
read the contents and convert it internally to GCR/G64 structures

will determine extension of file_out (.d64 or .g64)
write out the G64 structures either as D64 or G64 file

## how to build ##

```gcc -fcommon -o conv_x64 conv_x64.c gcr.c rw_tracks.c```

please ignore the warnings/infos.. its just type conversion


2026/02/14 - fook42

