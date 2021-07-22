UCS stands for Universal Character Set.
ISO/IEC 10646 is a standard, defining a set of characters.
Unicode is a standard, defining a set of characters.
UCS-2 is an older character encoding of Unicode codepoints.
UCS-2 is a fixed width character encoding, always using 2 bytes / 16 bits.
UCS-2 (as opposed to UTF-16) cannot encode with surrogate pairs.
ISO/IEC 8859-1:1997 is undefined in the inclusive hexadecimal ranges 0x00-0x1f and 0x7f-0x9f.
ISO/IEC 8859-1 was incorporated as the first 256 code points of ISO/IEC 10646 and Unicode.
ID3v2.3 is contradicting itself when stating that ISO/IEC 8859-1:1997 strings may contain characters in the range 0x20-0xff, because some of those vaules are undefined.
ID3v2.4 is contradicting itself when stating that ISO/IEC 8859-1:1997 strings may contain characters in the range 0x20-0xff, because some of those vaules are undefined.
ID3v2.3 is not sufficiently precise on whether strings, ending on a frame boundary, may be terminated.
ID3v2.3 is using the MIME type specification [RFC 2045] but contradicts its definition of what character encoding scheme is to be used. [ID3v2.3] allows [ISO/IEC 8859-1:1997] characters whereas [RFC 2045] allows only [US-ASCII] characters with further restrictions.
ID3v2.2 is not stating clearly, if the frame size should be given unsynchronized or plain. I will assume that unsynchronized size descriptors are used.
ID3v2.2 is not specific on whether UCS-2 strings must contain a byte order marker or not. I will assume that the byte order marker is required.
ID3v2.2, in section 3.2, specifies all string to be represented in ISO/IEC 8859-1. In particular, this applies to the three character string "image format" in a PIC frame.

ASF is vague in the description of field Codec Name Length of Codec Entries in Codec List Objects: it specifies "the number of Unicode characters stored in the Codec Name field". The content of the Codec Name field is a UTF16LE string, which means, that characters might also be 32 bits long with surrogate pairs. Additionally, some Unicode code points are not really characters, but parts of a glyph. To address the latter issue, I will assume, that this actually means the number of code points, although I am pretty sure, that this will fail when we hit the first surrogate pair code point.

ID3v2.3
=======

1) It is not stated clearly WHAT is being unsynchronized under the unsynchronization scheme!
Therefore is is equally unclear, WHAT needs to be de-unsynchronized when reading the tag. Without having read the Unsynchronization flag from the tag header, it would be wrong to set up any de-unsynchronization mechanism, because, while writing the tag, unsynchronization is not strictly necessary, even in the face of false synchronizations. (see ID3v2.3 chapter 5) I think it is only logical, that, should we choose to get rid of false synchronizations while writing the tag, AFTER unsynchronizing, all of the tag is synchronization free. Since the header takes measures to be free of false synchronizations (ASCII characters and synch-free integers), only the tag body (everything after the 10 byte header) requires to be handled by de-unsynchronizing.

2) It is not stated clearly whether the size of a frame is given as BEFORE or AFTER unsychronization has been performed while writing the tag.
My guess would be BEFORE! As stated in point 1), the unsynchronization is probably run on all of the tag body, without heed for frame content. I think, therefore, that the unsychronization is performed AFTER the frames have been assembled - with sizes, that do not take the unsynchronization into account.

ID3v1 in combination with ID3v2
===============================

1) Although highly implausible, it is absolutely possible to successfully read an ID3v1 tag from the 128 bytes at the end of a file, which ONLY contains a carefully prepared ID3v2 tag. There is no indication in the specification, that these two tags MUST NOT overlap!
=> I will consider an overlap to be impossible!
