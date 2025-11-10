#include "grafo.h"
#include "json.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <limits> // Necessário para numeric_limits

using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

void toLower(string& s) {
    transform(s.begin(), s.end(), s.begin(),
                  [](unsigned char c){ return tolower(c); });
}

int main(int argc, char* argv[]) {
    // --- 1. Validação dos Argumentos ---
    if (argc < 4) {
        cerr << "Erro: Argumentos insuficientes." << endl;
        cerr << "Uso: ./analyzer <arquivo> <representacao> <dijkstra_tipo> [media_amostral]" << endl;
        cerr << "Exemplo: ./analyzer in.txt lista h t" << endl;
        return 1;
    }

    string nomeArquivo = argv[1];
    string repr_str = argv[2];
    string dijkstra_check = argv[3];
    // O argumento 4 (media_amostral) é opcional
    string mediaAmostral = (argc > 4) ? argv[4] : "f"; // 'f' para falso por padrão

    toLower(repr_str);
    toLower(dijkstra_check);
    toLower(mediaAmostral);

    Representacao tipo;
    if (repr_str == "lista") tipo = LISTA;
    else if (repr_str == "matriz") tipo = MATRIZ;
    else {
        cerr << "Erro: Tipo de representacao invalido. Use 'lista' ou 'matriz'." << endl;
        return 1;
    }

    json resultadosJson;

    try {
        resultadosJson["configuracao"]["arquivoDeEntrada"] = nomeArquivo;
        resultadosJson["configuracao"]["representacao"] = (tipo == LISTA)
            ? "Lista de Adjacencia" : "Matriz de Adjacencia";
        resultadosJson["configuracao"]["dijkstraTipo"] = (dijkstra_check == "h") ? "Heap" : "Vetor";

        Grafo g = Grafo::lerDeArquivo(nomeArquivo, tipo);
        map<string, double> estatisticas = g.getEstatisticas();

        resultadosJson["informacoesBasicas"]["vertices"] = estatisticas["vertices"];
        resultadosJson["informacoesBasicas"]["arestas"] = estatisticas["arestas"];
        resultadosJson["informacoesBasicas"]["grauMinimo"] = estatisticas["grauMinimo"];
        resultadosJson["informacoesBasicas"]["grauMaximo"] = estatisticas["grauMaximo"];
        resultadosJson["informacoesBasicas"]["grauMedio"] = estatisticas["grauMedio"];
        resultadosJson["informacoesBasicas"]["grauMediana"] = estatisticas["grauMediana"];

        json &desempenho = resultadosJson["desempenho"];
        vector<pair<float, int>> CustoPai;

        // ----------- DIJKSTRA HEAP -----------
        if (dijkstra_check == "h") {
            // Executa uma vez para o estudo de caso dos caminhos
            CustoPai = g.DijkstraHeap(2722); 
            
            if (mediaAmostral == "t") {
                auto inicio_loop = high_resolution_clock::now();
                for (int i = 1; i <= 100 && i <= g.getNumVertices(); i++) {
                    g.DijkstraHeap(i);
                }
                auto fim_loop = high_resolution_clock::now();
                // *** CORREÇÃO AQUI: Chave JSON única ***
                desempenho["tempoMedio_DijkstraHeap_ms"] = 
                    duration_cast<milliseconds>(fim_loop - inicio_loop).count() / 100.0;
            }
        }

        // ----------- DIJKSTRA VETOR -----------
        else if (dijkstra_check == "v") {
            // Executa uma vez para o estudo de caso dos caminhos
            CustoPai = g.DijkstraVetor(2722);
            
            if (mediaAmostral == "t") {
                auto inicio_loop = high_resolution_clock::now();
                for (int i = 1; i <= 100 && i <= g.getNumVertices(); i++) {
                    g.DijkstraVetor(i);
                }
                auto fim_loop = high_resolution_clock::now();
                // *** CORREÇÃO AQUI: Chave JSON única ***
                desempenho["tempoMedio_DijkstraVetor_ms"] = 
                    duration_cast<milliseconds>(fim_loop - inicio_loop).count() / 100.0;
            }
        }
        else {
             cerr << "Erro: Tipo de Dijkstra invalido. Use 'h' (heap) ou 'v' (vetor)." << endl;
             return 1;
        }

        // ----------- ANÁLISE DOS CAMINHOS -----------
        // Esta seção só será preenchida se CustoPai foi inicializado (ou seja, se 'h' ou 'v' foi passado)
        json& analises = resultadosJson["analises"];
        vector<int> alvos = {11365, 509510, 5709, 11386 ,343930}; // Vértices de interesse

        if (!CustoPai.empty()) { // Só executa se Dijkstra rodou
            for (int alvo : alvos) {
                if (alvo >= (int)CustoPai.size()) continue; // segurança
                float dist = CustoPai[alvo].first;
                vector<int> caminho;

                if (dist == numeric_limits<float>::infinity()) {
                    caminho = {};
                } else {
                    int atual = alvo;
                    while (atual != 0 && atual != -1) { // -1 é o pai de um vértice inalcançável
                        caminho.push_back(atual);
                        if(CustoPai[atual].second == 0 && atual != 2722) // Evita loop se a raiz (10) for 0
                            break;
                        atual = CustoPai[atual].second;
                    }
                    if(atual == 2722) // Adiciona a raiz se ela foi alcançada
                         caminho.push_back(2722);
                    reverse(caminho.begin(), caminho.end());
                }

                analises["caminhos"]["vertice_" + to_string(alvo)] = caminho;
                analises["distancias"]["vertice_" + to_string(alvo)] =
                    (dist == numeric_limits<float>::infinity()) ? -1 : dist;
            }
        }

        cout << resultadosJson.dump(4) << endl;
    }

    catch (const exception& e) {
        json erro;
        erro["erro"] = e.what();
        cerr << erro.dump(4) << endl;
        return 1;
    }

    return 0;
}