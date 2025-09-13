#include "grafo.h"
#include <fstream>
#include <stdexcept>
#include <numeric>
#include <algorithm>

using namespace std;

Grafo::Grafo(int vertices, Representacao t) : V(vertices), E(0), tipo(t) {
    if (tipo == LISTA) {
        listaAdj.resize(V + 1);
    } else {
        matrizAdj.assign(V + 1, std::vector<int>(V + 1, 0));
    }
}

void Grafo::adicionarAresta(int u, int v) {
    if (u < 1 || v < 1 || u > V || v > V) return;

    if (tipo == LISTA) {
        listaAdj[u].push_back(v);
        listaAdj[v].push_back(u);
    } else {
        matrizAdj[u][v] = 1;
        matrizAdj[v][u] = 1;
    }

    E++;
}

Grafo Grafo::lerDeArquivo(const std::string& nomeArquivo, Representacao t) {
    std::ifstream arq(nomeArquivo);
    if (!arq.is_open()) throw std::runtime_error("Erro ao abrir arquivo");

    int nVertices;
    arq >> nVertices;
    Grafo g(nVertices, t);

    int u, v;
    while (arq >> u >> v) {
        g.adicionarAresta(u, v);
    }
    return g;
}

std::vector<int> Grafo::graus() const {
    std::vector<int> g(V + 1);

    if (tipo == LISTA) {
        for (int i = 1; i <= V; i++) {
            g[i] = listaAdj[i].size();
        }
    } else {
        for (int i = 1; i <= V; i++) {
            int grau = 0;
            for (int j = 1; j <= V; j++) {
                grau += matrizAdj[i][j];
            }
            g[i] = grau;
        }
    }
    return g;
}

// Função auxiliar (já pronta)
void salvarInfos(const Grafo& g, const std::string& nomeSaida) {
    std::ofstream out(nomeSaida);
    if (!out.is_open()) throw std::runtime_error("Erro ao abrir arquivo de saída");

    auto graus = g.graus();
    graus.erase(graus.begin()); // remove índice 0

    int minG = *std::min_element(graus.begin(), graus.end());
    int maxG = *std::max_element(graus.begin(), graus.end());
    double media = std::accumulate(graus.begin(), graus.end(), 0.0) / graus.size();

    std::sort(graus.begin(), graus.end());
    double mediana;
    int n = graus.size();
    if (n % 2 == 0) {
        mediana = (graus[n/2 - 1] + graus[n/2]) / 2.0;
    } else {
        mediana = graus[n/2];
    }

    out << "Numero de vertices: " << g.getNumVertices() << "\n";
    out << "Numero de arestas: " << g.getNumArestas() << "\n";
    out << "Grau minimo: " << minG << "\n";
    out << "Grau maximo: " << maxG << "\n";
    out << "Grau medio: " << media << "\n";
    out << "Mediana do grau: " << mediana << "\n";
}
