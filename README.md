# scc
SCC is a simple C compiler written in pure C language. It applies top-down parsing and does not take use of tools 
like flex, yacc/bison. 

There is a lot of more work to do including: 
1. have better register allocation algorithm
2. do compiler optimization
3. do assembling and linking within SCC rather than using the support of GCC

I've written a bunch of tests to cover most cases I can think about. SCC has also been tested against two small but 
popular open-source project, mongoose and memcached, which use C language.

More improvements will come later.
