// config.h: Compile flags

#pragma once


// Supress all screen output on boot.
// Serial output is not supressed.
#define HELOS_RUNTIME_QUIET


// util/tree.h uses a Treap model for balancing.
//
// If no TREE_TYPE flags are defined, no balancing is employed.
#define HELOS_UTIL_TREE_TYPE_TREAP
