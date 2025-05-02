import os
import re
import matplotlib.pyplot as plt

# Ruta a la carpeta donde están los archivos
carpeta = '../PE_Bandwidth'

# Obtener lista de archivos dentro de la carpeta
archivos = [f for f in os.listdir(carpeta) if re.match(r'PE\d+_bandwidth', f)]

pes = []
bandwidths = []

# Extraer datos de cada archivo
for archivo in archivos:
    ruta_completa = os.path.join(carpeta, archivo)
    with open(ruta_completa, 'r') as f:
        contenido = f.read()
        match_pe = re.search(r'PE\s*(\d+)', contenido)
        match_bw = re.search(r'Bandwidth \(Bytes/s\):\s*([0-9.]+)', contenido)
        if match_pe and match_bw:
            pe = int(match_pe.group(1))
            bw = float(match_bw.group(1))
            pes.append(pe)
            bandwidths.append(bw)

# Ordenar los datos por PE
pes, bandwidths = zip(*sorted(zip(pes, bandwidths)))

# Graficar
plt.figure(figsize=(10, 6))
plt.bar([f'PE {pe}' for pe in pes], bandwidths, color='skyblue')
plt.xlabel('Processing Element (PE)')
plt.ylabel('Bandwidth (Bytes/s)')
plt.title('Bandwidth por PE')
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.show()
