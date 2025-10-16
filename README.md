# Multithreaded Monte Carlo Tree Search (MCTS) for Hex

Written for a group project in my third year of university.

This project implements a **multithreaded Monte Carlo Tree Search (MCTS)** AI for the board game **Hex**.  
It uses multi-core parallelisation to accelerate the search process and supports **RAVE (Rapid Action Value Estimation)**, **UCB1**, and **AMAF (All Moves As First)** updates for efficient decision-making.

---

- **Full Monte Carlo Tree Search** implementation:
  - Selection → Expansion → Simulation → Backpropagation
- **Thread-parallel exploration** using C++11 threads
- **RAVE / AMAF scoring** for faster convergence
- **Automatic best-move selection** using UCB-based metrics
- **Dynamic time control** (configurable duration in seconds)
- **Thread-safe node expansion and backpropagation**
- **Works on any Hex board size** supported by `HexGrid`
- Modular design with `Node`, `HexGrid`, and `Coord` abstractions
