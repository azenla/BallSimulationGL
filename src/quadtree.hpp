#pragma once

#include "rectangle.hpp"
#include <vector>
#include <array>
#include <functional>
#include <cstddef>

template <typename T, int MaxObjects, int MaxLevels>
class Quadtree {
public:
    typedef std::reference_wrapper<T> RefT;

private:
    static constexpr std::size_t NODE_TOP_LEFT     = 0;
    static constexpr std::size_t NODE_TOP_RIGHT    = 1;
    static constexpr std::size_t NODE_BOTTOM_LEFT  = 2;
    static constexpr std::size_t NODE_BOTTOM_RIGHT = 3;

    std::vector<RefT> _objects, _stuck;
    int _level;
    Rectangle<float> _bounds;
    std::unique_ptr<std::array<Quadtree, 4>> _nodes;

    constexpr Quadtree& get_node(int idx) { return _nodes->at(idx); }
    constexpr const Quadtree& get_node(int idx) const { return _nodes->at(idx); }

    void split() {
        auto subWidth = _bounds.w / 2.0f;
        auto subHeight = _bounds.h / 2.0f;
        auto x = _bounds.x;
        auto y = _bounds.y;

        auto nextLevel = _level + 1;
        _nodes = std::make_unique<std::array<Quadtree, 4>>(std::array<Quadtree, 4>{
            Quadtree(nextLevel, { x + subWidth, y, subWidth, subHeight }),
            Quadtree(nextLevel, { x, y, subWidth, subHeight }),
            Quadtree(nextLevel, { x, y + subHeight, subWidth, subHeight }),
            Quadtree(nextLevel, { x + subWidth, y + subHeight, subWidth, subHeight })
        });
    }

    int get_index(const RefT object) const {
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

    void append_our_objects(std::vector<RefT>& objects) const {
        objects.insert(std::end(objects), std::cbegin(_objects), std::cend(_objects));
        objects.insert(std::end(objects), std::cbegin(_stuck), std::cend(_stuck));
    }

public:
    constexpr Quadtree(int level, const Rectangle<float>& bounds) :
        _level(level),
        _bounds(bounds) {
    }

    constexpr const std::vector<T*>& objects() const { return _objects; }
    constexpr const Rectangle<float>& bounds() const { return _bounds; }

    void clear() {
        _objects.clear();
        _stuck.clear();
        _nodes.reset();
    }

    void insert(RefT item) {
        if (_nodes != nullptr) {
            auto idx = get_index(item);

            if (idx != -1) {
                auto& node = get_node(idx);

                if (node._bounds.is_inside(item.get().rect())) {
                    node.insert(item);
                }
                else {
                    _stuck.emplace_back(std::ref(item));
                }

                return;
            }
        }

        _objects.emplace_back(std::ref(item));

        if (_objects.size() > MaxObjects && _level < MaxLevels) {
            if (_nodes == nullptr) {
                split();
            }

            unsigned int i = 0;
            while (i < _objects.size()) {
                auto obj = _objects.at(i);
                auto idx = get_index(obj);
                if (idx != -1) {
                    _objects.erase(_objects.begin() + i);
                    get_node(idx).insert(obj);
                } else {
                    i++;
                }
            }
        }
    }

    void retrieve(std::vector<RefT>& objects, const RefT item) const {
        auto idx = get_index(item);

        if (idx != -1 && _nodes != nullptr) {
            const auto& node = get_node(idx);
            auto& bounds = node._bounds;
            const auto rect = item.get().rect();
            if (bounds.is_inside(rect)) {
                node.retrieve(objects, item);
            } else {
                if (rect.x <= get_node(NODE_TOP_RIGHT)._bounds.x) {
                    if (rect.y <= get_node(NODE_BOTTOM_LEFT)._bounds.y) {
                        get_node(NODE_TOP_LEFT).append_our_objects(objects);
                    }

                    if (rect.y + rect.h > get_node(NODE_BOTTOM_LEFT)._bounds.y) {
                        get_node(NODE_BOTTOM_LEFT).append_our_objects(objects);
                    }
                }

                if (rect.x + rect.w > get_node(NODE_TOP_RIGHT)._bounds.x) {
                    if (rect.y <= get_node(NODE_BOTTOM_RIGHT)._bounds.y) {
                        get_node(NODE_TOP_RIGHT).append_our_objects(objects);
                    }

                    if (rect.y + rect.h > get_node(NODE_BOTTOM_RIGHT)._bounds.y) {
                        get_node(NODE_BOTTOM_RIGHT).append_our_objects(objects);
                    }
                }
            }
        }

        append_our_objects(objects);
    }

    template <typename F>
    void for_each_node(F func) const {
        if (_nodes != nullptr) {
            for (const auto& node : *_nodes) {
                func(node);
            }
        }
    }
};
