#include "grafo.h"
#include <fstream>
#include <stdexcept>
#include <numeric>
#include <algorithm>
#include <queue>
#include <sstream>
#include <functional>
#include <iostream>

using namespace std;

// --- Parte 1: Construtor e Funções Básicas ---

Grafo::Grafo(int vertices, Representacao t) : V(vertices), E(0), tipo(t) {
    if (tipo == LISTA) {
        listaAdj.resize(V + 1);
    } else {
        matrizAdj.assign(V + 1, vector<int>(V + 1, 0));
    }
}

void Grafo::adicionarAresta(int u, int v) {
    if (u < 1 || v < 1 || u > V || v > V) return;

    if (tipo == LISTA) {
        // Evita adicionar arestas duplicadas
        if (find(listaAdj[u].begin(), listaAdj[u].end(), v) == listaAdj[u].end()) {
            listaAdj[u].push_back(v);
            listaAdj[v].push_back(u);
            E++;
        }
    } else {
        if (matrizAdj[u][v] == 0) {
            matrizAdj[u][v] = 1;
            matrizAdj[v][u] = 1;
            E++;
        }
    }
}

Grafo Grafo::lerDeArquivo(const string& nomeArquivo, Representacao t) {
    ifstream arq(nomeArquivo);
    if (!arq.is_open()) throw runtime_error("Erro ao abrir arquivo de entrada");

    int nVertices;
    arq >> nVertices;
    Grafo g(nVertices, t);

    int u, v;
    while (arq >> u >> v) {
        g.adicionarAresta(u, v);
    }
    return g;
}

vector<int> Grafo::graus() const {
    vector<int> g(V + 1, 0);
    if (tipo == LISTA) {
        for (int i = 1; i <= V; i++) {
            g[i] = listaAdj[i].size();
        }
    } else {
        for (int i = 1; i <= V; i++) {
            g[i] = accumulate(matrizAdj[i].begin(), matrizAdj[i].end(), 0);
        }
    }
    return g;
}

// --- Parte 2: Buscas em Grafos ---

void Grafo::Largura(int u) {
    ofstream out("Largura.txt");
    if (!out.is_open()) throw runtime_error("Erro ao abrir arquivo de saída Largura.txt");

    ResultadoBFS res = BFS_interno(u);

    out << "Vertice, Pai, Nivel\n";
    for (int i = 1; i <= V; ++i) {
        if (res.niveis[i] != -1) {
            out << i << ", " << (res.pais[i] == 0 ? "nil" : to_string(res.pais[i])) << ", " << res.niveis[i] << "\n";
        }
    }
}

void Grafo::Profundidade(int u) {
    ofstream out("Profundidade.txt");
    if (!out.is_open()) throw runtime_error("Erro ao abrir arquivo de saída Profundidade.txt");

    vector<bool> visitado(V + 1, false);
    vector<int> pai(V + 1, 0);
    vector<int> nivel(V + 1, -1);

    function<void(int, int)> dfs_visit = 
        [&](int v_atual, int nivel_atual) {
            visitado[v_atual] = true;
            nivel[v_atual] = nivel_atual;
            if (tipo == LISTA) {
                for (int vizinho : listaAdj[v_atual]) {
                    if (!visitado[vizinho]) {
                        pai[vizinho] = v_atual;
                        dfs_visit(vizinho, nivel_atual + 1);
                    }
                }
            } else {
                for (int vizinho = 1; vizinho <= V; ++vizinho) {
                    if (matrizAdj[v_atual][vizinho] == 1 && !visitado[vizinho]) {
                        pai[vizinho] = v_atual;
                        dfs_visit(vizinho, nivel_atual + 1);
                    }
                }
            }
        };

    if (u >= 1 && u <= V && !visitado[u]) {
        pai[u] = 0;
        dfs_visit(u, 0);
    }
    for (int i = 1; i <= V; ++i) {
        if (!visitado[i]) {
            pai[i] = 0;
            dfs_visit(i, 0);
        }
    }

    out << "Vertice, Pai, Nivel\n";
    for (int i = 1; i <= V; ++i) {
        if (nivel[i] != -1) {
            out << i << ", " << (pai[i] == 0 ? "nil" : to_string(pai[i])) << ", " << nivel[i] << "\n";
        }
    }
}


// --- Parte 3: Distâncias e Diâmetro ---

