import itertools
import json
import sys


def canonical_tree(adj):
    """Canonical AHU string for an unrooted tree."""
    n = len(adj)
    if n == 1:
        return "()"
    deg = [len(adj[i]) for i in range(n)]
    leaves = [i for i in range(n) if deg[i] <= 1]
    rem = n
    while rem > 2:
        rem -= len(leaves)
        nxt = []
        for u in leaves:
            deg[u] = 0
            for v in adj[u]:
                if deg[v] > 0:
                    deg[v] -= 1
                    if deg[v] == 1:
                        nxt.append(v)
        leaves = nxt

    def encode(u, p):
        return "(" + "".join(sorted(encode(v, u) for v in adj[u] if v != p)) + ")"

    return min(encode(c, -1) for c in leaves)


def gen_all_trees(max_n):
    trees = {1: [[[]]]}
    for k in range(2, max_n + 1):
        seen = set()
        cur = []
        for adj in trees[k - 1]:
            for u in range(k - 1):
                new_adj = [list(nbrs) for nbrs in adj] + [[u]]
                new_adj[u].append(k - 1)
                c = canonical_tree(new_adj)
                if c not in seen:
                    seen.add(c)
                    cur.append(new_adj)
        trees[k] = cur
    return trees


def enumerate_shapes(e_target):
    max_int = 2 * e_target - 2
    all_int_trees = gen_all_trees(max_int)
    seen_shapes = set()
    shapes = []
    for m in range(2, max_int + 1):
        for int_adj in all_int_trees[m]:
            int_deg = [len(int_adj[i]) for i in range(m)]
            choices = []
            for i in range(m):
                if int_deg[i] == 1:
                    choices.append([1])
                elif int_deg[i] == 2:
                    choices.append([0])
                else:
                    choices.append([0, 1])
            for leaf_attach in itertools.product(*choices):
                even_cnt = sum(
                    1 for i in range(m) if (int_deg[i] + leaf_attach[i]) % 2 == 0
                )
                if even_cnt != e_target:
                    continue
                n = m + sum(leaf_attach)
                adj = [list(nbrs) for nbrs in int_adj]
                next_v = m
                for i in range(m):
                    if leaf_attach[i] == 1:
                        adj[i].append(next_v)
                        adj.append([i])
                        next_v += 1
                c = canonical_tree(adj)
                if c not in seen_shapes:
                    seen_shapes.add(c)
                    shapes.append((n, m, adj))
    shapes.sort(key=lambda x: (x[0], x[1]))
    return shapes


def export_shapes(e_target, out_path):
    shapes = enumerate_shapes(e_target)
    data = []
    for idx, (n, m, adj) in enumerate(shapes):
        internal_edges = []
        leaf_edges = []
        for u in range(n):
            for v in adj[u]:
                if u < v:
                    if u < m and v < m:
                        internal_edges.append([u, v])
                    else:
                        leaf_edges.append([u, v])
        assert len(internal_edges) == m - 1
        assert len(internal_edges) + len(leaf_edges) == n - 1
        data.append({
            "id": idx,
            "n": n,
            "m": m,
            "internal_edges": internal_edges,
            "leaf_edges": leaf_edges,
        })
    with open(out_path, "w") as f:
        json.dump(data, f)
    print(f"Exported {len(data)} shapes for e={e_target} to {out_path}")


if __name__ == "__main__":
    for e_val in [3, 5]:
        export_shapes(e_val, f"repo/data/shapes_e{e_val}.json")
