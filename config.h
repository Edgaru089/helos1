// config.h: Compile flags

#pragma once


// Supress all screen output on boot.
// Serial output is not supressed.
#define HELOS_RUNTIME_QUIET


// This defines a default target display mode for graphics_Init().
//
// 1600x900 is absent on most physical displays,
// but is available on most emulators.
#define HELOS_GRAPHICS_TARGET_MODE_WIDTH  1600
#define HELOS_GRAPHICS_TARGET_MODE_HEIGHT 900


// util/tree.h uses a Treap model for balancing.
//
// If no TREE_TYPE flags are defined, no balancing is employed.
#define HELOS_UTIL_TREE_TYPE_TREAP
