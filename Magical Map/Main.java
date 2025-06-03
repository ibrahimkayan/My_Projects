import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.Collections;

// MinHeap class for managing elements in a sorted order
class MinHeap{
    private final ArrayList<Node> myHeap;// Represents the MinHeap as an ArrayList
    private final Comparator<Node> comparator;// Comparator for element ordering

    public MinHeap(Comparator<Node> comparator) {
        this.myHeap = new ArrayList<>();
        this.comparator = comparator;
    }

    //Method to insert an element to heap
    public void add(Node element) {
        myHeap.add(element);
        percolateUp(myHeap.size() - 1);
    }
    // Retrieves and removes the smallest element in the MinHeap
    public Node RemoveMin() {
        if (myHeap.isEmpty()) {
            return null;// Returns null if the heap is empty
        }

        Node rootElement = myHeap.get(0);
        Node lastElement = myHeap.remove(myHeap.size() - 1);// Removes the last element

        if (!myHeap.isEmpty()) {
            myHeap.set(0, lastElement);
            percolateDown(0);// Adjusts the heap to maintain the heap property
        }

        return rootElement;
    }

    // Checks if the heap is empty
    public boolean isEmpty() {
        return myHeap.isEmpty();
    }

    // Restores the heap property by moving an element up the tree
    private void percolateUp(int index) {
        int parentIndex = (index - 1) / 2;//parent index

        while (index > 0 && comparator.compare(myHeap.get(index), myHeap.get(parentIndex)) < 0) {
            swapElements(index, parentIndex);// Swaps the element with its parent
            index = parentIndex;
            parentIndex = (index - 1) / 2;
        }
    }

    // Restores the heap property by moving an element down the tree
    private void percolateDown(int index) {
        int leftChild = 2 * index + 1;//index of left child
        int rightChild = 2 * index + 2;//index of right child
        int smallestElem = index; // Tracks the smallest element's index
        // Compares the current element with its left child
        if (leftChild < myHeap.size() && comparator.compare(myHeap.get(leftChild), myHeap.get(smallestElem)) < 0) {
            smallestElem = leftChild;
        }
        // Compares the current smallest with the right child
        if (rightChild < myHeap.size() && comparator.compare(myHeap.get(rightChild), myHeap.get(smallestElem)) < 0) {
            smallestElem = rightChild;
        }

        if (smallestElem != index) {
            swapElements(index, smallestElem);// Swaps the element with the smaller child
            percolateDown(smallestElem);// Continues adjusting down the tree
        }
    }
    // Method to swap two elements in the heap
    private void swapElements(int i, int j) {
        Node temp = myHeap.get(i);
        myHeap.set(i, myHeap.get(j));
        myHeap.set(j, temp);
    }
}

// Represents a grid node with coordinates, type, and various properties
class Node {
    int x, y;//Coordinates of the node
    int nodeType;//Node type
    double cost;//Cost or weight of the node
    boolean isPassable;//Indıcates if the node can be passable
    double rightDistance, leftDistance, upDistance, downDistance;//Distance to the neighbor nodes

    public Node(int x, int y, int nodeType) {
        this.x = x;
        this.y = y;
        this.nodeType = nodeType;
        this.cost = Double.MAX_VALUE; // Initially set to infinity
        this.isPassable = nodeType != 1; // Only nodeType == 1 is impassable initially
        this.rightDistance = 100000001;
        this.leftDistance = 100000001;
        this.upDistance = 100000001;
        this.downDistance = 100000001;
    }
}
// Class to represent an objective with coordinates and wizard options
class Objective {
    int x, y; // Coordinates of the objective
    ArrayList<Integer> wizardOptions;// List of wizard options for node transformations

    public Objective(int x, int y) {
        this.x = x;
        this.y = y;
        this.wizardOptions = new ArrayList<>();
    }

    // Checks if there are wizard options available
    public boolean hasWizardOffer() {
        return !wizardOptions.isEmpty();
    }
}

public class Main {

    static int absoluteStart_X;
    static int absoluteStart_Y;
    static boolean recalculated = false;
    static Node[][] grid;// The grid containing all nodes
    static ArrayList<Objective> objectives = new ArrayList<>(); // List of objectives to complete
    static int radius;// Visibility radius for obstacle updates
    static int startX, startY;// Current starting position

