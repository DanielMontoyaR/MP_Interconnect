import os
import re
import matplotlib.pyplot as plt

# Archive Rout
route = '../PE_Bandwidth'

# File list
archives = [f for f in os.listdir(route) if re.match(r'PE\d+_bandwidth', f)]

pes = []
bytes_transferred = []
time_spent = []

# Extraer datos de cada file
for file in archives:
    ruta_completa = os.path.join(route, file)
    with open(ruta_completa, 'r') as f:
        content = f.read()
        match_pe = re.search(r'PE\s*(\d+)', content)
        match_bytes = re.search(r'Total Bytes Transferred:\s*(\d+)', content)
        match_time = re.search(r'Total Time Spent \(s\):\s*([0-9.]+)', content)
        if match_pe and match_bytes and match_time:
            pe = int(match_pe.group(1))
            total_bytes = int(match_bytes.group(1))
            total_time = float(match_time.group(1))
            pes.append(pe)
            bytes_transferred.append(total_bytes)
            time_spent.append(total_time)

# Order data by PE
pes, bytes_transferred, time_spent = zip(*sorted(zip(pes, bytes_transferred, time_spent)))

# --- Plot 1: Total Bytes Transferred ---
plt.figure(figsize=(10, 6))
plt.bar([f'PE {pe}' for pe in pes], bytes_transferred, color='mediumseagreen')
plt.xlabel('Processing Element (PE)')
plt.ylabel('Total Bytes Transferred')
plt.title('Total Bytes Transferred by PE')
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.show()

# --- Plot 2: Total Time Spent ---
plt.figure(figsize=(10, 6))
plt.bar([f'PE {pe}' for pe in pes], time_spent, color='coral')
plt.xlabel('Processing Element (PE)')
plt.ylabel('Total Time Spent (s)')
plt.title('Tiempo total by PE')
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.show()
