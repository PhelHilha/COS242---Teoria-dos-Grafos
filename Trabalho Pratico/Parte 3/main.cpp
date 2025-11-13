#include "grafo.h"
#include "json.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <limits>
#include <stdexcept>

using json = nlohmann::json;
using namespace std;
using namespace std::chrono;

void toLower(string& s) {
    transform(s.begin(), s.end(), s.begin(),
              [](unsigned char c){ return tolower(c); });
}

int main(int argc, char* argv[]) {
    // --- 1. Validação dos Argumentos ---
    if (argc < 7) {
        cerr << "Erro: Argumentos insuficientes." << endl;
        cerr << "Uso: ./analyzer <arquivo> <representacao> <direcionado_flag> <inverter_flag> <algoritmo> <source_vertex>" << endl;
        cerr << "Exemplo: ./analyzer rede.txt lista t t bellman 100" << endl;
        return 1;
    }

    string nomeArquivo    = argv[1];
    string repr_str       = argv[2];
    string dir_str        = argv[3];
    string inv_str        = argv[4];
    string alg_str        = argv[5];
    string source_str     = argv[6];

    toLower(repr_str);
    toLower(dir_str);
    toLower(inv_str);
    toLower(alg_str);

    Representacao tipo;
    if (repr_str == "lista") tipo = LISTA;
    else if (repr_str == "matriz") tipo = MATRIZ;
    else {
        cerr << "Erro: Representacao invalida. Use 'lista' ou 'matriz'." << endl;
        return 1;
    }

    bool direcionado = (dir_str == "t");
    bool inverter = (inv_str == "t");
    int source = stoi(source_str);
    const int N_RODADAS = 10;
    vector<int> alvos = {10, 20, 30}; // Alvos de interesse

    json resultadosJson;
    resultadosJson["configuracao"]["arquivo"] = nomeArquivo;
    resultadosJson["configuracao"]["representacao"] = repr_str;
    resultadosJson["configuracao"]["direcionado"] = direcionado;
    resultadosJson["configuracao"]["invertido"] = inverter;
    resultadosJson["configuracao"]["algoritmo"] = alg_str;
    resultadosJson["configuracao"]["source"] = source;

    try {
        // --- 2. Carregar Grafo ---
        // Medição de tempo não inclui a leitura do arquivo
        Grafo g = Grafo::lerDeArquivo(nomeArquivo, tipo, direcionado, inverter);

        double tempo_total_ms = 0;
        vector<pair<float, int>> CustoPai;
        bool cicloNegativo = false;

        // --- 3. Executar Algoritmos (10 rodadas para tempo) ---
        
        if (alg_str == "bellman") {
            // Roda N vezes para medir o tempo
            for (int i = 0; i < N_RODADAS; ++i) {
                auto inicio = high_resolution_clock::now();
                g.BellmanFord(source);
                auto fim = high_resolution_clock::now();
                tempo_total_ms += duration_cast<microseconds>(fim - inicio).count() / 1000.0;
            }
            
            // Roda mais uma vez para obter os resultados
            auto res_bf = g.BellmanFord(source);
            cicloNegativo = !res_bf.first;
            CustoPai = res_bf.second;

        } else if (alg_str == "dijkstra") {
            // Roda N vezes para medir o tempo
            for (int i = 0; i < N_RODADAS; ++i) {
                auto inicio = high_resolution_clock::now();
                g.DijkstraHeap(source); // Usando Heap por padrão
                auto fim = high_resolution_clock::now();
                tempo_total_ms += duration_cast<microseconds>(fim - inicio).count() / 1000.0;
            }
            
            // Roda mais uma vez para obter os resultados
            CustoPai = g.DijkstraHeap(source);

        } else {
            throw runtime_error("Algoritmo '" + alg_str + "' desconhecido. Use 'bellman' ou 'dijkstra'.");
        }

        // --- 4. Processar Resultados ---
        resultadosJson["tempoMedio_ms"] = tempo_total_ms / N_RODADAS;
        resultadosJson["cicloNegativoDetectado"] = cicloNegativo;

        float INF = numeric_limits<float>::infinity();

        for (int alvo : alvos) {
            if (alvo >= (int)CustoPai.size()) {
                 resultadosJson["distancias"]["vertice_" + to_string(alvo)] = "Vertice Invalido";
                 continue;
            }

            float dist = CustoPai[alvo].first;
            
            // Usar 'null' para JSON é melhor que -1 para "infinito"
            if (dist == INF) {
                resultadosJson["distancias"]["vertice_" + to_string(alvo)] = json(nullptr);
            } else {
                resultadosJson["distancias"]["vertice_" + to_string(alvo)] = dist;
            }
        }

        cout << resultadosJson.dump(4) << endl;

    } catch (const exception& e) {
        json erro;
        erro["erro"] = e.what();
        cerr << erro.dump(4) << endl;
        return 1;
    }

    return 0;
}
