#include <string> 
namespace slg { namespace ocl { 
std::string KernelSource_tonemap_reduce_funcs = 
"#line 2 \"tonemap_reduce_funcs.cl\"\n" 
"/***************************************************************************\n" 
"* Copyright 1998-2015 by authors (see AUTHORS.txt)                        *\n" 
"*                                                                         *\n" 
"*   This file is part of LuxRender.                                       *\n" 
"*                                                                         *\n" 
"* Licensed under the Apache License, Version 2.0 (the \"License\");         *\n" 
"* you may not use this file except in compliance with the License.        *\n" 
"* You may obtain a copy of the License at                                 *\n" 
"*                                                                         *\n" 
"*     http://www.apache.org/licenses/LICENSE-2.0                          *\n" 
"*                                                                         *\n" 
"* Unless required by applicable law or agreed to in writing, software     *\n" 
"* distributed under the License is distributed on an \"AS IS\" BASIS,       *\n" 
"* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*\n" 
"* See the License for the specific language governing permissions and     *\n" 
"* limitations under the License.                                          *\n" 
"***************************************************************************/\n" 
"//------------------------------------------------------------------------------\n" 
"// Compute the REDUCE_OP of all frame buffer RGB values\n" 
"//------------------------------------------------------------------------------\n" 
"__attribute__((reqd_work_group_size(64, 1, 1))) __kernel void OpRGBValuesReduce(\n" 
"const uint filmWidth, const uint filmHeight,\n" 
"__global float *channel_IMAGEPIPELINE,\n" 
"__global uint *channel_FRAMEBUFFER_MASK,\n" 
"__global float *accumBuffer) {\n" 
"// Workgroup local shared memory\n" 
"__local float3 localMemBuffer[64];\n" 
"const uint tid = get_local_id(0);\n" 
"const uint gid = get_global_id(0);\n" 
"const uint localSize = get_local_size(0);\n" 
"const uint pixelCount = filmWidth * filmHeight;\n" 
"localMemBuffer[tid] = 0.f;\n" 
"// Read the first pixel\n" 
"const uint stride0 = gid * 2;\n" 
"const uint maskValue0 = channel_FRAMEBUFFER_MASK[stride0];\n" 
"if (maskValue0 && (stride0 < pixelCount)) {\n" 
"const uint stride03 = stride0 * 3;\n" 
"localMemBuffer[tid] = REDUCE_OP(localMemBuffer[tid], VLOAD3F(&channel_IMAGEPIPELINE[stride03]));\n" 
"}\n" 
"// Read the second pixel\n" 
"const uint stride1 = stride0 + 1;\n" 
"const uint maskValue1 = channel_FRAMEBUFFER_MASK[stride1];\n" 
"if (maskValue1 && (stride1 < pixelCount)) {\n" 
"const uint stride13 = stride1 * 3;\n" 
"localMemBuffer[tid] = REDUCE_OP(localMemBuffer[tid], VLOAD3F(&channel_IMAGEPIPELINE[stride13]));\n" 
"}\n" 
"barrier(CLK_LOCAL_MEM_FENCE);\n" 
"// Do reduction in local memory\n" 
"for (uint s = localSize >> 1; s > 0; s >>= 1) {\n" 
"if (tid < s) {\n" 
"localMemBuffer[tid] = ACCUM_OP(localMemBuffer[tid], localMemBuffer[tid + s]);\n" 
"}\n" 
"barrier(CLK_LOCAL_MEM_FENCE);\n" 
"}\n" 
"// Write result for this block to global memory\n" 
"if (tid == 0) {\n" 
"const uint bid = get_group_id(0) * 3;\n" 
"accumBuffer[bid] = localMemBuffer[0].s0;\n" 
"accumBuffer[bid + 1] = localMemBuffer[0].s1;\n" 
"accumBuffer[bid + 2] = localMemBuffer[0].s2;\n" 
"}\n" 
"}\n" 
"__attribute__((reqd_work_group_size(64, 1, 1))) __kernel void OpRGBValueAccumulate(\n" 
"const uint size,\n" 
"__global float *accumBuffer) {\n" 
"if (get_global_id(0) == 0) {\n" 
"float3 totalRGBValue = 0.f;\n" 
"for(uint i = 0; i < size; ++i) {\n" 
"const uint index = i * 3;\n" 
"totalRGBValue = ACCUM_OP(totalRGBValue, VLOAD3F(&accumBuffer[index]));\n" 
"}\n" 
"// Write the result\n" 
"accumBuffer[0] = totalRGBValue.s0;\n" 
"accumBuffer[1] = totalRGBValue.s1;\n" 
"accumBuffer[2] = totalRGBValue.s2;\n" 
"}\n" 
"}\n" 
; } } 