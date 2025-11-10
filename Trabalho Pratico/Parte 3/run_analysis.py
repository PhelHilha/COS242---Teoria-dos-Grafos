import subprocess
import json
import sys
import os
import gzip
import shutil
import csv
from pathlib import Path
import requests
# --- Configurações ---
CPP_EXECUTABLE = './analyzer.exe'
GRAPH_DIR = './Grafos'
OUTPUT_CSV_FILE = 'resultados_analise.csv'
# Define se queremos rodar a média amostral (k=100)
# 't' para true, 'f' para false
RUN_MEDIA_AMOSTRAL = 'f' 

def run_cpp_analysis(graph_file: str, representation: str, dijkstra_type: str, media_amostral: str) -> dict:
    """
    Executa o programa de análise de grafos em C++ e retorna os resultados.
    """
    command = [CPP_EXECUTABLE, graph_file, representation, dijkstra_type, media_amostral]
    
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
        return None
    except FileNotFoundError:
        print(f"ERRO CRÍTICO: Executável '{CPP_EXECUTABLE}' não encontrado.", file=sys.stderr)
        print("Por favor, compile o código C++ primeiro.", file=sys.stderr)
        sys.exit(1)
    except json.JSONDecodeError as e:
        print(f"  [AVISO] Falha ao decodificar a saída JSON.", file=sys.stderr)
        print(f"  Saida (stdout): {e.doc}", file=sys.stderr)
        return None


def decompress_gz_file(gz_path: Path) -> Path:
    """Descompacta um arquivo .gz e retorna o caminho do arquivo de saída."""
    txt_path = gz_path.with_suffix('') # Remove a extensão .gz
    
    with gzip.open(gz_path, 'rb') as f_in:
        with open(txt_path, 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)
            
    return txt_path


def flatten_json_results(graph_name: str, representation: str, dijkstra_type: str, data: dict) -> dict:
    """Extrai os dados do JSON e os formata em um dicionário plano para o CSV."""
    if not data:
        return {
            "Grafo": graph_name,
            "Representacao": representation,
            "Dijkstra_Tipo": dijkstra_type,
            "Status": "Falhou (JSON Vazio)"
        }
    
    if "erro" in data:
         return {
            "Grafo": graph_name,
            "Representacao": representation,
            "Dijkstra_Tipo": dijkstra_type,
            "Status": f"Falhou (Erro C++)",
            "Erro_Msg": data["erro"]
        }


    # Funções auxiliares para extrair dados aninhados com segurança
    def get_nested(d, *keys, default='N/A'):
        for key in keys:
            if isinstance(d, dict):
                d = d.get(key)
            else:
                return default
        return d if d is not None else default

    # Lê as chaves específicas de tempo
    if dijkstra_type == 'h':
        tempo_key = 'tempoMedio_DijkstraHeap_ms'
        dijkstra_nome = 'Heap'
    else:
        tempo_key = 'tempoMedio_DijkstraVetor_ms'
        dijkstra_nome = 'Vetor'

    # ===================================================================
    # === INÍCIO DA MODIFICAÇÃO: ADIÇÃO DE NOVAS COLUNAS ===
    # ===================================================================
    row = {
        "Grafo": graph_name,
        "Representacao": representation,
        "Dijkstra_Tipo": dijkstra_nome,
        "Status": "Sucesso",
        "Vertices": get_nested(data, 'informacoesBasicas', 'vertices'),
        "Arestas": get_nested(data, 'informacoesBasicas', 'arestas'),
        
        # --- ADICIONADO: Estatísticas de Grau ---
        "Grau_Minimo": get_nested(data, 'informacoesBasicas', 'grauMinimo'),
        "Grau_Maximo": get_nested(data, 'informacoesBasicas', 'grauMaximo'),
        "Grau_Medio": get_nested(data, 'informacoesBasicas', 'grauMedio'),
        "Grau_Mediana": get_nested(data, 'informacoesBasicas', 'grauMediana'),
        
        # --- Tempo de Execução ---
        "Tempo_Medio_Dijkstra_ms": get_nested(data, 'desempenho', tempo_key),
        
        # --- Distâncias ---
        "Dist_10_20": get_nested(data, 'analises', 'distancias', 'vertice_11365'),
        "Dist_10_30": get_nested(data, 'analises', 'distancias', 'vertice_509510'),
        "Dist_10_40": get_nested(data, 'analises', 'distancias', 'vertice_5709'),
        "Dist_10_50": get_nested(data, 'analises', 'distancias', 'vertice_11386'),
        "Dist_10_60": get_nested(data, 'analises', 'distancias', 'vertice_343930'),

        # --- ADICIONADO/DESCOMENTADO: Caminhos ---
        # Convertemos a lista do JSON (ex: [10, 15, 20]) para uma string
        "Caminho_10_20": str(get_nested(data, 'analises', 'caminhos', 'vertice_11365')),
        "Caminho_10_30": str(get_nested(data, 'analises', 'caminhos', 'vertice_509510')),
        "Caminho_10_40": str(get_nested(data, 'analises', 'caminhos', 'vertice_5709')),
        "Caminho_10_50": str(get_nested(data, 'analises', 'caminhos', 'vertice_11386')),
        "Caminho_10_60": str(get_nested(data, 'analises', 'caminhos', 'vertice_343930')),
    }
    # ===================================================================
    # === FIM DA MODIFICAÇÃO                                          ===
    # ===================================================================
    
    
    # Adiciona colunas de erro se existirem
    if "Erro_Msg" in row:
        row["Erro_Msg"] = get_nested(data, 'erro', default='Erro desconhecido')

    return row


