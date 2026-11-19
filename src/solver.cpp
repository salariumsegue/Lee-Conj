#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

struct RNG {
    uint64_t s;
    RNG(uint64_t seed = 123456789ULL) : s(seed ? seed : 88172645463325252ULL) {}
    inline uint64_t next() {
        uint64_t x = s;
        x ^= x >> 12;
        x ^= x << 25;
        x ^= x >> 27;
        s = x;
        return x * 2685821657736338717ULL;
    }
    inline int randint(int n) { return (int)(next() % (uint64_t)n); }
    inline double rand01() { return (next() >> 11) * (1.0 / 9007199254740992.0); }
};

struct Shape {
    int id;
    int n, m;
    vector<pair<int, int>> internal_edges;
    vector<pair<int, int>> leaf_edges;
};

struct ConcreteTree {
    int n, h;
    vector<pair<int, int>> edges;
    vector<int> open_edge_idx;
    vector<int> open_orig_id;
    vector<int> open_L;
};

struct LabelingResult {
    bool found = false;
    int root = 0;
    vector<int> labels;
    vector<int> d_vals;
    int excess = 1e9;
    int num_fail = 1e9;
};

ConcreteTree build_tree(const Shape &sh, const vector<int> &state) {
    ConcreteTree T;
    int next_v = sh.n;
    for (int i = 0; i < (int)sh.internal_edges.size(); ++i) {
        auto [u, v] = sh.internal_edges[i];
        if (state[i] < 0) {
            int idx = (int)T.edges.size();
            T.edges.push_back({u, v});
            T.open_edge_idx.push_back(idx);
            T.open_orig_id.push_back(i);
            T.open_L.push_back(-state[i]);
        } else if (state[i] == 0) {
            T.edges.push_back({u, v});
        } else {
            int len = state[i];
            int prev = u;
            for (int k = 0; k < len; ++k) {
                int w = next_v++;
                int lf = next_v++;
                T.edges.push_back({prev, w});
                T.edges.push_back({w, lf});
                prev = w;
            }
            T.edges.push_back({prev, v});
        }
    }
    for (auto [u, v] : sh.leaf_edges) T.edges.push_back({u, v});
    T.n = next_v;
    T.h = (T.n - 1) / 2;
    return T;
}

void eval_root(const ConcreteTree &T, int root, const vector<int> &labels,
               const vector<int> &sums, const vector<vector<int>> &adj,
               vector<int> &d_out, int &excess_out, int &nfail_out) {
    int n = T.n;
    static thread_local vector<int> parent, q;
    parent.assign(n, -1);
    q.clear();
    q.push_back(root);
    parent[root] = root;
    for (size_t qi = 0; qi < q.size(); ++qi) {
        int u = q[qi];
        for (int v : adj[u]) {
            if (parent[v] == -1) {
                parent[v] = u;
                q.push_back(v);
            }
        }
    }
    int K = (int)T.open_edge_idx.size();
    d_out.resize(K);
    excess_out = 0;
    nfail_out = 0;
    for (int k = 0; k < K; ++k) {
        int eidx = T.open_edge_idx[k];
        auto [u, v] = T.edges[eidx];
        int c = (parent[v] == u) ? v : u;
        int d = abs(sums[c] - labels[eidx]);
        d_out[k] = d;
        int need = T.open_L[k] - 1;
        if (d > need) {
            excess_out += (d - need);
            nfail_out++;
        }
    }
}

void pick_best_root(const ConcreteTree &T, const vector<int> &labels,
                    const vector<int> &sums, const vector<vector<int>> &adj,
                    LabelingResult &res) {
    int K = (int)T.open_edge_idx.size();
    if (K == 0) {
        res.found = true;
        res.root = 0;
        res.labels = labels;
        res.d_vals.clear();
        res.excess = 0;
        res.num_fail = 0;
        return;
    }
    vector<int> cur_d, best_d;
    int best_r = -1, best_ex = 1e9, best_nf = 1e9;
    for (int r = 0; r < T.n; ++r) {
        if (adj[r].size() < 2) continue;
        int ex, nf;
        eval_root(T, r, labels, sums, adj, cur_d, ex, nf);
        if (ex < best_ex || (ex == best_ex && nf < best_nf)) {
            best_ex = ex;
            best_nf = nf;
            best_r = r;
            best_d = cur_d;
            if (best_ex == 0) break;
        }
    }
    if (best_ex < res.excess || (best_ex == res.excess && best_nf < res.num_fail)) {
        res.found = true;
        res.root = best_r;
        res.labels = labels;
        res.d_vals = best_d;
        res.excess = best_ex;
        res.num_fail = best_nf;
    }
}

