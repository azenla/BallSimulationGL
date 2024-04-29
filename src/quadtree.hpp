#pragma once

#include "rectangle.hpp"
#include <vector>
#include <array>
#include <functional>

template <typename T, int MaxObjects, int MaxLevels>
class Quadtree {
public:
    typedef std::reference_wrapper<T> RefT;

private:
    class Quad {
        std::array<Quadtree, 4> _nodes;

    public:
        constexpr Quad(
            Quadtree&& new_top_left, Quadtree&& new_top_right, Quadtree&& new_bottom_left, Quadtree&& new_bottom_right
        ) : _nodes{
            std::move(new_top_left), std::move(new_top_right), std::move(new_bottom_left), std::move(new_bottom_right)
        } {}

        inline constexpr const Quadtree& top_left() const     { return _nodes[0]; }
        inline constexpr const Quadtree& top_right() const    { return _nodes[1]; }
        inline constexpr const Quadtree& bottom_left() const  { return _nodes[2]; }
        inline constexpr const Quadtree& bottom_right() const { return _nodes[3]; }

        typedef typename std::array<Quadtree, 4>::size_type size_type;
        inline constexpr Quadtree& at(size_type i) { return _nodes.at(i); }
        inline constexpr const Quadtree& at(size_type i) const { return _nodes.at(i); }
        inline constexpr Quadtree& operator [](size_type i) { return _nodes[i]; }

        typedef typename std::array<Quadtree, 4>::iterator iterator;
        inline constexpr iterator begin() { return std::begin(_nodes); }
        inline constexpr iterator end()   { return std::end(_nodes); }
    };

    std::vector<RefT> _objects, _stuck;
    int _level;
    Rectangle<float> _bounds;
    std::unique_ptr<Quad> _nodes;

    void split() {
        auto subWidth = _bounds.w / 2.0f;
        auto subHeight = _bounds.h / 2.0f;
        auto x = _bounds.x;
        auto y = _bounds.y;

        auto nextLevel = _level + 1;
        _nodes = std::make_unique<Quad>(
            Quadtree(nextLevel, { x + subWidth, y, subWidth, subHeight }),
            Quadtree(nextLevel, { x, y, subWidth, subHeight }),
            Quadtree(nextLevel, { x, y + subHeight, subWidth, subHeight }),
            Quadtree(nextLevel, { x + subWidth, y + subHeight, subWidth, subHeight })
        );
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
    constexpr Quadtree(): _level(0), _bounds(Rectangle<float>::zero()) {}
    constexpr Quadtree(int level, const Rectangle<float>& bounds) :
        _level(level),
        _bounds(bounds) {
    }

    constexpr const std::vector<RefT>& objects() const { return _objects; }
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
                auto& node = _nodes->at(idx);

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
                    _nodes->at(idx).insert(obj);
                } else {
                    i++;
                }
            }
        }
    }

    void retrieve(std::vector<RefT>& objects, const RefT item) const {
        auto idx = get_index(item);

        if (idx != -1 && _nodes != nullptr) {
            const auto& node = _nodes->at(idx);
            const auto& bounds = node._bounds;
            const auto rect = item.get().rect();
            if (bounds.is_inside(rect)) {
                node.retrieve(objects, item);
            } else {
                const auto& quad = *_nodes;
                if (rect.x <= quad.top_right()._bounds.x) {
                    if (rect.y <= quad.bottom_left()._bounds.y) {
                        quad.top_left().append_our_objects(objects);
                    }

                    if (rect.y + rect.h > quad.bottom_left()._bounds.y) {
                        quad.bottom_left().append_our_objects(objects);
                    }
                }

                if (rect.x + rect.w > quad.top_right()._bounds.x) {
                    if (rect.y <= quad.bottom_right()._bounds.y) {
                        quad.top_right().append_our_objects(objects);
                    }

                    if (rect.y + rect.h > quad.bottom_right()._bounds.y) {
                        quad.bottom_right().append_our_objects(objects);
                    }
                }
            }
        }

        append_our_objects(objects);
    }

    constexpr bool has_child_nodes() const {
        return _nodes != nullptr;
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
