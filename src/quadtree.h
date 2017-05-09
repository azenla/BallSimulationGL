#pragma once

#include <vector>

template <typename T>
struct Rectangle {
    float x, y, w, h;
    T value;

    Rectangle(float xx, float yy, float ww, float hh) {
        x = xx;
        y = yy;
        w = ww;
        h = hh;
    }

    Rectangle(float xx, float yy, float ww, float hh, T value) {
        x = xx;
        y = yy;
        w = ww;
        h = hh;

        this->value = value;
    }
};

template <typename T, int MaxObjects, int MaxLevels>
class Quadtree {
private:
    std::vector<Rectangle<T>*> *_objects;
    Quadtree *_nodes[4] = {nullptr, nullptr, nullptr, nullptr};
    int _level;
    Rectangle<T> *_bounds;

    void split() {
        auto subWidth = _bounds->w / 2.0f;
        auto subHeight = _bounds->h / 2.0f;
        auto x = _bounds->x;
        auto y = _bounds->y;

        _nodes[0] = new Quadtree(_level + 1, new Rectangle<T>{
                x + subWidth, y, subWidth, subHeight
        });

        _nodes[1] = new Quadtree(_level + 1, new Rectangle<T>{
                x, y, subWidth, subHeight
        });

        _nodes[2] = new Quadtree(_level + 1, new Rectangle<T>{
                x, y + subHeight, subWidth, subHeight
        });

        _nodes[3] = new Quadtree(_level + 1, new Rectangle<T>{
                x + subWidth, y + subHeight, subWidth, subHeight
        });
    }

    int getIndex(Rectangle<T> *rect) {
        auto idx = -1;
        auto verticalMidpoint = _bounds->x + (_bounds->w / 2.0f);
        auto horizontalMidpoint = _bounds->y + (_bounds->h / 2.0f);
        auto topQuadrant = (rect->y < horizontalMidpoint && rect->y + rect->h < horizontalMidpoint);
        auto bottomQuadrant = (rect->y > horizontalMidpoint);

        if (rect->x < verticalMidpoint && rect->x + rect->w < verticalMidpoint) {
            if (topQuadrant) {
                idx = 1;
            } else if (bottomQuadrant) {
                idx = 2;
            }
        } else if (rect->x > verticalMidpoint) {
            if (topQuadrant) {
                idx = 0;
            } else if (bottomQuadrant) {
                idx = 3;
            }
        }

        return idx;
    }

public:
    Quadtree(int level, Rectangle<T> *bounds) {
        _level = level;
        _bounds = bounds;
        _objects = new std::vector<Rectangle<T>*>();
    }

    ~Quadtree() {
        if (_objects) {
            _objects->clear();
            delete _objects;
        }

        for (auto i = 0; i < 4; i++) {
            if (_nodes[i] != nullptr) {
                delete _nodes[i];
            }
        }
    }

    void clear() {
        _objects->clear();
        for (auto i = 0; i < 4; i++) {
            if (_nodes[i] != nullptr) {
                _nodes[i]->clear();
                _nodes[i] = nullptr;
            }
        }
    }

    void insert(Rectangle<T> *rect) {
        if (_nodes[0] != nullptr) {
            auto idx = getIndex(rect);
            if (idx != -1 && _nodes[idx] != nullptr) {
                _nodes[idx]->insert(rect);
            }
        }

        _objects->push_back(rect);

        if (_objects->size() > MaxObjects && _level < MaxLevels) {
            if (_nodes[0] == nullptr) {
                split();
            }

            unsigned int i = 0;
            while (i < _objects->size()) {
                auto obj = _objects->at(i);
                auto idx = getIndex(obj);
                if (idx != -1) {
                    _objects->erase(_objects->begin() + i);
                    _nodes[idx]->insert(obj);
                } else {
                    i++;
                }
            }
        }
    }

    void retrieve(std::vector<Rectangle<T>*> *objects, Rectangle<T> *rect) {
        auto idx = getIndex(rect);
        if (idx != -1 && _nodes[idx] != nullptr) {
            _nodes[idx]->retrieve(objects, rect);
        }

        for (auto it = _objects->begin(); it != _objects->end(); it++) {
            auto rectangle = (Rectangle<T>*) *it;
            objects->push_back(rectangle);
        }
    }

    std::vector<Rectangle<T> *> *objects() {
        return _objects;
    }
};
