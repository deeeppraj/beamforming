# beamforming
# Complete Beginner's Guide: Versal AI Engine Simulation & Adaptive Beamforming
 
---
 
## PART 1: ABSOLUTE BASICS
 
### What is an FPGA? (You MUST understand this first)
 
An **FPGA** (Field-Programmable Gate Array) is like a Swiss Army knife of computing:
- A traditional computer has a **fixed** design (CPU, GPU, etc.)
- An FPGA is **programmable hardware** — you can reshape it to solve ANY problem
- You tell it: "be a signal processor," "be a neural network accelerator," "be a crypto engine"
- It reconfigures itself to do exactly what you want, at incredible speeds
**Analogy**: 
- CPU = Assembly line with fixed stations
- FPGA = Empty factory floor where YOU decide where each station goes
### What is Xilinx Versal?
 
**Versal** is Xilinx's newest FPGA architecture. It has **THREE different types of computing resources**:
 
```
┌─────────────────────────────────────────┐
│         Versal AI Core Device           │
├─────────────┬─────────────┬─────────────┤
│  AI Engines │   Scalar    │ Programmable│
│  (Matrix    │   Engines   │  Logic (PL) │
│   Math)     │  (General   │  (Flexible  │
│             │   Control)  │  Logic)     │
└─────────────┴─────────────┴─────────────┘
```
 
1. **AI Engines**: 32 MAC (Multiply-Accumulate) operations per clock
   - PERFECT for beamforming (lots of matrix math)
   
2. **Scalar Engines**: Control & general-purpose processing
   
3. **Programmable Logic (PL)**: Traditional FPGA for data routing
### What is Beamforming?
 
Imagine a concert stage with multiple speakers:
- **Traditional setup**: One speaker broadcasts to everyone (wasteful)
- **Smart setup**: Each speaker focuses sound to specific audience areas (efficient)
**5G Beamforming does the SAME THING with wireless signals:**
- Multiple antennas (like speakers)
- Send focused signals to specific users
- Receive from specific directions
- Uses **matrix math** to calculate the right phase/amplitude for each antenna
---
 
## PART 2: KEY CONCEPTS YOU NEED
 
### What is Simulation?
 
Before you deploy code to real hardware, you **test it virtually**:
 
```
Your Code (C) → Compile → Simulation Model → Test Results
                             (Virtual Hardware)
```
 
Think of it like:
- Writing flight simulator code BEFORE putting it in a real plane
- Testing a bridge design in a computer BEFORE building it
### Tool Chain Overview
 
Here's what you'll use:
 
```
┌──────────────────────────────────────────────────┐
│  1. Vitis Unified IDE                            │
│     (Xilinx's main development environment)      │
├──────────────────────────────────────────────────┤
│  2. Vitis Model Composer (or Vivado HLS)         │
│     (Write C/C++ for AI Engine)                  │
├──────────────────────────────────────────────────┤
│  3. AI Engine Compiler                           │
│     (Converts C → AI Engine instructions)        │
├──────────────────────────────────────────────────┤
│  4. AI Engine Simulator                          │
│     (Virtual execution environment)              │
├──────────────────────────────────────────────────┤
│  5. System Simulator (Optional: Hardware in loop)│
│     (Connects AI Engine to PL logic)             │
└──────────────────────────────────────────────────┘
```
 
### Important Terminology
 
| Term | Meaning |
|------|---------|
| **Kernel** | A C function that runs on AI Engine |
| **Graph** | Blueprint connecting multiple kernels |
| **Tile** | One AI Engine hardware unit |
| **Vector Processor** | The 32-MAC math engine inside AI Engine |
| **MAC** | Multiply-Accumulate (one math operation) |
| **GMAC** | Giga-MAC = 1 billion MACs per second |
| **CMAC** | Complex-MAC (multiply-accumulate of complex numbers) |
| **Cascade** | Chain AI Engines together for larger computations |
| **DMA** | Direct Memory Access (automated data movement) |
| **AXI** | Standard interface for moving data between components |
 
