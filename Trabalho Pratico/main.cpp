#include "grafo.h"      // Sua classe de grafo
#include "json.hpp"     // Biblioteca nlohmann/json
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>

// Para facilitar a escrita
using json = nlohmann::json;
using namespace std::chrono;
using namespace std;

// Função para converter string para minúsculas para facilitar a comparação
void toLower(string& s) {
    transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return tolower(c); });
}

// Assinatura de main modificada para aceitar argumentos de linha de comando
int main(int argc, char* argv[]) {

    string nomeArquivo = argv[1];
    string repr_str = argv[2];
    string dijkstra_check = argv[3];
    string mediaAmostral = argv[4];
    toLower(repr_str); // Normaliza para minúsculas

    Representacao tipo;
    if (repr_str == "lista") {
        tipo = LISTA;
    } else if (repr_str == "matriz") {
        tipo = MATRIZ;
    } else {
        cerr << "Erro: Tipo de representacao invalido. Use 'lista' ou 'matriz'." << endl;
        return 1;
    }

    json resultadosJson; // Objeto JSON principal

    try {
        // --- 2. Lógica do Programa (sem interação) ---
        resultadosJson["configuracao"]["arquivoDeEntrada"] = nomeArquivo;
        resultadosJson["configuracao"]["representacao"] = (tipo == LISTA) ? "Lista de Adjacencia" : "Matriz de Adjacencia";

        Grafo g = Grafo::lerDeArquivo(nomeArquivo, tipo);
        map<string, double> estatisticas = g.getEstatisticas();

        resultadosJson["informacoesBasicas"]["vertices"] = estatisticas["vertices"];
        resultadosJson["informacoesBasicas"]["arestas"] = estatisticas["arestas"];
        resultadosJson["informacoesBasicas"]["grauMinimo"] = estatisticas["grauMinimo"];
        resultadosJson["informacoesBasicas"]["grauMaximo"] = estatisticas["grauMaximo"];
        resultadosJson["informacoesBasicas"]["grauMedio"] = estatisticas["grauMedio"];
        resultadosJson["informacoesBasicas"]["grauMediana"] = estatisticas["grauMediana"];

        json & desempenho = resultadosJson["desempenho"];
        
        vector<pair<float, int>> CustoPai;

        if (dijkstra_check == "h") {
            auto inicio_dijkstra = high_resolution_clock::now();
            CustoPai = g.DijkstraHeap(10);
            auto fim_dijkstra = high_resolution_clock::now();

            if (mediaAmostral == "t") {
                auto inicio_dijkstra = high_resolution_clock::now();
                for (int i = 1; i <= 100 && i <= g.getNumVertices(); i++) {
                    g.DijkstraHeap(i);
                }
                auto fim_dijkstra = high_resolution_clock::now();
                desempenho["tempoMedio_Dijkstra_ms"] = duration_cast<milliseconds>(fim_dijkstra - inicio_dijkstra).count() / 100.0;
            }
        }

        // --- 4. Análises Específicas ---
        json& analises = resultadosJson["analises"];
        
        vector<int> inicios = {10};
        vector<int> alvos = {20, 30, 40, 50, 60};
        for (int alvo : alvos) {
            float dist = CustoPai[alvo].first;
            vector<int> caminho;

            if (dist == numeric_limits<float>::infinity()) {
                caminho = {}; // Sem caminho
            } else {
                // Reconstruir o caminho a partir de CustoPai
                int atual = alvo;
                while (atual != 0) { // 0 indica que chegamos à raiz
                    caminho.push_back(atual);
                    atual = CustoPai[atual].second;
                }
                reverse(caminho.begin(),caminho.end());
            }

            analises["caminhos"]["Dijkstra"]["vertice_" + to_string(alvo)] = caminho;
            analises["distanciasDijkstra"]["vertice_" + to_string(alvo)] = (dist == numeric_limits<float>::infinity()) ? -1 : dist;
        }
        
        // --- 5. Saída Final ---
        // Imprime o JSON para 'stdout'. Esta deve ser a ÚNICA saída para cout.
        cout << resultadosJson.dump(4) << endl;

    } catch (const exception& e) {
        // Em caso de erro durante a execução, cria um JSON de erro
        resultadosJson.clear();
        resultadosJson["erro"] = e.what();
        // Imprime o erro no 'stderr'
        cerr << resultadosJson.dump(4) << endl;
        return 1; // Retorna código de erro
    }

    return 0; // Sucesso
}
