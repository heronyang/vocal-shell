## CMU Sphinx - Installation on Mac

### sphinxbase-5prealpha, pocketsphinx-5prealpha

- ```sudo port install swig swig-python```
- ```./autogen.sh PYTHON_EXTRA_LDFLAGS="-u _PyMac_Error"  LDFLAGS="-L/opt/local/lib `python-config --ldflags` `python-config --libs`" CXXFLAGS="-I/opt/local/include -I/opt/local//Library/Frameworks/Python.framework/Versions/2.7/include" --prefix=/opt/local/  --with-boost-python=boost_python-mt --disable-sparsehash CXX=clang``` (read [this port ticket](https://trac.macports.org/ticket/39363))
- ```make```
- ```sudo make install```

### Usage

\> ```pocketsphinx_continuous -inmic yes -keyphrase "ok google" -kws_threshold 1e-30```

### Compile words.txt

http://www.speech.cs.cmu.edu/tools/lmtool-new.html
