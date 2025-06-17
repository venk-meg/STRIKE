#!/usr/bin/env python3
import csv
import os
import matplotlib.pyplot as plt
from collections import defaultdict

# ── EDIT THESE ───────────────────────────────────────────────────────────────
FILES = [
    "make_fist.csv",
    "release_fist.csv",
    "listen.csv",
    "next_recording.csv",
    "previous_recording.csv",
    "delete_recording.csv",
    "slice.csv",
    "select_end.csv",
    "select_beginning.csv",
    "next_harmony.csv",
    "previous_harmony.csv"
]
# ───────────────────────────────────────────────────────────────────────────────

# map number of components → labels
COMP_LABELS = {
    3: ["X", "Y", "Z"],
    4: ["w", "x", "y", "z"]
}

def load_sensor_data(csv_file):
    """
    Returns a dict: { channel_name: [ (time, [v1, v2, ...]), ... ] }
    """
    data = defaultdict(list)
    if not os.path.exists(csv_file):
        print(f"❌ File not found: {csv_file}")
        return data

    with open(csv_file, newline='') as f:
        reader = csv.reader(f)
        for row in reader:
            if len(row) < 3:
                continue
            try:
                t = float(row[0])
                ch = row[1]
                vals = [float(x) for x in row[2:]]
            except ValueError:
                continue
            data[ch].append((t, vals))
    return data

def main():
    # 1) load each file into its own sensor‐dict
    all_data = [load_sensor_data(fn) for fn in FILES]

    # 2) find each file’s baseline (smallest timestamp across all channels)
    baselines = []
    for data in all_data:
        all_times = [t for entries in data.values() for t, _ in entries]
        baselines.append(min(all_times) if all_times else 0.0)

    # 3) collect superset of channels
    channels = sorted({ ch for data in all_data for ch in data.keys() })
    if not channels:
        print("❌ No sensor data found in any files.")
        return

    n_files = len(FILES)
    n_ch    = len(channels)

    # 4) create a grid: rows = channels, cols = files
    fig, axes = plt.subplots(
        n_ch, n_files,
        sharex=True,
        figsize=(4*n_files, 2.5*n_ch),
        squeeze=False
    )

    for col, (fn, data, base) in enumerate(zip(FILES, all_data, baselines)):
        for row, ch in enumerate(channels):
            ax = axes[row][col]
            entries = data.get(ch, [])
            if entries:
                # rebase times
                times = [t - base for t, _ in entries]
                comps = list(zip(*[vals for _, vals in entries]))
                labels = COMP_LABELS.get(len(comps), [f"c{i}" for i in range(len(comps))])
                for ci, series in enumerate(comps):
                    ax.plot(times, series, label=labels[ci])
                ax.legend(fontsize=6, loc="upper right", ncol=1)
            ax.set_ylabel(ch, fontsize=8)
            if row == 0:
                ax.set_title(os.path.basename(fn), fontsize=10)
            if row == n_ch - 1:
                ax.set_xlabel("Time (ms, rebased)")
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
