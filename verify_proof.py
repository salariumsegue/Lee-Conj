#!/usr/bin/env python3
"""
Standalone zero-dependency verifier for the computer-assisted proof that
every finite tree with exactly e even-degree vertices (e = 3, 5, or 7)
is super-edge-graceful (and hence edge-graceful).

Supports both compact binary (.bin.xz) and text (.txt / .txt.gz) certificate formats.

Usage:
    python3 verify_proof.py --e 7 --shapes data/shapes_e7.json --certs data/certs_e7.bin.xz --spot-check 500
"""

import argparse
import gzip
import itertools
import json
import lzma
import random
import struct
import time
from collections import deque


def canonical_tree(adj):
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


def enumerate_underlying_shapes(e_target):
    max_int = 2 * e_target - 2
    trees = {1: [[[]]]}
    for k in range(2, max_int + 1):
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

    shapes = set()
    for m in range(2, max_int + 1):
        for int_adj in trees[m]:
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
                adj = [list(nbrs) for nbrs in int_adj]
                next_v = m
                for i in range(m):
                    if leaf_attach[i] == 1:
                        adj[i].append(next_v)
                        adj.append([i])
                        next_v += 1
                shapes.add(canonical_tree(adj))
    return shapes


def build_concrete_tree(shape, state):
    n = shape["n"]
    edges = []
    open_info = []
    next_v = n
    for i, (u, v) in enumerate(shape["internal_edges"]):
        st = state[i]
        if st < 0:
            open_info.append((len(edges), -st))
            edges.append((u, v))
        elif st == 0:
            edges.append((u, v))
        else:
            prev = u
            for _ in range(st):
                w = next_v
                lf = next_v + 1
                next_v += 2
                edges.append((prev, w))
                edges.append((w, lf))
                prev = w
            edges.append((prev, v))
    for u, v in shape["leaf_edges"]:
        edges.append((u, v))
    return next_v, edges, open_info


def verify_seg_direct(n, edges, labels):
    if n % 2 != 1:
        raise AssertionError(f"Tree order {n} is not odd")
    h = (n - 1) // 2
    E = 2 * h
    if len(edges) != E or len(labels) != E:
        raise AssertionError(f"Edge/label count mismatch: {len(edges)} vs {len(labels)} vs {E}")

    seen_e = bytearray(2 * h + 1)
    sums = [0] * n
    adj = [[] for _ in range(n)]
    for (u, v), w in zip(edges, labels):
        if w == 0 or w < -h or w > h:
            raise AssertionError(f"Edge label {w} out of range")
        idx = w + h
        if seen_e[idx]:
            raise AssertionError(f"Duplicate edge label {w}")
        seen_e[idx] = 1
        sums[u] += w
        sums[v] += w
        adj[u].append(v)
        adj[v].append(u)

    seen_v = bytearray(2 * h + 1)
    for s in sums:
        if s < -h or s > h:
            raise AssertionError(f"Vertex sum {s} out of range")
        idx = s + h
        if seen_v[idx]:
            raise AssertionError(f"Duplicate vertex sum {s}")
        seen_v[idx] = 1

    return sums, adj


