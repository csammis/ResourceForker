# ResourceForker
Extracts individual resources from a classic Mac applicaton's resource fork

## Build
Built on OS X 10.10.3 using `clang -o ResourceForker ResourceForker.c`

## Usage
ResourceForker analyzes and extracts resources from a resource fork stream. If -r is specified, the resource fork stream is sourced from a binary file representing the resource fork. If -r is omitted, ResourceForker will attempt to source the resource fork stream from the `com.apple.ResourceFork` extended attribute of \<filename\>

```
Usage: ResourceForker -[<flags>] <filename>
Supported flags:
	v: Verbose output
	d: Dump the individual resources as binary datafiles to the 'dump' subdirectory
	e: Extract known resource types and write datafiles to the 'resources' subdirectory
	r: <filename> is the raw content of a resource fork (see Resource Forks)
	h, ?: Display usage
Known types (extracted with -e):
	'icl8', extracted as 24bpp uncompressed BMPs
	'ICN#', extracted as 24bpp uncompressed BMPs
	'snd ', depends on compression.
		IMA4: extracted as raw signed 16bit PCM with a descriptor file
		Other: not extracted
	'STR', 'STR#', extracted with CR-LF line endings
	'TEXT', extracted with CR-LF line endings
	'NAME', 'DLL#', extracted as lists with one signature per line
File format options:
	The -r flag is used when <filename> is a raw binary file representing the resource fork of an application.
	When -r is not used, ResourceForker will try to read <filename>'s 'com.apple.ResourceFork' extended attribute and analyze that instead.
```