---
 
## PART 3: ADAPTIVE BEAMFORMING MATH (SIMPLIFIED)
 
### The Problem
 
You have:
- **N antennas** receiving/transmitting signals
- **M data streams** (users)
- **H matrix**: Contains the "steering" information (64 × 32 = 2,048 coefficients in 5G case)
### The Solution
 
**Downlink** (base station → users):
```
transmit_signals = H × data_streams
```
 
Each antenna's signal = weighted sum of all data streams
 
**Uplink** (users → base station):
```
data_streams = H† × received_signals
```
 
Extract multiple users from mixed antenna signals
 
### Why It's Hard
 
```
5G NR: 64 antennas × 32 streams × 100 MHz = 204.8 Billion operations per second!
```
 
This is why you need **specialized hardware** (AI Engine).
 
### Adaptive Beamforming
 
**Adaptive** = Automatically **adjusts H matrix** based on:
- Channel conditions (signal strength, reflections)
- User locations
- Interference
**Simple Algorithm (LMS - Least Mean Squares)**:
 
```
error = received_signal - expected_signal
H_new = H_old + learning_rate × error × signal^*
```
 
This gradually **adjusts H** to minimize error.
 
---
 
## PART 4: PRACTICAL PREREQUISITES
 
### Software You Need (FREE)
 
1. **Vivado Design Suite** (Free version available)
   - Download from Xilinx (xilinx.com)
   - Includes: Vivado, Vitis IDE, AI Engine tools
   - Size: ~30-50 GB (takes time to download)
2. **MATLAB** (Optional but helpful)
   - For signal processing & validation
   - Free alternative: GNU Octave
3. **Git** (Version control)
   ```bash
   sudo apt-get install git
   ```
 
### Hardware (Optional for real testing)
 
- **Versal VCK190 Evaluation Board** (~$3,000)
  - Good for learning, but expensive
  - For now, simulation is enough
### System Requirements
 
- **Linux** (Ubuntu 20.04 or later) - Xilinx tools run best on Linux
- **8+ GB RAM** (preferably 16 GB)
- **Fast SSD** (Vivado is huge)
- **Internet** (for downloading tools)
---
 
## PART 5: STEP-BY-STEP SIMULATION SETUP
 
### Step 1: Install Vivado & Vitis
 
**On Ubuntu:**
 
```bash
# Download Vivado installer from Xilinx website
# (Register and accept license)
 
# Extract the installer
tar -xzf Xilinx_Vivado_Vitis_2023.2_1013_1.tar.gz
 
# Run installer
cd Xilinx_Vivado_Vitis_2023.2_1013_1
./xsetup
 
# Follow GUI wizard:
# - Select "Vivado + Vitis + AI Engine"
# - Accept licenses
# - Choose installation directory (needs ~50 GB space)
# - Wait (can take 30+ minutes)
```
 
**After Installation:**
 
```bash
# Add to .bashrc (permanently)
echo 'source /opt/Xilinx/Vivado/2023.2/settings64.sh' >> ~/.bashrc
source ~/.bashrc
```
 
Verify installation:
```bash
vivado -version
xsim -version
```
 
### Step 2: Understand the Project Structure
 
For beamforming, you'll create this structure:
 
```
beamforming_project/
├── aie/                          # AI Engine code
│   ├── kernels/
│   │   ├── beamforming_kernel.cpp    # Main beamforming code
│   │   └── beamforming_kernel.h      # Function declarations
│   ├── graphs/
│   │   └── beamforming_graph.cpp     # Graph definition
│   └── compile/                      # Compiler outputs
│
├── system_files/                 # System integration (PL logic)
│   ├── platform.tcl
│   └── constraints.xdc
│
├── testbenches/
│   ├── test_input.txt            # Sample antenna signals
│   ├── test_reference.txt        # Expected output
│   └── testbench.cpp             # Validation code
│
└── scripts/
    ├── compile.sh                # Compile script
    ├── simulate.sh               # Run simulation
    └── compare_results.py        # Check correctness
```
 
