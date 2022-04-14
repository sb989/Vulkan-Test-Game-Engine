#!/bin/bash
glslc shaders/object.vert -o shaders/obj_vert.spv
glslc shaders/object.frag -o shaders/obj_frag.spv

glslc shaders/light.vert -o shaders/light_vert.spv
glslc shaders/light.frag -o shaders/light_frag.spv

glslc shaders/shadow.vert -o shaders/shadow_vert.spv
glslc shaders/shadow.frag -o shaders/shadow_frag.spv