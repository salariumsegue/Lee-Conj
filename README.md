# Super-Edge-Graceful Trees with $e \in \{3, 5, 7\}$ Even-Degree Vertices & Improved Linear Cutoff

A complete, independently verifiable computer-assisted proof that **every finite tree with exactly 3, 5, or 7 vertices of even degree is super-edge-graceful**, and consequently satisfies **Lee's Edge-Graceful Tree Conjecture**, together with an improved **$25e - 40$** general linear finite cutoff for all odd $e \ge 3$, an infinite-family extension to all odd $e \ge 9$, and an exhaustive order-17 experimental sweep of the `{R1, R3, R4}` reduction.

## Verified Results

| Even-degree vertices ($e$) | Underlying shapes | Max shape order | Exhaustive splits | Terminal certificates | Constructive spot-checks | Verification time |
|---:|---:|---:|---:|---:|---:|---:|
| **3** | 2 | 7 | 11 | 13 | 10 (up to order 41) | 0.00 s |
| **5** | 12 | 13 | 647 | 659 | 100 (up to order 77) | 0.01 s |
| **7** | 116 | 19 | 808,934 | 809,050 | 375 (up to order 157) | 28.24 s |

- **General linear cutoff (Theorem 2):** For every odd $e \ge 3$, if a counterexample to super-edge-gracefulness exists with $e$ even-degree vertices, one exists on at most **$25e - 40$** vertices (improving $28e - 49$ for every $e \ge 5$; e.g., order $\le 185$ for $e = 9$).
- **Infinite family beyond $e = 7$ (Corollary 3):** Every tree of order $\ge 17$ with $e = 7$ even-degree vertices extends to a super-edge-graceful tree with $e = 9$ even-degree vertices (and inductively to every odd $e \ge 9$).
- **Order-17 `{R1, R3, R4}` sweep:** Across all 57,967 odd-order trees through order 17, 100% of non-path trees admit a 2-vertex SEG reduction (at order 17: 3,448/3,448 trees with a degree-$\ge 3$ pendant-$P_2$ site reduce by R3, and 277/277 remaining trees reduce by R4).
- **Status note:** Lee and Ho (2007) proved the $e = 3$ case (*J. Combin. Math. Combin. Comput.* 62, 53–64). Publication priority and independent peer review for the $e = 5$, $e = 7$, and $25e - 40$ results have not been established. Lee's unrestricted conjecture for all odd $e \ge 9$ remains open.

## Quick Verification (Zero External Dependencies)

Requires Python 3.8+ (standard library only):

```bash
python3 verify_proof.py --e 3 --shapes data/shapes_e3.json --certs data/certs_e3.bin.xz --spot-check 10
python3 verify_proof.py --e 5 --shapes data/shapes_e5.json --certs data/certs_e5.bin.xz --spot-check 100
python3 verify_proof.py --e 7 --shapes data/shapes_e7.json --certs data/certs_e7.bin.xz --spot-check 500
```

## Reproducing Certificates from Scratch

```bash
python3 src/enum_shapes.py
clang++ -O3 -std=c++17 -pthread src/solver.cpp -o solver
./solver data/shapes_e3.json certs_e3.txt 4
./solver data/shapes_e5.json certs_e5.txt 8
./solver data/shapes_e7.json certs_e7.txt 8
```

## Repository Layout

- `paper/main.tex` — arXiv-ready LaTeX preprint (Theorems 1–2, Lemma 1, Corollary 3, and the order-17 `{R1, R3, R4}` sweep).
- `PROOF.md` — Full mathematical proof in Markdown.
- `verify_proof.py` — Standalone Python 3 verifier (supports `.bin.xz`, `.txt.gz`, and `.txt`; re-enumerates shapes from scratch and runs `--spot-check` constructive extension checks).
- `src/enum_shapes.py` & `src/solver.cpp` — Complete shape enumerator and multithreaded C++17 solver.
- `data/shapes_e{3,5,7}.json` & `data/certs_e{3,5,7}.bin.xz` — Underlying shapes and compact binary LZMA proof certificates (`certs_e7.bin.xz` is 33 MB, well under GitHub's 50 MB warning threshold).
- `.github/workflows/verify.yml` — GitHub Actions CI workflow.
- `LICENSE` — MIT License.
