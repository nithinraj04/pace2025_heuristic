// Scored: 94.80

// #include </opt/homebrew/include/glpk.h>
#include <glpk.h>
#include <bits/stdc++.h>
using namespace std;
using namespace chrono;

const int SMALL_COMPONENT_THRESHOLD = 256; // Threshold for brute-force small components

int n, m;
vector<vector<int>> adj;
vector<bool> dominated, in_dset;
vector<int> degree;
vector<int> dom_set;

high_resolution_clock::time_point start_time;
const int TIME_LIMIT_SECONDS = 285; // a bit under 5 mins
const int TIME_LIMIT_SECONDS_2 = 297; // a bit under 5 mins

bool time_exceeded() {
    return duration_cast<seconds>(high_resolution_clock::now() - start_time).count() > TIME_LIMIT_SECONDS;
}

bool time_exceeded2() {
    return duration_cast<seconds>(high_resolution_clock::now() - start_time).count() > TIME_LIMIT_SECONDS_2;
}

void isolated_node_prune(){
    int count = 0;
    for (int i = 0; i < n; ++i) {
        if (!dominated[i] && adj[i].empty()) {
            count++;
            in_dset[i] = true;
            dom_set.push_back(i);
            dominated[i] = true;
        }
    }
    cerr << "Removed " << count << " isolated nodes...\n";
}

void degree1_prune() {
    int count=0;
    for (int i = 0; i < n; ++i) {
        if (!dominated[i] && degree[i] == 1) {
            int neighbor = adj[i][0];
            if (!in_dset[neighbor]) {
                count++;
                in_dset[neighbor] = true;
                dom_set.push_back(neighbor);
                dominated[neighbor] = true;
                for (int v : adj[neighbor]) dominated[v] = true;
            }
        }
    }
    cerr << "Removed " << count << " nodes...\n";
}

void triangle_prune() {
    int count = 0;
    for (int u = 0; u < n; ++u) {
        if (!dominated[u] && adj[u].size() == 2) {
            int v = adj[u][0], w = adj[u][1];
            if (find(adj[v].begin(), adj[v].end(), w) != adj[v].end()) {
                if (!in_dset[v] && !in_dset[w]) {
                    count++;
                    int undom_v=0, undom_w=0;
                    for (int x : adj[v]) if (!dominated[x]) undom_v++;
                    for (int x : adj[w]) if (!dominated[x]) undom_w++;
                    if (degree[w] == 2 || (degree[v] != 2 && undom_v > undom_w)){
                        in_dset[v] = true;
                        dom_set.push_back(v);
                        dominated[v] = true;
                        for (int x : adj[v]) dominated[x] = true;
                    }else{
                        in_dset[w] = true;
                        dom_set.push_back(w);
                        dominated[w] = true;
                        for (int x : adj[w]) dominated[x] = true;
                    }
                }
            }
        }
    }
    cerr << "Removed " << count << " nodes...\n";
}

void crown_reduction() {
    // Find crown structures and reduce them
    vector<bool> in_crown(n, false);
    vector<bool> crown_head(n, false);
    int count = 0;
    
    for (int u = 0; u < n; ++u) {
        if (dominated[u]) continue;
        
        // Check if u can only be dominated by a small set of vertices
        vector<int> dominators;
        dominators.push_back(u); // u can dominate itself
        for (int v : adj[u]) {
            if (!dominated[v]) dominators.push_back(v);
        }
        
        if (dominators.size() <= 3) {
            // Check if this forms a crown structure
            bool is_crown = true;
            for (int v : dominators) {
                int external_connections = 0;
                for (int w : adj[v]) {
                    if (find(dominators.begin(), dominators.end(), w) == dominators.end()) {
                        external_connections++;
                    }
                }
                if (external_connections > dominators.size()) {
                    is_crown = false;
                    break;
                }
            }
            
            if (is_crown && dominators.size() > 1) {
                // Select the best dominator
                int best = dominators[0];
                int max_coverage = 0;
                for (int v : dominators) {
                    int coverage = 0;
                    for (int w : adj[v]) if (!dominated[w]) coverage++;
                    if (!dominated[v]) coverage++;
                    if (coverage > max_coverage) {
                        max_coverage = coverage;
                        best = v;
                    }
                }
                count++;
                in_dset[best] = true;
                dom_set.push_back(best);
                dominated[best] = true;
                for (int w : adj[best]) dominated[w] = true;
            }
        }
    }
    cerr << "Removed " << count << " nodes...\n";
}

