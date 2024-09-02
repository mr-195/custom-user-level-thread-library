#include <foothread.h>
#include <stdio.h>
#include <stdlib.h>


#define MAX_INPUT_LEN 100
#define MAX_NODES 100

// Global variables
int num_nodes;                             // Number of nodes in the tree
int root_node;                             // Root node of the tree
int parent_nodes[MAX_NODES];               // Array storing the parent node of each node
int partial_sums[MAX_NODES];               // Array storing the partial sums at each node
int num_children[MAX_NODES];               // Array storing the number of children for each node
foothread_mutex_t node_mutexes[MAX_NODES]; // Mutexes for each node
foothread_mutex_t common_mutex;            // Mutex for shared resources
foothread_barrier_t node_barrier[MAX_NODES]; // Barrier for synchronization between nodes
foothread_barrier_t common_barrier;          // Common barrier for synchronization

// Thread function for each node in the tree
int node_function(void *args)
{
    int node = (int)args; // Current node

    // Read user input for leaf nodes
    if (num_children[node] == 0)
    {
        foothread_mutex_lock(&common_mutex);
        printf("Leaf node %d :: Enter a positive integer: ", node);
        int value;
        scanf("%d", &value);
        update_sum(node, value); // Update the sum at the current node
        foothread_mutex_unlock(&common_mutex);
        foothread_barrier_wait(&node_barrier[parent_nodes[node]]); // Wait for parent to finish
    }

    // Calculate partial sum for internal nodes
    if (num_children[node] != 0)
    {
        foothread_barrier_wait(&node_barrier[node]); // Wait for all children to finish
        foothread_mutex_lock(&common_mutex);
        foothread_mutex_lock(&node_mutexes[node]);
        // Calculate partial sum by summing up partial sums from children
        for (int i = 0; i < num_nodes; i++)
        {
            if (parent_nodes[i] == node)
            {
                partial_sums[node] += partial_sums[i];
            }
        }
        foothread_mutex_unlock(&node_mutexes[node]);
        printf("Internal node %d gets the partial sum %d from its children\n", node, partial_sums[node]);
        foothread_mutex_unlock(&common_mutex);
        if (root_node != node)
            foothread_barrier_wait(&node_barrier[parent_nodes[node]]); // Wait for parent to finish
    }
    foothread_barrier_wait(&common_barrier); // Wait for all nodes to reach this point

    foothread_exit(); // Exit the thread
    return 0;
}

// Read tree information from file
void read_tree(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fscanf(file, "%d", &num_nodes);
    for (int i = 0; i < num_nodes; i++)
    {
        int child_node, parent_node;
        fscanf(file, "%d %d", &child_node, &parent_node);
        if (child_node == parent_node)
        {
            root_node = child_node;
            continue;
        }
        parent_nodes[child_node] = parent_node;
        num_children[parent_node]++;
        partial_sums[child_node] = 0;
    }
    fclose(file);
}

// Update sum at parent node
void update_sum(int node, int value)
{
    foothread_mutex_lock(&node_mutexes[node]); // Lock the mutex for the current node
    partial_sums[node] += value; // Update the partial sum at the current node
    foothread_mutex_unlock(&node_mutexes[node]); // Unlock the mutex for the current node
}

// Print total sum at root node
void print_total_sum()
{
    printf("Sum at root(node %d): %d\n", root_node, partial_sums[root_node]);
}

int main()
{
    int i;
    foothread_attr_t attr;
    foothread_attr_setstacksize(&attr, 2097152);
    foothread_attr_setjointype(&attr, FOOTHREAD_JOINABLE);

    // Initialize num_children array
    for (i = 0; i < MAX_NODES; i++)
    {
        num_children[i] = 0;
    }

    // Read tree information from file
    read_tree("tree.txt");

    // Initialize synchronization resources
    for (i = 0; i < num_nodes; i++)
    {
        foothread_mutex_init(&node_mutexes[i]); // Initialize mutex for each node
    }

    foothread_mutex_init(&common_mutex); // Initialize common mutex

    // Create barrier for each node
    for (i = 0; i < num_nodes; i++)
    {
        foothread_barrier_init(&node_barrier[i], num_children[i]);
    }

    foothread_barrier_init(&common_barrier, num_nodes); // Initialize common barrier

    // Create threads for each node in the tree
    foothread_t threads[MAX_NODES];
    for (i = 0; i < num_nodes; i++)
    {
        foothread_create(&threads[i], &attr, node_function, (void *)i);
    }

    // Wait for all threads to complete
    foothread_barrier_wait(&common_barrier);

    // Print total sum at root node
    print_total_sum();

    // Clean up resources
    for (i = 0; i < num_nodes; i++)
    {
        foothread_mutex_destroy(&node_mutexes[i]); // Destroy mutex for each node
        foothread_barrier_destroy(&node_barrier[i]); // Destroy barrier for each node
    }

    foothread_exit(); // Exit the main thread

    return 0;
}
