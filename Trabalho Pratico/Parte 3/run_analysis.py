import subprocess
import json
import sys
from pathlib import Path
import os
import gzip
import shutil


# import requests # REMOVIDO

# --- Configurações ---
CPP_EXECUTABLE = './analyzer' # ATENÇÃO: Mude para './analyzer.exe' se estiver no Windows
GRAPH_DIR = './Grafos'
Path(GRAPH_DIR).mkdir(exist_ok=True) # Cria o diretório se não existir

# REMOVIDO: A lista de grafos para baixar
# GRAFOS_PARA_ANALISAR = [...]

# REMOVIDO: Função de download
# def download_graph(url: str, dest_path: Path) -> bool:
#    ...

def decompress_gz_file(gz_path: Path) -> Path:
    """Descompacta um arquivo .gz e retorna o caminho do arquivo de saída."""
    txt_path = gz_path.with_suffix('') # Remove a extensão .gz
    
    with gzip.open(gz_path, 'rb') as f_in:
        with open(txt_path, 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)
            
    return txt_path

def run_cpp_analysis(graph_file: str, representation: str, is_directed: str, 
                     invert: str, algorithm: str, source: str) -> dict:
    """
    Executa o programa de análise de grafos em C++ e retorna os resultados.
    """
    command = [
        CPP_EXECUTABLE, 
        graph_file, 
        representation, 
        is_directed, 
        invert, 
        algorithm, 
        source
    ]
    
    print(f"  -> Executando C++: {' '.join(command)}")
    
    try:
        result = subprocess.run(
            command, 
            capture_output=True, 
            text=True, 
            check=True,
            encoding='utf-8'
        )
        return json.loads(result.stdout)
    except subprocess.CalledProcessError as e:
        print(f"  [AVISO] Ocorreu um erro ao executar o processo C++.", file=sys.stderr)
        print(f"  Comando: {' '.join(e.cmd)}", file=sys.stderr)
        print(f"  Saída de Erro (stderr): {e.stderr.strip()}", file=sys.stderr)
        # Tenta carregar o JSON do stdout mesmo em erro, se houver
        try:
            return json.loads(e.stdout) if e.stdout else {"erro_processo": e.stderr.strip()}
        except json.JSONDecodeError:
            return {"erro_processo": e.stderr.strip(), "stdout_raw": e.stdout}
    except FileNotFoundError:
        print(f"ERRO CRÍTICO: Executável '{CPP_EXECUTABLE}' não encontrado.", file=sys.stderr)
        print("Por favor, compile o código C++ (ex: g++ main.cpp grafo.cpp -o analyzer -O3 -std=c++17)", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"  [AVISO] Falha ao decodificar a saída JSON.", file=sys.stderr)
        print(f"  Saida (stdout): {e.doc}", file=sys.stderr)
        return {"erro_json": "Falha ao decodificar JSON", "stdout_raw": e.doc}

