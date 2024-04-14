#pragma once

#include "rectangle.hpp"
#include <vector>
#include <array>
#include <functional>

template <typename T, int MaxObjects, int MaxLevels>
class Quadtree {
public:
    typedef Quadtree<T, MaxObjects, MaxLevels> NodeType;
    typedef std::reference_wrapper<T> RefT;

private:
    static const auto NODE_TOP_LEFT = 0;
    static const auto NODE_TOP_RIGHT = 1;
    static const auto NODE_BOTTOM_LEFT = 2;
    static const auto NODE_BOTTOM_RIGHT = 3;

    std::vector<RefT> _objects;
    std::vector<RefT> _stuck;

    //TODO: replace with optional
    std::array<NodeType*, 4> _nodes = { nullptr, nullptr, nullptr, nullptr };

    int _level;
    Rectangle<float> _bounds;

    void split() {
        auto subWidth = _bounds.w / 2.0f;
        auto subHeight = _bounds.h / 2.0f;
        auto x = _bounds.x;
        auto y = _bounds.y;

        _nodes[NODE_TOP_LEFT] = new Quadtree<T, MaxObjects, MaxLevels>(_level + 1, {
            x + subWidth, y, subWidth, subHeight
        });

        _nodes[NODE_TOP_RIGHT] = new Quadtree<T, MaxObjects, MaxLevels>(_level + 1, {
            x, y, subWidth, subHeight
        });

        _nodes[NODE_BOTTOM_LEFT] = new Quadtree<T, MaxObjects, MaxLevels>(_level + 1, {
            x, y + subHeight, subWidth, subHeight
        });

        _nodes[NODE_BOTTOM_RIGHT] = new Quadtree<T, MaxObjects, MaxLevels>(_level + 1, {
            x + subWidth, y + subHeight, subWidth, subHeight
        });
    }

    int get_index(const RefT object) {
        auto idx = -1;
        auto verticalMidpoint = _bounds.x + (_bounds.w / 2.0f);
        auto horizontalMidpoint = _bounds.y + (_bounds.h / 2.0f);
        const auto rect = object.get().rect();
        auto topQuadrant = (rect.y < horizontalMidpoint && rect.y + rect.h < horizontalMidpoint);
        auto bottomQuadrant = (rect.y > horizontalMidpoint);

        if (rect.x < verticalMidpoint && rect.x + rect.w < verticalMidpoint) {
            if (topQuadrant) {
                idx = 1;
            } else if (bottomQuadrant) {
                idx = 2;
            }
        } else if (rect.x > verticalMidpoint) {
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

    void append_our_objects(std::vector<RefT>& objects) {
        objects.insert(std::end(objects), std::begin(_objects), std::end(_objects));
        objects.insert(std::end(objects), std::begin(_stuck), std::end(_stuck));
    }

public:
    Quadtree(int level, const Rectangle<float>& bounds) :
        _level(level),
        _bounds(bounds) {
    }

    ~Quadtree() {
        for (auto i = 0; i < 4; i++) {
            if (_nodes[i] != nullptr) {
                delete _nodes[i];
            }
        }
    }

    void clear() {
        _objects.clear();
        _stuck.clear();
        for (auto i = 0; i < 4; i++) {
            if (_nodes[i] != nullptr) {
                delete _nodes[i];
                _nodes[i] = nullptr;
            }
        }
    }

    void insert(RefT item) {
        if (_nodes[0] != nullptr) {
            auto idx = get_index(item);

            if (idx != -1) {
                auto node = find_node(idx);

                if (node->_bounds.is_inside(item.get().rect())) {
                    node->insert(item);
                }
                else {
                    _stuck.emplace_back(std::ref(item));
                }

                return;
            }
        }

        _objects.emplace_back(std::ref(item));

        if (_objects.size() > MaxObjects && _level < MaxLevels) {
            if (_nodes[0] == nullptr) {
                split();
            }

            unsigned int i = 0;
            while (i < _objects.size()) {
                auto obj = _objects.at(i);
                auto idx = get_index(obj);
                if (idx != -1) {
                    _objects.erase(_objects.begin() + i);
                    find_node(idx)->insert(obj);
                } else {
                    i++;
                }
            }
        }
    }

    void retrieve(std::vector<RefT>& objects, const RefT item) {
        auto idx = get_index(item);

        if (idx != -1 && _nodes[idx] != nullptr) {
            auto node = _nodes[idx];
            auto bounds = node->_bounds;
            const auto rect = item.get().rect();
            if (bounds.is_inside(rect)) {
                node->retrieve(objects, item);
            } else {
                if (rect.x <= _nodes[NODE_TOP_RIGHT]->_bounds.x) {
                    if (rect.y <= _nodes[NODE_BOTTOM_LEFT]->_bounds.y) {
                        _nodes[NODE_TOP_LEFT]->append_our_objects(objects);
                    }

                    if (rect.y + rect.h > _nodes[NODE_BOTTOM_LEFT]->_bounds.y) {
                        _nodes[NODE_BOTTOM_LEFT]->append_our_objects(objects);
                    }
                }

                if (rect.x + rect.w > _nodes[NODE_TOP_RIGHT]->_bounds.x) {
                    if (rect.y <= _nodes[NODE_BOTTOM_RIGHT]->_bounds.y) {
                        _nodes[NODE_TOP_RIGHT]->append_our_objects(objects);
                    }

                    if (rect.y + rect.h > _nodes[NODE_BOTTOM_RIGHT]->_bounds.y) {
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

    template <typename F>
    void for_each_node(F func) const {
        for (auto i = 0; i < 4; i++) {
            if (_nodes[i] != nullptr) {
                func(const_cast<const NodeType&>(*_nodes[i]));
            }
        }
    }

    constexpr const std::vector<T*>& objects() const { return _objects; }

    constexpr const Rectangle<float>& bounds() const { return _bounds; }
};
