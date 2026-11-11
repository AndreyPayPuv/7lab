#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

static int g_print = 1;

double now_us(void)
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

int** create_matrix(int n)
{
    int** g = (int**)malloc((size_t)n * sizeof(int*));
    if (g == NULL)
        return NULL;

    for (int i = 0; i < n; i++) {
        g[i] = (int*)calloc((size_t)n, sizeof(int));
        if (g[i] == NULL) {
            for (int k = 0; k < i; k++)
                free(g[k]);
            free(g);
            return NULL;
        }
    }
    return g;
}

void free_matrix(int** g, int n)
{
    if (g == NULL)
        return;
    for (int i = 0; i < n; i++)
        free(g[i]);
    free(g);
}

void generate_graph(int** g, int n)
{
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            g[i][j] = g[j][i] = rand() % 2;
}

void print_matrix(int** g, int n)
{
    printf("\nМатрица смежности:\n    ");
    for (int j = 0; j < n; j++) printf("%3d", j + 1);
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("%3d ", i + 1);
        for (int j = 0; j < n; j++) printf("%3d", g[i][j]);
        printf("\n");
    }
}

typedef struct Node {
    int v;
    struct Node* next;
} Node;

void free_lists(Node** adj, int n)
{
    if (adj == NULL)
        return;
    for (int i = 0; i < n; i++) {
        Node* p = adj[i];
        while (p != NULL) {
            Node* t = p;
            p = p->next;
            free(t);
        }
    }
    free(adj);
}

Node** matrix_to_lists(int** g, int n)
{
    Node** adj = (Node**)calloc((size_t)n, sizeof(Node*));
    Node** tail = (Node**)calloc((size_t)n, sizeof(Node*));
    if (adj == NULL || tail == NULL) {
        free(adj);
        free(tail);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (g[i][j] != 1)
                continue;

            Node* p = (Node*)malloc(sizeof(Node));
            if (p == NULL) {
                free_lists(adj, n);
                free(tail);
                return NULL;
            }
            p->v = j;
            p->next = NULL;

            if (tail[i] == NULL)
                adj[i] = p;
            else
                tail[i]->next = p;
            tail[i] = p;
        }
    }
    free(tail);
    return adj;
}

void print_lists(Node** adj, int n)
{
    printf("\nСписки смежности:\n");
    for (int i = 0; i < n; i++) {
        printf("%3d:", i + 1);
        if (adj[i] == NULL)
            printf(" (нет смежных вершин)");
        for (Node* p = adj[i]; p != NULL; p = p->next)
            printf(" %d", p->v + 1);
        printf("\n");
    }
}

void dfs_rec(int** g, int n, int* num, int v)
{
    num[v] = 1;
    if (g_print) printf("%d ", v + 1);
    for (int i = 0; i < n; i++)
        if (g[v][i] == 1 && num[i] == 0)
            dfs_rec(g, n, num, i);
}

void dfs_matrix_recursive(int** g, Node** adj, int n, int start)
{
    (void)adj;
    int* num = (int*)calloc((size_t)n, sizeof(int));
    if (num == NULL) {
        printf("Ошибка выделения памяти.\n");
        return;
    }
    dfs_rec(g, n, num, start);
    for (int v = 0; v < n; v++)
        if (!num[v])
            dfs_rec(g, n, num, v);
    free(num);
}

void dfs_list_rec(Node** adj, int* num, int v)
{
    num[v] = 1;
    if (g_print) printf("%d ", v + 1);
    for (Node* p = adj[v]; p != NULL; p = p->next)
        if (!num[p->v])
            dfs_list_rec(adj, num, p->v);
}

void dfs_lists_recursive(int** g, Node** adj, int n, int start)
{
    (void)g;
    int* num = (int*)calloc((size_t)n, sizeof(int));
    if (num == NULL) {
        printf("Ошибка выделения памяти.\n");
        return;
    }
    dfs_list_rec(adj, num, start);
    for (int v = 0; v < n; v++)
        if (!num[v])
            dfs_list_rec(adj, num, v);
    free(num);
}

