import subprocess
import json
import sys
import os
import gzip
import shutil
import csv
from pathlib import Path

# --- Configurações ---
CPP_EXECUTABLE = './analyzer.exe'
GRAPH_DIR = './Grafos'
OUTPUT_CSV_FILE = 'resultados_analise.csv'

def run_cpp_analysis(graph_file: str, representation: str) -> dict:
    """
    Executa o programa de análise de grafos em C++ e retorna os resultados.
    """
    command = [CPP_EXECUTABLE, graph_file, representation]
    
    try:
        result = subprocess.run(
            command, 
            capture_output=True, 
            text=True, 
            check=True,
            encoding='utf-8' # Adicionado para garantir a decodificação correta
        )
        return json.loads(result.stdout)
    except subprocess.CalledProcessError as e:
        # Erro na execução do C++, como grafo muito grande para matriz
        print(f"  [AVISO] Ocorreu um erro ao executar o processo C++ para '{representation}'.", file=sys.stderr)
        print(f"  Comando: {' '.join(e.cmd)}", file=sys.stderr)
        print(f"  Saída de Erro (stderr): {e.stderr.strip()}", file=sys.stderr)
        return None # Retorna None para indicar falha
    except FileNotFoundError:
        print(f"ERRO CRÍTICO: Executável '{CPP_EXECUTABLE}' não encontrado.", file=sys.stderr)
        print("Por favor, compile o código C++ primeiro.", file=sys.stderr)
        sys.exit(1) # Aborta o script
    except json.JSONDecodeError:
        print(f"  [AVISO] Falha ao decodificar a saída JSON para '{representation}'.", file=sys.stderr)
        return None


def decompress_gz_file(gz_path: Path) -> Path:
    """Descompacta um arquivo .gz e retorna o caminho do arquivo de saída."""
    txt_path = gz_path.with_suffix('') # Remove a extensão .gz
    
    with gzip.open(gz_path, 'rb') as f_in:
        with open(txt_path, 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)
            
    return txt_path


def flatten_json_results(graph_name: str, representation: str, data: dict) -> dict:
    """Extrai os dados do JSON e os formata em um dicionário plano para o CSV."""
    if not data:
        return {
            "Grafo": graph_name,
            "Representacao": representation,
            "Status": "Falhou"
        }

    # Funções auxiliares para extrair dados aninhados com segurança
    def get_nested(d, *keys, default='N/A'):
        for key in keys:
            if isinstance(d, dict):
                d = d.get(key)
            else:
                return default
        return d if d is not None else default

    # Extração dos dados
    row = {
        "Grafo": graph_name,
        "Representacao": representation,
        "Status": "Sucesso",
        "Vertices": get_nested(data, 'informacoesBasicas', 'vertices'),
        "Arestas": get_nested(data, 'informacoesBasicas', 'arestas'),
        "Grau_Minimo": get_nested(data, 'informacoesBasicas', 'grauMinimo'),
        "Grau_Maximo": get_nested(data, 'informacoesBasicas', 'grauMaximo'),
        "Grau_Medio": get_nested(data, 'informacoesBasicas', 'grauMedio'),
        "Grau_Mediana": get_nested(data, 'informacoesBasicas', 'grauMediana'),
        "Memoria_MB": get_nested(data, 'desempenho', 'memoriaEstimada_MB'),
        "Tempo_BFS_ms": get_nested(data, 'desempenho', 'tempoMedio_BFS_ms'),
        "Tempo_DFS_ms": get_nested(data, 'desempenho', 'tempoMedio_DFS_ms'),
        "CC_Quantidade": get_nested(data, 'analises', 'componentesConexas', 'quantidade'),
        "CC_Maior": get_nested(data, 'analises', 'componentesConexas', 'tamanhoMaior'),
        "CC_Menor": get_nested(data, 'analises', 'componentesConexas', 'tamanhoMenor'),
        "Diametro": get_nested(data, 'analises', 'diametro')
    }

    # Extrai distâncias específicas
    distancias = get_nested(data, 'analises', 'distancias', default=[])
    dist_map = {item.get('par'): item.get('distancia') for item in distancias}
    row["Dist_10_20"] = dist_map.get('10-20', 'N/A')
    row["Dist_10_30"] = dist_map.get('10-30', 'N/A')
    row["Dist_20_30"] = dist_map.get('20-30', 'N/A')

    # Extrai pais específicos (exemplo para alguns casos)
    for busca in ['BFS', 'DFS']:
        for inicio in [1, 2, 3]:
            for alvo in [10, 20, 30]:
                col_name = f"Pai_{busca}_S{inicio}_V{alvo}"
                row[col_name] = get_nested(data, 'analises', 'pais', busca, f'inicio_{inicio}', f'vertice_{alvo}')

    return row


def main():
    """Função principal para orquestrar a análise."""
    graph_files_gz = sorted(list(Path(GRAPH_DIR).glob('*.txt.gz')))
    
    if not graph_files_gz:
        print(f"ERRO: Nenhum arquivo de grafo .txt.gz encontrado no diretório '{GRAPH_DIR}'", file=sys.stderr)
        return

    all_results = []
    
    # Define os cabeçalhos do CSV de forma dinâmica para garantir que tudo seja incluído
    # Pega as chaves da primeira análise bem-sucedida como modelo
    csv_headers = []

    print("--- Iniciando Análise de Grafos ---")

    for gz_file in graph_files_gz:
        graph_name = gz_file.name
        print(f"\n[ Processando {graph_name} ]")
        
        # 1. Descompactar o arquivo
        uncompressed_file = None
        try:
            uncompressed_file = decompress_gz_file(gz_file)
            print(f"  -> Descompactado para: {uncompressed_file.name}")
            
            # 2. Analisar com ambas as representações
            for representation in ['lista', 'matriz']:
                print(f"  -> Analisando com representacao: '{representation}'...")
                
                json_data = run_cpp_analysis(str(uncompressed_file), representation)
                
                flat_data = flatten_json_results(graph_name, representation, json_data)
                all_results.append(flat_data)

                # Define o cabeçalho do CSV na primeira execução bem-sucedida
                if not csv_headers and flat_data.get("Status") == "Sucesso":
                    csv_headers = list(flat_data.keys())

            # 3. Salvar progresso no CSV
            if csv_headers:
                print(f"  -> Salvando resultados parciais em '{OUTPUT_CSV_FILE}'...")
                with open(OUTPUT_CSV_FILE, 'w', newline='', encoding='utf-8') as f:
                    writer = csv.DictWriter(f, fieldnames=csv_headers)
                    writer.writeheader()
                    # Escreve apenas as linhas que têm todas as colunas
                    for row in all_results:
                        # Preenche colunas ausentes para linhas de falha
                        full_row = {h: row.get(h, 'Falhou') for h in csv_headers}
                        writer.writerow(full_row)

        finally:
            # 4. Limpar o arquivo descompactado
            if uncompressed_file and uncompressed_file.exists():
                os.remove(uncompressed_file)
                print(f"  -> Arquivo temporário '{uncompressed_file.name}' removido.")
    
    print("\n--- Análise Concluída! ---")
    print(f"Resultados finais salvos em '{OUTPUT_CSV_FILE}'.")


if __name__ == '__main__':
    main()
