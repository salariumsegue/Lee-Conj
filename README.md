# Super-Edge-Graceful Trees with $e \in \{3, 5, 7\}$ Even-Degree Vertices & $23e - 36$ Linear Cutoff

[[https://zenodo.org/badge/DOI/10.5281/zenodo.24382630.svg]](https://doi.org/10.5281/zenodo.24382630)

A complete, independently verifiable computer-assisted proof that **every finite tree with exactly 3, 5, or 7 vertices of even degree is super-edge-graceful**, and consequently satisfies **Lee's Edge-Graceful Tree Conjecture**, together with a **$23e - 36$** general linear finite cutoff for all odd $e \ge 3$, verified base certificates for all **1,526 underlying shapes of $e = 9$**, a conditional two-leaf extension, Lean 4 arithmetic sanity checks, and an exhaustive order-17 experimental sweep of the `{R1, R3, R4}` reduction.

## Verified Results

| Even-degree vertices ($e$) | Underlying shapes | Max shape order | Exhaustive splits | Terminal certificates | Constructive spot-checks | Verification time |
|---:|---:|---:|---:|---:|---:|---:|
| **3** | 2 | 7 | 11 | 13 | 10 (up to order 41) | 0.00 s |
| **5** | 12 | 13 | 647 | 659 | 100 (up to order 77) | 0.01 s |
| **7** | 116 | 19 | 808,934 | 809,050 | 375 (up to order 157) | 28.24 s |
| **9 (base)** | 1,526 | 25 | — | 1,526 ($\max d_j \le 11$) | Base labeling checks | < 1 s |

- **general linear cutoff (Theorem 2):** For every odd $e \ge 3$, if a counterexample to super-edge-gracefulness exists with $e$ even-degree vertices, one exists on at most **$23e - 36$** vertices (e.g., order $\le 171$ unconditionally for $e = 9$).
- **Two-leaf extension (Corollary 3):** Every tree of order $\ge 17$ with $e = 7$ even-degree vertices extends to a super-edge-graceful tree with $e = 9$ even-degree vertices (each further step requires the current order $n \ge 2e+3$).
- **Order-17 `{R1, R3, R4}` sweep (`src/check_n17.cpp`):** Across all 57,967 odd-order trees through order 17, 100% of non-path trees admit a 2-vertex SEG reduction (at order 17: 3,448/3,448 trees with a degree-$\ge 3$ pendant-$P_2$ site reduce by R3, and 277/277 remaining trees reduce by R4).
- **Status note:** Lee and Ho (2007) proved the $e = 3$ case (*J. Combin. Math. Combin. Comput.* 62, 53–64). Publication priority and independent peer review for the $e = 5$, $e = 7$, and $23e - 36$ results have not been established. The unrestricted conjectures remain open.

## Quick Verification (Zero External Dependencies)

Requires Python 3.8+ (standard library only):

```bash
python3 verify_proof.py --e 3 --shapes data/shapes_e3.json --certs data/certs_e3.bin.xz --spot-check 10
python3 verify_proof.py --e 5 --shapes data/shapes_e5.json --certs data/certs_e5.bin.xz --spot-check 100
python3 verify_proof.py --e 7 --shapes data/shapes_e7.json --certs data/certs_e7.bin.xz --spot-check 500
```

To reproduce the 0.48 s order-17 `{R1, R3, R4}` sweep (Table 2):
```bash
clang++ -O3 -std=c++17 -pthread src/check_n17.cpp -o check_n17 && ./check_n17
```

## Repository Layout

- `paper/main.tex` — arXiv-ready LaTeX preprint (Theorems 1–2, Lemma 5, Corollary 3, $e = 9$ base remark, and order-17 `{R1, R3, R4}` sweep).
- `lean/SimultaneousExtension.lean` — Lean 4 arithmetic sanity checks for identities used in the proofs. The tree and labeling arguments are in the manuscript.
- `PROOF.md` — Mathematical proof in Markdown.
- `verify_proof.py` — Standalone Python 3 verifier (`.bin.xz`, `.txt.gz`, `.txt` + `--spot-check` constructive checks).
- `src/enum_shapes.py`, `src/solver.cpp`, `src/check_n17.cpp` — Enumerator, multithreaded C++17 solver, and order-17 sweep checker.
- `data/shapes_e{3,5,7,9}.json`, `data/certs_e{3,5,7}.bin.xz`, `data/certs_e9_base.txt.gz` — Shapes and verified certificates (`certs_e7.bin.xz` is 33 MB).
- `.github/workflows/verify.yml` — GitHub Actions CI workflow.
- `LICENSE` — MIT License.
