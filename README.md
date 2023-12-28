# AStarNN
A software library for locality sensitive hashing vectors using the mathematics of an A* lattice.


# Licence
See the file `LICENCE`.


# Compiling From Source

Windows

1. Install Visual Studio.
2. Open `lib_source\AStarNN.sln`.
3. Build for appropriate configurations and platforms.
4. DLL files should be automatically copied to directory `astarnn`.

Linux

1. Run: `cd lib_source`
2. Run: `make install`
3. Shared library files should be automatically copied to directory `astarnn`.

Macintosh

1. Ensure developer tools (gcc and make) are installed.
2. Open a terminal window.
3. Run: `cd lib_source`
4. Run: `make install`
5. Shared library files should be automatically copied to directory `astarnn`.


# Running Tests and Demos

1. Ensure an appropriate shared library is installed in directory `astarnn`.
   See _Compiling from source_ above.
2. Ensure Python is installed.
3. Ensure directory `astarnn` is in the PYTHONPATH. 
   If using Pycharm, mark `astarnn` as a source directory in the project structure.
4. Unit tests may be run using the script `astarnn/all_test.py`.
5. Demo scripts in the directory `demo`.


# Further Reading

_Multi-Probe LSH: Efficient Indexing for High-Dimensional Similarity Search_.
Lv, Q., Josephson, W., Wang, Z., Charikar, M., & Li, K.
Proceedings of the 33rd international conference on Very large data bases, 2007; pp. 950-961
https://www.cs.princeton.edu/courses/archive/spring13/cos598C/p950-lv.pdf

_A Time-Space Efficient Locality Sensitive Hashing Method for Similarity Search in High Dimensions_.
Lv, Q., Josephson, W., Wang, Z., Charikar, M., Li, K.
Princeton University, Princeton, June 2006.
https://www.cs.princeton.edu/techreports/2006/759.pdf

_Query adaptative locality sensitive hashing_.
Jégou, H., Amsaleg, L., Schmid, C., Gros, P. 
In IEEE International Conference on Acoustics, Speech and Signal Processing, 2008; p 825-828.
https://inria.hal.science/inria-00318614/document

_Vector Quantising Feature Space with a Regular Lattice_.
Tuytelaars, T.,  Schmid, C.
Proceedings from 11th IEEE International Conference on Computer Vision, Rio de Janeiro, Brazil, October 2007.
https://inria.hal.science/inria-00548675/document

_Near-Optimal Hashing Algorithms for Approximate Nearest Neighbor in High Dimensions_.
Andoni, A., Indyk, P. 
Proceedings from 47th Annual IEEE Symposium on Foundations of Computer Science, Berkeley, California, USA, 2006; p459-468.
Communications of the ACM, Volume 51, Issue 1, pp 117–122.
https://doi.org/10.1145/1327452.1327494

_Fast approximate nearest neighbors with automatic algorithm configuration_.
Muja, M., Lowe, D.G.
Proceedings from International Conference on Computer Vision Theory and Applications (VISAPP 2009), Lisboa, Portugal, February 2009.
https://lear.inrialpes.fr/~douze/enseignement/2014-2015/presentation_papers/muja_flann.pdf

_Entropy based nearest neighbor search in high dimensions_.
Panigrahy, R.
In Proceedings of the Seventeenth Annual ACM-SIAM Symposium on Discrete Algorithms, (SODA 2006), Miami, Florida, USA, 2006; ACM: 2006; p 1195.
https://arxiv.org/pdf/cs/0510019.pdf

_Locality-sensitive hashing scheme based on p-stable distributions_.
Datar, M., Immorlica, N., Indyk, P., Mirrokni, V.S.
In Proceedings of the twentieth annual symposium on Computational geometry, 2004; p253-262.
https://www.cs.princeton.edu/courses/archive/spring05/cos598E/bib/p253-datar.pdf

_Memory Efficient Recognition of Specific Objects with Local Features_. 
Kise, K., Noguchi, K., Iwamura, M.
In Proceedings from 19th International Conference on Pattern Recognition, Tampa, Florida, USA, 2008; IEEE.
https://ieeexplore.ieee.org/document/4761711

_Simple Representation and Approximate Search of Feature Vectors for Large-Scale Object Recognition_.
Kise, K., Noguchi, K., Iwamura, M.
In Proceedings 18th British Machine Vision Conference, (BMVC 2007), UK, September 2007; p 182-191.
https://www.dcs.warwick.ac.uk/bmvc2007/proceedings/CD-ROM/papers/paper-231.pdf

_Image retrieval method_.
Barry Drake, Scott Rudkin, Alan Tonisson. 2011.
https://patents.google.com/patent/US10289702B2

_Method, system and apparatus for generating hash codes_.
Barry Drake, Andrew Downing. 2015.
https://patents.google.com/patent/US20170075887A1/


# Contact

barry@ropeless.com
