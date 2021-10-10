#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <climits>

using namespace std;

struct edge {
    size_t u;
    size_t v;

    bool operator==(const edge& e) {
        if ((this->u == e.u) && (this->v == e.v)) {
            return true;
        }
        return false;
    }
};

list<size_t> get_aug_path(const vector<vector<bool>>& G, size_t n, size_t v,
                          vector<bool>& used, const list<size_t>& exposed);

vector<vector<int>> get_reduced_matrix(vector<vector<int>> M, size_t n);

// Решение задачи о назначении, венгерский алгоритм

// Вход: число n и матрица nxn
// Выход: перестановка, являющаяся решением задачи о назначении

int main() {
    size_t n;
    std::cin>>n; //Ввод значения n

    vector<vector<int>> assignment_matrix(n, vector<int>(n));
    vector<vector<int>> reduced(n, vector<int>(n));

    // Невычеркнутые строки и столбцы
    vector<bool> relevant_lines(n), relevant_columns(n);

    // Ввод матрицы
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            cin>>assignment_matrix[i][j];
        }
    }

    //Редуцированная матрица
    reduced = get_reduced_matrix(assignment_matrix, n);

    // Двудольный граф машин и работ
    vector<vector<bool>> bigraph(2*n + 1, vector<bool>(2*n + 1, false));
    list<size_t> exposed; // Непокрытые вершины
    list<edge> matching; // Паросочетание

    for (size_t i = 0; i < 2*n; i++) {
        exposed.push_back(i);
    }

    list<size_t> aug_path; //Увеличивающий путь

    while (matching.size() < n) { // Составление паросочетания (соответствия машины и работы)
        for (size_t i = 0; i < n; i++) {
            relevant_lines[i] = relevant_columns[i] = true;
        }

        // Ориентированный граф (машины -> работы)
        for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
                if (reduced[i][j] == 0) {
                    bigraph[i][n + j] = true;
                }
            }
            bigraph[2*n][i] = true; // Вершина соединена с непокрытыми вершинами (машинами)
        }

        do { // Увеличение паросочетания (увеличивающий путь)
            vector<vector<bool>> search_graph = bigraph;
            for (list<edge>::iterator i = matching.begin(); i != matching.end(); ++i) {
                search_graph[(*i).u][(*i).v] = 0; // Смена ориентации ребра
                search_graph[(*i).v][(*i).u] = 1; // в паросочетании
                exposed.remove((*i).u); // Вершины ребра становятся покрытыми
                exposed.remove((*i).v);
                search_graph[2*n][(*i).u] = false;
            }

            vector<bool> used(n, false); // Посещённые вершины при обходе в глубину
            aug_path = get_aug_path(search_graph, n, 2*n, used, exposed);
            if (aug_path.size() > 0) {  // Если есть увеличивающий путь, увеличиваем паросочетание
                aug_path.pop_front();
                for (list<size_t>::iterator i = aug_path.begin(); i != aug_path.end(); ) {
                    size_t u = *i;
                    size_t v = *(++i);
                    edge e;
                    e.u = (u < v ? u : v);
                    e.v = (u < v ? v : u);
                    matching.push_back(e);

                    u = *i;
                    if ((++i) != aug_path.end()) {
                        v = *i;
                        e.u = (u < v ? u : v);
                        e.v = (u < v ? v : u);
                        matching.remove(e);
                    }
                }
            } else { // Если нет увеличивающего пути, вычёркиваем строки/столбцы
                for (size_t i = 0; i < n; i++) {
                    if (used[i] == false) {
                        relevant_lines[i] = false;
                    }
                    if (used[n + i] == true) {
                        relevant_columns[i] = false;
                    }
                }
            }
        } while (aug_path.size() > 0);

        // Когда паросочетание максимальное, добавляем нули в редуцированную

        int min = INT_MAX;
        for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
                if (relevant_lines[i] == true && relevant_columns[j] == true && min > reduced[i][j]) {
                    min = reduced[i][j]; // Находим минимальный ненулевой элемент в матрице
                }
            }
        }
        for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < n; j++) {
                if (relevant_lines[i] == true && relevant_columns[j] == true) {
                    reduced[i][j] -= min; // Вычитаем его из невычеркнутых элементов
                } else if (relevant_lines[i] == false && relevant_columns[j] == false) {
                    reduced[i][j] += min; // Прибавляем к элементам на пересечении зачеркнутых строк/столбцов
                }
            }
        }
    }

    // Сортируем пары машина/работа по возрастанию номера машины, выводим перестановку
    vector<edge> result;
    for (edge const &e: matching) {
        result.push_back(e);
    }
    std::sort(result.begin(), result.end(), [](const edge& a, const edge& b){return a.u < b.u;});

    cout<<"Answer: ";
    for (size_t i = 0; i < n; i++) {
        cout<<(result[i].v - n + 1)<<" ";
    }
    cout<<endl;

    return 0;
}

// Составление редуцированной матрицы
vector<vector<int>> get_reduced_matrix(vector<vector<int>> M, size_t n) {
    for (size_t i = 0; i < n; i++) {
        int min = INT_MAX;
        for (size_t j = 0; j < n; j++) {
            if (min > M[i][j]) {
                min = M[i][j];
            }
        }
        for (size_t j = 0; j < n; j++) {
            M[i][j] -= min;
        }
    }

    for (size_t j = 0; j < n; j++) {
        int min = INT_MAX;
        for (size_t i = 0; i < n; i++) {
            if (min > M[i][j]) {
                min = M[i][j];
            }
        }
        for (size_t i = 0; i < n; i++) {
            M[i][j] -= min;
        }
    }
    return M;
}

// Составление увеличивающего пути на основе поиска в глубину
list<size_t> get_aug_path(const vector<vector<bool>>& G, size_t n, size_t v,
                          vector<bool>& used, const list<size_t>& exposed) {
    list<size_t> path; // Увеличивающий путь
    used[v] = true; // Посещённые вершины

    // Начало и конец пути в разных долях графа
    size_t start = v < n ? n : 0;
    size_t finish = v < n ? 2*n : n;

    for (size_t i = start; path.size() == 0 && i < finish; i++) {
        if ((G[v][i] == true) && (used[i] == false)) {
            if ((find(exposed.begin(), exposed.end(), i) != exposed.end()) && (i >= n)) {
                path.push_front(i);
            } else {
                path = get_aug_path(G, n, i, used, exposed);
            }
        }
    }

    if (path.size() != 0) {
        path.push_front(v);
    }

    return path;
}
