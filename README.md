# BB

Small files are difficult to compress due to lack of internal repetition.

However, we often compress large numbers of small documents -- such as JSON transmissions
over HTTP -- which are very similar to each other.

Consider the text, 'the quick brown fox jumps over the lazy dog'. This is non-compressible.
However, if both parties had the same tokenizer with an English vocabulary, one could reduce it
significantly.

BB is an attempt to compress small, separate documents by way of a common model. It consists
of a protocol and reference implementation.

## Considerations

Compressions can't be made or intepreted without a matching model. However, in many situations,
the system producing the compressions and the system intepreting them is the same systems. (For example,
a database.)

If transfered, a model is obviously significantly larger than any particular compression. However,
it will not be larger than millions or billions of compressions.

Loss of model would render compressions unreadable, however in many applications, compressions are
shortlived and disposable.

Another consideration is the computational cost of the compressor compared to the benefit of the reduction.
This can only be weighed empirically with a better developed encoder and decoder. A problem for later.

## Manifest

Find at the root of this repository a Makefile which produces a set of binaries.

### forge

`forge` reads from `stdin`, identifies repetitions and emits a model to `stdout`.

This interface is a little nonsensical. A corpus needs a definite scope, and the pipe
interface implies indefinite scope. The need for a definite scope is dictated by the
fact the format opens with a hash of its contents. This got me far enough to try it,
but I will refactor it. I ran it like so:

`cat corpora/gutenberg/*.txt | forge > gutenberg.bbm`

### encode

`encode` takes a single model as an input parameter. It reads raw content from `stdin`,
and emits a compression to `stdout`. I ran it like so:

`cat test.txt | encode gutenberg.bbm > test.bb`

### decode

`decode` takes a single model as an input parameter. It reads a compressed from `stdin`,
and emits the original data to `stdout`.

`cat test.bb | decode gutenberg.bbm`

### ncat

`ncat` behaves like a simplified `cat`, except it will emit each file repeatedly.

This may have been a mistake, and caused obvious over-fittings in early tests. I will
probably remove it.

`ncat -n 3 corpora/gutenberg/*.txt | forge > gutenberg.bbm`

## Protocol

At the time of writing, BB's design is not stable. As revise the design, I will update
the reference implementation and this document together.

### Model

A model consists of an ordered set of fragments. Each fragment is a series of bytes with definite length.
Represented in SQL, it might look like so:

````
CREATE TABLE fragment (id SERIAL PRIMARY KEY, body BLOB);
````

The model file opens with a 32-bit hash of the model's content (described above). After
this, each fragment is serialized, one after the other, as Pascal strings. That is, one
byte indicating content length followed by the body. The fragment's id is stored implicitly
by its position in the file.

### Compression

A compression is a series of tokens. A token is either a reference to a model
fragment, or a literal.

A compression opens with the 32-bit hash of the relevant model's content. This
is followed by the tokens, one after another.

Each token opens with a single byte, whose value is intepreted according to the
following model:

* [0x00, 0x80) -- this byte represents a literal of the following length. This byte
    will be followed by the given number of bytes, which represent raw document body.
* [0x80, 0xFC) -- this byte represents an index, of range [0, 124), of a model fragment.
* 0xFD -- the next byte will represent an index of range [0, 256) of a model fragment.
* 0xFE -- the next two bytes represent an index of range [0, 65536) of a model fragment.