def verify_leaf(shape, state, root, labels, do_constructive=False, rng=None):
    n, edges, open_info = build_concrete_tree(shape, state)
    if not (0 <= root < n):
        raise AssertionError(f"Invalid root {root} for n={n}")

    sums, adj = verify_seg_direct(n, edges, labels)

    parent = [-1] * n
    parent[root] = root
    q = deque([root])
    visited = 1
    while q:
        u = q.popleft()
        for v in adj[u]:
            if parent[v] == -1:
                parent[v] = u
                visited += 1
                q.append(v)
    if visited != n:
        raise AssertionError("Graph is not connected")

    for eidx, L in open_info:
        u, v = edges[eidx]
        c = v if parent[u] != v else u
        assert parent[c] == (u if c == v else v)
        d = abs(sums[c] - labels[eidx])
        if d > L - 1:
            raise AssertionError(
                f"Extension condition failed on open edge {eidx}: d={d} > L-1={L-1}"
            )

    if do_constructive and open_info:
        h = (n - 1) // 2
        ext_map = {}
        for eidx, L in open_info:
            if rng.random() < 0.7:
                ext_map[eidx] = rng.randint(L, L + 12)
        if ext_map:
            new_edges = []
            new_labels = []
            next_v = n
            next_mag = h + 1
            for eidx, (u, v) in enumerate(edges):
                a = labels[eidx]
                if eidx not in ext_map:
                    new_edges.append((u, v))
                    new_labels.append(a)
                    continue
                m = ext_map[eidx]
                p, c = (u, v) if parent[v] == u else (v, u)
                delta = sums[c] - a
                d = abs(delta)
                B = next_mag - 1
                next_mag += m
                vs = [next_v + i for i in range(m)]
                ls = [next_v + m + i for i in range(m)]
                next_v += 2 * m
                if d == 0:
                    new_edges.append((p, vs[0]))
                    new_labels.append(a)
                    for k in range(m):
                        mag = B + 1 + k
                        nxt = vs[k + 1] if k + 1 < m else c
                        new_edges.append((vs[k], nxt))
                        new_labels.append(mag)
                        new_edges.append((vs[k], ls[k]))
                        new_labels.append(-mag)
                else:
                    sigma = 1 if delta > 0 else -1
                    t1 = B + 1
                    t2 = B + 1 + d
                    us = [x for x in range(B + 1, B + m + 1) if x != t1 and x != t2]
                    spine = [a, sigma * t2] + [sigma * u_val for u_val in us] + [sigma * t1]
                    leafs = [-sigma * t1, -sigma * t2] + [-sigma * u_val for u_val in us]
                    new_edges.append((p, vs[0]))
                    new_labels.append(spine[0])
                    for k in range(m):
                        nxt = vs[k + 1] if k + 1 < m else c
                        new_edges.append((vs[k], nxt))
                        new_labels.append(spine[k + 1])
                        new_edges.append((vs[k], ls[k]))
                        new_labels.append(leafs[k])
            verify_seg_direct(next_v, new_edges, new_labels)
            return next_v
    return 0


def iter_shape_trees(path, expected_count):
    if path.endswith(".bin.xz") or path.endswith(".bin"):
        raw = lzma.open(path, "rb").read() if path.endswith(".xz") else open(path, "rb").read()
        mv = memoryview(raw)
        assert mv[:4].tobytes() == b"SEG1", "Invalid binary certificate magic"
        (n_shapes,) = struct.unpack_from("<H", mv, 4)
        assert n_shapes == expected_count
        pos = 6
        for _ in range(n_shapes):
            sid, num_nodes = struct.unpack_from("<HI", mv, pos)
            pos += 6
            nodes = []
            for _ in range(num_nodes):
                tag = mv[pos]
                pos += 1
                if tag == 0:
                    root = mv[pos]
                    k = mv[pos + 1]
                    pos += 2
                    labels = struct.unpack_from(f"<{k}b", mv, pos)
                    pos += k
                    nodes.append(("L", root, labels))
                elif tag == 1:
                    orig_id, split_val, lid, rid = struct.unpack_from("<BBII", mv, pos)
                    pos += 10
                    nodes.append(("S", orig_id, split_val, lid, rid))
                else:
                    raise AssertionError(f"Unknown binary node tag {tag}")
            yield sid, nodes
    else:
        opener = (lambda p: gzip.open(p, "rt", encoding="ascii")) if path.endswith(".gz") else (lambda p: open(p, "r", encoding="ascii"))
        with opener(path) as f:
            header = f.readline().strip().split()
            assert header[0] == "SHAPES" and int(header[1]) == expected_count
            for _ in range(expected_count):
                line = f.readline().strip().split()
                assert line[0] == "SHAPE"
                sid, num_nodes = int(line[1]), int(line[2])
                nodes = []
                for _ in range(num_nodes):
                    parts = f.readline().strip().split()
                    if parts[0] == "L":
                        root = int(parts[1])
                        k = int(parts[2])
                        labels = [int(x) for x in parts[3:]]
                        assert len(labels) == k
                        nodes.append(("L", root, labels))
                    elif parts[0] == "S":
                        nodes.append(("S", int(parts[1]), int(parts[2]), int(parts[3]), int(parts[4])))
                    else:
                        raise AssertionError(f"Unknown node tag: {parts[0]}")
                yield sid, nodes