    public static void main(String[] args) throws IOException {

        // Reading input files for nodes, edges, and objectives
        BufferedReader landReader = new BufferedReader(new FileReader(args[0]));
        BufferedReader edgeReader = new BufferedReader(new FileReader(args[1]));
        BufferedReader objectiveReader = new BufferedReader(new FileReader(args[2]));
        PrintWriter outputWriter = new PrintWriter(args[3]);

        // Reading grid dimensions
        String[] dimensions = landReader.readLine().split(" ");
        int rows = Integer.parseInt(dimensions[0]);
        int cols = Integer.parseInt(dimensions[1]);
        grid = new Node[rows][cols];

        // Reading nodes and initializing the grid
        String line;
        while ((line = landReader.readLine()) != null) {
            String[] parts = line.split(" ");
            int x = Integer.parseInt(parts[0]);
            int y = Integer.parseInt(parts[1]);
            int nodeType = Integer.parseInt(parts[2]);
            grid[x][y] = new Node(x, y, nodeType);
        }
        landReader.close();

        // Reading edges and setting distances between connected nodes
        while ((line = edgeReader.readLine()) != null) {
            String[] parts = line.split(" ");
            String[] node1Coords = parts[0].split(",")[0].split("-");
            String[] node2Coords = parts[0].split(",")[1].split("-");

            double distance = Double.parseDouble(parts[1]);

            // Coordinates for nodes
            int x1 = Integer.parseInt(node1Coords[0]);
            int y1 = Integer.parseInt(node1Coords[1]);
            int x2 = Integer.parseInt(node2Coords[0]);
            int y2 = Integer.parseInt(node2Coords[1]);

            // Determine the direction of the edge and assign distances accordingly
            if (x1 + 1 == x2 && y1 == y2) {
                grid[x1][y1].rightDistance = distance;
                grid[x2][y2].leftDistance = distance;
            } else if (x1 - 1 == x2 && y1 == y2) {
                grid[x1][y1].leftDistance = distance;
                grid[x2][y2].rightDistance = distance;
            } else if (x1 == x2 && y1 - 1 == y2) {
                grid[x1][y1].downDistance = distance;
                grid[x2][y2].upDistance = distance;
            } else if (x1 == x2 && y1 + 1 == y2) {
                grid[x1][y1].upDistance = distance;
                grid[x2][y2].downDistance = distance;
            }
        }
        edgeReader.close();

        // Parse objective radius and starting point
        //StartX and StartY will be used for any path finding step
        //AbsoluteX and AbsoluteY represent just the first start point of whole objectives
        radius = Integer.parseInt(objectiveReader.readLine().trim());
        String[] startCoords = objectiveReader.readLine().split(" ");
        startX = Integer.parseInt(startCoords[0]);
        startY = Integer.parseInt(startCoords[1]);
        absoluteStart_X = startX;
        absoluteStart_Y = startY;
        // Populate objectives with wizard options
        //Add each objective to the list with the coordinates and options
        while ((line = objectiveReader.readLine()) != null) {
            String[] parts = line.split(" ");
            int x = Integer.parseInt(parts[0]);
            int y = Integer.parseInt(parts[1]);
            Objective obj = new Objective(x, y);
            for (int i = 2; i < parts.length; i++) {
                obj.wizardOptions.add(Integer.parseInt(parts[i]));
            }
            objectives.add(obj);
        }
        objectiveReader.close();

        // Initialize obstacle detection based on radius
        //Before the move start check if any obstacle is around the point (according to the radius)
        for (int i = -radius; i <= radius; i++) {
            for (int j = -radius; j <= radius; j++) {
                int nx = absoluteStart_X + i;
                int ny = absoluteStart_Y + j;
                if (nx >= 0 && nx < grid.length && ny >= 0 && ny < grid[0].length) {
                    Node neighbor = grid[nx][ny];
                    double distance = Math.sqrt(i * i + j * j);

                    if (distance <= radius && neighbor.nodeType >= 2 && neighbor.isPassable) {
                        neighbor.isPassable = false;
                    }
                }
            }
        }

        // Process each objective sequentially
        for (Objective obj : objectives) {
            boolean reached = processObjective(obj, outputWriter);//Main process management method
            if (!reached) {
                break;
            }
        }
        outputWriter.close();
    }
    //Method to detect the new obstacles step by step on the sight with given radius
    static void updateVisibleObstacles(Node current, ArrayList<Node> currentPath,PrintWriter outputWriter) {
        boolean obstacleOnPath = false;
        // Iterate through all nodes within the defined radius
        for (int i = -radius; i <= radius; i++) {
            for (int j = -radius; j <= radius; j++) {
                int nx = current.x + i;
                int ny = current.y + j;
                // Check if the neighboring node is within the grid boundaries
                if (nx >= 0 && nx < grid.length && ny >= 0 && ny < grid[0].length) {
                    Node neighbor = grid[nx][ny];
                    double distance = Math.sqrt(i * i + j * j);
                    // Mark nodes as impassable if within the radius and of specific types
                    if (distance <= radius && neighbor.nodeType >= 2 && neighbor.isPassable) {
                        neighbor.isPassable = false;
                        // Check if the obstacle is on the current path
                        if (currentPath.contains(neighbor)) {
                            obstacleOnPath = true;
                        }
                    }
                }
            }
        }
        // Handle cases where an obstacle makes the path impassable
        if(obstacleOnPath){
            if(current.x == absoluteStart_X && current.y == absoluteStart_Y){
                recalculated=true;// Trigger recalculation if at the starting position
            }
            else
            {
                outputWriter.println("Path is impassable!");
                recalculated = true;
            }
        }
    }
    //Method to handle the given objective
    static boolean processObjective(Objective obj, PrintWriter outputWriter) {
        Dijkstra dijkstra = new Dijkstra(grid, startX, startY);
        while (true) {
            // Find the shortest path to the current objective
            ArrayList<Node> path = dijkstra.findShortestPath(obj.x, obj.y);
            recalculated = false;
            // Traverse the calculated path step by step
            for (int i = 1; i < path.size(); i++) {
                Node step = path.get(i);
                outputWriter.println("Moving to " + step.x + "-" + step.y);
                startX = step.x;
                startY = step.y;
                // Update visible obstacles as the path progresses
                updateVisibleObstacles(step,path,outputWriter);
                // If recalculation is needed, break out of the loop
                if (recalculated) {
                    break;
                }
            }
            // Recalculate the path if necessary
            if (recalculated) {
                dijkstra = new Dijkstra(grid, startX, startY);
                continue;
            }
            // Mark the objective as reached
            outputWriter.println("Objective " + (objectives.indexOf(obj) + 1) + " reached!");
            // Handle wizard options if available
            if (obj.hasWizardOffer()) {
                int chosenOption = selectBestOption(obj, objectives);
                outputWriter.println("Number " + chosenOption + " is chosen!");
                updateGridWithOption(chosenOption);
            }
            return true;
        }
    }

