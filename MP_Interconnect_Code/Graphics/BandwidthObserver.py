import os
import re
import matplotlib.pyplot as plt

# Archive route
route = '../PE_Bandwidth'

# Archive list
archives = [f for f in os.listdir(route) if re.match(r'PE\d+_bandwidth', f)]

pes = []
bandwidths = []

# Extract data from file
for file in archives:
    ruta_completa = os.path.join(route, file)
    with open(ruta_completa, 'r') as f:
        content = f.read()
        match_pe = re.search(r'PE\s*(\d+)', content)
        match_bw = re.search(r'Bandwidth \(Bytes/s\):\s*([0-9.]+)', content)
        if match_pe and match_bw:
            pe = int(match_pe.group(1))
            bw = float(match_bw.group(1))
            pes.append(pe)
            bandwidths.append(bw)

# Order data by PE
pes, bandwidths = zip(*sorted(zip(pes, bandwidths)))

# Plot the results
plt.figure(figsize=(10, 6))
plt.bar([f'PE {pe}' for pe in pes], bandwidths, color='skyblue')
plt.xlabel('Processing Element (PE)')
plt.ylabel('Bandwidth (Bytes/s)')
plt.title('Bandwidth by PE')
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.tight_layout()
plt.show()
