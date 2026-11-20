# Computer-Assisted Proof: Super-Edge-Gracefulness for Trees with $e \in \{3, 5, 7\}$ Even-Degree Vertices and General Linear Cutoff

## Main Results

1. **Theorem 1 ($e = 3, 5, 7$ Complete Closure).** *Every finite tree $T$ with exactly $e \in \{3, 5, 7\}$ vertices of even degree is super-edge-graceful, and consequently satisfies Lee's edge-graceful tree conjecture.*
   - $e = 3$: 2 underlying shapes (independently recovers Lee & Ho, 2007).
   - $e = 5$: 12 underlying shapes, 647 exhaustive splits, 659 terminal certificates.
   - $e = 7$: 116 underlying shapes, 808,934 exhaustive splits, 809,050 terminal certificates.

2. **Theorem 2 (Linear Finite Cutoff for Every Odd $e \ge 3$).** *For every odd $e \ge 3$, if there exists a tree with exactly $e$ even-degree vertices that is not super-edge-graceful, then a counterexample exists on at most*
   $$23e - 36 \text{ vertices}$$

3. **Corollary 3 (Two-Leaf Extension).** *Every tree of order at least $17$ with $e = 7$ even-degree vertices admits a two-leaf extension to a super-edge-graceful tree with $e = 9$ even-degree vertices; each further step requires current order $n \ge 2e+3$.*

---

## 1. Definitions and Structural Reduction

A tree $T = (V, E)$ of odd order $n = 2h + 1$ ($q = 2h$ edges) is **super-edge-graceful (SEG)** if there exists a bijection $f : E(T) \to \{\pm 1, \dots, \pm h\}$ whose induced vertex sums $f^*(v) = \sum_{u \sim v} f(uv)$ form a bijection $f^* : V(T) \to \{0, \pm 1, \dots, \pm h\}$ (Mitchem & Simoson, 1994). Taking $f$ and $f^*$ modulo $n$ immediately yields an edge-graceful labeling.