Grafo::ResultadoBFS Grafo::BFS_interno(int u) const {
    ResultadoBFS res;
    res.pais.assign(V + 1, 0);
    res.niveis.assign(V + 1, -1);
    queue<int> fila;

    res.niveis[u] = 0;
    fila.push(u);

    while (!fila.empty()) {
        int v_atual = fila.front();
        fila.pop();
        if (tipo == LISTA) {
            for (int vizinho : listaAdj[v_atual]) {
                if (res.niveis[vizinho] == -1) {
                    res.niveis[vizinho] = res.niveis[v_atual] + 1;
                    res.pais[vizinho] = v_atual;
                    fila.push(vizinho);
                }
            }
        } else {
            for (int vizinho = 1; vizinho <= V; ++vizinho) {
                if (matrizAdj[v_atual][vizinho] == 1 && res.niveis[vizinho] == -1) {
                    res.niveis[vizinho] = res.niveis[v_atual] + 1;
                    res.pais[vizinho] = v_atual;
                    fila.push(vizinho);
                }
            }
        }
    }
    return res;
}

int Grafo::distancia(int u, int v) {
    ResultadoBFS res = this->BFS_interno(u);
    return res.niveis[v]; // se for -1, não há caminho
}


int Grafo::diametro() {
    int max_dist = 0;
    for (int i = 1; i <= V; ++i) {
        ResultadoBFS res = this->BFS_interno(i);
        for (int j = 1; j <= V; ++j) {
            if (res.niveis[j] == -1) return -1; // Grafo desconexo
            if (res.niveis[j] > max_dist) {
                max_dist = res.niveis[j];
            }
        }
    }
    return max_dist;
}

// --- Parte 4: Componentes Conexas ---

void Grafo::componentesConexas(const string& nomeArquivo) {
    ofstream out(nomeArquivo);
    if (!out.is_open()) throw runtime_error("Erro ao abrir o arquivo de saida: " + nomeArquivo);

    vector<bool> visitado(V + 1, false);
    vector<vector<int>> componentes;

    for (int i = 1; i <= V; ++i) {
        if (!visitado[i]) {
            vector<int> componente_atual;
            queue<int> fila;
            fila.push(i);
            visitado[i] = true;
            while (!fila.empty()) {
                int u_comp = fila.front();
                fila.pop();
                componente_atual.push_back(u_comp);
                if (tipo == LISTA) {
                    for (int vizinho : listaAdj[u_comp]) {
                        if (!visitado[vizinho]) {
                            visitado[vizinho] = true;
                            fila.push(vizinho);
                        }
                    }
                } else {
                    for (int vizinho = 1; vizinho <= V; ++vizinho) {
                        if (matrizAdj[u_comp][vizinho] == 1 && !visitado[vizinho]) {
                            visitado[vizinho] = true;
                            fila.push(vizinho);
                        }
                    }
                }
            }
            componentes.push_back(componente_atual);
        }
    }

    sort(componentes.begin(), componentes.end(), [](const vector<int>& a, const vector<int>& b) {
        return a.size() > b.size();
    });

    out << "Numero de componentes conexas: " << componentes.size() << "\n\n";
    int id_componente = 1;
    for (const auto& comp : componentes) {
        out << "Componente " << id_componente++ << ":\n";
        out << "  Tamanho: " << comp.size() << " vertices\n";
        out << "  Vertices: ";
        for (size_t j = 0; j < comp.size(); ++j) {
            out << comp[j] << (j == comp.size() - 1 ? "" : ", ");
        }
        out << "\n\n";
    }
}

// --- Função Auxiliar `salvarInfos` ---

void salvarInfos(const Grafo& g, const string& nomeSaida) {
    ofstream out(nomeSaida);
    if (!out.is_open()) throw runtime_error("Erro ao abrir arquivo de saída");

    auto graus_vec = g.graus();
    // Remove o índice 0 que não é usado
    if (!graus_vec.empty()) {
        graus_vec.erase(graus_vec.begin());
    }
    if (graus_vec.empty()) { // Grafo sem vértices
        out << "Grafo vazio.\n";
        return;
    }

    int minG = *min_element(graus_vec.begin(), graus_vec.end());
    int maxG = *max_element(graus_vec.begin(), graus_vec.end());
    double media = accumulate(graus_vec.begin(), graus_vec.end(), 0.0) / graus_vec.size();
    
    sort(graus_vec.begin(), graus_vec.end());
    double mediana;
    int n = graus_vec.size();
    if (n % 2 == 0) {
        mediana = (graus_vec[n/2 - 1] + graus_vec[n/2]) / 2.0;
    } else {
        mediana = graus_vec[n/2];
    }

    out << "Numero de vertices: " << g.getNumVertices() << "\n";
    out << "Numero de arestas: " << g.getNumArestas() << "\n";
    out << "Grau minimo: " << minG << "\n";
    out << "Grau maximo: " << maxG << "\n";
    out << "Grau medio: " << media << "\n";
    out << "Mediana do grau: " << mediana << "\n";
}

//Calcular Memoria Usada
size_t Grafo::memoriaUsada() const {
    if (tipo == LISTA) {
        size_t memoria = sizeof(vector<int>) * (V + 1);
        for (int i = 1; i <= V; i++) {
            memoria += sizeof(int) * listaAdj[i].size();
        }
        return memoria;
    } else {
        return sizeof(int) * V * V;
    }
}
