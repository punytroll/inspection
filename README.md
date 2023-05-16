# What?

_inspection_ is a project to design and implement a general approach to file inspection.

An inspector from the _inspection_ project provides a hierarchically structured view of a file's content, if the formats of that file or parts thereof are known. It is the express goal of this project to account for each and every bit in the relevant part of the file.

The tools are currently designed to present clear output on the command line. New [file formats](#file-formats) can be defined to extend the reach of the project. [Specialized inspectors](#inspectors) are provided to filter for certain file contents.

## File formats

This project started out as an attempt to analyze ID3 tags in audio files. In that vein, more audio and other media formats have been added partially.

- ID3 tags (ID3v1, ID3v1.1, ID3v2.2, ID3v2.3, ID3v2.4)
- FLAC (full support, right down to the bits of the residual, including vorbis comments)
- APE tags (APEv2)
- MPEGv1 (all frame headers in a stream)
- RIFF
- ASF
- Vorbis comments and file structure
- WavPack
- AppleSingle
- BMP, ICO

## inspectors

**Note**: Inspectors don't consider the file extension when inspecting a file - instead, they always investigate the file's content!

### generalinspector

First and foremost is the [_generalinspector_](source/inspectors/general). This program is meant to parse any file and does a best effort to recognize its content. Of course, this is limited by the number of known formats. For example, an MP3 file may be structured in any number of different ways (using the type names from the [type library](data/type_library) and the pipe symbol to denote sequential parts):

- MPEG.1.Stream
- MPEG.1.Stream | ID3.v1.Tag
- MPEG.1.Stream | APE.Tag
- MPEG.1.Stream | APE.Tag | ID3.v1.Tag
- ID3.v2_Tag | MPEG.1.Stream
- ID3.v2_Tag | MPEG.1.Stream | ID3.v1.Tag
- ID3.v2_Tag | MPEG.1.Stream | APE.Tag
- ID3.v2_Tag | MPEG.1.Stream | APE.Tag | ID3.v1.Tag
- ...

At the moment the generalinspector has a [predefined and hardcoded](source/inspectors/general/generalinspector.cpp#L26) list of possible parts in a file. It is the intention of further development to ease this restriction and become more generic.

The _generalinspector_ has two powerful options to help investigate a file:

#### `--types=<type>;...`

With this option, you can force the _generalinspector_ to interpret the file as a specific sequence of types as taken from the type library.

#### `--query=<query>`

With this option, you select a certain part of the output and only display that. This allows you to drill down to the level of individual data pieces and tags, query for the existance of fields or tags and  select fields based on their properties.

### id3inspector

The [_id3inspector_](source/inspectors/id3) can be used to display only the ID3 parts of a file and skip all other audio data. For this to work ID3v2 must be at the beginning of the file and ID3v1 must be at the end, as per specification. ID3 tags of a certain version can be requested (```--id3v1-only``` and ```--id3v2-only```).

### flacinspector

The [_flacinspector_](source/inspectors/flac) displays the FLAC stream's meta data blocks, including the vorbis comment. All audio data is skipped by default, except when requested (```--with-frames``` - prepare for a **LONG** wait).

### mpeginspector

The [_mpeginspector_](source/inspectors/mpeg) displays the header information of **all** MPEG frames in an MPEG stream. By default, this program is very strict and doesn't allow any extra-frame data (i.e. tags of any kind). It can be instructed to seek for all MPEG frames though (```--seek```).

### apeinspector

The [_apeinspector_](source/inspectors/ape) searches for all APE tags in a file and displays them. All other content is skipped with a comment.

### asfinspector

The [_asfinspector_](source/inspectors/asf) tries to interpret the input file as one ASF file and displays all meta data. The file might be a .wmv or a .wma. The actual video or audio data is not yet inspected.

### riffinspector

The [_riffinspector_](source/inspectors/riff) tries to interpret the input file as one RIFF chunk with all content and displays all meta data. Predominantly, a RIFF file might have an extension .avi or .wav. The actual video or audio data is not yet inspected.

### vorbisinspector

The [_vorbisinspector_](source/inspectors/vorbis) tries to interpret the input file as one Ogg Stream and displays the file's structure, as well as any Vorbis comments contained therein. It does not interpret audio data.

# Technical

## Getting the code

Simply do:

```bash
git clone https://github.com/punytroll/inspection.git
```

## Building

TL;DR

```bash
meson setup build
meson compile -C build
meson test -C build
meson compile check -C build
```

For more information, please consult the file [documentation/BUILDING.md](documentation/BUILDING.md).