void dfs_matrix_iterative(int** g, Node** adj, int n, int start)
{
    (void)adj;
    int* num = (int*)calloc((size_t)n, sizeof(int));
    int* stack = (int*)calloc((size_t)n, sizeof(int));
    int* pos = (int*)calloc((size_t)n, sizeof(int));

    if (num == NULL || stack == NULL || pos == NULL) {
        printf("Ошибка выделения памяти.\n");
        free(num);
        free(stack);
        free(pos);
        return;
    }

    for (int k = 0; k <= n; k++) {
        int s = (k == 0) ? start : k - 1;
        if (num[s])
            continue;

        int top = 0;
        stack[top] = s;
        pos[s] = 0;
        num[s] = 1;
        if (g_print) printf("%d ", s + 1);

        while (top >= 0) {
            int v = stack[top];
            int found = 0;

            while (pos[v] < n) {
                int i = pos[v]++;
                if (g[v][i] == 1 && num[i] == 0) {
                    num[i] = 1;
                    if (g_print) printf("%d ", i + 1);
                    pos[i] = 0;
                    stack[++top] = i;
                    found = 1;
                    break;
                }
            }
            if (!found)
                top--;
        }
    }
    free(num);
    free(stack);
    free(pos);
}

void dfs_lists_iterative(int** g, Node** adj, int n, int start)
{
    (void)g;
    int* num = (int*)calloc((size_t)n, sizeof(int));
    int* stack = (int*)calloc((size_t)n, sizeof(int));
    Node** cur = (Node**)malloc((size_t)n * sizeof(Node*));

    if (num == NULL || stack == NULL || cur == NULL) {
        printf("Ошибка выделения памяти.\n");
        free(num);
        free(stack);
        free(cur);
        return;
    }
    for (int i = 0; i < n; i++)
        cur[i] = adj[i];

    for (int k = 0; k <= n; k++) {
        int s = (k == 0) ? start : k - 1;
        if (num[s])
            continue;

        int top = 0;
        stack[top] = s;
        num[s] = 1;
        if (g_print) printf("%d ", s + 1);

        while (top >= 0) {
            int v = stack[top];
            Node* p = cur[v];
            if (p == NULL) {
                top--;
                continue;
            }
            cur[v] = p->next;
            if (!num[p->v]) {
                num[p->v] = 1;
                if (g_print) printf("%d ", p->v + 1);
                stack[++top] = p->v;
            }
        }
    }
    free(num);
    free(stack);
    free(cur);
}

typedef void (*Traversal)(int** g, Node** adj, int n, int start);

void run_traversal(const char* title, Traversal f, int** g, Node** adj, int n, int start)
{
    printf("\n%s\nПорядок обхода: ", title);
    g_print = 1;
    f(g, adj, n, start);
    printf("\n");

    long long reps = 20000000LL / ((long long)n * n + 1);
    if (reps < 1) reps = 1;
    if (reps > 100000) reps = 100000;

    g_print = 0;
    double t0 = now_us();
    for (long long r = 0; r < reps; r++)
        f(g, adj, n, start);
    double t1 = now_us();
    g_print = 1;

    printf("Время обхода: %.3f мкс (среднее по %lld запускам)\n",
        (t1 - t0) / (double)reps, reps);
}

int main(void)
{
    int n;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    srand((unsigned)time(NULL));

    printf("Введите число вершин графа: ");
    if (scanf("%d", &n) != 1 || n <= 0) {
        printf("Некорректное число вершин.\n");
        return 1;
    }

    int** g = create_matrix(n);
    if (g == NULL) {
        printf("Ошибка выделения памяти.\n");
        return 1;
    }
    generate_graph(g, n);
    print_matrix(g, n);

    Node** adj = matrix_to_lists(g, n);
    if (adj == NULL) {
        printf("Ошибка выделения памяти.\n");
        free_matrix(g, n);
        return 1;
    }
    print_lists(adj, n);

    for (;;) {
        int start;
        printf("\nВведите номер начальной вершины (1..%d, 0 - выход): ", n);
        if (scanf("%d", &start) != 1) {
            printf("Некорректный ввод.\n");
            break;
        }
        if (start == 0)
            break;
        if (start < 1 || start > n) {
            printf("Такой вершины нет.\n");
            continue;
        }
        start--;

        run_traversal("1) Рекурсивный обход, матрица смежности:",
            dfs_matrix_recursive, g, adj, n, start);
        run_traversal("2) Рекурсивный обход, списки смежности:",
            dfs_lists_recursive, g, adj, n, start);
        run_traversal("3) Нерекурсивный обход, матрица смежности:",
            dfs_matrix_iterative, g, adj, n, start);
        run_traversal("4) Нерекурсивный обход, списки смежности:",
            dfs_lists_iterative, g, adj, n, start);
    }

    free_lists(adj, n);
    free_matrix(g, n);
    return 0;
}