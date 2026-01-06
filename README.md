# VenomEngine: Ultra-Low Latency LOB & Risk Suite

## 1. Project Overview & Objectives
VenomEngine is a high-performance, C++23-based matching engine and quantitative risk suite. It is designed to simulate the environment of a High-Frequency Trading (HFT) firm, focusing on nanosecond-level execution and hardware-aware mathematical computations.

### Key Objectives:
* **Deterministic Latency:** Sub-microsecond order matching via cache-optimized data structures.
* **Quantitative Signal Generation:** Real-time calculation of **VPIN** (Volume-synchronized Probability of Informed Trading) using SIMD.
* **HPC Integration:** Utilization of Lock-free SPSC queues, CPU pinning, and AVX-512 intrinsics.

---

## 2. Project Etymology: Why "Venom"?
The name **VenomEngine** is a play on the project's primary mathematical focus: **Order Flow Toxicity**.

* **The Concept:** In high-frequency markets, "Toxic Flow" occurs when liquidity providers are adversely selected by informed traders. 
* **The "Venom":** This engine is specifically architected to detect the "venom" (toxic informed flow) in real-time using the **VPIN** metric.
* **The Engine:** Just as an antivenom must act instantly, this engine utilizes **AVX-512 vectorization** to process volume buckets at hardware speeds, allowing the system to identify toxic shifts in the limit order book before price discovery "bites" the market maker.

---

## 3. Technical Architecture

### A. Memory Management
* **Zero-Allocation Policy:** No `std::malloc`, `new`, or smart pointers (`std::shared_ptr`) are permitted during the matching loop. 
* **Memory Pooling:** All `Order` objects must be retrieved from a pre-allocated `OrderPool` (Arena).
* **Data-Oriented Design (DOD):** Structures are designed to fit within a 64-byte L1 cache line to prevent cache misses and false sharing.

### B. Core Data Structures
* **Intrusive Doubly-Linked List:** Price levels maintain `head` and `tail` indices. Orders store `next_idx` and `prev_idx` as `int32_t` to save space and avoid pointer chasing.
* **Sparse Array Price Ladder:** Indexing price levels via $O(1)$ direct mapping rather than binary search trees.
* **Bitset Optimization:** Uses `std::bitset` combined with `std::countr_zero` (hardware-accelerated TZCNT) to identify the best bid/ask instantly.

---

## 4. Mathematical Specifications
The risk engine calculates **VPIN** to measure order flow toxicity. 

### VPIN Derivation
The engine groups trade flow into volume buckets of size $V$. The VPIN metric is calculated as:
$$VPIN = \frac{\sum_{\tau=1}^n |V_\tau^B - V_\tau^S|}{n \cdot V}$$

### Vectorization Strategy
* **SIMD Batching:** Volume imbalances across 16+ buckets are calculated in parallel using **AVX-512** registers.
* **Floating Point Precision:** Use `float` for SIMD throughput unless `double` is strictly required for precision.

---

## 5. Hardware & Concurrency Layout
The system is partitioned into isolated execution cores to prevent OS context switching.

| Component | Responsibility | Technical Requirement |
| :--- | :--- | :--- |
| **Matching Engine** | Order entry, Price-Time priority | Core Pinning, `std::atomic` |
| **SPSC Queue** | Inter-thread message passing | Lock-free, Cache-line padding |
| **Risk Engine** | VPIN, Micro-price, Greeks | AVX-512, SIMD Intrinsics |
| **Market Feed** | UDP/TCP Simulation | `mmap` / Shared Memory |
