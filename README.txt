PURPOSE
----------
PGNLearn is a console app for statistical analysis of PGN files and automatic selection of the coefficients of the evaluation function.
In this version, it determines only the optimal values of chess pieces.


CATALOGS
--------
src - source code and build files
pgn - sample test files with batches

RELEASES
--------
pgnlearn.exe - located in releases page and only runs (for me in mingw/console)


ASSEMBLY
------
It can be built under Windows (MSVC compiler or MinGW/gcc) or under Linux (gcc).
Use pgnlearn.sln or Makefile.


LAUNCH
------
pgnlearn.exe <file.pgn>
The program performs 10,000 steps of gradient descent and outputs a vector of figure costs to the console.


GENERATED FILES
-----------------
file.mat - feature matrix (material balance values and game results)
iterations.txt - dynamics of changes in weights and minimized function over iterations
probability.txt - statistics of points scored by White depending on the material balance


(c) 2015 Vladimir Medvedev
vladimir.medvedev@gmail.com