---
 
## PART 6: SIMPLE BEAMFORMING EXAMPLE (START HERE)
 
### Example 1: Minimal "Hello World" Beamformer
 
This is a **tiny 4×2 beamformer** (4 antennas, 2 streams) to teach concepts.
 
**File: aie/kernels/simple_beamformer.cpp**
 
```cpp
#include <aie_api/aie.hpp>
#include <aie_api/utils.hpp>
 
using namespace aie;
 
// Beamforming kernel: multiply H matrix by X vector
// Input: H[4x2], X[2x1]
// Output: Y[4x1]
void beamformer_kernel(
    input_stream_cint16* H_stream,  // Coefficient matrix (4x2 complex numbers)
    input_stream_cint16* X_stream,  // Data stream (2 complex numbers)
    output_stream_cint16* Y_stream  // Output (4 complex results)
) {
    // Load coefficients into registers
    cint16 H[4][2];  // 4 antennas, 2 streams
    cint16 X[2];     // 2 data streams
    cint16 Y[4];     // 4 antenna outputs
    
    // Read coefficient matrix
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            H[i][j] = readincr(H_stream);  // Read from input stream
        }
    }
    
    // Read data vector
    for (int i = 0; i < 2; i++) {
        X[i] = readincr(X_stream);
    }
    
    // Compute: Y = H * X (matrix multiplication)
    for (int i = 0; i < 4; i++) {
        cint16 sum = {0, 0};  // Accumulator for antenna i
        
        for (int j = 0; j < 2; j++) {
            // Multiply H[i][j] * X[j] and accumulate
            cint16 product = H[i][j] * X[j];
            sum = sum + product;
        }
        
        Y[i] = sum;  // Store result for antenna i
        writeincr(Y_stream, Y[i]);  // Write to output stream
    }
}
```
 
**What this does (step by step):**
 
```
Input signals from 2 users:
X = [user1, user2]
 
Steering matrix (learned or pre-calculated):
H = [h00 h01]  ← antenna 0 gets these weights
    [h10 h11]  ← antenna 1
    [h20 h21]  ← antenna 2
    [h30 h31]  ← antenna 3
 
Beamforming computation:
antenna_0_output = h00*user1 + h01*user2
antenna_1_output = h10*user1 + h11*user2
antenna_2_output = h20*user1 + h21*user2
antenna_3_output = h30*user1 + h31*user2
 
Each antenna gets a weighted mix of both users (steered in specific direction)
```
 
### Example 2: Graph Definition
 
**File: aie/graphs/beamformer_graph.cpp**
 
```cpp
#include <aie_api/aie.hpp>
#include "simple_beamformer.cpp"  // Include kernel
 
using namespace aie;
 
// Graph class - connects kernels and ports
class BeamformerGraph : public graph {
public:
    // Input/Output ports
    input_port H_in;      // Coefficient matrix
    input_port X_in;      // Data stream
    output_port Y_out;    // Output stream
    
    // Constructor: define what kernels exist and how they connect
    BeamformerGraph() {
        // Create kernel instance
        kernel k1 = kernel::create(beamformer_kernel);
        
        // Connect I/O
        connect<stream> (H_in, k1.in[0]);      // H stream → kernel input 0
        connect<stream> (X_in, k1.in[1]);      // X stream → kernel input 1
        connect<stream> (k1.out[0], Y_out);    // kernel output 0 → Y stream
        
        // Specify which AI Engine tile this runs on
        location<kernel>(k1) = tile(0, 0);     // Use tile at column 0, row 0
        
        // Specify data buffer size in tile memory
        fifo_depth<k1.in[0]>(2);  // Buffer for H coefficients
        fifo_depth<k1.in[1]>(2);  // Buffer for X data
    }
};
```
 