### Step 1: Removing Sibling-Leaf Pairs
Deleting two leaves sharing a parent preserves the count $e$ of even-degree vertices. Conversely, attaching a pair of sibling leaves to any vertex $v$ with edge labels $+(h+1)$ and $-(h+1)$ preserves $f^*(v)$ and produces new leaf sums $\pm(h+1)$ (Mitchem & Simoson's Growing Tree Algorithm). Hence we may assume every vertex has at most one leaf neighbor.

### Step 2 (Proposition 4): Shortening Chains of Leaf-Bearing Degree-3 Vertices
In a tree $T'$ with no sibling leaves, let $d_k$ be the number of degree-$k$ vertices and $u$ the number of internal vertices with no leaf neighbor. Counting leaves in two ways gives $|L(T')| = |I(T')| - u = 2 + \sum_{\deg(v) \ge 3}(\deg(v) - 2)$, hence
$$u = d_2 - 2 - \sum_{\deg(v) \ge 4}(\deg(v) - 3).$$
Every degree-3 vertex $v$ carrying a leaf $\ell$ has two internal neighbors $x, y$ (since $T'$ has no sibling leaves). Contracting $x - v - y$ to $xy$ and deleting $v, \ell$ preserves all other degrees and preserves $e$. Repeating until no leaf-bearing degree-3 vertex remains produces an **underlying tree shape** $T_0$ satisfying:
$$|I(T_0)| \le d_2 + d_{\ge 4} + u = 2d_2 - 2 - \sum_{\deg(v) \ge 4}(\deg(v) - 4) \le 2d_2 - 2 \le 2e - 2.$$
Moreover, since every degree-3 vertex of $T_0$ has no leaf neighbor, $d_3 \le u = d_2 - 2 - \sum_{\deg \ge 4}(\deg - 3)$. Therefore
$$|V(T_0)| = |I(T_0)| + |L(T_0)| = d_2 + 2 + 2d_3 + \sum_{\deg \ge 4}(\deg - 1) \le 3d_2 - 2 + d_4 - \sum_{\deg \ge 5}(\deg - 5) \le 3e - 2.$$

Exact enumeration of all non-isomorphic shapes gives:
- $e = 3$: **2 shapes**, max order $7 = 3(3)-2$, $r \le 3$;
- $e = 5$: **12 shapes**, max order $13 = 3(5)-2$, $r \le 7$;
- $e = 7$: **116 shapes**, max order $19 = 3(7)-2$, $r \le 11$;
- $e = 9$: **1,526 shapes**, max order $25 = 3(9)-2$, $r \le 15$.

Every finite tree with $e$ even-degree vertices is obtained from one of these underlying shapes $T_0$ (with $r = |I(T_0)| - 1$ internal edges) by inserting a chain of $m_j \ge 0$ leaf-bearing degree-3 vertices along each internal edge $e_j$ and restoring sibling-leaf pairs.

---

## 2. Simultaneous Chain-Extension Lemma

**Lemma 5 (Simultaneous Chain Extension).** *Let $(T, f)$ be a super-edge-graceful tree of order $n = 2h + 1$, rooted at $R$ so each edge $e_j$ is oriented from parent $p_j$ to child $c_j$. For a set $O$ of edges, set $\Delta_j = f^*(c_j) - f(p_j, c_j)$ and $d_j = |\Delta_j|$. Suppose we choose chain lengths $(m_j)_{j \in O}$ with active set $S = \{j \in O : m_j > 0\}$ and total $M = \sum_{j \in S} m_j$. If*
1. *$m_j \ge 2$ for every $j \in S$ with $d_j > 0$, and*
2. *there exist pairwise disjoint pairs $(x_j, x_j + d_j) \subseteq \{1, 2, \dots, M\}$ for all $j \in S$ with $d_j > 0$,*

*then the tree $T'$ obtained by simultaneously inserting a chain of $m_j$ leaf-bearing degree-3 vertices along every $e_j$ ($j \in S$) is super-edge-graceful.*

**Proof.** Shift the reserved pairs to $t_1^{(j)} = h + x_j$, $t_2^{(j)} = h + x_j + d_j \in \{h+1, \dots, h+M\}$ for each $j \in S$ with $d_j > 0$. Partition the remaining $M - 2|\{j \in S : d_j > 0\}|$ magnitudes of $\{h+1, \dots, h+M\}$ arbitrarily so that each $j \in S$ with $d_j > 0$ receives $m_j - 2$ extra magnitudes $u_1^{(j)}, \dots, u_{m_j - 2}^{(j)}$, and each $j \in S$ with $d_j = 0$ receives $m_j$ magnitudes $u_1^{(j)}, \dots, u_{m_j}^{(j)}$.

On each $e_j = (p_j, c_j)$ ($j \in S$) of old label $a = f(p_j, c_j)$, replace $(p_j, c_j)$ by $p_j - v_1 - \dots - v_{m_j} - c_j$ with pendant leaves $\ell_k$ at $v_k$, setting $f(p_j, v_1) = a$:
- **If $d_j = 0$:** set spine edges $e_k = +u_k^{(j)}$ and leaf edges $w_k = -u_k^{(j)}$ ($1 \le k \le m_j$). Then $f^*(v_1) = a = f^*(c_j)$, $f^*(v_k) = +u_{k-1}^{(j)}$ ($2 \le k \le m_j$), $f^*(\ell_k) = -u_k^{(j)}$, and $f^*_{\text{new}}(c_j) = +u_{m_j}^{(j)}$.
- **If $d_j > 0$:** let $\sigma = \operatorname{sgn}(\Delta_j)$, set spine edges $(e_1, \dots, e_{m_j}) = (\sigma t_2^{(j)}, \sigma u_1^{(j)}, \dots, \sigma u_{m_j - 2}^{(j)}, \sigma t_1^{(j)})$ and leaf edges $(w_1, \dots, w_{m_j}) = (-\sigma t_1^{(j)}, -\sigma t_2^{(j)}, -\sigma u_1^{(j)}, \dots, -\sigma u_{m_j - 2}^{(j)})$. Then $f^*(v_1) = a + \sigma(t_2^{(j)} - t_1^{(j)}) = f^*(c_j)$, $f^*(v_k) = \sigma u_{k-1}^{(j)}$ ($2 \le k \le m_j - 1$), $f^*(v_{m_j}) = \sigma t_1^{(j)}$, $f^*_{\text{new}}(c_j) = \Delta_j + \sigma t_1^{(j)} = \sigma t_2^{(j)}$, and the leaves yield $-\sigma t_1^{(j)}, -\sigma t_2^{(j)}, -\sigma u_k^{(j)}$.

Because $T$ is rooted at $R$, no vertex is the child endpoint $c_j$ of two distinct edges in $O$, and every parent endpoint $p_j$ retains its original edge label $a$. Thus all extensions act independently on vertex sums, producing the exact bijection $\{0, \pm 1, \dots, \pm(h + M)\}$. $\square$

**Immediate Special Case (Block Condition):** Whenever $m_j \ge d_j + 1$ for all $j \in S$, partitioning $\{1, \dots, M\}$ into disjoint contiguous blocks of length $m_j$ and picking $(x_j, x_j + d_j)$ inside block $j$ automatically satisfies Lemma 5.

---

## 3. Proof of Theorem 1 ($e = 3, 5, 7$)

For each underlying shape $T_0$ with $r$ internal edges, a finite binary decision tree covers $\mathbb{Z}_{\ge 0}^r$:
- Root state: every internal edge $i$ is $\text{OPEN}(1)$, representing $m_i \in \{0\} \cup [1, \infty) = \mathbb{Z}_{\ge 0}$.
- Split node on edge $i$ in state $\text{OPEN}(L)$: branches into **Left** $\text{FIXED}(L)$ ($m_i = L$, absorbed into the concrete tree) and **Right** $\text{OPEN}(L+1)$ ($m_i \in \{0\} \cup [L+1, \infty)$), an exact partition.
- Terminal leaf: provides $(R, f)$ for the concrete tree $T$ such that $f$ is SEG and $d_j \le L_j - 1$ for every open edge $j$, covering the entire box by the Block Condition of Lemma 5.

Independent verification (`verify_proof.py`) confirms all three cases ($e = 3, 5, 7$) from scratch.

---

## 4. Proof of Theorem 2 ($23e - 36$ Linear Cutoff for All Odd $e \ge 3$)

Let $T$ be a minimal counterexample with $e \ge 3$ even-degree vertices. Then $T$ has no sibling leaves and arises from an underlying shape $T_0$ ($|V(T_0)| \le 3e - 2$, $r \le 2e - 3$ by Proposition 4). Absorb all length-$1$ chains into $T_0$ to form $T_1$, leaving $k \le r$ open edges with lengths $m_j \ge 2$ and total $M = \sum_{j=1}^k m_j$. If $k = 0$, then $|V(T)| \le (3e - 2) + 2(2e - 3) = 7e - 8 \le 23e - 36$.

Otherwise $k \ge 1$, so $T_1$ has strictly smaller order $n_1 = 2h_1 + 1 \le (3e - 2) + 2(r - k) \le 7e - 8 - 2k$ and by minimality admits an SEG labeling $f$. Root $T_1$ at any vertex $R$ and sort the open-edge differences in descending order $d_1 \ge d_2 \ge \dots \ge d_k$. Because the $c_i$ are distinct vertices and the $e_i$ are distinct edges, the top $2m+1$ magnitudes in $\{-h_1, \dots, h_1\}$ sum to at most $(2m+1)h_1 - m^2$. Hence
$$
(2m+1) d_{2m+1} \le \sum_{i=1}^{2m+1} \bigl(|f^*(c_i)| + |f(e_i)|\bigr) \le 2(2m+1)h_1 - 2m^2,
$$
which gives $d_{2m+1} \le 2h_1 - \lceil 2m^2/(2m+1) \rceil = 2h_1 - m$, and therefore $d_j \le 2h_1 - \lfloor (j-1)/2 \rfloor = n_1 - 1 - \lfloor (j-1)/2 \rfloor$ for every $1 \le j \le k$.

For the indices with $d_j>0$, place disjoint pairs $(x_j, x_j + d_j) \subseteq \{1, \dots, M\}$ greedily in descending order. At index $j$ at most $2(j-1)$ elements have been used and block at most $4(j-1)$ of the $M - d_j$ candidate pairs. A packing therefore exists whenever $M \ge d_j + 4(j-1) + 1$ for all $1 \le j \le k$, which is guaranteed by $M \ge n_1 + 4(k-1) - \lfloor (k-1)/2 \rfloor = n_1 + \lceil 7(k-1)/2 \rceil$. By Lemma 5, a minimal counterexample satisfies $M \le n_1 + \lceil 7(k-1)/2 \rceil - 1$, so $2M \le 2n_1 + 7k - 8$ for all $k \ge 1$, and $2M \le 2n_1 + 7k - 9$ whenever $k$ is odd (in particular when $k = 2e - 3$). Consequently,
$$
|V(T)| = n_1 + 2M \le \begin{cases} 3(7e - 8 - 2k) + 7k - 9 = 21e + k - 33 = 23e - 36 & (k = 2e - 3), \\ 3(7e - 8 - 2k) + 7k - 8 = 21e + k - 32 \le 23e - 36 & (k \le 2e - 4). \end{cases}
$$

---

## 5. Proof of Corollary 3 (Two-Leaf Extension)

Let $T$ be any tree with $e$ even-degree vertices and order $n = 2h + 1 \ge 2e + 3$ (so $h \ge e + 1$; for $e = 7$, $n \ge 17$), admitting an SEG labeling $f$. Partition $\{-h, \dots, -1, 1, \dots, h\}$ into the $h$ disjoint pairs $(-h, 1), (-h+1, 2), \dots, (-1, h)$, each differing by $h + 1$. The $e$ even-degree vertices can occupy at most $e$ of these $h$ pairs; since $h \ge e + 1$, at least one pair $(s, s + h + 1)$ consists of two odd-degree vertices $u, v$. Attaching one new leaf to $u$ with edge label $+(h+1)$ and one new leaf to $v$ with edge label $-(h+1)$ swaps the vertex sums of $u$ and $v$, supplies new leaf sums $\pm(h+1)$, leaves all other vertex sums fixed, and increases the number of even-degree vertices from $e$ to $e + 2$. Applying Theorem 1 ($e = 7$) yields super-edge-graceful trees with $e = 9$. Starting from order $n$ with $e=7$, step $t+1$ is guaranteed when $n+2t\ge 17+4t$. This gives $(n-15)/2$ steps from a fixed starting tree. Arbitrarily large starting trees are available by adding sibling-leaf pairs, so every fixed odd target $e\ge9$ is attainable at arbitrarily large orders. $\square$
