import pickle
import matplotlib.pyplot as plt
import numpy as np

# Load Coffea histogram
with open("flip_probs_topcoffea_UL17.pkl.gz", "rb") as f:
    h = pickle.load(f)

# Extract numpy arrays from the histogram
# For a 2D hist: h.values() → dict with arrays, h.axis(name).edges() → bin edges
values = list(h.values().values())[0]
pt_edges = h.axis("pt").edges()
eta_edges = h.axis("eta").edges()

# Plot 2D heatmap
plt.figure(figsize=(8,6))
mesh = plt.pcolormesh(pt_edges, eta_edges, values.T, cmap="viridis", shading="auto")
plt.xlabel(r"Electron $p_T$ [GeV]")
plt.ylabel(r"$|\eta|$")
plt.title("Charge MisID Rate")
cbar = plt.colorbar(mesh)
cbar.set_label("Rate")
plt.tight_layout()
plt.show()
