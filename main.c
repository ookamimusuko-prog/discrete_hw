#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_VERTICES 100
#define INF 99999999

// 전역 변수로 인접 행렬 선언 (메모리 초기화 용이 및 스택 오버플로우 방지)
// 1번 인덱스부터 사용하기 위해 +1 크기 할당
int adjMatrix[MAX_VERTICES + 1][MAX_VERTICES + 1];
int visited[MAX_VERTICES + 1];
int n; // 현재 그래프의 정점 개수

// BFS를 위한 큐 구조체
typedef struct {
    int items[MAX_VERTICES + 1];
    int front;
    int rear;
} Queue;

void initQueue(Queue* q) {
    q->front = -1;
    q->rear = -1;
}

int isEmpty(Queue* q) {
    return q->front == -1;
}

void enqueue(Queue* q, int value) {
    if (q->rear == MAX_VERTICES) return;
    if (q->front == -1) q->front = 0;
    q->rear++;
    q->items[q->rear] = value;
}

int dequeue(Queue* q) {
    int item;
    if (isEmpty(q)) return -1;
    item = q->items[q->front];
    q->front++;
    if (q->front > q->rear) {
        q->front = q->rear = -1;
    }
    return item;
}

// 그래프 초기화 함수
void resetGraph() {
    for (int i = 0; i <= MAX_VERTICES; i++) {
        for (int j = 0; j <= MAX_VERTICES; j++) {
            adjMatrix[i][j] = 0;
        }
        visited[i] = 0;
    }
}

// --- 1. 깊이 우선 탐색 (DFS) ---
int isFirstDFS = 1; // 출력 형식을 맞추기 위한 플래그

void dfs(int v) {
    visited[v] = 1;

    if (isFirstDFS) {
        printf("%d", v);
        isFirstDFS = 0;
    } else {
        printf("-%d", v);
    }

    for (int i = 1; i <= n; i++) {
        // 연결되어 있고(1), 방문하지 않았다면
        if (adjMatrix[v][i] == 1 && !visited[i]) {
            dfs(i);
        }
    }
}

// --- 2. 너비 우선 탐색 (BFS) ---
void bfs(int startNode) {
    Queue q;
    initQueue(&q);

    // visited 배열 재사용을 위해 초기화
    for(int i=0; i<=n; i++) visited[i] = 0;

    visited[startNode] = 1;
    enqueue(&q, startNode);

    int first = 1;

    while (!isEmpty(&q)) {
        int current = dequeue(&q);

        if (first) {
            printf("%d", current);
            first = 0;
        } else {
            printf("-%d", current);
        }

        for (int i = 1; i <= n; i++) {
            if (adjMatrix[current][i] == 1 && !visited[i]) {
                visited[i] = 1;
                enqueue(&q, i);
            }
        }
    }
    printf("\n");
}

// --- 3. 최단 거리 (Dijkstra) ---
// 경로 역추적을 위한 재귀 함수
void printPath(int parent[], int v) {
    if (parent[v] == -1) {
        printf("%d", v);
        return;
    }
    printPath(parent, parent[v]);
    printf("-%d", v);
}

void dijkstra(int startNode) {
    int dist[MAX_VERTICES + 1];
    int parent[MAX_VERTICES + 1]; // 경로 저장을 위한 배열
    int visitedS[MAX_VERTICES + 1] = {0,}; // Dijkstra용 방문 배열

    // 초기화
    for (int i = 1; i <= n; i++) {
        dist[i] = INF;
        parent[i] = -1;
    }
    dist[startNode] = 0;

    for (int i = 0; i < n; i++) {
        // 1. 방문하지 않은 노드 중 최소 거리 노드 선택
        int min = INF;
        int u = -1;

        for (int j = 1; j <= n; j++) {
            if (!visitedS[j] && dist[j] < min) {
                min = dist[j];
                u = j;
            }
        }

        if (u == -1) break; // 더 이상 갈 곳이 없음
        visitedS[u] = 1;

        // 2. 인접 노드 거리 갱신
        for (int v = 1; v <= n; v++) {
            if (adjMatrix[u][v] != 0) { // 간선이 존재하면 (가중치 > 0)
                // 중요: 경로 길이가 작을 경우(<)에만 갱신
                if ((long long)dist[u] + adjMatrix[u][v] < dist[v]) { // 오버플로우 방지를 위해 long long 비교
                    dist[v] = dist[u] + adjMatrix[u][v];
                    parent[v] = u;
                }
            }
        }
    }

    // 결과 출력
    printf("시작점: %d\n", startNode);
    for (int i = 2; i <= n; i++) { // 1번은 시작점이므로 제외하고 2번부터 출력
        printf("정점 [%d]: ", i);
        if (dist[i] == INF) {
             printf("도달 불가능 (경고: 비연결 그래프)\n"); // 🔴 경고 메시지 추가
        } else {
            printPath(parent, i);
            printf(", 길이: %d\n", dist[i]);
        }
    }
}

// 파일 처리 및 메인 로직
void processFile(const char* filename, int mode) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("%s 파일을 열 수 없습니다.\n", filename);
        return;
    }

    char line[1024];
    int graphCount = 1;

    // 파일에서 그래프 개수 및 데이터 읽기
    while (fscanf(fp, "%d", &n) != EOF) {
        // 줄바꿈 문자 소진
        fgetc(fp);

        resetGraph();

        // N개의 줄을 읽어서 그래프 구성
        for (int i = 0; i < n; i++) {
            if (fgets(line, sizeof(line), fp) == NULL) break;

            // 문자열 파싱
            char* token = strtok(line, " \n\r");
            if (token == NULL) continue;

            int u = atoi(token); // 현재 정점

            while (1) {
                token = strtok(NULL, " \n\r");
                if (token == NULL) break;
                int v = atoi(token); // 인접 정점

                if (mode == 1) { // input1.txt: 가중치 없음 (1로 설정)
                    adjMatrix[u][v] = 1;
                } else { // input2.txt: 가중치 있음
                    token = strtok(NULL, " \n\r"); // 가중치 읽기
                    if (token != NULL) {
                        int w = atoi(token);
                        adjMatrix[u][v] = w;
                    }
                }
            }
        }

        // 알고리즘 실행 및 출력
        printf("그래프 [%d]\n", graphCount++);
        printf("----------------------------\n");

        if (mode == 1) {
            // DFS
            printf("깊이 우선 탐색\n");
            for(int k=0; k<=n; k++) visited[k] = 0; // 방문 초기화
            isFirstDFS = 1;
            dfs(1);
            printf("\n");

            // BFS
            printf("너비 우선 탐색\n");
            bfs(1);
        } else {
            // Dijkstra
            dijkstra(1);
        }

        // 다음 그래프가 있다면 구분선 출력 (파일 끝 확인)
        long currentPos = ftell(fp);
        int nextChar;
        // 공백 건너뛰기
        while((nextChar = fgetc(fp)) != EOF && (nextChar == ' ' || nextChar == '\n' || nextChar == '\r'));

        if (nextChar != EOF) {
            printf("============================\n\n");
            fseek(fp, currentPos, SEEK_SET); // 위치 복구
        }
    }

    fclose(fp);
}

int main() {
    // 1. 그래프 탐방 (input1.txt)
    printf("1. 그래프 탐방 수행 결과\n\n");
    processFile("input1.txt", 1);
    printf("============================\n\n");

    printf("\n2. 최단 경로 구하기 수행 결과\n\n");
    // 2. 최단 거리 (input2.txt)
    processFile("input2.txt", 2);

    return 0;
}
