#include "converter.h"

#pragma region HELPERS

#include <unordered_set>
#include <stack>

std::vector<std::vector<Indices>> splitIntoComponents(
    const std::vector<Vertex>& vert,
    const std::vector<Indices>& tri
) {
    std::vector<std::unordered_set<int>> adj(vert.size());
    for (const auto& t : tri) {
        adj[t.a].insert({t.b, t.c});
        adj[t.b].insert({t.a, t.c});
        adj[t.c].insert({t.b, t.a});
    }
    
    std::vector<std::vector<Indices>> result;
    std::vector<bool> visited(vert.size(), false);
    for (int i = 0; i < vert.size(); ++i) {
        if (!visited[i]) {
            auto& tmp = result.emplace_back();
            std::stack<int> st;
            st.push(i);
            while (!st.empty()) {
                int v = st.top();
                st.pop();
                if (!visited[v]) {
                    visited[v] = true;
                    tmp.push_back(tri[v]);
                    for (int neighbor : adj[v]) {
                        if (!visited[neighbor]) {
                            st.push(neighbor);
                        }
                    }
                }
            }
        }
    }

    return result;
};

#pragma endregion

std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>> Converter::toOVX(
    std::vector<Vertex>& vert, 
    std::vector<Indices>& tri
) {
    std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>> result;

    auto components = splitIntoComponents(vert, tri);
    std::cout << components.size();
    for(const auto& comp : components){

    }

    //const int num_tri = tri.size();
    //std::vector<int> O(3 * num_tri, -1);
    //std::vector<int> V(3 + num_tri);
    //for(const auto& t : tri)
    //    V.insert(V.end(), {t.a, t.b, t.c});
    //std::vector<int> dummy;
    //result.push_back(std::make_tuple(O, V, dummy));

    return result;
}

std::pair<std::vector<Vertex>, std::vector<Indices>> Converter::fromOVX(
    const std::vector<int>& O, 
    const std::vector<int>& V, 
    const std::vector<int>& dummy
) {
    return std::pair<std::vector<Vertex>, std::vector<Indices>>();
}