**This says:**
- "I have a kernel (beamformer_kernel)"
- "Put it on AI Engine tile at position (0,0)"
- "Wire up three ports: H input, X input, Y output"
- "Allocate memory for data buffering"
### Example 3: Testbench (Validation)
 
**File: testbenches/simple_test.cpp**
 
```cpp
#include <iostream>
#include <complex>
#include <fstream>
using namespace std;
 
typedef complex<int16_t> cint16;
 
// Reference implementation (CPU version for comparison)
void beamformer_reference(
    cint16 H[4][2],
    cint16 X[2],
    cint16 Y[4]
) {
    for (int i = 0; i < 4; i++) {
        Y[i] = H[i][0] * X[0] + H[i][1] * X[1];
    }
}
 
int main() {
    // Test input data
    cint16 H[4][2] = {
        {{1, 0}, {0, 0}},  // antenna 0: takes 100% user 1
        {{0, 0}, {1, 0}},  // antenna 1: takes 100% user 2
        {{1, 0}, {1, 0}},  // antenna 2: takes 50% each
        {{1, 0}, {-1, 0}}  // antenna 3: phase difference
    };
    
    cint16 X[2] = {{10, 0}, {5, 0}};  // user 1 = 10, user 2 = 5
    
    cint16 Y_expected[4];
    cint16 Y_actual[4];
    
    // Compute with reference implementation
    beamformer_reference(H, X, Y_expected);
    
    // AFTER you run AI Engine simulation, load Y_actual from output file
    
    // Compare
    bool match = true;
    for (int i = 0; i < 4; i++) {
        if (Y_expected[i] != Y_actual[i]) {
            cout << "ERROR: antenna " << i << endl;
            cout << "  Expected: " << Y_expected[i] << endl;
            cout << "  Actual:   " << Y_actual[i] << endl;
            match = false;
        }
    }
    
    if (match) {
        cout << "✓ Test PASSED!" << endl;
    } else {
        cout << "✗ Test FAILED!" << endl;
    }
    
    return 0;
}
```
 
---
 
## PART 7: HOW TO ACTUALLY RUN THE SIMULATION
 
### Step 1: Create Project in Vitis
 
```bash
# Create new workspace
mkdir -p ~/versal_workspace
cd ~/versal_workspace
 
# Open Vitis IDE
vitis &
```
 
**In the GUI:**
1. File → New → Vitis Application Project
2. Select Platform: "Versal AI Core (VCK190)"
3. Create new platform if needed
4. Choose "Empty Application"
5. Click Finish
### Step 2: Add Your Code
 
**In Vitis Project Explorer:**
1. Right-click `src/` → New File → "beamformer.cpp"
2. Paste kernel code (from Example 1 above)
3. Right-click `src/` → New File → "graph.cpp"
4. Paste graph code (from Example 2 above)
### Step 3: Create Test Data
 
**File: testbenches/create_test_data.py**
 
```python
import numpy as np
import struct
 
# Create simple test data
H = np.array([
    [1+0j, 0+0j],
    [0+0j, 1+0j],
    [1+0j, 1+0j],
    [1+0j, -1+0j]
], dtype=np.complex64)
 
X = np.array([10+0j, 5+0j], dtype=np.complex64)
Y_expected = H @ X  # Matrix multiply
 
# Write to binary file
with open('input_H.bin', 'wb') as f:
    H.astype(np.complex64).tofile(f)
 
with open('input_X.bin', 'wb') as f:
    X.astype(np.complex64).tofile(f)
 
with open('output_expected.bin', 'wb') as f:
    Y_expected.astype(np.complex64).tofile(f)
 
print("Test data created!")
print(f"Expected output: {Y_expected}")
```
 
Run it:
```bash
cd testbenches
python3 create_test_data.py
```
 
### Step 4: Compile for Simulation
 