def main():
    """Função principal para orquestrar a análise."""
    graph_files_gz = sorted(list(Path(GRAPH_DIR).glob('*.txt.gz')))
    
    if not graph_files_gz:
        print(f"ERRO: Nenhum arquivo de grafo .txt.gz encontrado no diretório '{GRAPH_DIR}'", file=sys.stderr)
        return

    all_results = []
    csv_headers = []

    print("--- Iniciando Análise de Grafos ---")
    for representation in ['lista']:
        for dijkstra_type in ['h']: # 'h' para heap, 'v' para vetor
            graph_name = "Rede de Colaboração"
            print(f"\n[ Processando {graph_name} ]")
            
            uncompressed_file = None
            try:
                url = "https://www.cos.ufrj.br/~daniel/grafos-2021/data/rede_colaboracao.txt"
                with open("rede_colaboracao.txt", 'w', encoding='latin-1') as f:
                    f.write(requests.get(url, timeout=10).text)
                
                
                
                # Loop aninhado para rodar TUDO
                dijkstra_nome = 'Heap' if dijkstra_type == 'h' else 'Vetor'
                print(f"  -> Analisando: Repr='{representation}', Dijkstra='{dijkstra_nome}'...")
                
                json_data = run_cpp_analysis(
                    "rede_colaboracao.txt", 
                    representation,
                    dijkstra_type,
                    RUN_MEDIA_AMOSTRAL
                )
                
                flat_data = flatten_json_results(graph_name, representation, dijkstra_type, json_data)
                all_results.append(flat_data)

                # Define o cabeçalho do CSV na primeira execução bem-sucedida
                if not csv_headers and flat_data.get("Status") == "Sucesso":
                    csv_headers = list(flat_data.keys())
                    
                # Garante que os cabeçalhos sejam definidos mesmo se todos falharem
                if not csv_headers:
                    # Pega as chaves da primeira linha, seja ela de sucesso ou falha
                    csv_headers = list(all_results[0].keys())

                print(f"\nSalvando resultados finais em '{OUTPUT_CSV_FILE}'...")
                with open(OUTPUT_CSV_FILE, 'w', newline='', encoding='utf-8') as f:
                    # extrasaction='ignore' é importante para caso uma linha de falha tenha menos colunas
                    writer = csv.DictWriter(f, fieldnames=csv_headers, extrasaction='ignore')
                    writer.writeheader()
                    for row in all_results:
                        writer.writerow(row)

            except Exception as e:
                print(f"ERRO Inesperado no processamento do {graph_name}: {e}", file=sys.stderr)
            
            #finally:
                # Limpar o arquivo descompactado
                
                #if uncompressed_file and uncompressed_file.exists():
                #    os.remove(uncompressed_file)
                #    print(f"  -> Arquivo temporário '{uncompressed_file.name}' removido.")
    
    # --- 3. Salvar CSV Final ---
    if not all_results:
        print("Nenhum resultado foi gerado.")
        return

    # Garante que os cabeçalhos sejam definidos mesmo se todos falharem
    if not csv_headers:
        # Pega as chaves da primeira linha, seja ela de sucesso ou falha
        csv_headers = list(all_results[0].keys())

    print(f"\nSalvando resultados finais em '{OUTPUT_CSV_FILE}'...")
    with open(OUTPUT_CSV_FILE, 'w', newline='', encoding='utf-8') as f:
        # extrasaction='ignore' é importante para caso uma linha de falha tenha menos colunas
        writer = csv.DictWriter(f, fieldnames=csv_headers, extrasaction='ignore')
        writer.writeheader()
        for row in all_results:
            writer.writerow(row)
    
    print("\n--- Análise Concluída! ---")


if __name__ == '__main__':
    main()