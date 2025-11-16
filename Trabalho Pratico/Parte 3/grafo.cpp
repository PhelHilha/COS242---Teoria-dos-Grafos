// grafo.cpp

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

Grafo::Grafo(int vertices, Representacao t, bool direcionado) : V(vertices), E(0), tipo(t), direcionado(direcionado) {
    this->direcionado = direcionado;
    if (tipo == LISTA) {
        listaAdj.resize(V + 1);
    } else {
        matrizAdj.assign(V + 1, vector<float>(V + 1, SEM_ARESTA));
    }
}

void Grafo::adicionarAresta(int u, int v, float peso) {
    if (u < 1 || v < 1 || u > V || v > V) return;
    if (peso < 0) hasNegativeWeights = true;

    if (tipo == LISTA) {
        // Precisamos checar se a aresta já existe antes de adicionar
        // O seu `find` original comparava o peso, o que não é o ideal.
        // Vamos checar apenas se o vizinho 'v' já está na lista de 'u'.
        

        //listaAdj[u].push_back(make_pair(v, peso));
        //    if (!direcionado) {
        //        listaAdj[v].push_back(make_pair(u, peso));
        //    }
        //E++;


        bool arestaExiste = false;
        for (const auto& par : listaAdj[u]) {
           if (par.first == v) {
                arestaExiste = true;
                break;
            }
        }   
        if (!arestaExiste) {
            listaAdj[u].push_back(make_pair(v, peso));
            if (!direcionado) {
                listaAdj[v].push_back(make_pair(u, peso));
            }
            E++;
        }
        
        
    } else { // MATRIZ
        if (matrizAdj[u][v] == SEM_ARESTA) {
            matrizAdj[u][v] = peso;
            if (!direcionado) {
                matrizAdj[v][u] = peso;
            }
            E++;
        }
    }
}

Grafo Grafo::lerDeArquivo(const string& nomeArquivo, Representacao t, bool direcionado, bool inverter) {
    ifstream arq(nomeArquivo);
    if (!arq.is_open()) {
        throw runtime_error("Erro ao abrir arquivo de entrada: " + nomeArquivo);
    }

    int nVertices;
    arq >> nVertices;
    Grafo g(nVertices, t, direcionado); 

    int u, v;
    float w;
    while (arq >> u >> v >> w) {
        if (inverter) {
            g.adicionarAresta(v, u, w); // Inverte a aresta
        } else {
            g.adicionarAresta(u, v, w); // Mantém a aresta original
        }
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
                               [this](int acc, float w) { return acc + (w != SEM_ARESTA); });
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
                if (matrizAdj[vertice_atual][vizinho] != SEM_ARESTA) {
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

vector<pair<float, int>> Grafo::DijkstraVetor(int u) const {
    if (hasNegativeWeights) {
        throw runtime_error("O grafo contém arestas com pesos negativos. Dijkstra não pode ser aplicado.");
    }

    vector<pair<float, int>> CustoPai(V + 1, make_pair(numeric_limits<float>::infinity(), -1));
    vector<bool> visitado(V + 1, false);

    CustoPai[u].first = 0.f;
    CustoPai[u].second = 0; // raiz

    for (int i = 1; i <= V; ++i) {
        // 1. Escolher o vértice não visitado com menor distância
        float menor = numeric_limits<float>::infinity();
        int vertice_atual = -1;
        for (int v = 1; v <= V; ++v) {
            if (!visitado[v] && CustoPai[v].first < menor) {
                menor = CustoPai[v].first;
                vertice_atual = v;
            }
        }

        if (vertice_atual == -1) break; // acabou
        visitado[vertice_atual] = true;

        // 2. Relaxar as arestas
        if (tipo == LISTA) {
            for (const auto& [vizinho, peso] : listaAdj[vertice_atual]) {
                if (CustoPai[vertice_atual].first + peso < CustoPai[vizinho].first) {
                    CustoPai[vizinho].first = CustoPai[vertice_atual].first + peso;
                    CustoPai[vizinho].second = vertice_atual;
                }
            }
        } else {
            for (int vizinho = 1; vizinho <= V; ++vizinho) {
                float peso = matrizAdj[vertice_atual][vizinho];
                if (peso != SEM_ARESTA && CustoPai[vertice_atual].first + peso < CustoPai[vizinho].first) {
                    CustoPai[vizinho].first = CustoPai[vertice_atual].first + peso;
                    CustoPai[vizinho].second = vertice_atual;
                }
            }
        }
    }

    return CustoPai;
}

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
                    int vizinho = it->first;
                    if (!visitado[vizinho]) {
                        pai[vizinho] = v_atual;
                        pilha.push(vizinho);
                    }
                }
            } else { // MATRIZ
                // Itera de trás para frente para manter a consistência
                for (int vizinho = V; vizinho >= 1; --vizinho) {
                    if (matrizAdj[v_atual][vizinho] != SEM_ARESTA && !visitado[vizinho]) {
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
            for (const auto& [vizinho, peso] : listaAdj[v_atual]) {
                if (res.niveis[vizinho] == -1) {
                    res.niveis[vizinho] = res.niveis[v_atual] + 1;
                    res.pais[vizinho] = v_atual;
                    fila.push(vizinho);
                }
            }
        } else { // MATRIZ
            for (int vizinho = 1; vizinho <= V; ++vizinho) {
                if (matrizAdj[v_atual][vizinho] != SEM_ARESTA && res.niveis[vizinho] == -1) {
                    res.niveis[vizinho] = res.niveis[v_atual] + 1;
                    res.pais[vizinho] = v_atual;
                    fila.push(vizinho);
                }
            }
        }
    }
    return res;
}


