#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

using namespace std;

struct RNG {
    uint64_t s;
    RNG(uint64_t seed) : s(seed ? seed : 88172645463325252ULL) {}
    inline uint64_t next() {
        uint64_t x = s;
        x ^= x >> 12; x ^= x << 25; x ^= x >> 27;
        s = x;
        return x * 2685821657736338717ULL;
    }
    inline int randint(int n) { return (int)(next() % (uint64_t)n); }
    inline double rand01() { return (next() >> 11) * (1.0 / 9007199254740992.0); }
};

string ahu_encode(const vector<vector<int>> &adj, int u, int p) {
    vector<string> subs;
    for (int v : adj[u]) if (v != p) subs.push_back(ahu_encode(adj, v, u));
    sort(subs.begin(), subs.end());
    string res = "(";
    for (auto &s : subs) res += s;
    res += ")";
    return res;
}

string canonical_tree(const vector<vector<int>> &adj) {
    int n = (int)adj.size();
    if (n == 1) return "()";
    vector<int> deg(n);
    vector<int> leaves;
    for (int i = 0; i < n; ++i) {
        deg[i] = (int)adj[i].size();
        if (deg[i] <= 1) leaves.push_back(i);
    }
    int rem = n;
    while (rem > 2) {
        rem -= (int)leaves.size();
        vector<int> nxt;
        for (int u : leaves) {
            deg[u] = 0;
            for (int v : adj[u]) {
                if (deg[v] > 0 && --deg[v] == 1) nxt.push_back(v);
            }
        }
        leaves = std::move(nxt);
    }
    string best = ahu_encode(adj, leaves[0], -1);
    if (leaves.size() == 2) {
        string s2 = ahu_encode(adj, leaves[1], -1);
        if (s2 < best) best = s2;
    }
    return best;
}

bool find_seg_cond(int n, const vector<pair<int, int>> &edges,
                   int cond_type, int v1, int v2, int target,
                   RNG &rng, int max_restarts = 20, int steps = 4500) {
    int h = (n - 1) / 2, E = 2 * h;
    const int OFF = 300;
    int cnt[2 * OFF + 1] = {0};
    vector<int> labels(E), sums(n);
    auto pen_out = [&](int s) { return (s < -h) ? (-h - s) * 3 : (s > h ? (s - h) * 3 : 0); };
    auto cond_cost = [&](const vector<int> &sm) {
        return (cond_type == 0) ? abs(sm[v1]) : abs(abs(sm[v1] - sm[v2]) - target);
    };
    for (int r = 0; r < max_restarts; ++r) {
        int idx = 0;
        for (int x = 1; x <= h; ++x) { labels[idx++] = x; labels[idx++] = -x; }
        for (int i = E - 1; i > 0; --i) swap(labels[i], labels[rng.randint(i + 1)]);
        fill(sums.begin(), sums.end(), 0);
        for (int i = 0; i < E; ++i) { sums[edges[i].first] += labels[i]; sums[edges[i].second] += labels[i]; }
        int seg = 0;
        for (int i = 0; i < n; ++i) {
            int s = sums[i];
            if (s >= -h && s <= h) { if (cnt[s + OFF] > 0) seg += 4; cnt[s + OFF]++; }
            else { seg += pen_out(s) + 4; cnt[s + OFF]++; }
        }
        int cc = cond_cost(sums);
        if (seg == 0 && cc == 0) { for (int i = 0; i < n; ++i) cnt[sums[i] + OFF] = 0; return true; }
        double temp = 3.0, alpha = pow(0.05 / 3.0, 1.0 / steps);
        for (int st = 0; st < steps; ++st) {
            int a = rng.randint(E), b = rng.randint(E - 1); if (b >= a) b++;
            int la = labels[a], lb = labels[b], diff = lb - la;
            int vs[4] = {edges[a].first, edges[a].second, edges[b].first, edges[b].second};
            int dv[4] = {diff, diff, -diff, -diff};
            int nv = 0, uv[4], udv[4];
            for (int t = 0; t < 4; ++t) {
                bool m = false;
                for (int j = 0; j < nv; ++j) if (uv[j] == vs[t]) { udv[j] += dv[t]; m = true; break; }
                if (!m) { uv[nv] = vs[t]; udv[nv] = dv[t]; nv++; }
            }
            int nseg = seg;
            for (int j = 0; j < nv; ++j) {
                if (!udv[j]) continue;
                int s = sums[uv[j]]; cnt[s + OFF]--;
                if (s >= -h && s <= h) { if (cnt[s + OFF] > 0) nseg -= 4; } else nseg -= (pen_out(s) + 4);
            }
            for (int j = 0; j < nv; ++j) {
                if (!udv[j]) continue;
                int s = sums[uv[j]] + udv[j];
                if (s >= -h && s <= h) { if (cnt[s + OFF] > 0) nseg += 4; } else nseg += (pen_out(s) + 4);
                cnt[s + OFF]++;
            }
            for (int j = 0; j < nv; ++j) sums[uv[j]] += udv[j];
            int ncc = cond_cost(sums);
            int delta = (nseg * 2 + ncc) - (seg * 2 + cc);
            if (delta <= 0 || rng.rand01() < exp(-delta / temp)) {
                labels[a] = lb; labels[b] = la; seg = nseg; cc = ncc;
                if (seg == 0 && cc == 0) { for (int i = 0; i < n; ++i) cnt[sums[i] + OFF] = 0; return true; }
            } else {
                for (int j = 0; j < nv; ++j) sums[uv[j]] -= udv[j];
                for (int j = 0; j < nv; ++j) {
                    if (!udv[j]) continue;
                    cnt[sums[uv[j]] + udv[j] + OFF]--; cnt[sums[uv[j]] + OFF]++;
                }
            }
            temp *= alpha;
        }
        for (int i = 0; i < n; ++i) cnt[sums[i] + OFF] = 0;
    }
    return false;
}