void dfs_component(int u, vector<int>& comp, vector<bool>& visited) {
    stack<int> s;
    s.push(u);
    visited[u] = true;

    while (!s.empty()) {
        int curr = s.top(); s.pop();
        comp.push_back(curr);

        for (int v : adj[curr]) {
            if (!visited[v]) {
                visited[v] = true;
                s.push(v);
            }
        }
    }
}

bool is_valid_dset(const vector<int>& subset, const vector<int>& comp) {
    unordered_set<int> d(subset.begin(), subset.end());
    unordered_set<int> covered;
    for (int u : d) {
        covered.insert(u);
        for (int v : adj[u]) covered.insert(v);
    }
    for (int u : comp) if (covered.find(u) == covered.end()) return false;
    return true;
}

bool solve_dominating_set_ilp(const vector<int>& comp, vector<int>& solution) {
    if (time_exceeded()) return false;

    int sz = comp.size();
    glp_prob *lp = glp_create_prob();
    glp_set_prob_name(lp, "dom_set");
    glp_set_obj_dir(lp, GLP_MIN);

    glp_add_cols(lp, sz);
    for (int i = 0; i < sz; ++i) {
        if (time_exceeded()) {
            glp_delete_prob(lp);
            return false;
        }
        glp_set_col_kind(lp, i + 1, GLP_BV);
        glp_set_obj_coef(lp, i + 1, 1.0);

        if (in_dset[comp[i]]) {
            glp_set_col_bnds(lp, i + 1, GLP_FX, 1.0, 1.0); // force in-dset
        }
    }

    vector<int> ia(1), ja(1); // 1-based
    vector<double> ar(1);
    int constraint_count = 0;

    for (int i = 0; i < sz; ++i) {
        if (time_exceeded()) {
            glp_delete_prob(lp);
            return false;
        }

        int v = comp[i];
        vector<int> neighborhood = {i}; // include self
        for (int u : adj[v]) {
            auto it = find(comp.begin(), comp.end(), u);
            if (it != comp.end())
                neighborhood.push_back(it - comp.begin());
        }

        constraint_count++;
        glp_add_rows(lp, 1);
        glp_set_row_bnds(lp, constraint_count, GLP_LO, 1.0, 0.0);

        for (int j : neighborhood) {
            ia.push_back(constraint_count);
            ja.push_back(j + 1);
            ar.push_back(1.0);
        }
    }

    glp_load_matrix(lp, ia.size() - 1, ia.data(), ja.data(), ar.data());

    if (time_exceeded()) {
        glp_delete_prob(lp);
        return false;
    }

    glp_iocp parm;
    glp_init_iocp(&parm);
    parm.presolve = GLP_ON;

    glp_term_out(GLP_OFF);

    int res = glp_intopt(lp, &parm);

    if (time_exceeded()) {
        glp_delete_prob(lp);
        return false;
    }

    if (res == 0) {
        solution.clear();
        for (int i = 0; i < sz; ++i) {
            if (glp_mip_col_val(lp, i + 1) > 0.5) {
                solution.push_back(comp[i]);
            }
        }
        glp_delete_prob(lp);
        return true;
    }

    glp_delete_prob(lp);
    return false;
}