LabelingResult find_best_labeling(const ConcreteTree &T, RNG &rng,
                                  int max_restarts, int steps_per_restart,
                                  const LabelingResult *warm = nullptr) {
    int n = T.n, h = T.h, E = 2 * h, K = (int)T.open_edge_idx.size();
    vector<vector<int>> adj(n);
    for (int i = 0; i < E; ++i) {
        adj[T.edges[i].first].push_back(T.edges[i].second);
        adj[T.edges[i].second].push_back(T.edges[i].first);
    }

    LabelingResult best;
    if (warm && warm->found && (int)warm->labels.size() == E) {
        vector<int> sums(n, 0);
        for (int i = 0; i < E; ++i) {
            sums[T.edges[i].first] += warm->labels[i];
            sums[T.edges[i].second] += warm->labels[i];
        }
        pick_best_root(T, warm->labels, sums, adj, best);
        if (best.excess == 0) return best;
    }

    vector<int> int_verts;
    for (int i = 0; i < n; ++i) if (adj[i].size() >= 2) int_verts.push_back(i);

    vector<int> eu(E), ev(E);
    for (int i = 0; i < E; ++i) { eu[i] = T.edges[i].first; ev[i] = T.edges[i].second; }

    const int OFF = 2500;
    vector<int> cnt(2 * OFF + 1, 0), labels(E), sums(n), open_child(K);
    vector<vector<int>> v_open(n);
    vector<int> e_open(E, -1);
    for (int k = 0; k < K; ++k) e_open[T.open_edge_idx[k]] = k;

    auto calc_seg_pen = [&](int s) -> int {
        if (s < -h) return (-h - s) * 3;
        if (s > h) return (s - h) * 3;
        return 0;
    };

    for (int restart = 0; restart < max_restarts; ++restart) {
        int guide_root = (restart < (int)int_verts.size())
                             ? int_verts[restart]
                             : int_verts[rng.randint((int)int_verts.size())];
        vector<int> parent(n, -1), q;
        q.push_back(guide_root);
        parent[guide_root] = guide_root;
        for (size_t qi = 0; qi < q.size(); ++qi) {
            int u = q[qi];
            for (int v : adj[u]) {
                if (parent[v] == -1) { parent[v] = u; q.push_back(v); }
            }
        }
        for (int i = 0; i < n; ++i) v_open[i].clear();
        for (int k = 0; k < K; ++k) {
            int eidx = T.open_edge_idx[k];
            int c = (parent[ev[eidx]] == eu[eidx]) ? ev[eidx] : eu[eidx];
            open_child[k] = c;
            v_open[c].push_back(k);
        }

        int idx = 0;
        for (int x = 1; x <= h; ++x) { labels[idx++] = x; labels[idx++] = -x; }
        for (int i = E - 1; i > 0; --i) swap(labels[i], labels[rng.randint(i + 1)]);

        fill(sums.begin(), sums.end(), 0);
        for (int i = 0; i < E; ++i) { sums[eu[i]] += labels[i]; sums[ev[i]] += labels[i]; }
        int seg_cost = 0;
        for (int i = 0; i < n; ++i) {
            int s = sums[i];
            if (s >= -h && s <= h) { if (cnt[s + OFF] > 0) seg_cost += 4; cnt[s + OFF]++; }
            else { seg_cost += calc_seg_pen(s) + 4; cnt[s + OFF]++; }
        }
        int ext_cost = 0;
        for (int k = 0; k < K; ++k) {
            int eidx = T.open_edge_idx[k];
            int d = abs(sums[open_child[k]] - labels[eidx]);
            int need = T.open_L[k] - 1;
            if (d > need) ext_cost += (d - need);
        }

        if (seg_cost == 0) {
            pick_best_root(T, labels, sums, adj, best);
            if (best.excess == 0) { for (int i = 0; i < n; ++i) cnt[sums[i] + OFF] = 0; return best; }
        }

        double T_start = 4.0, T_end = 0.08;
        double alpha = pow(T_end / T_start, 1.0 / steps_per_restart);
        double temp = T_start;

        for (int step = 0; step < steps_per_restart; ++step) {
            int a = rng.randint(E);
            int b = rng.randint(E - 1);
            if (b >= a) b++;
            int la = labels[a], lb = labels[b], diff = lb - la;
            int vs[4] = {eu[a], ev[a], eu[b], ev[b]};
            int dv[4] = {diff, diff, -diff, -diff};
            int nv = 0, uv[4], udv[4];
            for (int t = 0; t < 4; ++t) {
                bool merged = false;
                for (int j = 0; j < nv; ++j) if (uv[j] == vs[t]) { udv[j] += dv[t]; merged = true; break; }
                if (!merged) { uv[nv] = vs[t]; udv[nv] = dv[t]; nv++; }
            }

            int old_seg = seg_cost, new_seg = seg_cost;
            for (int j = 0; j < nv; ++j) {
                if (udv[j] == 0) continue;
                int s = sums[uv[j]];
                cnt[s + OFF]--;
                if (s >= -h && s <= h) { if (cnt[s + OFF] > 0) new_seg -= 4; }
                else new_seg -= (calc_seg_pen(s) + 4);
            }
            for (int j = 0; j < nv; ++j) {
                if (udv[j] == 0) continue;
                int s = sums[uv[j]] + udv[j];
                if (s >= -h && s <= h) { if (cnt[s + OFF] > 0) new_seg += 4; }
                else new_seg += (calc_seg_pen(s) + 4);
                cnt[s + OFF]++;
            }

            int aff_k[8], naff = 0;
            auto add_k = [&](int k) {
                if (k < 0) return;
                for (int j = 0; j < naff; ++j) if (aff_k[j] == k) return;
                aff_k[naff++] = k;
            };
            add_k(e_open[a]); add_k(e_open[b]);
            for (int j = 0; j < nv; ++j) if (udv[j] != 0) for (int k : v_open[uv[j]]) add_k(k);

            int old_ext = ext_cost, new_ext = ext_cost;
            for (int j = 0; j < naff; ++j) {
                int k = aff_k[j], eidx = T.open_edge_idx[k], c = open_child[k], need = T.open_L[k] - 1;
                int old_d = abs(sums[c] - labels[eidx]);
                if (old_d > need) new_ext -= (old_d - need);
                int ns = sums[c];
                for (int t = 0; t < nv; ++t) if (uv[t] == c) { ns += udv[t]; break; }
                int nl = (eidx == a) ? lb : (eidx == b ? la : labels[eidx]);
                int new_d = abs(ns - nl);
                if (new_d > need) new_ext += (new_d - need);
            }

            int delta = (new_seg * 2 + new_ext) - (old_seg * 2 + old_ext);
            if (delta <= 0 || rng.rand01() < exp(-delta / temp)) {
                labels[a] = lb; labels[b] = la;
                for (int j = 0; j < nv; ++j) sums[uv[j]] += udv[j];
                seg_cost = new_seg; ext_cost = new_ext;
                if (seg_cost == 0) {
                    if (ext_cost == 0) {
                        best.found = true; best.root = guide_root; best.labels = labels;
                        best.d_vals.resize(K);
                        for (int k = 0; k < K; ++k) best.d_vals[k] = abs(sums[open_child[k]] - labels[T.open_edge_idx[k]]);
                        best.excess = 0; best.num_fail = 0;
                        for (int i = 0; i < n; ++i) cnt[sums[i] + OFF] = 0;
                        return best;
                    }
                    if (ext_cost < best.excess + 3) {
                        pick_best_root(T, labels, sums, adj, best);
                        if (best.excess == 0) { for (int i = 0; i < n; ++i) cnt[sums[i] + OFF] = 0; return best; }
                    }
                }
            } else {
                for (int j = 0; j < nv; ++j) {
                    if (udv[j] == 0) continue;
                    cnt[sums[uv[j]] + udv[j] + OFF]--;
                    cnt[sums[uv[j]] + OFF]++;
                }
            }
            temp *= alpha;
        }
        for (int i = 0; i < n; ++i) cnt[sums[i] + OFF] = 0;
    }
    return best;
}

