# quadgraph

A quadtree approach to building a interconnected graph network (for path finding/ path optimization purposes) for resolving "open" areas (i.e. without firm obstacles) but with a varying attribute indirectly influencing edge weights.

The current implementation does:

* Subdivides a node based on a non-spatial criterion (binning/clustering of non-spatial attribute)
* Keeps track of (pointers) to each leaf node's neighbors during insertion
* Allows for merging adjacent quadgraphs (and updates the neigbors of edge leaf nodes) for dynamic creation of a larger graph network

WIP (and relatively untested) :)

It should be noted that I am testing a simpler? way of writing C++, abandoning classes and using structs for everything OO. The responsibility of not "messing up" is thereby handed down to the user fo this piece of code rather than having the designer trying to prevent all kinds of mishaps from various users. Compare with Python.

Feel free to comment and/or suggest improvements. 