void brute_force_small_components() {
    int count = 0;
    vector<bool> visited(n);

    priority_queue<pair<int, vector<int>>, vector<pair<int, vector<int>>>, greater<>> pq;
    
    // cerr << "Exceeded components:\n";
    for (int i = 0; i < n; ++i) {
        if (!visited[i]) {
            vector<int> comp;
            dfs_component(i, comp, visited);
            if ((int)comp.size() <= SMALL_COMPONENT_THRESHOLD) pq.emplace((int)comp.size(), comp);
            // else cerr << "Component size :" << comp.size() << "\n";
        }
    }
    
    // cerr << "Time\tSize\n";

    while (!pq.empty()) {
        auto [sz, comp] = pq.top(); pq.pop();
        if (time_exceeded()) return;

        // auto s_time = high_resolution_clock::now();
        
        if (sz > 30){
            vector<int> solution;
            if (solve_dominating_set_ilp(comp, solution)) {
                for (int u : solution) {
                    if (!in_dset[u]) {
                        count++;
                        in_dset[u] = true;
                        dom_set.push_back(u);
                        dominated[u] = true;
                        for (int v : adj[u]) dominated[v] = true;
                    }
                }
                // cerr << duration_cast<seconds>(high_resolution_clock::now() - s_time).count() << '\t' << sz << '\n';
            }
            continue;
        }

        vector<int> fixed_indices;
        vector<int> candidate_indices;
        bool undominated = false;

        for (int i = 0; i < sz; ++i) {
            if (in_dset[comp[i]]) {
                fixed_indices.push_back(i);
            } else {
                candidate_indices.push_back(i);
            }
            if (!dominated[comp[i]]) undominated = true;
        }
        if (!undominated) continue;

        bool found = false;
        int need = candidate_indices.size();
        
        vector<int> subset;
        for (int fi : fixed_indices)
            subset.push_back(comp[fi]);

        for (int r = 0; r <= need && !found; ++r) {
            vector<bool> select(need, false);
            fill(select.end() - r, select.end(), true); // choose r nodes
            
            do {
                if (time_exceeded()) return;

                for (int i = 0; i < need; ++i) {
                    if (select[i]) {
                        int idx = candidate_indices[i];
                        subset.push_back(comp[idx]);
                    }
                }

                if (is_valid_dset(subset, comp)) {
                    for (int u : subset) {
                        if (!in_dset[u]) {
                            count++;
                            in_dset[u] = true;
                            dom_set.push_back(u);
                            dominated[u] = true;
                            for (int v : adj[u]) dominated[v] = true;
                        }
                    }
                    found = true;
                    break;
                }else {
                    subset.erase(subset.end() - r, subset.end());
                }
            } while (next_permutation(select.begin(), select.end()));
            // cerr << duration_cast<seconds>(high_resolution_clock::now() - s_time).count() << '\t' << sz << '\n';
        }
    }
    // cerr << "Removed " << count << " nodes...\n";
}

void enhanced_greedy() {
    int ncount=0;

    vector<int> uncov_count(n);
    for (int i = 0; i < n; ++i){
        if (!dominated[i]) uncov_count[i]++;
        for (int v : adj[i])
            if (!dominated[v]) uncov_count[i]++;
    }

    priority_queue<pair<int, int>> pq;
    for (int i = 0; i < n; ++i)
        if (!dominated[i])
            pq.emplace(uncov_count[i], i);

    while (!pq.empty()) {
        if (time_exceeded2()) return;
        auto [score, u] = pq.top(); pq.pop();

        int count = 0;
        for (int v : adj[u])
            if (!dominated[v]) count++;
        if (!dominated[u]) count++;

        if (score != count) {
            pq.emplace(count, u);
            continue;
        }

        if (count >= 2 || !dominated[u]) {
            ncount++;
            in_dset[u] = true;
            dom_set.push_back(u);
            dominated[u] = true;
            for (int v : adj[u]) dominated[v] = true;
        }
    }
    cerr << "Removed " << ncount << " nodes...\n";
}

void remove_redundant_covercount() {
    vector<int> cover_count(n, 0);
    for (int u : dom_set) {
        cover_count[u]++;
        for (int v : adj[u]) cover_count[v]++;
    }

    vector<int> new_set;
    for (int u : dom_set) {
        if (time_exceeded2()) {return;}
        if (cover_count[u] <= 1) {
            new_set.push_back(u);
            continue;
        }

        bool safe_to_remove = true;
        for (int v : adj[u]) {
            if (cover_count[v] <= 1) {
                safe_to_remove = false;
                break;
            }
        }

        if (safe_to_remove) {
            for (int v : adj[u]) cover_count[v]--;
            cover_count[u]--;
            in_dset[u] = false;
        } else {
            new_set.push_back(u);
        }
    }
    dom_set = new_set;
}