struct ProofNode {
    bool is_leaf = false;
    int root = 0;
    vector<int> labels;
    int split_orig_id = -1, split_val = 0, left_id = -1, right_id = -1;
};

int solve_recursive(const Shape &sh, vector<int> state, vector<ProofNode> &nodes,
                    RNG &rng, const LabelingResult *warm, int &leaves_cnt, int &splits_cnt) {
    ConcreteTree T = build_tree(sh, state);
    int K = (int)T.open_edge_idx.size();
    int restarts = (K >= 6) ? 12 : (K >= 3 ? 6 : 3);
    int steps = (K >= 6) ? 12000 : (K >= 3 ? 6000 : 3000);
    if (K == 0) { restarts = 5; steps = 3000; }

    LabelingResult res = find_best_labeling(T, rng, restarts, steps, warm);
    while (!res.found) {
        restarts *= 2; steps *= 2;
        res = find_best_labeling(T, rng, restarts, steps, nullptr);
    }

    int my_id = (int)nodes.size();
    nodes.emplace_back();

    if (res.excess == 0) {
        nodes[my_id].is_leaf = true;
        nodes[my_id].root = res.root;
        nodes[my_id].labels = std::move(res.labels);
        leaves_cnt++;
        return my_id;
    }

    int best_k = -1, best_gap = 1e9, best_L = 1e9;
    for (int k = 0; k < K; ++k) {
        int gap = res.d_vals[k] - (T.open_L[k] - 1);
        if (gap > 0 && (gap < best_gap || (gap == best_gap && T.open_L[k] < best_L))) {
            best_gap = gap; best_L = T.open_L[k]; best_k = k;
        }
    }

    int orig_id = T.open_orig_id[best_k], L = T.open_L[best_k];
    nodes[my_id].is_leaf = false;
    nodes[my_id].split_orig_id = orig_id;
    nodes[my_id].split_val = L;
    splits_cnt++;

    vector<int> left_state = state; left_state[orig_id] = L;
    int lid = solve_recursive(sh, left_state, nodes, rng, nullptr, leaves_cnt, splits_cnt);

    vector<int> right_state = state; right_state[orig_id] = -(L + 1);
    int rid = solve_recursive(sh, right_state, nodes, rng, &res, leaves_cnt, splits_cnt);

    nodes[my_id].left_id = lid;
    nodes[my_id].right_id = rid;
    return my_id;
}

