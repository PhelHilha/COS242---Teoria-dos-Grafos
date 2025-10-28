import requests
import io
import sys

def encontrar_indice_por_nome(nome_procurado):
    '''
    Procura um nome em um arquivo de texto online e retorna o índice correspondente.
    '''
    url = "https://www.cos.ufrj.br/~daniel/grafos-2021/data/rede_colaboracao_vertices.txt"
    
    try:
        response = requests.get(url)
        response.encoding = 'latin-1'
        
        if response.status_code == 200:
            f = io.StringIO(response.text)
            encontrado = False
            
            for line in f:
                line = line.strip()
                if not line:
                    continue
                
                try:
                    index, name = line.split(',', 1)
                    name = name.strip()
                except ValueError:
                    continue

                if index == str(nome_procurado):

                    print(f"{name}\\\\")
                    encontrado = True
                    break
            
            if not encontrado:
                print(f"\n--- Resultado ---")
                print(f"O nome '{nome_procurado}' não foi encontrado no arquivo.")
                print(f"-------------------")
                
        else:
            print(f"Falha ao acessar a URL. Código de status: {response.status_code}")
            
    except requests.RequestException as e:
        print(f"Ocorreu um erro de conexão: {e}")

if __name__ == "__main__":
    for i in [2722, 217250, 11456, 768, 11448, 101826, 12242, 11834, 9608, 5709]:
        encontrar_indice_por_nome(i)
    else:
        print("Nenhum nome fornecido.")
