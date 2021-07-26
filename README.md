# What?

_inspection_ is a project to design and implement a general approach to file inspection.

An inspector from the _inspection_ project provides a hierarchically structured view of a file's content, if the formats of that file or parts thereof are known. It is the express goal of this project to account for each and every bit in the relevant part of the file.

The tools are currently designed to present clear output on the command line. New [file formats](#file-formats) can be defined to extend the reach of the project. [Specialized inspectors](#inspectors) are provided to filter for certain file contents.

## File formats

This project started out as an attempt to analyze ID3 tags in audio files. In that vein, more audio formats have been added partially.

- ID3 tags (ID3v1, ID3v1.1, ID3v2.2, ID3v2.3, ID3v2.4)
- FLAC (full support, right down to the bits of the residual, including vorbis comments)
- APE tags (APEv2)
- MPEGv1 (all frame headers in a stream)
- RIFF
- ASF
- Vorbis comments and file structure

## inspectors

**Note**: No inspector works on the file extension but always the file content!

### generalinspector

First and foremost is the [_generalinspector_](inspectors/general). This program is meant to parse any file and does a best effort to recognize its content. Of course, this is limited by the number of known formats. For example, an MP3 file may be structured in any number of different ways (using the type names from the [type library](common/types) and the pipe symbol to denote sequential parts):

- MPEG.1.Stream
- MPEG.1.Stream | ID3.v1.Tag
- MPEG.1.Stream | APE.Tag
- MPEG.1.Stream | APE.Tag | ID3.v1.Tag
- ID3.v2_Tag | MPEG.1.Stream
- ID3.v2_Tag | MPEG.1.Stream | ID3.v1.Tag
- ID3.v2_Tag | MPEG.1.Stream | APE.Tag
- ID3.v2_Tag | MPEG.1.Stream | APE.Tag | ID3.v1.Tag
- ...

At the moment the generalinspector has a [predefined and hardcoded](inspectors/general/generalinspector.cpp) list of possible parts in a file. It is the intention of further development to ease this restriction and become more generic.

### id3inspector

The [_id3inspector_](inspectors/id3) can be used to display only the ID3 parts of a file and skip all other audio data. For this to work ID3v2 must be at the beginning of the file and ID3v1 must be at the end, as per specification. ID3 tags of a certain version can be requested (```--id3v1-only``` and ```--id3v2-only```).

### flacinspector

The [_flacinspector_](inspectors/flac) displays the FLAC stream's meta data blocks, including the vorbis comment. All audio data is skipped by default, except when requested (```--with-frames``` - prepare for a **LONG** wait).

### mpeginspector

The [_mpeginspector_](inspectors/mpeg) displays the header information of **all** MPEG frames in an MPEG stream. By default, this program is very strict and doesn't allow any extra-frame data (i.e. tags of any kind). It can be instructed to seek for all MPEG frames though (```--seek```).

### apeinspector

...

### asfinspector

...

### riffinspector

...

### vorbisinspector

...

# Technical

## Getting the code

At the moment, this project uses submodules, that you can pull in at cloning time with:

```bash
git clone https://github.com/punytroll/inspection.git --recurse-submodules
```

This is not required however, as the build will pull in the submodules automatically if they aren't available yet.

## Building

TL;DR

```bash
make
```

For more information, please consult the file [documentation/BUILDING.md](documentation/BUILDING.md).