vector<Shape> load_shapes(const string &path) {
    ifstream ifs(path);
    string s((istreambuf_iterator<char>(ifs)), istreambuf_iterator<char>());
    vector<Shape> shapes;
    size_t pos = 0;
    auto find_int = [&](const string &key) -> int {
        size_t p = s.find(key, pos);
        p = s.find(':', p) + 1;
        while (p < s.size() && (s[p] == ' ' || s[p] == '\n')) p++;
        size_t endp;
        int val = stoi(s.substr(p), &endp);
        pos = p + endp;
        return val;
    };
    auto find_edges = [&](const string &key) -> vector<pair<int, int>> {
        size_t p = s.find(key, pos);
        p = s.find('[', p) + 1;
        vector<pair<int, int>> res;
        while (p < s.size()) {
            while (p < s.size() && s[p] != '[' && s[p] != ']') p++;
            if (p >= s.size() || s[p] == ']') { pos = p + 1; break; }
            p++;
            size_t endp;
            int u = stoi(s.substr(p), &endp); p += endp;
            while (p < s.size() && (s[p] == ',' || s[p] == ' ')) p++;
            int v = stoi(s.substr(p), &endp); p += endp;
            res.push_back({u, v});
            while (p < s.size() && s[p] != ']') p++;
            p++;
        }
        return res;
    };
    while (true) {
        size_t p = s.find("\"id\"", pos);
        if (p == string::npos) break;
        Shape sh;
        sh.id = find_int("\"id\"");
        sh.n = find_int("\"n\"");
        sh.m = find_int("\"m\"");
        sh.internal_edges = find_edges("\"internal_edges\"");
        sh.leaf_edges = find_edges("\"leaf_edges\"");
        shapes.push_back(sh);
    }
    return shapes;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <shapes.json> <out_certs.txt> [num_threads]\n";
        return 1;
    }
    auto shapes = load_shapes(argv[1]);
    int N = (int)shapes.size();
    int num_threads = (argc >= 4) ? stoi(argv[3]) : 8;

    vector<vector<ProofNode>> all_nodes(N);
    vector<int> shape_leaves(N, 0), shape_splits(N, 0);
    atomic<int> next_idx(0);

    auto worker = [&]() {
        while (true) {
            int i = next_idx.fetch_add(1);
            if (i >= N) break;
            const Shape &sh = shapes[i];
            vector<int> init_state(sh.internal_edges.size(), -1);
            RNG rng(1000003ULL * (i + 1) + 77777ULL);
            int lc = 0, sc = 0;
            solve_recursive(sh, init_state, all_nodes[i], rng, nullptr, lc, sc);
            shape_leaves[i] = lc; shape_splits[i] = sc;
        }
    };

    vector<thread> threads;
    for (int t = 0; t < num_threads; ++t) threads.emplace_back(worker);
    for (auto &th : threads) th.join();

    ofstream ofs(argv[2]);
    ofs << "SHAPES " << N << "\n";
    for (int i = 0; i < N; ++i) {
        ofs << "SHAPE " << shapes[i].id << " " << all_nodes[i].size() << "\n";
        for (const auto &nd : all_nodes[i]) {
            if (nd.is_leaf) {
                ofs << "L " << nd.root << " " << nd.labels.size();
                for (int w : nd.labels) ofs << " " << w;
                ofs << "\n";
            } else {
                ofs << "S " << nd.split_orig_id << " " << nd.split_val
                    << " " << nd.left_id << " " << nd.right_id << "\n";
            }
        }
    }
    return 0;
}