def print_tables(graph_name: str, bf_data: dict, dijkstra_data: dict):
    """Imprime as tabelas de comparação formatadas."""
    
    alvos = ["10", "20", "30"]
    
    # Função auxiliar para formatar distância (json 'null' vira 'Inalcançável')
    def format_dist(dist):
        return "Inalcançável" if dist is None else f"{dist:.2f}"

    print("\n" + "="*70)
    print(f"Resultados para o Grafo: {graph_name}")
    print("="*70)

    # --- Tabela 1: Comparação de Distâncias (Vértices 10, 20, 30 para 100) ---
    print("\nTABELA 1: Distâncias (Origem -> 100) - Grafo Invertido (100 -> Origem)")
    # Tenta obter a fonte do Bellman-Ford, se falhar, usa 'N/A'
    source_vertex = bf_data.get('configuracao', {}).get('source', 'N/A')
    print(f"Fonte no grafo invertido: {source_vertex}")
    print("-"*70)
    print(f"| {'Vértice (Origem)':<16} | {'Distância Bellman-Ford':<24} | {'Distância Dijkstra':<20} |")
    print(f"|{'-'*18}|{'-'*26}|{'-'*22}|")
    
    for alvo in alvos:
        key = f"vertice_{alvo}"
        bf_dist = format_dist(bf_data.get("distancias", {}).get(key, "Erro"))
        d_dist = format_dist(dijkstra_data.get("distancias", {}).get(key, "Erro"))
        print(f"| {alvo:<16} | {bf_dist:<24} | {d_dist:<20} |")
        
    print("-"*70)

    # --- Tabela 2: Comparação de Tempo de Execução ---
    print("\nTABELA 2: Tempo Médio de Execução (10 rodadas)")
    representacao = bf_data.get('configuracao', {}).get('representacao', 'lista')
    print(f"Fonte: {source_vertex}, Representação: {representacao}")
    print("-"*70)
    print(f"| {'Algoritmo':<20} | {'Tempo Médio (ms)':<20} | Ciclo Negativo? |")
    print(f"|{'-'*22}|{'-'*22}|{'-'*17}|")
    
    bf_time = bf_data.get('tempoMedio_ms', 0)
    bf_ciclo = "Sim" if bf_data.get('cicloNegativoDetectado', False) else "Não"
    print(f"| {'Bellman-Ford':<20} | {bf_time:<20.4f} | {bf_ciclo:<15} |")
    
    d_time = dijkstra_data.get('tempoMedio_ms', 0)
    # Dijkstra vai falhar (retornar um erro no JSON) se C++ detectar pesos negativos
    d_ciclo = "Não (ignorado)"
    if 'erro' in dijkstra_data:
        d_time = 0.0 # Zera o tempo se deu erro
        print(f"| {'Dijkstra (Heap)':<20} | {'ERRO (ex: peso neg.)':<20} | {d_ciclo:<15} |")
    else:
         print(f"| {'Dijkstra (Heap)':<20} | {d_time:<20.4f} | {d_ciclo:<15} |")
    
    print("-"*70)

def main():
    """Função principal para orquestrar a análise."""
    uncompressed_file = None
    
    # MODIFICAÇÃO: Encontra todos os arquivos .txt na pasta de grafos
    graph_files_gz = sorted(list(Path(GRAPH_DIR).glob('*.txt.gz')))
    
    if not graph_files_gz:
        print(f"ERRO: Nenhum arquivo '.txt.gz' encontrado no diretório '{GRAPH_DIR}'", file=sys.stderr)
        return

    # MODIFICAÇÃO: Itera sobre os arquivos encontrados
    for graph_file_path in graph_files_gz:
        graph_name = graph_file_path.name
        print(f"\n--- Processando Grafo: {graph_name}")
        uncompressed_file = decompress_gz_file(graph_file_path)
        print(f"  -> Descompactado para: {uncompressed_file.name}")
        # REMOVIDO: Lógica de download
        
        # --- 1. Executar Bellman-Ford ---
        # Queremos dist(X -> 100), então rodamos BF(100) no grafo INVERTIDO
        print("\n[Análise com Bellman-Ford]")
        bf_json = run_cpp_analysis(
            graph_file=str(uncompressed_file),
            representation="lista",
            is_directed="t",
            invert="f", # Inverte o grafo
            algorithm="bellman",
            source="100"
        )
        if "erro_processo" in bf_json or "erro_json" in bf_json:
            print(f"  [ERRO FATAL] Bellman-Ford falhou, pulando análise de {graph_name}.")
            continue

        # --- 2. Executar Dijkstra ---
        # MODIFICAÇÃO: Removemos a flag 'has_neg_weights' e sempre tentamos rodar Dijkstra.
        # O C++ irá falhar se detectar pesos negativos, e o script irá capturar isso.
        print("\n[Análise com Dijkstra (Heap)]")
        dijkstra_json = run_cpp_analysis(
            graph_file=str(graph_file_path),
            representation="lista",
            is_directed="t",
            invert="t", # Inverte o grafo
            algorithm="dijkstra",
            source="100"
        )
        if "erro_processo" in dijkstra_json or "erro_json" in dijkstra_json or "erro" in dijkstra_json:
            print(f"  [AVISO] Dijkstra falhou. (Provavelmente devido a pesos negativos)")
            # Zera os dados para a tabela não quebrar, mas mantém a config
            dijkstra_json = {"configuracao": bf_json["configuracao"], "erro": dijkstra_json.get("erro", "Erro C++")} 

        # --- 3. Imprimir Tabelas de Comparação ---
        print_tables(graph_name, bf_json, dijkstra_json)

    print("\n--- Análise Concluída! ---")

if __name__ == '__main__':
    main()