int main() {
    auto t0 = chrono::steady_clock::now();
    // Step 1: Generate all unrooted trees up to m = 14 (since non-path sibling-free tree of order 17 has >= 3 leaves => |I| <= 14, and |L| <= |I| => |I| >= 9)
    vector<vector<vector<vector<int>>>> trees(15);
    trees[1] = {{{}}};
    for (int k = 2; k <= 14; ++k) {
        unordered_set<string> seen;
        for (const auto &adj : trees[k - 1]) {
            for (int u = 0; u < k - 1; ++u) {
                auto nadj = adj;
                nadj.push_back({u});
                nadj[u].push_back(k - 1);
                string c = canonical_tree(nadj);
                if (seen.insert(c).second) trees[k].push_back(std::move(nadj));
            }
        }
    }

    // Step 2: Generate all sibling-free non-path trees of order 17
    unordered_set<string> seen17;
    vector<vector<pair<int, int>>> nosib17;
    for (int m = 9; m <= 14; ++m) {
        int need_leaves = 17 - m; // 3..8
        for (const auto &iadj : trees[m]) {
            vector<int> int_leaves, int_nonleaves;
            for (int i = 0; i < m; ++i) {
                if (iadj[i].size() == 1) int_leaves.push_back(i);
                else int_nonleaves.push_back(i);
            }
            int forced = (int)int_leaves.size();
            if (forced > need_leaves) continue;
            int extra = need_leaves - forced;
            if (extra > (int)int_nonleaves.size()) continue;

            // Choose `extra` vertices from int_nonleaves
            vector<int> sel(int_nonleaves.size(), 0);
            fill(sel.end() - extra, sel.end(), 1);
            do {
                auto adj = iadj;
                int nxt = m;
                for (int u : int_leaves) { adj[u].push_back(nxt); adj.push_back({u}); nxt++; }
                for (size_t j = 0; j < int_nonleaves.size(); ++j) {
                    if (sel[j]) { int u = int_nonleaves[j]; adj[u].push_back(nxt); adj.push_back({u}); nxt++; }
                }
                string c = canonical_tree(adj);
                if (seen17.insert(c).second) {
                    vector<pair<int, int>> edges;
                    for (int u = 0; u < 17; ++u)
                        for (int v : adj[u]) if (u < v) edges.push_back({u, v});
                    nosib17.push_back(std::move(edges));
                }
            } while (next_permutation(sel.begin(), sel.end()));
        }
    }

    auto dt1 = chrono::duration<double>(chrono::steady_clock::now() - t0).count();
    int N = (int)nosib17.size();
    cout << "Enumerated " << N << " sibling-free non-path trees of order 17 in " << dt1 << "s.\n" << flush;

    atomic<int> idx(0), has_p2_deg3(0), r3_ok(0), r3_fail_with_deg3(0), all_p3(0), r4_ok(0), total_fail(0);
    auto worker = [&](int tid) {
        RNG rng(77777ULL * (tid + 1) + 98765ULL);
        while (true) {
            int i = idx.fetch_add(1);
            if (i >= N) break;
            const auto &edges = nosib17[i];
            vector<vector<int>> adj(17);
            for (auto [u, v] : edges) { adj[u].push_back(v); adj[v].push_back(u); }
            vector<int> leaves;
            for (int v = 0; v < 17; ++v) if (adj[v].size() == 1) leaves.push_back(v);

            auto make_sub = [&](int r1, int r2, vector<int> &mp) {
                mp.assign(17, -1); int c = 0;
                for (int v = 0; v < 17; ++v) if (v != r1 && v != r2) mp[v] = c++;
                vector<pair<int, int>> se;
                for (auto [u, v] : edges) if (mp[u] != -1 && mp[v] != -1) se.push_back({mp[u], mp[v]});
                return se;
            };

            vector<tuple<int, int, int>> deg3_sites;
            for (int v0 : leaves) {
                int v1 = adj[v0][0];
                if (adj[v1].size() == 2) {
                    int v2 = (adj[v1][0] == v0) ? adj[v1][1] : adj[v1][0];
                    if (adj[v2].size() >= 3) deg3_sites.push_back({v0, v1, v2});
                }
            }

            if (!deg3_sites.empty()) {
                has_p2_deg3++;
                bool ok3 = false;
                for (auto [v0, v1, v2] : deg3_sites) {
                    vector<int> mp;
                    auto se = make_sub(v0, v1, mp);
                    if (find_seg_cond(15, se, 0, mp[v2], -1, 0, rng, 20, 4500)) { ok3 = true; break; }
                }
                if (ok3) { r3_ok++; continue; }
                // Retry harder before declaring R3 failure on deg>=3 site
                for (auto [v0, v1, v2] : deg3_sites) {
                    vector<int> mp;
                    auto se = make_sub(v0, v1, mp);
                    if (find_seg_cond(15, se, 0, mp[v2], -1, 0, rng, 60, 8000)) { ok3 = true; break; }
                }
                if (ok3) { r3_ok++; continue; }
                r3_fail_with_deg3++;
            } else {
                all_p3++;
            }

            // Fallback R4 (target diff = h_new = 8)
            bool ok4 = false;
            for (size_t a = 0; a < leaves.size() && !ok4; ++a) {
                for (size_t b = a + 1; b < leaves.size() && !ok4; ++b) {
                    int l1 = leaves[a], l2 = leaves[b];
                    int u = adj[l1][0], v = adj[l2][0];
                    vector<int> mp;
                    auto se = make_sub(l1, l2, mp);
                    if (find_seg_cond(15, se, 1, mp[u], mp[v], 8, rng, 20, 4500)) { ok4 = true; }
                }
            }
            if (ok4) r4_ok++;
            else total_fail++;
        }
    };

    vector<thread> ths;
    for (int t = 0; t < 8; ++t) ths.emplace_back(worker, t);
    for (auto &t : ths) t.join();

    auto dt2 = chrono::duration<double>(chrono::steady_clock::now() - t0).count();
    cout << "Order 17 DONE in " << dt2 << "s:\n"
         << "  Total sibling-free non-path trees: " << N << "\n"
         << "  With deg>=3 pendant-P2 site: " << has_p2_deg3 << " (R3 ok: " << r3_ok << ", R3 fail: " << r3_fail_with_deg3 << ")\n"
         << "  All pendant paths length >= 3: " << all_p3 << "\n"
         << "  R4 fallback ok: " << r4_ok << "\n"
         << "  Total covered: " << (r3_ok + r4_ok) << "/" << N << ", Total failed: " << total_fail << "\n";
    return 0;
}
