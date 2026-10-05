
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATA_N 100
#define KEY_N  50
#define MAX_VAL 1000

typedef struct Node {
    int data;
    struct Node* left;
    struct Node* right;
} Node;

static Node* new_node(int v) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) { fprintf(stderr, "메모리 할당 실패\n"); exit(1); }
    n->data = v;
    n->left = n->right = NULL;
    return n;
}

static Node* bst_insert(Node* root, int v, long* cmp) {
    if (!root) return new_node(v);
    Node* cur = root;
    while (1) {
        (*cmp)++;
        if (v < cur->data) {
            if (!cur->left) { cur->left = new_node(v); break; }
            cur = cur->left;
        }
        else if (v > cur->data) {
            if (!cur->right) { cur->right = new_node(v); break; }
            cur = cur->right;
        }
        else {
            break; 
        }
    }
    return root;
}

static int bst_search(Node* root, int key, long* cmp) {
    Node* cur = root;
    while (cur) {
        (*cmp)++;
        if (key == cur->data) return 1;
        cur = (key < cur->data) ? cur->left : cur->right;
    }
    return 0;
}

static int seq_search(const int* arr, int n, int key, long* cmp) {
    for (int i = 0; i < n; i++) {
        (*cmp)++;
        if (arr[i] == key) return 1;
    }
    return 0;
}

static int height(Node* n) {
    if (!n) return 0;
    int l = height(n->left), r = height(n->right);
    return (l > r ? l : r) + 1;
}

static void free_tree(Node* n) {
    if (!n) return;
    free_tree(n->left);
    free_tree(n->right);
    free(n);
}

static int rnd(void) { return rand() % (MAX_VAL + 1); }

static int cmp_int(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

int main(int argc, char* argv[]) {
    unsigned int seed = (argc > 1) ? (unsigned int)strtoul(argv[1], NULL, 10)
        : (unsigned int)time(NULL);
    srand(seed);
    printf("Random seed : %u\n\n", seed);

    int arr[DATA_N];
    int used[MAX_VAL + 1] = { 0 };
    int cnt = 0;
    while (cnt < DATA_N) {
        int v = rnd();
        if (used[v]) continue;      
        used[v] = 1;
        arr[cnt++] = v;
    }

    Node* root = NULL;
    long build_cmp = 0;
    for (int i = 0; i < DATA_N; i++) root = bst_insert(root, arr[i], &build_cmp);

    printf("===== 생성된 %d개의 정수 (발생 순서) =====\n", DATA_N);
    for (int i = 0; i < DATA_N; i++) {
        printf("%4d%s", arr[i], (i % 10 == 9) ? "\n" : " ");
    }
    printf("\nBST 생성 과정의 총 비교 횟수 : %ld\n", build_cmp);
    printf("BST 높이(노드 수 기준)       : %d\n\n", height(root));

    int keys[KEY_N];
    for (int i = 0; i < KEY_N; i++) keys[i] = rnd();

    printf("===== 탐색 결과 =====\n");
    printf("%-4s %-10s %-8s %-12s %-12s\n", "No.", "Key", "Result", "Seq Cmp", "BST Cmp");

    long seq_total = 0, bst_total = 0;
    long seq_succ = 0, bst_succ = 0, seq_fail = 0, bst_fail = 0;
    int n_succ = 0, n_fail = 0;

    for (int i = 0; i < KEY_N; i++) {
        long sc = 0, bc = 0;
        int sf = seq_search(arr, DATA_N, keys[i], &sc);
        int bf = bst_search(root, keys[i], &bc);
        if (sf != bf) { fprintf(stderr, "오류: 두 탐색 결과가 다릅니다.\n"); return 1; }

        printf("%-4d %-10d %-8s %-12ld %-12ld\n", i + 1, keys[i],
            sf ? "Found" : "Failed", sc, bc);

        seq_total += sc;
        bst_total += bc;
        if (sf) { n_succ++; seq_succ += sc; bst_succ += bc; }
        else { n_fail++; seq_fail += sc; bst_fail += bc; }
    }

    printf("\n===== 요약 =====\n");
    printf("Number of searches : %d (성공 %d, 실패 %d)\n\n", KEY_N, n_succ, n_fail);
    printf("Sequential Search\n");
    printf("  Total comparisons   : %ld\n", seq_total);
    printf("  Average comparisons : %.2f\n\n", (double)seq_total / KEY_N);
    printf("BST Search\n");
    printf("  Total comparisons   : %ld\n", bst_total);
    printf("  Average comparisons : %.2f\n\n", (double)bst_total / KEY_N);

    printf("[추가 통계] 성공/실패별 평균 비교 횟수\n");
    if (n_succ) printf("  성공 탐색: 순차 %.2f, BST %.2f\n",
        (double)seq_succ / n_succ, (double)bst_succ / n_succ);
    if (n_fail) printf("  실패 탐색: 순차 %.2f, BST %.2f\n",
        (double)seq_fail / n_fail, (double)bst_fail / n_fail);

    long bst_all = build_cmp + bst_total;
    printf("\n===== BST 생성 비용을 고려한 비교 =====\n");
    printf("BST 생성 비교 횟수           : %ld\n", build_cmp);
    printf("순차 탐색 총 비교 (50회)      : %ld\n", seq_total);
    printf("BST 탐색 총 비교 (50회)       : %ld\n", bst_total);
    printf("BST 생성 + 탐색 총 비교       : %ld\n", bst_all);
    printf("순차 - (BST 생성+탐색) 차이   : %ld (양수면 BST가 유리)\n", seq_total - bst_all);
    if (seq_total > bst_total) {
        double saving = (double)(seq_total - bst_total) / KEY_N;
        printf("탐색 1회당 평균 절감 비교 수  : %.2f\n", saving);
        printf("손익분기 탐색 횟수(약)        : %.1f회\n", build_cmp / saving);
    }

    int sorted[DATA_N];
    for (int i = 0; i < DATA_N; i++) sorted[i] = arr[i];
    qsort(sorted, DATA_N, sizeof(int), cmp_int);

    Node* skew = NULL;
    long skew_build = 0, skew_total = 0;
    for (int i = 0; i < DATA_N; i++) skew = bst_insert(skew, sorted[i], &skew_build);
    for (int i = 0; i < KEY_N; i++) { long c = 0; bst_search(skew, keys[i], &c); skew_total += c; }

    printf("\n===== [추가 실험] 같은 값을 오름차순으로 삽입한 편향 BST =====\n");
    printf("BST 높이                : %d\n", height(skew));
    printf("생성 비교 횟수          : %ld\n", skew_build);
    printf("탐색 총 비교 (50회)     : %ld\n", skew_total);
    printf("탐색 평균 비교          : %.2f\n", (double)skew_total / KEY_N);

    free_tree(root);
    free_tree(skew);
    return 0;
}
