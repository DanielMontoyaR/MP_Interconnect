import os
import re
import matplotlib.pyplot as plt

# Ruta a la carpeta donde están los archivos
carpeta = '../PE_Bandwidth'

# Obtener lista de archivos dentro de la carpeta
archivos = [f for f in os.listdir(carpeta) if re.match(r'PE\d+_bandwidth', f)]

pes = []
shared_mem_times = []

# Extraer datos de cada archivo
for archivo in archivos:
    ruta_completa = os.path.join(carpeta, archivo)
    with open(ruta_completa, 'r') as f:
        contenido = f.read()
        match_pe = re.search(r'PE\s*(\d+)', contenido)
        match_shared = re.search(r'Shared Memory Access Time:\s*([0-9.]+)', contenido)
        if match_pe and match_shared:
            pe = int(match_pe.group(1))
            shared_time = float(match_shared.group(1))
            pes.append(pe)
            shared_mem_times.append(shared_time)

# Ordenar los datos por PE
pes, shared_mem_times = zip(*sorted(zip(pes, shared_mem_times)))

# --- Gráfico: Shared Memory Access Time ---
plt.figure(figsize=(10, 6))
plt.bar([f'PE {pe}' for pe in pes], shared_mem_times, color='mediumpurple')
plt.xlabel('Processing Element (PE)')
plt.ylabel('Shared Memory Access Time (s)')
plt.title('Tiempo de acceso a memoria compartida por PE')
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.show()