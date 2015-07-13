# ResourceForker
Extracts individual resources from a classic Mac applicaton's resource fork

## Build
Built on OS X 10.10.3 using `clang -o ResourceForker ResourceForker.c`

## Usage
ResourceForker analyzes and extracts resources from a resource fork stream that has been saved as a discrete file. At this time it doesn't read the 'com.apple.ResourceFork' extended attribute directly.

```
Usage: ResourceForker -[<flags>] <filename>
Supported flags:
	v: Verbose output
	d: Dump the individual resources as binary datafiles to the 'dump' subdirectory
	e: Extract known resource types and write datafiles to the 'resources' subdirectory
	h, ?: Display usage
Known types (extracted with -e):
	'icl8', extracted as 24bpp uncompressed BMPs
	'ICN#', extracted as 24bpp uncompressed BMPs
	'snd ', depends on compression.
		IMA4: extracted as raw signed 16bit PCM with a descriptor file
		Other: not extracted
	'TEXT', extracted with CR-LF line endings
```
