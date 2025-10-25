// grafo.h

#ifndef GRAFO_H
#define GRAFO_H

#include <vector>
#include <string>
#include <map> // Incluído para o retorno das estatísticas

// Usar o namespace std para simplificar
using namespace std;

// Tipo de representação do grafo
enum Representacao { LISTA, MATRIZ };

class Grafo {
public:
    // --- Parte 1: Construtor e Funções Básicas ---
    Grafo(int vertices, Representacao t);
    void adicionarAresta(int u, int v, float peso);
    static Grafo lerDeArquivo(const string& nomeArquivo, Representacao t);
    int getNumVertices() const { return V; }
    int getNumArestas() const { return E; }
    map<string, double> getEstatisticas() const;

    vector<pair<float, int>> DijkstraHeap(int u) const;
    vector<pair<float, int>> DijkstraVetor(int u) const;

    // --- Parte 5: Memória Usada ---
    size_t memoriaUsada() const;

private:

    int V; // número de vértices
    int E; // número de arestas
    Representacao tipo; // lista ou matriz

    // Estruturas de dados para representar o grafo
    vector<vector<pair<int, float>>> listaAdj;
    vector<vector<float>> matrizAdj;
    bool hasNegativeWeights = false;
};

#endif // GRAFO_H
