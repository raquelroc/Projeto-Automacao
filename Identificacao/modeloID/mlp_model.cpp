#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

std::vector<double> load_vector(const std::string& filename) {
    std::vector<double> v;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line, ',')) {
        v.push_back(std::stod(line));
    }
    return v;
}

std::vector<std::vector<double>> load_matrix(const std::string& filename) {
    std::vector<std::vector<double>> M;
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        std::vector<double> row;
        std::stringstream ss(line);
        std::string val;

        while (std::getline(ss, val, ',')) {
            row.push_back(std::stod(val));
        }
        M.push_back(row);
    }
    return M;
}

std::vector<std::vector<double>> W1, W2, W3;
std::vector<double> b1, b2, b3;
double X_mean, X_scale, Y_mean, Y_scale;

void load_mlp() {
    W1 = load_matrix("W1.csv");
    b1 = load_vector("b1.csv");

    W2 = load_matrix("W2.csv");
    b2 = load_vector("b2.csv");

    W3 = load_matrix("W3.csv");
    b3 = load_vector("b3.csv");

    X_mean = load_vector("scaler_X_mean.csv")[0];
    X_scale = load_vector("scaler_X_scale.csv")[0];
    Y_mean = load_vector("scaler_y_mean.csv")[0];
    Y_scale = load_vector("scaler_y_scale.csv")[0];
}

double relu(double x) { return x > 0 ? x : 0; }

double mlp_predict(double pwm) {

    // normalizar entrada
    double x = (pwm - X_mean) / X_scale;

    // camada 1
    std::vector<double> h1(b1.size());
    for (size_t i = 0; i < h1.size(); i++)
        h1[i] = relu(W1[i][0] * x + b1[i]);

    // camada 2
    std::vector<double> h2(b2.size());
    for (size_t i = 0; i < h2.size(); i++) {
        double sum = b2[i];
        for (size_t j = 0; j < h1.size(); j++)
            sum += W2[i][j] * h1[j];
        h2[i] = relu(sum);
    }

    // saída
    double ynorm = b3[0];
    for (size_t j = 0; j < h2.size(); j++)
        ynorm += W3[0][j] * h2[j];

    return ynorm * Y_scale + Y_mean;
}