```bash
# In Vitis IDE, right-click project:
# AIE → Compile
 
# Or command line:
cd ~/versal_workspace
aiecompiler --target=sim \
    -v \
    --workdir=./aie.sim \
    aie/graphs/beamformer_graph.cpp
```
 
**What happens:**
```
Your C++ Code → Compiled to AI Engine instructions
                ↓
            Simulation model created
                ↓
            Ready to run!
```
 
### Step 5: Run the Simulator
 
```bash
# In aie.sim directory:
cd aie.sim
aiesimulator --input_file=../testbenches/input_H.bin \
             --input_file=../testbenches/input_X.bin \
             --output_file=beamformer_output.bin \
             --duration=1000ns
```
 
**What this does:**
- Feeds input data to virtual AI Engine
- Simulates execution instruction-by-instruction
- Records outputs
- Runs for 1000 nanoseconds (enough for matrix multiply)
### Step 6: Validate Results
 
```bash
python3 testbenches/compare_results.py
```
 
Create this file first:
 
**File: testbenches/compare_results.py**
 
```python
import numpy as np
 
# Load expected output
expected = np.fromfile('output_expected.bin', dtype=np.complex64)
 
# Load simulated output
actual = np.fromfile('../aie.sim/beamformer_output.bin', dtype=np.complex64)
 
print("Expected:", expected)
print("Actual:  ", actual)
 
# Check match (with small tolerance for rounding)
if np.allclose(expected, actual, rtol=1e-5):
    print("✓ Test PASSED!")
else:
    print("✗ Test FAILED!")
    print("Difference:", np.abs(expected - actual))
```
 
---
 
## PART 8: ADAPTIVE BEAMFORMING IMPLEMENTATION
 
### What Makes It "Adaptive"?
 
**Static:** H matrix is fixed (calculated once, never changes)
**Adaptive:** H matrix updates continuously based on received signals
 
### Simple Adaptive Algorithm (LMS)
 
```
Initialize H matrix (random or zero)
 
Loop forever:
    1. Receive antenna signals: Y_received
    2. Compute data streams: X_estimated = H† × Y_received
    3. Compare with reference: error = reference_signal - X_estimated
    4. Update H: H = H + step_size × error × Y_received†
    5. Go to step 1
```
 
The key: **error feedback adjusts H to minimize error**.
 
### Kernel Code for Adaptive Beamformer
 
**File: aie/kernels/adaptive_beamformer.cpp**
 
```cpp
#include <aie_api/aie.hpp>
 
using namespace aie;
 
// Adaptive beamformer kernel
// Updates H matrix based on error signal
void adaptive_beamformer_kernel(
    input_stream_cint16* Y_in,        // Received antenna signals (N antennas)
    input_stream_cint16* reference,   // Reference/training signal (M streams)
    input_plio* H_config,             // H matrix (stored in memory)
    output_stream_cint16* X_out,      // Extracted data (M streams)
    output_plio* H_new_config         // Updated H matrix
) {
    // AI Engine vector processor can do:
    // - 32 MACs per cycle (16-bit)
    // - Useful for: H matrix update, matrix multiply
    
    // Storage
    cint16 H[8][8];          // Coefficient matrix (can be larger)
    cint16 Y[8];             // Antenna signals
    cint16 X[8];             // Data streams
    cint16 reference_sig[8]; // Expected reference
    
    // Hyperparameters
    float step_size = 0.001;  // Learning rate
    
    // Main adaptive loop
    for (int iteration = 0; iteration < 1000; iteration++) {
        // Step 1: Read antenna signals
        for (int i = 0; i < 8; i++) {
            Y[i] = readincr(Y_in);
        }
        
        // Step 2: Compute data extraction: X = H† × Y
        // (Conjugate transpose of H times Y)
        for (int i = 0; i < 8; i++) {
            cint16 sum = {0, 0};
            for (int j = 0; j < 8; j++) {
                // H_conj_transpose[i][j] = conj(H[j][i])
                cint16 h_conj = {H[j][i].real(), -H[j][i].imag()};
                cint16 product = h_conj * Y[j];
                sum = sum + product;
            }
            X[i] = sum;
        }
        
        // Step 3: Read reference and compute error
        cint16 error[8];
        for (int i = 0; i < 8; i++) {
            reference_sig[i] = readincr(reference);
            error[i] = reference_sig[i] - X[i];  // error = reference - estimated
        }
        
        // Step 4: Update H matrix
        // H_new = H + step_size * error * Y†
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                // Y_conj[j] = conj(Y[j])
                cint16 y_conj = {Y[j].real(), -Y[j].imag()};
                
                // Scaled gradient: error[i] * y_conj[j] * step_size
                cint16 gradient = error[i] * y_conj;
                
                // Convert step_size to fixed-point and apply
                cint16 delta = {
                    (int16_t)(gradient.real() * step_size),
                    (int16_t)(gradient.imag() * step_size)
                };
                
                H[i][j] = H[i][j] + delta;
            }
        }
        
        // Write extracted data
        for (int i = 0; i < 8; i++) {
            writeincr(X_out, X[i]);
        }
    }
    
    // Write updated H matrix
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            writeincr(H_new_config, H[i][j]);
        }
    }
}
```
 
