#ifndef GRAFO_H
#define GRAFO_H

#include <vector>
#include <string>

// Usar o namespace std para simplificar
using namespace std;

// Tipo de representação do grafo
enum Representacao { LISTA, MATRIZ };

class Grafo {
public:
    // Estrutura para encapsular os resultados da BFS para uso interno
    struct ResultadoBFS {
        vector<int> pais;
        vector<int> niveis;
    };

    // --- Parte 1: Construtor e Funções Básicas ---
    Grafo(int vertices, Representacao t);
    void adicionarAresta(int u, int v);
    static Grafo lerDeArquivo(const string& nomeArquivo, Representacao t);
    int getNumVertices() const { return V; }
    int getNumArestas() const { return E; }
    vector<int> graus() const;

    // --- Parte 2: Buscas em Grafos ---
    void Largura(int u);
    void Profundidade(int u);

    // --- Parte 3: Distâncias e Diâmetro ---
    int distancia(int u, int v);
    int diametro();

    // --- Parte 4: Componentes Conexas ---
    void componentesConexas(const string& nomeArquivo);

    // --- Parte 5: Memoria Usada ---

    size_t memoriaUsada() const;


private:
    int V; // número de vértices
    int E; // número de arestas
    Representacao tipo; // lista ou matriz

    // Estruturas de dados para representar o grafo
    vector<vector<int>> listaAdj;
    vector<vector<int>> matrizAdj;

    // Método auxiliar privado para BFS, otimizando o reuso de código
    ResultadoBFS BFS_interno(int u) const;
};

// Função auxiliar para salvar informações básicas do grafo (grau min, max, etc.)
void salvarInfos(const Grafo& g, const string& nomeSaida);

#endif // GRAFO_H
