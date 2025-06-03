# Magical Map — CMPE250 Project 3

This project simulates a **dynamic pathfinding system** using **Dijkstra's algorithm** on a magical map grid. The world is gradually revealed, obstacles appear during traversal, and strategic decisions must be made using wizard offers to unlock blocked paths.

## 🧭 Overview

- The map is a 2D grid with different node types, each with passability and directional cost properties.
- The agent begins at a known start position and must reach a sequence of **mission objectives** (targets).
- As the agent moves, new obstacles may be revealed, triggering **real-time path recalculations**.
- In some cases, a **wizard** offers magical options to remove obstacles by converting node types.
- The system uses **Dijkstra’s shortest path algorithm** to compute the optimal path based on edge costs and visibility.


## 🔍 Key Features

- ✅ Dynamic grid updates as the agent discovers new obstacles
- ✅ Recalculates shortest path on-the-fly using Dijkstra
- ✅ Wizard interaction logic with decision-making for optimal cost
- ✅ Modular code: Dijkstra class, node system, and objective tracking
- ✅ Cost-based neighbor traversal using directional distances

## 🧠 Core Logic — Dijkstra's Algorithm

```java
// Initialize distances and previous node tracker
double[][] distances = new double[rows][cols];
Node[][] previous = new Node[rows][cols];

// MinHeap with custom comparator based on distances
MinHeap myMinHeap = new MinHeap(Comparator.comparingDouble(n -> distances[n.x][n.y]));

// Start from source node
distances[startX][startY] = 0;
myMinHeap.add(grid[startX][startY]);

// Main loop
while (!myMinHeap.isEmpty()) {
    Node current = myMinHeap.RemoveMin();
    if (current is target) break;

    for (neighbor in neighbors) {
        if (neighbor is passable) {
            double newDist = distances[current] + getDistance(current, neighbor);
            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist;
                previous[neighbor] = current;
                myMinHeap.add(neighbor);
            }
        }
    }
}