pair<bool, vector<pair<float, int>>> Grafo::BellmanFord(int u) const {
    // Inicialização
    float INF = numeric_limits<float>::infinity();
    vector<pair<float, int>> CustoPai(V + 1, make_pair(INF, -1));
    
    CustoPai[u].first = 0;
    CustoPai[u].second = 0; // Raiz

    // Relaxamento repetido V-1 vezes
    for (int i = 1; i <= V - 1; ++i) {
        bool trocou = false; // Otimização 1: Flag de parada antecipada

        for (int v_atual = 1; v_atual <= V; ++v_atual) {
            // Otimização 2: Só relaxa vizinhos se o vértice atual já foi alcançado
            if (CustoPai[v_atual].first == INF) continue;

            if (tipo == LISTA) {
                for (const auto& [vizinho, peso] : listaAdj[v_atual]) {
                    if (CustoPai[v_atual].first + peso < CustoPai[vizinho].first) {
                        CustoPai[vizinho].first = CustoPai[v_atual].first + peso;
                        CustoPai[vizinho].second = v_atual;
                        trocou = true;
                    }
                }
            } else { // MATRIZ
                for (int vizinho = 1; vizinho <= V; ++vizinho) {
                    if (matrizAdj[v_atual][vizinho] != SEM_ARESTA) {
                        float peso = matrizAdj[v_atual][vizinho];
                        if (CustoPai[v_atual].first + peso < CustoPai[vizinho].first) {
                            CustoPai[vizinho].first = CustoPai[v_atual].first + peso;
                            CustoPai[vizinho].second = v_atual;
                            trocou = true;
                        }
                    }
                }
            }
        }

        // Se nenhuma distância foi atualizada nesta passada, terminamos
        if (!trocou) break; 
    }

    // Verificação de Ciclo Negativo (N-ésima iteração)
    bool cicloNegativo = false;
    for (int v_atual = 1; v_atual <= V; ++v_atual) {
        if (CustoPai[v_atual].first == INF) continue;

        if (tipo == LISTA) {
            for (const auto& [vizinho, peso] : listaAdj[v_atual]) {
                if (CustoPai[v_atual].first + peso < CustoPai[vizinho].first) {
                    cicloNegativo = true;
                    break;
                }
            }
        } else {
            for (int vizinho = 1; vizinho <= V; ++vizinho) {
                if (matrizAdj[v_atual][vizinho] != SEM_ARESTA) {
                    float peso = matrizAdj[v_atual][vizinho];
                    if (CustoPai[v_atual].first + peso < CustoPai[vizinho].first) {
                        cicloNegativo = true;
                        break;
                    }
                }
            }
        }
        if (cicloNegativo) break;
    }

    // O primeiro elemento do par indica SUCESSO (true) se NÃO houver ciclo negativo
    return make_pair(!cicloNegativo, CustoPai);
}

// --- Parte 5: Memória Usada ---

size_t Grafo::memoriaUsada() const {
    size_t memoria = 0;
    if (tipo == LISTA) {
        memoria += sizeof(listaAdj); // Overhead do vetor principal
        for (int i = 1; i <= V; i++) {
            // CORREÇÃO: O tipo de dado agora é pair<int, float>
            memoria += listaAdj[i].capacity() * sizeof(pair<int, float>) + sizeof(vector<pair<int, float>>);
        }
    } else { // MATRIZ
        memoria += sizeof(matrizAdj); // Overhead do vetor principal
        for (int i = 0; i <= V; ++i) {
             // CORREÇÃO: O tipo de dado agora é float
             memoria += matrizAdj[i].capacity() * sizeof(float) + sizeof(vector<float>);
        }
    }
    return memoria;
}