def main():
    parser = argparse.ArgumentParser(description="Verify super-edge-graceful proof certificates.")
    parser.add_argument("--e", type=int, required=True, help="Number of even-degree vertices (3, 5, or 7)")
    parser.add_argument("--shapes", required=True, help="Path to shapes JSON file")
    parser.add_argument("--certs", required=True, help="Path to certificates file (.bin.xz, .txt.gz, or .txt)")
    parser.add_argument("--spot-check", type=int, default=0, help="Number of random leaves to constructively extend and verify")
    parser.add_argument("--seed", type=int, default=42, help="PRNG seed for constructive spot-checks")
    args = parser.parse_args()

    t0 = time.time()
    print(f"[1/3] Independently enumerating all underlying tree shapes for e={args.e}...")
    expected_shapes = enumerate_underlying_shapes(args.e)
    print(f"      Enumerated {len(expected_shapes)} distinct underlying shapes ({time.time()-t0:.2f}s).")

    with open(args.shapes, "r") as f:
        shapes = json.load(f)
    if len(shapes) != len(expected_shapes):
        raise AssertionError(f"Shape count mismatch: file has {len(shapes)}, expected {len(expected_shapes)}")

    seen_canonical = set()
    shape_by_id = {}
    for sh in shapes:
        n, m = sh["n"], sh["m"]
        adj = [[] for _ in range(n)]
        for u, v in sh["internal_edges"]:
            assert 0 <= u < m and 0 <= v < m
            adj[u].append(v)
            adj[v].append(u)
        for u, v in sh["leaf_edges"]:
            assert (0 <= u < m <= v < n) or (0 <= v < m <= u < n)
            adj[u].append(v)
            adj[v].append(u)
        c = canonical_tree(adj)
        if c not in expected_shapes:
            raise AssertionError(f"Shape {sh['id']} is not a valid underlying shape for e={args.e}")
        if c in seen_canonical:
            raise AssertionError(f"Duplicate shape {sh['id']}")
        seen_canonical.add(c)
        shape_by_id[sh["id"]] = sh
    assert seen_canonical == expected_shapes
    print(f"[2/3] Verified all {len(shapes)} shapes in {args.shapes} match independent enumeration.")

    print(f"[3/3] Verifying exhaustive case-split proof trees and terminal certificates from {args.certs}...")
    total_splits = 0
    total_leaves = 0
    covered_ids = set()
    rng = random.Random(args.seed)
    spot_done = 0
    max_spot_order = 0
    spot_interval = max(1, (800000 // args.spot_check)) if (args.spot_check > 0 and args.e == 7) else 1

    for sid, nodes in iter_shape_trees(args.certs, len(shapes)):
        sh = shape_by_id[sid]
        covered_ids.add(sid)
        r = len(sh["internal_edges"])
        stack = [(0, [-1] * r)]
        visited_nodes = 0
        while stack:
            nid, st = stack.pop()
            visited_nodes += 1
            nd = nodes[nid]
            if nd[0] == "L":
                do_c = (spot_done < args.spot_check) and (total_leaves % spot_interval == 0)
                nv = verify_leaf(sh, st, nd[1], nd[2], do_constructive=do_c, rng=rng)
                if nv > 0:
                    spot_done += 1
                    if nv > max_spot_order:
                        max_spot_order = nv
                total_leaves += 1
            else:
                _, orig_id, split_val, lid, rid = nd
                if st[orig_id] != -split_val:
                    raise AssertionError(
                        f"Shape {sid} node {nid}: expected OPEN({split_val}) on edge {orig_id}, got {st[orig_id]}"
                    )
                total_splits += 1
                st_left = list(st)
                st_left[orig_id] = split_val
                st_right = list(st)
                st_right[orig_id] = -(split_val + 1)
                stack.append((rid, st_right))
                stack.append((lid, st_left))
        if visited_nodes != len(nodes):
            raise AssertionError(f"Shape {sid}: unreachable nodes ({visited_nodes} != {len(nodes)})")

    assert covered_ids == set(shape_by_id.keys())
    elapsed = time.time() - t0
    extra = f" | constructive spot-checks: {spot_done} (max order {max_spot_order})" if args.spot_check > 0 else ""
    print(
        f"VERIFIED: e={args.e} | {len(shapes)} underlying shapes | "
        f"{total_splits:,} exhaustive splits | {total_leaves:,} terminal certificates{extra} | "
        f"elapsed {elapsed:.2f}s."
    )


if __name__ == "__main__":
    main()