    //Method to make decision for best option in order to change the chosen type to type 0 (to make passable)
    static int selectBestOption(Objective obj,  ArrayList<Objective> myObjectives) {
        int bestOption = -1;
        double minPathCost = Double.MAX_VALUE;
        // Iterate through each wizard option to determine the best choice
        for (int option : obj.wizardOptions) {
            Node[][] tempGrid = new Node[grid.length][grid[0].length];
            // Create a temporary grid to simulate changes with the selected option
            for (int i = 0; i < grid.length; i++) {
                for (int j = 0; j < grid[i].length; j++) {
                    tempGrid[i][j] = new Node(grid[i][j].x, grid[i][j].y, grid[i][j].nodeType);
                    tempGrid[i][j].isPassable = grid[i][j].isPassable;
                    tempGrid[i][j].cost = grid[i][j].cost;
                    tempGrid[i][j].rightDistance = grid[i][j].rightDistance;
                    tempGrid[i][j].leftDistance = grid[i][j].leftDistance;
                    tempGrid[i][j].upDistance = grid[i][j].upDistance;
                    tempGrid[i][j].downDistance = grid[i][j].downDistance;
                    // Convert nodes of the selected type to type 0 (passable)
                    if (tempGrid[i][j].nodeType == option) {
                        tempGrid[i][j].nodeType = 0;
                        tempGrid[i][j].isPassable=true;
                    }
                }
            }
            // Calculate the shortest path cost with the current option
            int index = myObjectives.indexOf(obj);
            Dijkstra dijkstra = new Dijkstra(tempGrid ,startX, startY);
            ArrayList<Node> path = dijkstra.findShortestPath(myObjectives.get(index+1).x, myObjectives.get(index+1).y);
            // Update the best option if a cheaper path is found
            if (!path.isEmpty()) {
                double pathCost = calculatePathCost(path);
                if (pathCost < minPathCost) {
                    minPathCost = pathCost;
                    bestOption = option;
                }
            }
        }
        return bestOption;
    }

