#!/usr/bin/python3
import sys

class Solution:
    # This class is used to store the solution of a graph
    # k denotes the solution size and vertices is a list containing the solution
    def __init__(self, k, vertices):
        self.k = k
        self.vertices = vertices

class Graph:
    # Create a graph from a list of edges
    # Edges are tuples of the form (u, v)
    def __init__(self, num_vertices, edges):
        self.vertices = list(range(1, num_vertices + 1))
        self.edges = {} # adjacency list

        if edges:
            edges = sorted(edges) # sort all edges such that we get a sorted adjacency list
            for e in edges:
                u = e[0]
                v = e[1]


                self.edges.setdefault(u, []).append(v)
                self.edges.setdefault(v, []).append(u)

    # Return the neighbors of the given vertex
    def neighbors(self, u):
        return self.edges[u] if u in self.edges else []
    
    
# Returns a graph from the given .gr file
# The file should be in the format of the DIMACS graph format
def read_graph(filename):
    with open(filename, "r", encoding='utf-8') as gr:
        n = 0
        edges = []
        for line in gr:
            if line.strip() == "" or line[0] == "c":
                continue

            if line[0] == "p":
                n = int(line.split(" ")[2])
            else:
                edges.append(tuple(map(int, line.split(" "))))

        return Graph(n, edges)
    
def read_graph_from_stdin(input_stream):
    n = 0
    edges = []
    for line in input_stream:
        if line.strip() == "" or line[0] == "c":
            continue

        if line[0] == "p":
            n = int(line.split(" ")[2])
        else:
            edges.append(tuple(map(int, line.split(" "))))

    return Graph(n, edges)    

# Write the graph to a .gr file
def write_graph(graph, filename):
    with open(filename, "w", encoding='utf-8') as f:
        f.write(f"p ds {len(graph.vertices)} {len(graph.edges)}\n")
        for u in graph.edges:
            for v in graph.edges[u]:
                if u < v:
                    f.write(f"{u} {v}\n")   

# Read the solution from a file
def read_solution(filename):
    vertices = []
    k = -1
    with open(filename, "r", encoding='utf-8') as f:
        # Read the first non-comment line to get the size of the solution
        # Then collect the vertices of the solution
        foundFirstLine = False
        for line in f.readlines():
            if line.strip() == "" or line[0] == "c":
                continue

            if not foundFirstLine:
                foundFirstLine = True
                k = int(line.strip())
            else:
                vertices.append(int(line.strip()))   
            
    return Solution(k, vertices)

def write_solution(solution, filename):
    with open(filename, "w", encoding='utf-8') as f:
        f.write(f"{solution.k}\n")
        for v in solution.vertices:
            f.write(f"{v}\n")

def verify_solution(graph, solution):
    # Check if the solution size matches the number of vertices in the solution
    if solution.k != len(solution.vertices):
        return False
    
    # Check if all vertices are unique
    if len(set(solution.vertices)) != len(solution.vertices):
        return False

    # Check if all vertices are in the range [1, n]
    for v in solution.vertices:
        if v < 1 or v > len(graph.vertices):
            return False


    # Check if the solution is a dominating set
    dom = set()
    for v in solution.vertices:
        dom.add(v)
        for u in graph.neighbors(v):
            dom.add(u)

    return len(dom) == len(graph.vertices)

def greedy_solution(graph):
    # Greedily find a dominating set
    dom = set()
    vertices = set(graph.vertices)
    while vertices:
        # Find the vertex with the maximum degree
        max_vertex = max(vertices, key=lambda v: len(graph.neighbors(v)))
        dom.add(max_vertex)
        vertices.remove(max_vertex)

        # Remove all neighbors of the selected vertex from the remaining vertices
        for u in graph.neighbors(max_vertex):
            if u in vertices:
                vertices.remove(u)

    return Solution(len(dom), list(dom))

def print_solution(solution):
    print(f"{solution.k}")
    for v in solution.vertices:
        print(f"{v}")

# Read graph and user solution
def main():
    graph = read_graph_from_stdin(sys.stdin)
    #graph = read_graph(sys.argv[1])
    sol = greedy_solution(graph)
    print_solution(sol)
    # Uncomment the following lines to write the solution to a file
    #write_solution(sol, sys.argv[2])

    return 0


if __name__ == '__main__':
    sys.exit(main())