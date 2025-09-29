// InterfacePrictice1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <vector>
#include <windows.h>
#include </Users/EugeneT/Desktop/Интерфейсы/Pr2/include/cxxopts.hpp>

using namespace std;

struct Shape {
public:
    virtual void print() const = 0;
    virtual ~Shape() = default;
};

struct Point : public Shape {
public:
    float x, y;

    Point(float x, float y) : x(x), y(y) {}

    void print() const override {
        cout << "Point(" << x << ", " << y << ")";
    }
};

struct Line : public Shape {
public:
    Point A, B;

    Line(Point A, Point B) : A(A), B(B) {}

    void print() const override {
        cout << "Line(";
        A.print();
        cout << ", ";
        B.print();
        cout << ")";
    }
};

struct Circle : public Shape {
public:
    Point point;
    float radius;

    Circle(Point point, float radius) : point(point), radius(radius) {}

    void print() const override {
        cout << "Circle(";
        point.print();
        cout << ", " << radius << ")";
    }
};

bool parsePoint(const string& str, Point& point) {
    regex pointRegex(R"(\s*Point\((-?\d+\.\d+),\s*(-?\d+\.\d+)\)\s*)");
    smatch match;
    if (regex_match(str, match, pointRegex)) {
        point.x = stof(match[1].str());
        point.y = stof(match[2].str());
        return true;
    }
    return false;
}

bool parseLine(const string& str, Line& line) {
    regex lineRegex(R"(\s*Line\((Point\((-?\d+\.\d+),\s*(-?\d+\.\d+)\)),\s*(Point\((-?\d+\.\d+),\s*(-?\d+\.\d+)\))\)\s*)");
    smatch match;
    if (regex_match(str, match, lineRegex)) {
        Point p1(stof(match[2].str()), stof(match[3].str()));
        Point p2(stof(match[5].str()), stof(match[6].str()));
        line = Line(p1, p2);
        return true;
    }
    return false;
}

bool parseCircle(const string& str, Circle& circle) {
    regex circleRegex(R"(\s*Circle\((Point\((-?\d+\.\d+),\s*(-?\d+\.\d+)\)),\s*(-?\d+\.\d+)\)\s*)");
    smatch match;
    if (regex_match(str, match, circleRegex)) {
        Point center(stof(match[2].str()), stof(match[3].str()));
        float radius = stof(match[4].str());
        circle = Circle(center, radius);
        return true;
    }
    return false;
}

vector<Shape*> parseFile(const string& filename) {
    ifstream file(filename);
    vector<Shape*> shapes;
    string line;

    if (!file.is_open()) {
        throw runtime_error("Не удалось открыть файл: " + filename);
    }

    while (getline(file, line)) {
        // Пропускаем пустые строки
        if (line.empty()) continue;

        Point point(0, 0);
        Line lineObj(Point(0, 0), Point(0, 0));
        Circle circle(Point(0, 0), 0);

        if (parsePoint(line, point)) {
            shapes.push_back(new Point(point));
        }
        else if (parseLine(line, lineObj)) {
            shapes.push_back(new Line(lineObj));
        }
        else if (parseCircle(line, circle)) {
            shapes.push_back(new Circle(circle));
        }
        else {
            cout << "Предупреждение: не удалось распознать строку: " << line << endl;
        }
    }

    file.close();
    return shapes;
}

void printShapes(const vector<Shape*>& shapes) {
    if (shapes.empty()) {
        cout << "Файл не содержит фигур" << endl;
        return;
    }

    cout << "Содержимое файла:" << endl;
    for (const auto& shape : shapes) {
        shape->print();
        cout << endl;
    }
}

void countShapes(const vector<Shape*>& shapes) {
    cout << "Количество фигур в файле: " << shapes.size() << endl;

    // Дополнительная статистика по типам фигур
    int points = 0, lines = 0, circles = 0;
    for (const auto& shape : shapes) {
        if (dynamic_cast<Point*>(shape)) points++;
        else if (dynamic_cast<Line*>(shape)) lines++;
        else if (dynamic_cast<Circle*>(shape)) circles++;
    }

    cout << "Точек: " << points << endl;
    cout << "Линий: " << lines << endl;
    cout << "Окружностей: " << circles << endl;
}

void cleanupShapes(vector<Shape*>& shapes) {
    for (auto shape : shapes) {
        delete shape;
    }
    shapes.clear();
}

int main()
{
    setlocale(LC_ALL, "Russian");
    SetConsoleOutputCP(65001);

    cxxopts::Options options("test", "Программа для работы с геометрическими фигурами");

    options.add_options()
        ("o,oper", "Операция (print или count)", cxxopts::value<std::string>())
        ("f,file", "Файл с объектами", cxxopts::value<string>())
        ("h,help", "Вывод справки")
        ;

    string commandLine;

    std::cout << options.help() << std::endl;
    std::cout << "\nВведите команду: ";

    vector<string> tokens;
    string token;
    istringstream iss(commandLine);
    while (iss >> token) {
        tokens.push_back(token);
    }

    vector<char*> argv;
    argv.push_back(const_cast<char*>("program"));
    for (auto& token : tokens) {
        argv.push_back(const_cast<char*>(token.c_str()));
    }
    int argc = argv.size();

    try {
        auto result = options.parse(argc, argv.data());

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (!result.count("file") || !result.count("oper")) {
            std::cerr << "Ошибка: необходимо указать оба обязательных аргумента" << std::endl;
            std::cout << options.help() << std::endl;
            return 1;
        }

        std::string filename = result["file"].as<std::string>();
        std::string operation = result["oper"].as<std::string>();

        if (operation != "print" && operation != "count") {
            std::cerr << "Ошибка: недопустимая операция '" << operation << "'" << std::endl;
            std::cout << "Допустимые операции: print, count" << std::endl;
            return 1;
        }

        vector<Shape*> shapes = parseFile(filename);

        if (operation == "print") {
            printShapes(shapes);
        }
        else if (operation == "count") {
            countShapes(shapes);
        }

        cleanupShapes(shapes);

    }
    catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Ошибка парсинга аргументов: " << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}