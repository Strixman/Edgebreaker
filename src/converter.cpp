#include "converter.h"

#include <algorithm>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>

#pragma region HELPERS

std::vector<std::vector<Indices>> splitIntoComponents(
    int vert_size,
    const std::vector<Indices>& tri
) {
    std::vector<std::unordered_set<int>> adj(vert_size);
    for (const auto& t : tri) {
        adj[t[0]].insert({t[1], t[2]});
        adj[t[1]].insert({t[0], t[2]});
        adj[t[2]].insert({t[1], t[0]});
    }
    
    std::vector<int> component(vert_size, -1);
    int comp_id = 0;
    for (int i = 0; i < vert_size; ++i) {
        if (component[i] == -1) {
            std::stack<int> st;
            st.push(i);
            component[i] = comp_id;
            while (!st.empty()) {
                int v = st.top();
                st.pop();
                for (int neighbor : adj[v]) {
                    if (component[neighbor] == -1) {
                        component[neighbor] = comp_id;
                        st.push(neighbor);
                    }
                }
            }
            comp_id++;
        }
    }

    std::vector<std::vector<Indices>> result(comp_id);
    for (const auto& t : tri) {
        result[component[t[0]]].push_back(t);
    }

    return result;
};

std::vector<Edge> findBoundaryEdges(
    const std::vector<Indices>& tri
) {
    std::set<Edge> edges;
    for(const auto& t : tri){
        for(int i = 0; i < 3; ++i){
            edges.insert({t[i], t[(i + 1) % 3]});
        }
    }

    std::vector<Edge> result;
    for(const auto& e : edges){
        if(edges.find({e[1], e[0]}) == edges.end()){
            result.push_back(e);
        }
    }

    return result;
};

void fill_holes(
    std::vector<Vertex>& vert, 
    std::vector<Indices>& tri, 
    const std::vector<Edge>& edges,
    std::vector<int>& dummy
) {
    std::unordered_map<int, std::vector<int>> adj;
    for (const auto& e : edges) {
        adj[e[0]].push_back(e[1]);
    }

    std::unordered_set<int> visited;
    int vertex_i = vert.size();

    for (const auto& entry : adj) {
        int start = entry.first;
        if (visited.find(start) != visited.end()) {
            continue;
        }

        int current = start;
        int prev = -1;
        std::vector<int> loop;

        while (true) {
            loop.push_back(current);
            visited.insert(current);

            std::vector<int> neighbors;
            for (int n : adj[current]) {
                if (n != prev) {
                    neighbors.push_back(n);
                }
            }

            if (neighbors.empty()) {
                break;
            }

            int next_node = neighbors[0];
            if (next_node == loop[0] && loop.size() > 1) {
                break;
            }

            prev = current;
            current = next_node;
        }

        if (loop.size() < 3) {
            continue;
        }

        reverse(loop.begin(), loop.end());

        std::array<float, 3> centroid = {0.0f, 0.0f, 0.0f};
        for (int v : loop) {
            centroid[0] += vert[v][0];
            centroid[1] += vert[v][1];
            centroid[2] += vert[v][2];
        }
        float inv_size = 1.0f / loop.size();
        centroid[0] *= inv_size;
        centroid[1] *= inv_size;
        centroid[2] *= inv_size;

        vert.push_back(centroid);
        dummy.push_back(vertex_i);

        for (size_t i = 0; i < loop.size(); ++i) {
            tri.push_back({vertex_i, loop[i], loop[(i + 1) % loop.size()]});
        }

        vertex_i++;
    }
};

#pragma endregion

std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>> Converter::toOVX(
    std::vector<Vertex>& vert, 
    std::vector<Indices>& tri
) {
    std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>> result;

    auto components = splitIntoComponents(vert.size(), tri);
    for(auto& c_tri : components){
        auto& [V, O, dummy] = result.emplace_back();
        
        auto edges = findBoundaryEdges(c_tri);
        fill_holes(vert, c_tri, edges, dummy);
        
        int tri_size = c_tri.size();
        for(const auto& t : c_tri){
            V.insert(V.end(), t.begin(), t.end());
        }
        O.insert(O.begin(), 3 * tri_size, -1);

        std::map<Edge, int> edge_dict;
        for (int t_idx = 0; t_idx < tri_size; ++t_idx) {
            const auto& tri = c_tri[t_idx];
            for (int i = 0; i < 3; ++i) {
                int c = 3 * t_idx + i;
                auto mm = std::minmax(tri[(i + 1) % 3], tri[(i - 1 + 3) % 3]);
                Edge edge({mm.first, mm.second});
    
                auto it = edge_dict.find(edge);
                if (it != edge_dict.end()) {
                    int c2 = it->second;
                    O[c] = c2;
                    O[c2] = c;
                } else {
                    edge_dict[edge] = c;
                }
            }
        }
    }
    
    return result;
}

std::pair<std::vector<Vertex>, std::vector<Indices>> Converter::fromOVX(
    const std::vector<int>& O, 
    const std::vector<int>& V, 
    const std::vector<int>& dummy
) {
    return std::pair<std::vector<Vertex>, std::vector<Indices>>();
}
