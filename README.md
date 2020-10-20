# MemLuv
An HLS-synthesizable Dynamic Memory Manager for FPGAs

The MemLuv library develops a dynamic memory manager (DMM) for effective resource utilization in many-accelerator architectures targeting to FPGA devices.  The library is developed in C99-complient C language and it is fully synthesizable with the Xilinx Vivado High-Level-Synthesis (HLS) tool, so far.

The library is customizable so that it tradeoffs memory allocation timing overhead and FPGA resources, i.e. FFs, LUTs, DSP48s and BRAMs.

The library has been evaluated with a set of key accelerators from emerging application domains. MemLuv delivers significant gains in FPGA's accelerators density, i.e. 3.8 × , and application throughput up to 3.1 × and 21.4 × for shared and private memory accelerators.

For any questions/suggestions/bug reporting you may contact MemLuv's source code main author Dionysios Diamantopoulos at "dionisios.diamantopoulos@REMOVETHIS gmail.com" .

Compilation steps :construction:: 

1. Clone repo & create temporary directory for Cmake
  * git clone https://github.com/diamantopoulos/memluv.git memluv
  * Navigate to top-level directory, i.e. cd ~/<...>/memluv
  * mkdir build && cd build
2. Compile source code
  * Create make-targets automatically with supplied CmakeLists.
    * cmake ../
  * Compile hardware heaps
    * make memluv_wrappers
  * Compile accelerators
    * make accelerators
3. Test MemLuv
  * Compile a testbench
    * make test-accelerators 
  * Analyze MemLuv's log report
    * vim memluv_stats_heap-<id>.txt
    * make pyplot
4. Synthesize MemLuv with Xilinx Vivado HLS
  * make vivado_accelerators

## Publications
* Dionysios Diamantopoulos, Sotirios Xydis, Kostas Siozios, Dimitrios Soudris, Dynamic Memory Management in Vivado-HLS for Scalable Many-Accelerator Architectures. ARC 2015, https://link.springer.com/chapter/10.1007/978-3-319-16214-0_10
* Dionysios Diamantopoulos, Sotirios Xydis, Kostas Siozios, Dimitrios Soudris, Mitigating Memory-Induced Dark Silicon in Many-Accelerator Architectures. IEEE Comput. Archit. Lett. 2015, https://ieeexplore.ieee.org/abstract/document/7056472

The Unixverse segfaults on Tue, 19 Jan 2038^e^3 03:14:07 +0000. Hurry up! 