### Understanding the Algorithm Step by Step
 
```
ITERATION 1:
  Y = [-2, 5, 3, 1, ...]  ← Real antenna signals (noisy)
  H = [[0.5, 0.1],         ← Initial guess for H
       [0.2, 0.5],
       ...]
  X = H† × Y = [est_user1, est_user2, ...]  ← What we think the data is
  Reference = [1, 0, ...]  ← What it should be
  Error = [1 - est_user1, 0 - est_user2, ...]  ← How wrong we are
  H_new = H + step_size × error × Y†  ← Adjust H to reduce error
  
ITERATION 2:
  Use H_new (improved)
  Repeat above
  Error should be smaller!
 
ITERATION 3, 4, ...
  Keep repeating
  Eventually, error → 0
  H converges to optimal values
```
 
---
 
## PART 9: FULL SIMULATION WORKFLOW
 
### Complete Simulation Flow Diagram
 
```
┌─────────────────────────────────────────────────────────────┐
│ 1. MATLAB/Python: Generate Test Signals                     │
│    - Channel matrix (H)                                      │
│    - User signals (X)                                        │
│    - Antenna signals (Y)                                     │
│    - Save as binary files                                    │
└────────────────┬────────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────────┐
│ 2. Write AI Engine Code (C++)                               │
│    - beamformer_kernel.cpp                                  │
│    - beamformer_graph.cpp                                   │
│    - adaptive_beamformer.cpp (if adaptive)                  │
└────────────────┬────────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────────┐
│ 3. Compile for Simulation                                   │
│    $ aiecompiler --target=sim beamformer_graph.cpp          │
│    Creates: aie.sim/                                        │
└────────────────┬────────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────────┐
│ 4. Run AI Engine Simulator                                  │
│    $ aiesimulator --input_file=data.bin \                   │
│                   --output_file=result.bin \                │
│                   --duration=10000ns                         │
│                                                              │
│    Simulator executes AI Engine instruction-by-instruction  │
│    Records all outputs with timestamps                      │
└────────────────┬────────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────────┐
│ 5. Extract Results                                          │
│    - Read output binary files                               │
│    - Parse time-stamped data                                │
│    - Calculate throughput                                   │
└────────────────┬────────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────────┐
│ 6. Validate Against Reference (MATLAB/Python)              │
│    - Compare AI Engine output vs. MATLAB reference           │
│    - Check for bit-exact match (or close tolerance)          │
│    - Verify performance metrics                             │
└────────────────┬────────────────────────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────────────────────────┐
│ 7. Iterate (if errors)                                      │
│    - Debug C code                                           │
│    - Recompile                                              │
│    - Re-simulate                                            │
│    - Repeat until ✓ PASS                                    │
└─────────────────────────────────────────────────────────────┘
```
 
