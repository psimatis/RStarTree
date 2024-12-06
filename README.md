# R*-Tree

The R*-Tree[^1] is the *star* of multidimensional indexing, but its implementations are hard to modify or require setups. Thus, I wrote an easy to read, run, and modify version, while also providing rare features (e.g., batch insertion).

## Features

1. **Insertion**: Insert a single object in R* fashion (e.g., trigger reinsertion when necessary). 

2. **Batch Insertion**: Insert multiple objects as a leaf.

3. **Bulk Loading**: Use STR[^2] to construct the tree from a set of objects.

4. **Range Queries**: Retrieve all objects overlapping a given query rectangle.

5. **Dimensionality**: The index supports any dimension.

6. **Statistics**: Retrieve information about the tree (e.g., such as height, number of nodes, and data entries).

## How to run
### For the impatient

```bash
$ ./run.sh
```
This compiles and executes and the code.

### How to compile

```bash
$ g++ main.cpp RStarTree.cpp -o rstar_tree
```

### Usage parameters
- **-n**: The number of objects to insert.
- **-d**: The object dimensionality (e.g., 2 means 2D rectangles).
- **-c**: The maximum number of entries per node. The minimum fill factor is 50%.
- **-q**: The number of range queries.
- **-v**: Validates range query results by comparing them against a linear search.
  
Example usage:
```bash
$ ./rstar_tree -d 2 -n 10000 -v
```

## Contributions
Contributions are welcome. Feel free to submit pull requests or open issues for discussions.

## References
[^1]: N. Beckmann, H. P. Kriegel, R. Schneider, and B. Seeger, "The R*-tree: an efficient and robust access method for points and rectangles", SIGMOD, 1990 https://doi.org/10.1145/93597.98741
[^2]: S. T. Leutenegger, M. A. Lopez, and J. Edgington, "STR: a simple and efficient algorithm for R-tree packing," Proceedings 13th International Conference on Data Engineering, 1997, doi: 10.1109/ICDE.1997.582015

