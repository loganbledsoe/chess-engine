# chess-engine
A work in progress chess engine written in C.

Features:
- bitboard move generation
- fail-soft alpha beta search with MVV-LVA move ordering
- transposition table
- fen parsing
- piece square tables for evaluation

Planned features:
- Iterative deepening
- Principal variation search
- Killer moves and history heuristic for move ordering
- Aspiration windows
- Null move pruning
- Late move reduction
- Futility Pruning
- Evaluation function overhaul
- UCI support
- Code cleanup