### Complete Script (Save as `full_simulation.sh`)
 
```bash
#!/bin/bash
 
# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color
 
PROJECT_DIR=$(pwd)
AIE_DIR=$PROJECT_DIR/aie
TEST_DIR=$PROJECT_DIR/testbenches
 
echo -e "${YELLOW}=== Versal Beamformer Simulation ===${NC}"
 
# Step 1: Generate test data
echo -e "${YELLOW}[1/5] Generating test data...${NC}"
cd $TEST_DIR
python3 create_test_data.py
if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Test data generation failed${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Test data created${NC}"
 
# Step 2: Compile for simulation
echo -e "${YELLOW}[2/5] Compiling AI Engine code...${NC}"
cd $AIE_DIR
aiecompiler --target=sim \
    -v \
    --workdir=./aie.sim \
    graphs/beamformer_graph.cpp
 
if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Compilation failed${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Compilation successful${NC}"
 
# Step 3: Run simulator
echo -e "${YELLOW}[3/5] Running AI Engine simulator...${NC}"
cd $AIE_DIR/aie.sim
aiesimulator --input_file=../../testbenches/input_H.bin \
             --input_file=../../testbenches/input_X.bin \
             --output_file=output_Y.bin \
             --duration=10000ns
 
if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Simulation failed${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Simulation completed${NC}"
 
# Step 4: Extract and validate results
echo -e "${YELLOW}[4/5] Validating results...${NC}"
python3 ../../testbenches/compare_results.py
 
# Step 5: Generate report
echo -e "${YELLOW}[5/5] Generating report...${NC}"
echo "Simulation completed on $(date)" > simulation_report.txt
echo "See aiesimulator.log for detailed trace" >> simulation_report.txt
 
echo -e "${GREEN}=== Simulation Completed Successfully ===${NC}"
```
 
Make executable and run:
```bash
chmod +x full_simulation.sh
./full_simulation.sh
```
 
---
 
## PART 10: DEBUGGING & PROFILING
 
### If Your Simulation Fails...
 
**Problem 1: Compilation Error**
```
Error: "undeclared variable"
```
 
**Solution:**
```cpp
// Make sure every variable is declared BEFORE use
cint16 sum = {0, 0};  // Initialize accumulator!
```
 
**Problem 2: Simulation Won't Start**
```
Error: "Input file not found"
```
 
**Solution:**
```bash
# Check paths are absolute or relative to simulator location
ls -la input_H.bin  # Must exist in current directory
```
 
**Problem 3: Output is All Zeros**
```
Expected: [10, 5, 15, 5]
Actual:   [0, 0, 0, 0]
```
 
**Debugging:**
```cpp
// Add debug output to your kernel
printf("H[0][0] = %d\n", H[0][0].real());
printf("X[0] = %d\n", X[0].real());
printf("Y[0] = %d\n", Y[0].real());
```
 
Recompile and check console output.
 
### Checking Performance
 
After simulation, view trace:
```bash
# All timing information
cat aiesimulator.log | grep "Cycle:"
 
# Find bottleneck
cat aiesimulator.log | grep "DMA\|Memory" | head -20
```
 
### Profiling Tools
 
Vitis includes **profiler**:
```bash
# Generate profile data
cd aie.sim
aiesimulator --profile ...
 
# View in GUI
vitis -profiling aie.sim/profiling_data
```
 
---
 
## PART 11: SCALING UP (5G CASE)
 
### From 4×2 to 64×32 Beamformer
 
**The Challenge:**
- Small version (4×2): Easy to fit in one AI Engine
- Real 5G (64×32): Need CASCADED AI Engines
**The Solution:**
 
