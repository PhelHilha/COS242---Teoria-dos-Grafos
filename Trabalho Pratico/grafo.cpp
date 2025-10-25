#include "grafo.h"
#include <fstream>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <queue>
#include <functional> // Para std::function na DFS
#include <cstdlib> // Para rand()
#include <ctime>   // Para time()
#include <stack>
#include <limits>   // Para numeric_limits

using namespace std;

// --- Parte 1: Construtor e Funções Básicas ---

Grafo::Grafo(int vertices, Representacao t) : V(vertices), E(0), tipo(t) {
    if (tipo == LISTA) {
        listaAdj.resize(V + 1);
    } else {
        matrizAdj.assign(V + 1, vector<float>(V + 1, -1));
    }
}

void Grafo::adicionarAresta(int u, int v, float peso) {
    if (u < 1 || v < 1 || u > V || v > V) return;
    if (peso < 0) hasNegativeWeights = true;

    if (tipo == LISTA) {
        if (find(listaAdj[u].begin(), listaAdj[u].end(), make_pair(v, peso)) == listaAdj[u].end()) {
            listaAdj[u].push_back(make_pair(v, peso));
            E++;
        }
        
    } else {
        if (matrizAdj[u][v] == -1) {
            matrizAdj[u][v] = peso;
            E++;
        }
    }
}

Grafo Grafo::lerDeArquivo(const string& nomeArquivo, Representacao t) {
    ifstream arq(nomeArquivo);
    if (!arq.is_open()) {
        throw runtime_error("Erro ao abrir arquivo de entrada: " + nomeArquivo);
    }

    int nVertices;
    arq >> nVertices;
    Grafo g(nVertices, t);

    int u, v;
    float w;
    while (arq >> u >> v >> w) {
        g.adicionarAresta(u, v, w);
    }
    return g;
}

// Nova função getEstatisticas (substitui a antiga salvarInfos)
map<string, double> Grafo::getEstatisticas() const {
    map<string, double> stats;
    stats["vertices"] = V;
    stats["arestas"] = E;

    vector<int> graus_vec(V);
    if (tipo == LISTA) {
        for (int i = 0; i < V; ++i) graus_vec[i] = listaAdj[i + 1].size();
    } else {
        for (int i = 0; i < V; ++i) {
            graus_vec[i] = std::accumulate(matrizAdj[i + 1].begin() + 1, matrizAdj[i + 1].end(), 0,
                               [](int acc, float w) { return acc + (w != -1.f); });
        }
    }

    if (graus_vec.empty()) {
        stats["grauMinimo"] = 0;
        stats["grauMaximo"] = 0;
        stats["grauMedio"] = 0;
        stats["grauMediana"] = 0;
        return stats;
    }

    stats["grauMinimo"] = *min_element(graus_vec.begin(), graus_vec.end());
    stats["grauMaximo"] = *max_element(graus_vec.begin(), graus_vec.end());
    stats["grauMedio"] = accumulate(graus_vec.begin(), graus_vec.end(), 0.0) / V;
    
    sort(graus_vec.begin(), graus_vec.end());
    if (V % 2 == 0) {
        stats["grauMediana"] = (graus_vec[V/2 - 1] + graus_vec[V/2]) / 2.0;
    } else {
        stats["grauMediana"] = graus_vec[V/2];
    }
    
    return stats;
}


vector<pair<float, int>> Grafo::DijkstraHeap(int u) const {
    if (hasNegativeWeights) {
        throw runtime_error("O grafo contém arestas com pesos negativos. Dijkstra não pode ser aplicado.");
    }

    vector<pair<float, int>> CustoPai(V + 1, make_pair(numeric_limits<float>::infinity(), -1));

    priority_queue<pair<float, int>, vector<pair<float, int>>, greater<pair<float, int>>> pq;

    CustoPai[u].first = 0.f;
    CustoPai[u].second = 0; // Raiz

    pq.push(make_pair(0.f, u));

    while (!pq.empty()) {
        float d_atual = pq.top().first;
        int vertice_atual = pq.top().second;
        pq.pop();
        if (d_atual > CustoPai[vertice_atual].first) continue;
        if (tipo == LISTA) {
            for (const auto& [vizinho, peso] : listaAdj[vertice_atual]) {
                if (CustoPai[vertice_atual].first + peso < CustoPai[vizinho].first) {
                    CustoPai[vizinho].first = CustoPai[vertice_atual].first + peso;
                    CustoPai[vizinho].second = vertice_atual;
                    pq.push(make_pair(CustoPai[vizinho].first, vizinho));
                }
            }
        } else { // MATRIZ
            for (int vizinho = 1; vizinho <= V; ++vizinho) {
                if (matrizAdj[vertice_atual][vizinho] != -1) {
                    float peso = matrizAdj[vertice_atual][vizinho];
                    if (CustoPai[vertice_atual].first + peso < CustoPai[vizinho].first) {
                        CustoPai[vizinho].first = CustoPai[vertice_atual].first + peso;
                        CustoPai[vizinho].second = vertice_atual;
                        pq.push(make_pair(CustoPai[vizinho].first, vizinho));
                    }
                }
            }
        }
    }

    return CustoPai;
}

// --- Parte 5: Memória Usada ---

size_t Grafo::memoriaUsada() const {
    size_t memoria = 0;
    if (tipo == LISTA) {
        memoria += sizeof(listaAdj); // Overhead do vetor principal
        for (int i = 1; i <= V; i++) {
            // Memória = (tamanho do vetor) * (tamanho do tipo) + overhead do vetor
            memoria += listaAdj[i].capacity() * sizeof(int) + sizeof(vector<int>);
        }
    } else { // MATRIZ
        // O tamanho é (V+1) x (V+1)
        memoria += sizeof(matrizAdj); // Overhead do vetor principal
        for (int i = 0; i <= V; ++i) {
             memoria += matrizAdj[i].capacity() * sizeof(int) + sizeof(vector<int>);
        }
    }
    return memoria;
}
