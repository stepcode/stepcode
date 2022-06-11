# SCL - A Python3 STEP Class Library

SCL is a Python3-based library for parsing and manipulating ISO 10303 Part 21 ("STEP") files. 

# Use

To parse a ISO 10303 Part 21 file, use the `Parser` class of `SCL.Part21`. On a successful parse, the result will be a Part 21 parse tree and associated STEP instances.

See [test_parser](tests/test_parser.py) for use of the `SCL.Part21` parser with a sample STEP file.


# Building

The SCL Python package can be built directly using [PyPA's build module](https://github.com/pypa/build). Run `python3 -m build` from this directory.

# Testing

SCL comes with a small test suite (additions welcome!) From this directory, `python3 -m unittest discover`.

# License

See [LICENSE](LICENSE)