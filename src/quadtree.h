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

    bool is_inside(Rectangle<T>* item) {
        return item->x >= x &&
            item->x + item->w <= x + w &&
            item->y >= y &&
            item->y + item->h <= y + h;
    }

    ~Rectangle() {}
};

template <typename T, int MaxObjects, int MaxLevels>
class Quadtree {
    static const auto NODE_TOP_LEFT = 0;
    static const auto NODE_TOP_RIGHT = 1;
    static const auto NODE_BOTTOM_LEFT = 2;
    static const auto NODE_BOTTOM_RIGHT = 3;

    std::vector<Rectangle<T>*>* _objects;
    std::vector<Rectangle<T>*>* _stuck;

    Quadtree<T, MaxObjects, MaxLevels>* _nodes[4] = {nullptr, nullptr, nullptr, nullptr};

    int _level;
    Rectangle<T>* _bounds;

    void split() {
        auto subWidth = _bounds->w / 2.0f;
        auto subHeight = _bounds->h / 2.0f;
        auto x = _bounds->x;
        auto y = _bounds->y;

        _nodes[NODE_TOP_LEFT] = new Quadtree<T, MaxObjects, MaxLevels>(_level + 1, new Rectangle<T>{
            x + subWidth, y, subWidth, subHeight
        });

        _nodes[NODE_TOP_RIGHT] = new Quadtree<T, MaxObjects, MaxLevels>(_level + 1, new Rectangle<T>{
            x, y, subWidth, subHeight
        });

        _nodes[NODE_BOTTOM_LEFT] = new Quadtree<T, MaxObjects, MaxLevels>(_level + 1, new Rectangle<T>{
            x, y + subHeight, subWidth, subHeight
        });

        _nodes[NODE_BOTTOM_RIGHT] = new Quadtree<T, MaxObjects, MaxLevels>(_level + 1, new Rectangle<T>{
            x + subWidth, y + subHeight, subWidth, subHeight
        });
    }

    int get_index(Rectangle<T>* rect) {
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

    Quadtree<T, MaxObjects, MaxLevels>* find_node(int idx) {
        return _nodes[idx];
    }

    void append_our_objects(std::vector<Rectangle<T>*>* objects) {
        objects->insert(objects->end(), _objects->begin(), _objects->end());
        objects->insert(objects->end(), _stuck->begin(), _stuck->end());
    }

public:
    Quadtree(int level, Rectangle<T>* bounds) {
        _level = level;
        _bounds = bounds;
        _objects = new std::vector<Rectangle<T>*>();
        _stuck = new std::vector<Rectangle<T>*>();
    }

    ~Quadtree() {
        if (_objects) {
            _objects->clear();
            delete _objects;
        }

        if (_stuck) {
            _stuck->clear();
            delete _stuck;
        }

        for (auto i = 0; i < 4; i++) {
            if (_nodes[i] != nullptr) {
                delete _nodes[i];
            }
        }

        if (_level != 0) {
            delete _bounds;
        }
    }

    void clear() {
        _objects->clear();
        _stuck->clear();
        for (auto i = 0; i < 4; i++) {
            if (_nodes[i] != nullptr) {
                delete _nodes[i];
                _nodes[i] = nullptr;
            }
        }
    }

    void insert(Rectangle<T>* item) {
        if (_nodes[0] != nullptr) {
            auto idx = get_index(item);

            if (idx != -1) {
                auto node = find_node(idx);

                if (node->_bounds->is_inside(item)) {
                    node->insert(item);
                }
                else {
                    _stuck->push_back(item);
                }

                return;
            }
        }

        _objects->push_back(item);

        if (_objects->size() > MaxObjects && _level < MaxLevels) {
            if (_nodes[0] == nullptr) {
                split();
            }

            unsigned int i = 0;
            while (i < _objects->size()) {
                auto obj = _objects->at(i);
                auto idx = get_index(obj);
                if (idx != -1) {
                    _objects->erase(_objects->begin() + i);
                    find_node(idx)->insert(obj);
                } else {
                    i++;
                }
            }
        }
    }

    void retrieve(std::vector<Rectangle<T>*>* objects, Rectangle<T>* item) {
        auto idx = get_index(item);

        if (idx != -1 && _nodes[idx] != nullptr) {
            auto node = _nodes[idx];
            auto bounds = node->_bounds;
            if (bounds->is_inside(item)) {
                node->retrieve(objects, item);
            } else {
                if (item->x <= _nodes[NODE_TOP_RIGHT]->_bounds->x) {
                    if (item->y <= _nodes[NODE_BOTTOM_LEFT]->_bounds->y) {
                        _nodes[NODE_TOP_LEFT]->append_our_objects(objects);
                    }

                    if (item->y + item->h > _nodes[NODE_BOTTOM_LEFT]->_bounds->y) {
                        _nodes[NODE_BOTTOM_LEFT]->append_our_objects(objects);
                    }
                }

                if (item->x + item->w > _nodes[NODE_TOP_RIGHT]->_bounds->x) {
                    if (item->y <= _nodes[NODE_BOTTOM_RIGHT]->_bounds->y) {
                        _nodes[NODE_TOP_RIGHT]->append_our_objects(objects);
                    }

                    if (item->y + item->h > _nodes[NODE_BOTTOM_RIGHT]->_bounds->y) {
                        _nodes[NODE_BOTTOM_RIGHT]->append_our_objects(objects);
                    }
                }
            }
        }

        append_our_objects(objects);
    }

    void for_each_node(void func(Quadtree<T, MaxObjects, MaxLevels>* tree)) {
        for (auto i = 0; i < 4; i++) {
            if (_nodes[i] != nullptr) {
                func(_nodes[i]);
            }
        }
    }

    std::vector<Rectangle<T> *>* objects() const {
        return _objects;
    }

    Rectangle<T>* bounds() const {
        return _bounds;
    }
};
