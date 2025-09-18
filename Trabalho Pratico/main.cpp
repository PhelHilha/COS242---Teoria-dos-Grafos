#include "grafo.h"
#include <iostream>
#include <limits>   // Para limpar o buffer de entrada (cin)
#include <chrono>   // Para medir tempo de execução

using namespace std::chrono;
using namespace std;

// Função auxiliar para limpar o buffer de entrada e evitar loops infinitos
void limparBufferEntrada() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main() {
    try {
        int escolha_repr;
        cout << "--- Biblioteca de Analise de Grafos ---\n\n";
        cout << "Escolha a representacao do grafo:\n";
        cout << "1 - Lista de Adjacencia\n";
        cout << "2 - Matriz de Adjacencia\n";
        cout << "Opcao: ";
        cin >> escolha_repr;

        if (escolha_repr != 1 && escolha_repr != 2) {
            cout << "Opcao invalida. Usando Lista de Adjacencia por padrao.\n";
            escolha_repr = 1;
        }

        Representacao tipo = (escolha_repr == 1) ? LISTA : MATRIZ;

        // 1. Leitura do Grafo a partir do arquivo
        cout << "\nLendo grafo do arquivo 'entrada.txt'...\n";
        Grafo g = Grafo::lerDeArquivo("entrada.txt", tipo);
        cout << "Grafo lido com sucesso!\n";
        cout << "-> Vertices: " << g.getNumVertices() << endl;
        cout << "-> Arestas: " << g.getNumArestas() << endl;

        // 2. Salvar Informações Básicas
        cout << "\nGerando estatisticas basicas (grau min/max, media, etc)..." << endl;
        salvarInfos(g, "infos.txt");
        cout << "-> Arquivo 'infos.txt' gerado com sucesso!" << endl;

        // 3. Buscas em Largura e Profundidade
        int vertice_inicial;
        cout << "\nDigite um vertice inicial para as buscas (ex: 1): ";
        cin >> vertice_inicial;
        limparBufferEntrada();

        cout << "\nExecutando Busca em Largura (BFS)..." << endl;
        g.Largura(vertice_inicial);
        cout << "-> Arvore de busca e niveis salvos em 'Largura.txt'." << endl;

        cout << "\nExecutando Busca em Profundidade (DFS)..." << endl;
        g.Profundidade(vertice_inicial);
        cout << "-> Arvore de busca e niveis salvos em 'Profundidade.txt'." << endl;
        
        // 4. Cálculo de Distância
        int vertice_final;
        cout << "\nDigite outro vertice para calcular a distancia a partir de " << vertice_inicial << ": ";
        cin >> vertice_final;
        limparBufferEntrada();

        cout << "\nCalculando a distancia entre " << vertice_inicial << " e " << vertice_final << "..." << endl;
        int dist = g.distancia(vertice_inicial, vertice_final);
        if (dist != -1) {
            cout << "-> A distancia (menor caminho) e: " << dist << endl;
        } else {
            cout << "-> Nao ha caminho entre os vertices (distancia infinita)." << endl;
        }

        // 5. Cálculo do Diâmetro
        cout << "\nCalculando o diametro do grafo (pode demorar um pouco)..." << endl;
        int diam = g.diametro();
        if (diam != -1) {
            cout << "-> O diametro do grafo e: " << diam << endl;
        } else {
            cout << "-> O grafo e desconexo, portanto seu diametro e infinito." << endl;
        }

        // 6. Encontrar Componentes Conexas
        cout << "\nEncontrando as componentes conexas..." << endl;
        g.componentesConexas("componentes.txt");
        cout << "-> Componentes salvas em 'componentes.txt'." << endl;

        // 7. Medição de desempenho (100 BFS e 100 DFS)
        cout << "\n\nExecutando 100 BFS e 100 DFS para medir tempo medio...\n";

        auto inicio = high_resolution_clock::now();
        for (int i = 1; i <= 100 && i <= g.getNumVertices(); i++) {
            g.Largura(i);
        }
        auto fim = high_resolution_clock::now();
        auto duracao_bfs = duration_cast<milliseconds>(fim - inicio).count() / 100.0;

        inicio = high_resolution_clock::now();
        for (int i = 1; i <= 100 && i <= g.getNumVertices(); i++) {
            g.Profundidade(i);
        }
        fim = high_resolution_clock::now();
        auto duracao_dfs = duration_cast<milliseconds>(fim - inicio).count() / 100.0;

        cout << "-> Tempo medio BFS: " << duracao_bfs << " ms\n";
        cout << "-> Tempo medio DFS: " << duracao_dfs << " ms\n";

        // 8. Estimativa de memória usada
        cout << "\nMemoria estimada utilizada: " 
             << g.memoriaUsada() / (1024.0 * 1024.0) << " MB\n";

        cout << "\n\n--- Analise concluida! Verifique os arquivos de saida. ---\n";

    } catch (const exception& e) {
        cerr << "\nERRO CRITICO: " << e.what() << endl;
    }
    return 0;
}
