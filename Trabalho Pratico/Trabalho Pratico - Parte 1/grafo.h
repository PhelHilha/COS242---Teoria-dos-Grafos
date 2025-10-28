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
    void adicionarAresta(int u, int v);
    static Grafo lerDeArquivo(const string& nomeArquivo, Representacao t);
    int getNumVertices() const { return V; }
    int getNumArestas() const { return E; }
    map<string, double> getEstatisticas() const;

    // --- Parte 2: Buscas que retornam dados ---
    vector<int> BFS_com_retorno(int u) const;
    vector<int> DFS_com_retorno(int u) const;

    // --- Parte 3: Distâncias e Diâmetro ---
    int distancia(int u, int v) const;
    int diametro() const;
    int diametroAproximado() const;

    // --- Parte 4: Componentes Conexas que retornam dados ---
    vector<vector<int>> getComponentesConexas() const;

    // --- Parte 5: Memória Usada ---
    size_t memoriaUsada() const;
        // --- Parte extra: salvar BFS e DFS em arquivo único ---
    void salvarBuscasConsolidadas(int verticeInicial, const string& nomeArquivo) const;

private:
    // Estrutura interna para resultados da BFS
    struct ResultadoBFS {
        vector<int> pais;
        vector<int> niveis;
    };

    int V; // número de vértices
    int E; // número de arestas
    Representacao tipo; // lista ou matriz

    // Estruturas de dados para representar o grafo
    vector<vector<int>> listaAdj;
    vector<vector<bool>> matrizAdj;

    // Método auxiliar privado para BFS, otimizando o reuso de código
    ResultadoBFS BFS_interno(int u) const;
};

#endif // GRAFO_H
