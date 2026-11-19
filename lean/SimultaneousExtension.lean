/-
  Lean 4 Formalization (Pure Lean 4 Core — Zero External Dependencies)
  of Core Structural & Extension Lemmas for Super-Edge-Graceful Trees.

  Formalizes:
    1. Proposition 3 (Shape Bounds):
       |I(T0)| <= 2e - 2 and |V(T0)| <= 3e - 2.
    2. Lemma 1 (Simultaneous Chain-Extension Algebra):
       Exact vertex-sum identities for both d_j = 0 and d_j > 0 chain extensions.
    3. Theorem 2 (Sharp Linear Cutoff 23e - 36):
       Arithmetic verification of the distinctness + greedy pair-packing bound.
-/

namespace SuperEdgeGraceful

/-- Proposition 3 (Part 1): Bound on surviving internal vertices |I(T0)| <= 2e - 2. -/
theorem internal_vertex_bound
    (e d2 d3 d_ge4 excess_ge4 : Nat)
    (h_d2_le_e : d2 ≤ e)
    (h_d2_ge2 : 2 ≤ d2)
    (h_d3_le_u : d3 + d_ge4 + excess_ge4 ≤ d2 - 2) :
    d2 + d_ge4 + d3 ≤ 2 * e - 2 := by
  omega

/-- Proposition 3 (Part 2): Bound on the order |V(T0)| <= 3e - 2 of any underlying shape T0. -/
theorem underlying_shape_order_bound
    (e d2 d3 d4 d_ge5 s_ge5 : Nat)
    (h_even : d2 + d4 ≤ e)
    (h_d2_ge2 : 2 ≤ d2)
    (h_u : d3 + d4 + 2 * d_ge5 + s_ge5 ≤ d2 - 2) :
    d2 + 2 + 2 * d3 + 3 * d4 + 4 * d_ge5 + s_ge5 ≤ 3 * e - 2 := by
  omega

/-- Lemma 1 (Case d = 0): Exact vertex-sum identities for a length-m chain when f*(c) = a. -/
theorem chain_extension_zero_delta
    (a s_c u1 u_prev u_curr u_last : Int)
    (h_zero : s_c - a = 0) :
    (a + u1 + (-u1) = s_c) ∧
    (u_prev + u_curr + (-u_curr) = u_prev) ∧
    (s_c - a + u_last = u_last) := by
  omega

/-- Lemma 1 (Case d > 0): Exact vertex-sum identities for a length-m (>= 2) chain
    with reserved pair (t1, t2) satisfying sigma * (t2 - t1) = s_c - a. -/
theorem chain_extension_nonzero_delta
    (a s_c sigma t1 t2 u_prev u_curr u_last : Int)
    (h_diff : sigma * (t2 - t1) = s_c - a) :
    (a + sigma * t2 + (- (sigma * t1)) = s_c) ∧
    (sigma * t2 + sigma * u_curr + (- (sigma * t2)) = sigma * u_curr) ∧
    (sigma * u_prev + sigma * u_curr + (- (sigma * u_prev)) = sigma * u_curr) ∧
    (sigma * u_last + sigma * t1 + (- (sigma * u_last)) = sigma * t1) ∧
    (s_c - a + sigma * t1 = sigma * t2) := by
  have h1 : sigma * t2 - sigma * t1 = s_c - a := by
    rw [← Int.mul_sub]; exact h_diff
  omega

/-- Lemma 1 (Case d > 0, m = 2 boundary): When m = 2, no intermediate u_k vertices exist. -/
theorem chain_extension_nonzero_m2
    (a s_c sigma t1 t2 : Int)
    (h_diff : sigma * (t2 - t1) = s_c - a) :
    (a + sigma * t2 + (- (sigma * t1)) = s_c) ∧
    (sigma * t2 + sigma * t1 + (- (sigma * t2)) = sigma * t1) ∧
    (s_c - a + sigma * t1 = sigma * t2) := by
  have h1 : sigma * t2 - sigma * t1 = s_c - a := by
    rw [← Int.mul_sub]; exact h_diff
  omega

/-- Theorem 2 (Sharp Linear Cutoff 23e - 36):
    If n1 <= 7e - 8 - 2k, 1 <= k <= 2e - 3, and 2M <= 2n1 + 7k - 8 (with 2M <= 2n1 + 7k - 9
    when k = 2e - 3), then n1 + 2M <= 23e - 36. -/
theorem linear_cutoff_23e_minus_36
    (e k n1 M : Nat)
    (h_e : 3 ≤ e)
    (h_k_pos : 1 ≤ k)
    (h_k_le : k ≤ 2 * e - 3)
    (h_n1 : n1 ≤ 7 * e - 8 - 2 * k)
    (h_M_even : 2 * M ≤ 2 * n1 + 7 * k - 8)
    (h_M_maxk : k = 2 * e - 3 → 2 * M ≤ 2 * n1 + 7 * k - 9) :
    n1 + 2 * M ≤ 23 * e - 36 := by
  by_cases hk : k = 2 * e - 3
  · have hM := h_M_maxk hk
    omega
  · omega

end SuperEdgeGraceful
