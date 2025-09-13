#include "grafo.h"
#include <iostream>
using namespace std;

int main() {
    try {
        int escolha;
        cout << "Escolha a representacao (1 = Lista, 2 = Matriz): ";
        cin >> escolha;

        Representacao tipo = (escolha == 1) ? LISTA : MATRIZ;

        Grafo g = Grafo::lerDeArquivo("entrada.txt", tipo);

        cout << "Vertices: " << g.getNumVertices() << endl;
        cout << "Arestas: " << g.getNumArestas() << endl;

        salvarInfos(g, "saida.txt");

        cout << "Arquivo 'saida.txt' gerado com sucesso!" << endl;
    } catch (const exception& e) {
        cerr << "Erro: " << e.what() << endl;
    }

    return 0;
}