```
Input: 64×32 matrix multiply = 64×32×32 = 65,536 MACs
 
Break into smaller pieces:
64×32 = (8×8) × (8×12)  ← Each piece fits in ONE AI Engine!
 
Then cascade 8 AI Engines:
┌─────────────┐   ┌─────────────┐       ┌─────────────┐
│ AI Engine 0 │──→│ AI Engine 1 │──→...→│ AI Engine 7 │
│ (8×8)×(8×12)│   │ (8×8)×(8×12)│       │ (8×8)×(8×12)│
└─────────────┘   └─────────────┘       └─────────────┘
     ↓                  ↓                      ↓
  Partial           Partial                Final
  Result            Result                 Result
```
 
**Updated Graph Code:**
 
```cpp
class BeamformerGraph5G : public graph {
public:
    input_port H_in;
    input_port X_in;
    output_port Y_out;
    
    BeamformerGraph5G() {
        // Create 8 kernel instances (cascaded)
        kernel k[8];
        
        for (int i = 0; i < 8; i++) {
            k[i] = kernel::create(beamformer_kernel_8x8);
            
            // Cascade connections
            if (i == 0) {
                // First kernel: external inputs
                connect<stream>(H_in, k[i].in[0]);
                connect<stream>(X_in, k[i].in[1]);
            } else {
                // Middle kernels: receive cascade from previous
                connect<cascade>(k[i-1].out[1], k[i].in[2]);
            }
            
            if (i == 7) {
                // Last kernel: external output
                connect<stream>(k[i].out[0], Y_out);
            }
            
            // Place on different tiles (arranged in array)
            location<kernel>(k[i]) = tile(i % 4, i / 4);
        }
    }
};
```
 
---
 
## PART 12: REAL HARDWARE DEPLOYMENT (After Simulation Works!)
 
### When Simulation is Complete
 
```
✓ Simulator says: Your design works!
         ↓
  Next step: Test on real hardware (Optional)
```
 
### Steps to Hardware
 
1. **Add PL (Programmable Logic) wrapper**
   - Connects AI Engine to external AXI interfaces
   - Handles data buffering
2. **Run System Simulation**
   - Simulates AI Engine + PL together
   - Checks data flow
3. **Synthesize & Place & Route**
   - Compile to actual FPGA bitstream
   - Takes hours on full designs
4. **Load on VCK190 Board**
   - Flash bitstream to board
   - Stream real antenna data
   - See adaptive algorithm working in real-time!
For now, **simulation is sufficient** to validate your algorithm.
 
---
 
## SUMMARY & NEXT STEPS
 
### You've Learned:
 
1. ✓ FPGA & Versal basics
2. ✓ Beamforming math (simple version)
3. ✓ How to write AI Engine kernels in C++
4. ✓ How to create graphs
5. ✓ How to simulate end-to-end
6. ✓ Adaptive beamforming concept
### Your Next Actions:
 
**Immediate (This Week):**
1. Install Vivado/Vitis
2. Copy code examples above
3. Run the 4×2 beamformer simulation
4. Verify "Test PASSED"
**Short Term (Next 2-4 Weeks):**
1. Modify code to 8×8 case
2. Test adaptive algorithm
3. Check performance metrics
4. Compare with MATLAB reference
**Long Term (1-3 Months):**
1. Scale to 64×32 with cascading
2. Add PL logic for data routing
3. Test system simulation
4. (Optional) Deploy to VCK190 board
### Resources to Study
 
- **Official:** Xilinx AI Engine documentation
  - https://xilinx.github.io/Vitis_Accel_Examples/2022.2/
- **DSP Background:**
  - "Digital Signal Processing" - Proakis & Manolakis (textbook)
- **Beamforming Theory:**
  - "Signal Processing for Communications" - Blahut
- **FPGA Basics:**
  - "FPGA Prototyping by SystemVerilog Examples" - Chu (optional, for PL)
