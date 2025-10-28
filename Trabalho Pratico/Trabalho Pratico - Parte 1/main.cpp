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
    // --- 1. Validação dos Argumentos ---
    if (argc < 3) {
        // Imprime mensagens de erro no 'stderr' (saída de erro padrão)
        cerr << "Erro: Uso incorreto." << endl;
        cerr << "Uso: ./analyzer <caminho_do_arquivo> <representacao>" << endl;
        cerr << "Exemplo: ./analyzer entrada.txt lista" << endl;
        return 1; // Retorna um código de erro
    }

    string nomeArquivo = argv[1];
    string repr_str = argv[2];
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
        // 3. Salvar BFS e DFS consolidados em arquivo único
        int vertice_inicial = 1; // padrão
        if (argc >= 4) {
            vertice_inicial = stoi(argv[3]); // usuário pode passar um vértice inicial
        }
        string saidaBuscas = nomeArquivo + "_buscas.txt";
        g.salvarBuscasConsolidadas(vertice_inicial, saidaBuscas);
        map<string, double> estatisticas = g.getEstatisticas();

        resultadosJson["informacoesBasicas"]["vertices"] = estatisticas["vertices"];
        resultadosJson["informacoesBasicas"]["arestas"] = estatisticas["arestas"];
        resultadosJson["informacoesBasicas"]["grauMinimo"] = estatisticas["grauMinimo"];
        resultadosJson["informacoesBasicas"]["grauMaximo"] = estatisticas["grauMaximo"];
        resultadosJson["informacoesBasicas"]["grauMedio"] = estatisticas["grauMedio"];
        resultadosJson["informacoesBasicas"]["grauMediana"] = estatisticas["grauMediana"];

        json& desempenho = resultadosJson["desempenho"];

        auto inicio_bfs = high_resolution_clock::now();
        for (int i = 1; i <= 100 && i <= g.getNumVertices(); i++) {
            g.BFS_com_retorno(i);
        }
        auto fim_bfs = high_resolution_clock::now();
        desempenho["tempoMedio_BFS_ms"] = duration_cast<milliseconds>(fim_bfs - inicio_bfs).count() / 100.0;

        auto inicio_dfs = high_resolution_clock::now();
        for (int i = 1; i <= 100 && i <= g.getNumVertices(); i++) {
            g.DFS_com_retorno(i);
        }
        auto fim_dfs = high_resolution_clock::now();
        desempenho["tempoMedio_DFS_ms"] = duration_cast<milliseconds>(fim_dfs - inicio_dfs).count() / 100.0;
        
        desempenho["memoriaEstimada_MB"] = g.memoriaUsada() / (1024.0 * 1024.0);
        
        // --- 4. Análises Específicas ---
        json& analises = resultadosJson["analises"];
        
        vector<int> inicios = {1, 2, 3};
        vector<int> alvos = {10, 20, 30};
        for (int v_inicio : inicios) {
            auto pais_bfs = g.BFS_com_retorno(v_inicio);
            auto pais_dfs = g.DFS_com_retorno(v_inicio);
            for (int v_alvo : alvos) {
                if (v_alvo < pais_bfs.size()) {
                    analises["pais"]["BFS"]["inicio_" + to_string(v_inicio)]["vertice_" + to_string(v_alvo)] = pais_bfs[v_alvo];
                }
                if (v_alvo < pais_dfs.size()) {
                    analises["pais"]["DFS"]["inicio_" + to_string(v_inicio)]["vertice_" + to_string(v_alvo)] = pais_dfs[v_alvo];
                }
            }
        }
        
        vector<pair<int, int>> pares = {{10, 20}, {10, 30}, {20, 30}};
        for (const auto& par : pares) {
            json dist_obj;
            dist_obj["par"] = to_string(par.first) + "-" + to_string(par.second);
            dist_obj["distancia"] = g.distancia(par.first, par.second);
            analises["distancias"].push_back(dist_obj);
        }
        
        auto componentes = g.getComponentesConexas();
        if (!componentes.empty()) {
            analises["componentesConexas"]["quantidade"] = componentes.size();
            auto it_maior = max_element(componentes.begin(), componentes.end(), [](const auto& a, const auto& b) { return a.size() < b.size(); });
            analises["componentesConexas"]["tamanhoMaior"] = it_maior->size();
            auto it_menor = min_element(componentes.begin(), componentes.end(), [](const auto& a, const auto& b) { return a.size() < b.size(); });
            analises["componentesConexas"]["tamanhoMenor"] = it_menor->size();
        } else {
            analises["componentesConexas"]["quantidade"] = 0;
        }
        
        // analises["diametro"] = g.diametro();
        analises["diametroAproximado"] = g.diametroAproximado();
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
