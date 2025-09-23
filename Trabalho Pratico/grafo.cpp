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
#include <iomanip> // para setw na formatação

using namespace std;

// --- Parte 1: Construtor e Funções Básicas ---

Grafo::Grafo(int vertices, Representacao t) : V(vertices), E(0), tipo(t) {
    if (tipo == LISTA) {
        listaAdj.resize(V + 1);
    } else {
        matrizAdj.assign(V + 1, vector<bool>(V + 1, false));
    }
}

void Grafo::adicionarAresta(int u, int v) {
    if (u < 1 || v < 1 || u > V || v > V) return;

    if (tipo == LISTA) {
    if (find(listaAdj[u].begin(), listaAdj[u].end(), v) == listaAdj[u].end()) {
            listaAdj[u].push_back(v);
            listaAdj[v].push_back(u);
            E++;
        }
        
    } else {
        if (matrizAdj[u][v] == false) {
            matrizAdj[u][v] = true;
            matrizAdj[v][u] = true;
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
    while (arq >> u >> v) {
        g.adicionarAresta(u, v);
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
            graus_vec[i] = accumulate(matrizAdj[i + 1].begin() + 1, matrizAdj[i + 1].end(), 0);
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

// --- Parte 2: Buscas que retornam dados ---

vector<int> Grafo::BFS_com_retorno(int u) const {
    // Reutiliza o método interno e retorna apenas o vetor de pais
    return BFS_interno(u).pais;
}

vector<int> Grafo::DFS_com_retorno(int u) const {
    vector<bool> visitado(V + 1, false);
    vector<int> pai(V + 1, 0);
    stack<int> pilha;

    // Função para processar um componente a partir de um vértice inicial
    auto dfs_iterativa_componente = [&](int vertice_inicial) {
        pilha.push(vertice_inicial);
        pai[vertice_inicial] = 0; // Raiz da árvore de busca

        while (!pilha.empty()) {
            int v_atual = pilha.top();
            pilha.pop();

            if (visitado[v_atual]) {
                continue;
            }
            visitado[v_atual] = true;

            // Para a lista, precisamos inverter a ordem para simular a recursão
            // (A recursão explora o primeiro vizinho, a pilha explora o último adicionado)
            if (tipo == LISTA) {
                const auto& vizinhos = listaAdj[v_atual];
                // Itera de trás para frente para que o primeiro vizinho seja processado primeiro
                for (auto it = vizinhos.rbegin(); it != vizinhos.rend(); ++it) {
                    int vizinho = *it;
                    if (!visitado[vizinho]) {
                        pai[vizinho] = v_atual;
                        pilha.push(vizinho);
                    }
                }
            } else { // MATRIZ
                // Itera de trás para frente para manter a consistência
                for (int vizinho = V; vizinho >= 1; --vizinho) {
                    if (matrizAdj[v_atual][vizinho] && !visitado[vizinho]) {
                        pai[vizinho] = v_atual;
                        pilha.push(vizinho);
                    }
                }
            }
        }
    };
    
    // Inicia a busca a partir do vértice 'u'
    if (u >= 1 && u <= V) {
        dfs_iterativa_componente(u);
    }
    
    // Garante que todos os vértices de outras componentes sejam visitados
    for (int i = 1; i <= V; ++i) {
        //if (!visitado[i]) { dfs_iterativa_componente(i);}
    }
    
    return pai;
}

// --- Parte 3: Distâncias e Diâmetro ---

// Método privado auxiliar, não modificado
Grafo::ResultadoBFS Grafo::BFS_interno(int u) const {
    if (u < 1 || u > V) throw out_of_range("Vértice inicial da BFS fora do intervalo.");
    
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
        } else { // MATRIZ
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

int Grafo::distancia(int u, int v) const {
    if (u < 1 || u > V || v < 1 || v > V) return -1;
    return BFS_interno(u).niveis[v];
}

int Grafo::diametro() const {
    int max_dist = 0;
    for (int i = 1; i <= V; ++i) {
        ResultadoBFS res = this->BFS_interno(i);
        for (int j = 1; j <= V; ++j) {
            if (res.niveis[j] == -1) return -1; // Grafo desconexo, diâmetro infinito
            if (res.niveis[j] > max_dist) {
                max_dist = res.niveis[j];
            }
        }
    }
    return max_dist;
}

int Grafo::diametroAproximado() const {
    if (V == 0) return 0;

    // 1. Escolha um vértice aleatório 'u'
    // O +1 garante que o resultado seja entre 1 e V
    int u = 1 + (rand() % V);

    // 2. Faça uma BFS a partir de 'u' para encontrar o vértice 'a' mais distante
    ResultadoBFS res1 = BFS_interno(u);
    int max_nivel1 = -1;
    int a = u;
    for (int i = 1; i <= V; ++i) {
        if (res1.niveis[i] == -1) return -1; // Grafo desconexo
        if (res1.niveis[i] > max_nivel1) {
            max_nivel1 = res1.niveis[i];
            a = i;
        }
    }

    // 3. Faça uma BFS a partir de 'a' para encontrar a maior distância
    ResultadoBFS res2 = BFS_interno(a);
    int max_nivel2 = -1;
    for (int i = 1; i <= V; ++i) {
        if (res2.niveis[i] == -1) return -1; // Grafo desconexo
        if (res2.niveis[i] > max_nivel2) {
            max_nivel2 = res2.niveis[i];
        }
    }
    
    // 4. A maior distância da segunda BFS é a nossa aproximação
    return max_nivel2;
}

// --- Parte 4: Componentes Conexas ---

vector<vector<int>> Grafo::getComponentesConexas() const {
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
                } else { // MATRIZ
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

    // Opcional: ordenar componentes por tamanho (maior primeiro)
    sort(componentes.begin(), componentes.end(), [](const vector<int>& a, const vector<int>& b) {
        return a.size() > b.size();
    });

    return componentes;
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

// --- Parte 6: Profundidade e Largura ---

void Grafo::salvarBuscasConsolidadas(int verticeInicial, const string& nomeArquivo) const {
    ofstream out(nomeArquivo);
    if (!out.is_open()) {
        throw runtime_error("Erro ao abrir arquivo de saída: " + nomeArquivo);
    }

    out << "=== RESULTADOS DAS BUSCAS ===\n";
    out << "Vertices: " << V << " | Arestas: " << E << "\n";
    out << "Vertice inicial: " << verticeInicial << "\n\n";

    // --- BFS ---
    ResultadoBFS bfsRes = BFS_interno(verticeInicial);
    out << "--- BFS (Busca em Largura) ---\n";
    out << setw(10) << "Vertice" << setw(10) << "Pai" << setw(10) << "Nivel\n";
    for (int i = 1; i <= V; i++) {
        out << setw(10) << i 
            << setw(10) << bfsRes.pais[i] 
            << setw(10) << bfsRes.niveis[i] << "\n";
    }
    out << "\n";

    // --- DFS ---
    vector<int> dfsPais = DFS_com_retorno(verticeInicial);
    out << "--- DFS (Busca em Profundidade) ---\n";
    out << setw(10) << "Vertice" << setw(10) << "Pai\n";
    for (int i = 1; i <= V; i++) {
        out << setw(10) << i << setw(10) << dfsPais[i] << "\n";
    }

    out.close();
}
