# R*-Tree

The R*-Tree[^1] is the *star* of multidimensional indexing. Nevertheless, I found its implementations complex or hard to modify. Thus, I wrote my own with a focus on simplicity and ease of modification, while also providing features like batch insertion and STR bulk loading.

## Features

1. **Insertion**: Insert a single object in R* fashion (e.g., trigger reinsertions). 

2. **Batch Insertion**: Insert multiple objects as a leaf.

3. **Bulk Loading**: Use STR[^2] to construct the tree from a set of objects.

4. **Range Queries**: Retrieve objects overlapping a query rectangle.

5. **Dimensionality**: The index supports any dimension.

6. **Statistics**: Retrieve tree information (e.g., height, number of nodes, and size in MB).

## How to run

### For the impatient

```bash
$ ./run.sh
```
`run.sh` compiles and executes `main.cpp` which performs toy R*-Tree operations.

### Testing

```bash
$ cd tests
$ ./runAllTests.sh
```
The `tests/` directory includes tests for the R*-Tree (e.g., comparing query results against a linear scan). 
Run `runAllTests.sh` to compile and execute them. 
Test details are documented within the corresponding `.cpp` files.

## Classes

- **`Rectangle`**: 
  A bounding box with utility methods (e.g., `area()`, `overlap()`, and `combine()`.

- **`Node`**:
  Each tree node is either `leaf` or `internal`. It holds pointers to its children and their rectangles.

- **`RStarTree`**:
  The overall tree structure and its operations (e.g., `insertion()`, and `query()`).


## Limitations
- No deletion.
- No support for disk-based storage.

## Contributions
Contributions are welcome. Feel free to submit pull requests or open issues for discussions.

## References
[^1]: N. Beckmann, H. P. Kriegel, R. Schneider, and B. Seeger, "The R*-tree: an efficient and robust access method for points and rectangles", SIGMOD, 1990 https://doi.org/10.1145/93597.98741
[^2]: S. T. Leutenegger, M. A. Lopez, and J. Edgington, "STR: a simple and efficient algorithm for R-tree packing," Proceedings 13th International Conference on Data Engineering, 1997, doi: 10.1109/ICDE.1997.582015

