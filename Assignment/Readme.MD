# Limit Order Book Simulation in C++17

**Assignment by:** Keshav Makkad -- 23BCS10002

**Assignment details:**- https://docs.google.com/document/d/1vQYv7E019rpnH6kcB3AQQhFXoehA30b2QVWKSxlEv7k/edit?tab=t.0#heading=h.t7g6yetiucc8

## Features Implemented

- **Add Order** – Insert new buy/sell orders into the book
- **Cancel Order** – Cancel an existing order by ID
- **Amend Order** – Update an order’s price or quantity
- **Get Order Book Snapshot** – View top N bid and ask levels with aggregated quantities
- **Print Order Book** – Displays both aggregated price levels and individual orders for buy and sell sides
- **Automatic Matching** – Matches buy and sell orders whenever a new order is added or amended
- **FIFO Within Price Level** – Orders at the same price follow first-in-first-out priority
- **Order Lookup Table** – O(1) access to orders by ID

## How to Run

1. **Compile the code using C++17:**

```bash
g++ -std=c++17 main.cpp Orderbook.cpp -o orderbook
```

2. **Run the executable:**

```bash
./orderbook
```