#Experiemnt comparing FIFO AND LRU page replacement algo

import subprocess
import matplotlib.pyplot as plt

FRAME_COUNTS = [32, 64, 128, 256, 512, 1024] # different frame count nums that are being tested doubling starting from 32
ADDRESS_FILE = "addresses.txt"

#this is the most important section that prevents the needs of making tedious changes to our mains
#main2.c & main3c have 128 frames written in to the code, this defintion makes a copy of our mains where every
#128 is swapped for the frame count we want to test.
def run_with_frames(source_file, frames):
    with open(source_file) as f:
        code = f.read()
    code = code.replace("128", str(frames))
    with open("temp.c", "w") as f:
        f.write(code)

    subprocess.run(["gcc", "temp.c", "-o", "temp"], check=True) #stopper if theres compiling failure

    output = subprocess.run(["./temp", ADDRESS_FILE], capture_output=True, text=True).stdout

    faults = 0
    rate = 0

    #part of output
    for line in output.split("\n"):
        if line.startswith("Page Faults:"):
            faults = int(line.split(":")[1])

        if line.startswith("Page Fault Rate:"):
            rate = float(line.split(":")[1])

    return faults, rate

#creation of lists to collect the results as the script is executed
fifo_faults = []
fifo_rates = []
lru_faults = []
lru_rates = []

for frames in FRAME_COUNTS:
    faults, rate = run_with_frames("main2.c", frames) #FIFO
    fifo_faults.append(faults)
    fifo_rates.append(rate)

    faults, rate = run_with_frames("main3.c", frames) #LRU
    lru_faults.append(faults)
    lru_rates.append(rate)

for i in range(len(FRAME_COUNTS)):
    print(f"Frames: {FRAME_COUNTS[i]}, FIFO faults: {fifo_faults[i]}, FIFO rate: {fifo_rates[i]:.3f} | LRU faults: {lru_faults[i]}, LRU rate: {lru_rates[i]:.3f}")

labels = [str(frames) for frames in FRAME_COUNTS]
fig, (left, right) = plt.subplots(1, 2, figsize=(12, 5)) #allows for two charts to be created and put side by side

left.plot(labels, fifo_faults, marker="o", label="FIFO")
left.plot(labels, lru_faults, marker="o", label="LRU")

left.set_xlabel("Number of frames")
left.set_ylabel("Page faults")
left.set_title("Page fault vs number of frames")
left.legend()

right.plot(labels, fifo_rates, marker="o", label="FIFO")
right.plot(labels, lru_rates, marker="o", label="LRU")

right.set_xlabel("Number of frames")
right.set_ylabel("Page fault rate")

right.set_title("Page fault rate vs number of frames")
right.legend()

plt.tight_layout() #stops our labels from overlapping since our result graph is more crowded than project 1
plt.savefig("fifo_vs_lru.png")

print("Saved plot to fifo_vs_lru.png")