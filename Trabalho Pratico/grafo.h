#ifndef GRAFO_H
#define GRAFO_H

#include <vector>
#include <string>

using namespace std;

// 🔹 Tipo de representação
enum Representacao { LISTA, MATRIZ };

class Grafo {
private:
    int V; // número de vértices
    int E; // número de arestas
    Representacao tipo; // lista ou matriz

    // Representações possíveis
    std::vector<std::vector<int>> listaAdj;     
    std::vector<std::vector<int>> matrizAdj;    

public:
    Grafo(int vertices, Representacao t);

    void adicionarAresta(int u, int v);
    static Grafo lerDeArquivo(const std::string& nomeArquivo, Representacao t);

    int getNumVertices() const { return V; }
    int getNumArestas() const { return E; }

    std::vector<int> graus() const;
};

// Função auxiliar (parte 2)
void salvarInfos(const Grafo& g, const std::string& nomeSaida);

#endif