void advanced_redundancy_removal() {
    vector<int> coverage_count(n, 0);
    
    // Count how many times each vertex is covered
    for (int u : dom_set) {
        coverage_count[u]++;
        for (int v : adj[u]) coverage_count[v]++;
    }
    
    // Sort vertices by their "importance" (how many barely-covered vertices they cover)
    vector<pair<double, int>> importance;
    for (int u : dom_set) {
        double score = 0;
        int critical_covers = 0;
        
        for (int v : adj[u]) {
            if (coverage_count[v] == 1) critical_covers++;
            else if (coverage_count[v] == 2) score += 0.5;
            else score += 1.0 / coverage_count[v];
        }
        if (coverage_count[u] == 1) critical_covers++;
        
        if (critical_covers == 0) {
            importance.push_back({score, u});
        }
    }
    
    sort(importance.begin(), importance.end());
    
    // Try removing vertices with lowest importance
    for (auto [score, u] : importance) {
        if (time_exceeded2()) return;
        
        bool can_remove = true;
        for (int v : adj[u]) {
            if (coverage_count[v] <= 1) {
                can_remove = false;
                break;
            }
        }
        if (coverage_count[u] <= 1) can_remove = false;
        
        if (can_remove) {
            auto it = find(dom_set.begin(), dom_set.end(), u);
            if (it != dom_set.end()) {
                dom_set.erase(it);
                coverage_count[u]--;
                for (int v : adj[u]) coverage_count[v]--;
                in_dset[u] = false;
            }
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    start_time = high_resolution_clock::now();

    string line;
    bool header_parsed = false;

    while (getline(cin, line)) {
        if (line.empty() || line[0] == 'c') continue;
        if (line[0] == 'p') {
            string p, ds; istringstream iss(line);
            iss >> p >> ds >> n >> m;
            adj.assign(n, {});
            dominated.assign(n, false);
            in_dset.assign(n, false);
            degree.assign(n, 0);
            header_parsed = true;
            break;
        }
    }

    if (!header_parsed) {
        return 1;
    }

    while (getline(cin, line)) {
        if (line.empty() || line[0] == 'c') continue;
        int u, v;
        istringstream iss(line);
        if (!(iss >> u >> v)) continue;
        --u; --v;
        if (u < 0 || u >= n || v < 0 || v >= n) {
            continue;
        }
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    for (int i = 0; i < n; ++i){
        degree[i] = adj[i].size();
        // maxDegree = max(maxDegree, degree[i]);
    }

    // cerr << "Max degree: " << maxDegree << '\n';

    cerr << "Running isolated_node_prune...\n";
    isolated_node_prune();

    cerr << "Running degree1_prune...\n";
    degree1_prune();    
    
    cerr << "Running brute_force_small_components...\n";
    brute_force_small_components();
    
    // cerr << "Running triangle_prune...\n";
    // triangle_prune();

    // cerr << "Running crown_reduction...\n";
    // crown_reduction();

    cerr << "Running enhanced_greedy...\n";
    enhanced_greedy();
    
    if (!time_exceeded2()) {
        cerr << "Running advanced_redundancy_removal...\n";
        advanced_redundancy_removal();
    }

    // if (!time_exceeded2()) {
    //     cerr << "Running remove_redundant_covercount...\n";
    //     remove_redundant_covercount();
    // }
    
    // --- FINAL PRUNING ---
    priority_queue<pair<int, int>> pq;
    int count=0;
    for (int i = 0; i < n; ++i) {
        if (!dominated[i]) {
            count++;
            pq.emplace(degree[i], i);
        }
    }
    cerr << "Found " << count << " undominated nodes, running final greedy selection...\n";

    while (!pq.empty()) {
        auto [deg, u] = pq.top(); pq.pop();
        if (dominated[u]) continue;

        in_dset[u] = true;
        dom_set.push_back(u);
        dominated[u] = true;
        for (int v : adj[u])
            dominated[v] = true;
    }

    // cerr << "Finished algorithm, printing result...\n";

    auto end = high_resolution_clock::now();
    duration<double> elapsed = end - start_time;

    // --- OUTPUT (DIMACS-style) ---
    cout << "c Dominating Set Solution\n";
    cerr << "Dominating Set Size: " << dom_set.size() <<"\n";
    cout << dom_set.size() << '\n';
    for (int v : dom_set)
        cout << (v + 1) << '\n';

    cerr << fixed << setprecision(6);
    cerr << "c Time taken: " << elapsed.count() << " seconds\n";

    return 0;
}
