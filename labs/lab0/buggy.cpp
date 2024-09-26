#include <iostream>
#include <cstring>

/**
 * @brief Defines a point with an x and y value on the coordinate plane
 * @param x position in coordinate plane
 * @param y position in coordinate plane
 */
struct Point {
    int x, y;

    Point () : x(), y() {}
    Point (int _x, int _y) : x(_x), y(_y) {}
};

/**
 * @brief Collection of points on the x-y plane called vertices
 * @param vertices  Number of points the shape has
 * @param points    Array of "Point" pointers
 */
class Shape {
    int vertices;
    Point** points;

public:
    Shape (int _vertices) : vertices(_vertices), points(new Point*[_vertices]) {}

    ~Shape () {
        for (int i = 0; i < vertices; ++i) {
            delete points[i];
        }
        delete[] points;
        points = nullptr;
    }

    /* Takes in an unsized array of Points, and copies it to member points* array */
    void addPoints (Point* pts) { /* formal parameter for unsized array called pts */
        for (int i = 0; i < vertices; ++i) {
            points[i] = new Point{};
            memcpy(points[i], &pts[i], sizeof(Point));
        }
    }

    // Shoelace Formula
    double area () {
        int temp = 0;
        for (int i = 1; i < vertices; i++) {
            // FIXME: there are two methods to access members of pointers
            //        use one to fix lhs and the other to fix rhs
            int lhs = points[i - 1]->x * points[i]->y;
            int rhs = (*points[i]).x * (*points[i - 1]).y;
            temp += (lhs - rhs);
        }
        return abs(temp)/2.0;
    }
};

int main () {
    // FIXME: create the following points using the three different methods
    //        of defining structs
    Point tri1 = {0, 0};
    Point tri2{1, 2};
    Point tri3;
    tri3.x = 2;
    tri3.y = 0;

    // adding points to tri
    Point triPts[3] = {tri1, tri2, tri3};
    Shape* tri = new Shape{3};
    tri->addPoints(triPts);

    // FIXME: create the following points using your preferred struct
    //        definition:
    Point quad1{0, 0};
    Point quad2{0, 2};
    Point quad3{2, 2};
    Point quad4{2, 0};

    // adding points to quad
    Point quadPts[4] = {quad1, quad2, quad3, quad4};
    Shape* quad = new Shape{4};
    quad->addPoints(quadPts);

    // FIXME: print out area of tri and area of quad
    std::cout << tri->area() << std::endl;
    std::cout << quad->area() << std::endl;

    delete tri;
    delete quad;
}