    //Method to update the grid after the option selection
    static void updateGridWithOption(int chosenOption) {
        // Update the grid by converting nodes of the chosen type to type 0 (passable)
        for (int i = 0; i < grid.length; i++) {
            for (int j = 0; j < grid[i].length; j++) {
                if (grid[i][j].nodeType == chosenOption) {
                    grid[i][j].nodeType = 0;
                    grid[i][j].isPassable = true;
                }
            }
        }
    }
    //Method to calculate the cost of given path , using the get Distance method
    static double calculatePathCost(ArrayList<Node> path) {
        double totalCost = 0.0;
        // Calculate the total cost of traversing the path
        for (int i = 1; i < path.size(); i++) {
            totalCost += getDistance(path.get(i - 1), path.get(i));
        }
        return totalCost;
    }

    //Dijkstra Class and Shortest Path Algorithm
    static class Dijkstra {
        Node[][] grid;// 2D grid representing the map of nodes
        int startX, startY;// Starting coordinates for the Dijkstra algorithm

        // Change rate  to explore neighbors (right, left, down, up)
        int[] changeX = {1, -1, 0, 0};
        int[] changeY = {0, 0, 1, -1};

        // Constructor to initialize the grid and starting position
        public Dijkstra(Node[][] grid, int startX, int startY) {
            this.grid = grid;
            this.startX = startX;
            this.startY = startY;
        }

         // Method to find the shortest path from the start node to the target node using Dijkstra's algorithm.
        // Takes the coordinate of the target and return the shortest path as ArrayList<Node>
        public ArrayList<Node> findShortestPath(int targetX, int targetY) {
            // Distance array to store the shortest distance to each node
            double[][] distances = new double[grid.length][grid[0].length];
            // Previous node array to reconstruct the path after finding the target
            Node[][] preData = new Node[grid.length][grid[0].length];
            // Initialize distances to infinity and previous nodes to null
            for (int i = 0; i < grid.length; i++) {
                for (int j = 0; j < grid[i].length; j++) {
                    distances[i][j] = Double.MAX_VALUE;
                    preData[i][j] = null;
                }
            }
            distances[startX][startY] = 0;
            // MinHeap (priority queue) to efficiently get the node with the smallest distance
            MinHeap myMinHeap = new MinHeap(Comparator.comparingDouble(n -> distances[n.x][n.y]));
            myMinHeap.add(grid[startX][startY]); // Add the starting node to the heap
            // Main loop of the algorithm
            while (!myMinHeap.isEmpty()) {
                // Get the node with the smallest distance
                Node currentMin = myMinHeap.RemoveMin();
                // If the target node is reached, reconstruct the path
                if (currentMin.x == targetX && currentMin.y == targetY) {
                    return reconstructPath(preData, targetX, targetY);
                }
                // Explore neighbors of the current node
                for (int i = 0; i < 4; i++) {
                    int neigh_x = currentMin.x + changeX[i];
                    int neigh_y = currentMin.y + changeY[i];
                    // Check if the neighbor is within grid bounds
                    if (neigh_x >= 0 && neigh_x < grid.length && neigh_y >= 0 && neigh_y < grid[0].length) {
                        Node neighbor = grid[neigh_x][neigh_y];
                        // Skip if the neighbor is not passable
                        if (!neighbor.isPassable) {
                            continue;
                        }
                        // Calculate the new distance to the neighbor
                        double newDist = distances[currentMin.x][currentMin.y] + getDistance(currentMin, neighbor);
                        // Update the distance and previous node if a shorter path is found
                        if (newDist < distances[neigh_x][neigh_y]) {
                            distances[neigh_x][neigh_y] = newDist;
                            preData[neigh_x][neigh_y] = currentMin;
                            myMinHeap.add(neighbor);
                        }
                    }
                }
            }
            // Return an empty list if no path is found
            return new ArrayList<>();
        }

        //Method to reconstruct the shortest path from the target node back to the start node.
        private ArrayList<Node> reconstructPath(Node[][] previous, int targetX, int targetY) {
            ArrayList<Node> path = new ArrayList<>();
            Node current = grid[targetX][targetY];
            // Traverse back through the previous nodes to construct the path
            while (current != null) {
                path.add(current);
                current = previous[current.x][current.y];
            }
            // Reverse the path to get the correct order (start to target)
            Collections.reverse(path);
            return path;
        }
    }
    //Method to calculate the cost (distance) btw. two neighbor nodes
    static double getDistance(Node current, Node neighbor) {
        // Check the direction of the neighbor and return the corresponding distance
        if (neighbor.x == current.x + 1) return current.rightDistance;
        if (neighbor.x == current.x - 1) return current.leftDistance;
        if (neighbor.y == current.y + 1) return current.upDistance;
        if (neighbor.y == current.y - 1) return current.downDistance;
        // Return a large value for invalid neighbors
        return Double.MAX_VALUE;
    }
}
