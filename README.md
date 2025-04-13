# R*-Tree

The R*-Tree[^1] is the *star* of multidimensional indexing. Nevertheless, I found its implementations complex or lacking features. Thus, I wrote my own, while also providing features like batch insertion and STR bulk loading.

## Features

1. **Insertion**: Insert a single object in R* fashion (e.g., trigger reinsertions). 
2. **Batch Insertion**: Insert multiple objects by grouping them in leaves.
3. **Bulk Loading**: Use STR[^2] to construct the tree from a set of objects.
4. **Range Queries**: Retrieve objects overlapping a query rectangle.
5. **Dimensionality**: The index supports any dimension.
6. **Statistics**: Retrieve tree information (e.g., height, number of nodes, and size in MB).

## How to run

### For the impatient

```bash
$ ./run.sh
```
`run.sh` compiles and executes `main.cpp` which benchmarks R*-Tree operations, including:
- R* insertions
- Batch insertions
- Bulk loading
- Range queries with validation against linear scan
- Time and memory usage measurements

## Classes

- **`Rectangle`**: 
  A bounding box with utility methods (e.g., `area()`, `overlap()`, and `combine()`).

- **`Node`**:
  A tree node, which is either `leaf` or `internal`, holds pointers to its children and their rectangles.

- **`RStarTree`**:
  The tree structure and its operations (e.g., `insert()`, and `query()`).

## Limitations
- No deletion.
- No disk-based storage.
- No nearest neighbor queries.

## Contributions
Contributions are welcome. Feel free to submit pull requests or open issues for discussions.

## References
[^1]: N. Beckmann, H. P. Kriegel, R. Schneider, and B. Seeger, "The R*-tree: an efficient and robust access method for points and rectangles", SIGMOD, 1990 https://doi.org/10.1145/93597.98741
[^2]: S. T. Leutenegger, M. A. Lopez, and J. Edgington, "STR: a simple and efficient algorithm for R-tree packing," Proceedings 13th International Conference on Data Engineering, 1997, doi: 10.1109/ICDE.1997.582015

