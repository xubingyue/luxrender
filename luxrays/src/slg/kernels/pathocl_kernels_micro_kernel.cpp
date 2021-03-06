#include <string> 
namespace slg { namespace ocl { 
std::string KernelSource_pathocl_kernels_micro = 
"#line 2 \"patchocl_kernels_micro.cl\"\n" 
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
"// AdvancePaths (Micro-Kernels)\n" 
"//------------------------------------------------------------------------------\n" 
"//------------------------------------------------------------------------------\n" 
"// Evaluation of the Path finite state machine.\n" 
"//\n" 
"// From: MK_RT_NEXT_VERTEX\n" 
"// To: MK_HIT_NOTHING or MK_HIT_OBJECT or MK_RT_NEXT_VERTEX\n" 
"//------------------------------------------------------------------------------\n" 
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths_MK_RT_NEXT_VERTEX(\n" 
"KERNEL_ARGS\n" 
") {\n" 
"const size_t gid = get_global_id(0);\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_RAYCOUNT)\n" 
"// This has to be done by the first kernel to run after RT kernel\n" 
"samples[gid].result.rayCount += 1;\n" 
"#endif\n" 
"// Read the path state\n" 
"__global GPUTaskState *taskState = &tasksState[gid];\n" 
"PathState pathState = taskState->state;\n" 
"if (pathState != MK_RT_NEXT_VERTEX)\n" 
"return;\n" 
"//--------------------------------------------------------------------------\n" 
"// Start of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"// Initialize image maps page pointer table\n" 
"INIT_IMAGEMAPS_PAGES\n" 
"//--------------------------------------------------------------------------\n" 
"// End of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"float3 connectionThroughput;\n" 
"const bool continueToTrace = Scene_Intersect(\n" 
"#if defined(PARAM_HAS_VOLUMES)\n" 
"&pathVolInfos[gid],\n" 
"&tasks[gid].tmpHitPoint,\n" 
"#endif\n" 
"#if defined(PARAM_HAS_PASSTHROUGH)\n" 
"taskState->bsdf.hitPoint.passThroughEvent,\n" 
"#endif\n" 
"&rays[gid], &rayHits[gid], &taskState->bsdf,\n" 
"&connectionThroughput, VLOAD3F(taskState->throughput.c),\n" 
"&samples[gid].result,\n" 
"// BSDF_Init parameters\n" 
"meshDescs,\n" 
"sceneObjs,\n" 
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n" 
"meshTriLightDefsOffset,\n" 
"#endif\n" 
"vertices,\n" 
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n" 
"vertNormals,\n" 
"#endif\n" 
"#if defined(PARAM_HAS_UVS_BUFFER)\n" 
"vertUVs,\n" 
"#endif\n" 
"#if defined(PARAM_HAS_COLS_BUFFER)\n" 
"vertCols,\n" 
"#endif\n" 
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n" 
"vertAlphas,\n" 
"#endif\n" 
"triangles\n" 
"MATERIALS_PARAM\n" 
");\n" 
"VSTORE3F(connectionThroughput * VLOAD3F(taskState->throughput.c), taskState->throughput.c);\n" 
"// If continueToTrace, there is nothing to do, just keep the same state\n" 
"if (!continueToTrace) {\n" 
"const bool rayMiss = (rayHits[gid].meshIndex == NULL_INDEX);\n" 
"taskState->state = rayMiss ? MK_HIT_NOTHING : MK_HIT_OBJECT;\n" 
"}\n" 
"#if defined(PARAM_HAS_PASSTHROUGH)\n" 
"else {\n" 
"// I generate a new random variable starting from the previous one. I'm\n" 
"// not really sure about the kind of correlation introduced by this\n" 
"// trick.\n" 
"taskState->bsdf.hitPoint.passThroughEvent = fabs(taskState->bsdf.hitPoint.passThroughEvent - .5f) * 2.f;\n" 
"}\n" 
"#endif\n" 
"}\n" 
"//------------------------------------------------------------------------------\n" 
"// Evaluation of the Path finite state machine.\n" 
"//\n" 
"// From: MK_HIT_NOTHING\n" 
"// To: MK_SPLAT_SAMPLE\n" 
"//------------------------------------------------------------------------------\n" 
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths_MK_HIT_NOTHING(\n" 
"KERNEL_ARGS\n" 
") {\n" 
"const size_t gid = get_global_id(0);\n" 
"// Read the path state\n" 
"__global GPUTaskState *taskState = &tasksState[gid];\n" 
"PathState pathState = taskState->state;\n" 
"if (pathState != MK_HIT_NOTHING)\n" 
"return;\n" 
"//--------------------------------------------------------------------------\n" 
"// Start of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"__global GPUTaskDirectLight *taskDirectLight = &tasksDirectLight[gid];\n" 
"__global Sample *sample = &samples[gid];\n" 
"// Initialize image maps page pointer table\n" 
"INIT_IMAGEMAPS_PAGES\n" 
"//--------------------------------------------------------------------------\n" 
"// End of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"// Nothing was hit, add environmental lights radiance\n" 
"#if defined(PARAM_HAS_ENVLIGHTS)\n" 
"#if defined(PARAM_FORCE_BLACK_BACKGROUND)\n" 
"if (!sample->result.passThroughPath)\n" 
"#endif\n" 
"DirectHitInfiniteLight(\n" 
"taskDirectLight->lastBSDFEvent,\n" 
"&taskState->throughput,\n" 
"VLOAD3F(&rays[gid].d.x), taskDirectLight->lastPdfW,\n" 
"&samples[gid].result\n" 
"LIGHTS_PARAM);\n" 
"#endif\n" 
"if (taskState->pathVertexCount == 1) {\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n" 
"sample->result.alpha = 0.f;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n" 
"sample->result.depth = INFINITY;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n" 
"sample->result.position.x = INFINITY;\n" 
"sample->result.position.y = INFINITY;\n" 
"sample->result.position.z = INFINITY;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n" 
"sample->result.geometryNormal.x = INFINITY;\n" 
"sample->result.geometryNormal.y = INFINITY;\n" 
"sample->result.geometryNormal.z = INFINITY;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n" 
"sample->result.shadingNormal.x = INFINITY;\n" 
"sample->result.shadingNormal.y = INFINITY;\n" 
"sample->result.shadingNormal.z = INFINITY;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n" 
"sample->result.materialID = NULL_INDEX;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_OBJECT_ID)\n" 
"sample->result.objectID = NULL_INDEX;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n" 
"sample->result.uv.u = INFINITY;\n" 
"sample->result.uv.v = INFINITY;\n" 
"#endif\n" 
"}\n" 
"taskState->state = MK_SPLAT_SAMPLE;\n" 
"}\n" 
"//------------------------------------------------------------------------------\n" 
"// Evaluation of the Path finite state machine.\n" 
"//\n" 
"// From: MK_HIT_OBJECT\n" 
"// To: MK_DL_ILLUMINATE or MK_SPLAT_SAMPLE\n" 
"//------------------------------------------------------------------------------\n" 
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths_MK_HIT_OBJECT(\n" 
"KERNEL_ARGS\n" 
") {\n" 
"const size_t gid = get_global_id(0);\n" 
"// Read the path state\n" 
"__global GPUTaskState *taskState = &tasksState[gid];\n" 
"PathState pathState = taskState->state;\n" 
"if (pathState != MK_HIT_OBJECT)\n" 
"return;\n" 
"//--------------------------------------------------------------------------\n" 
"// Start of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"__global BSDF *bsdf = &taskState->bsdf;\n" 
"__global Sample *sample = &samples[gid];\n" 
"//--------------------------------------------------------------------------\n" 
"// End of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"// Something was hit\n" 
"if (taskState->pathVertexCount == 1) {\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n" 
"sample->result.alpha = 1.f;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_DEPTH)\n" 
"sample->result.depth = rayHits[gid].t;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_POSITION)\n" 
"sample->result.position = bsdf->hitPoint.p;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_GEOMETRY_NORMAL)\n" 
"sample->result.geometryNormal = bsdf->hitPoint.geometryN;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_SHADING_NORMAL)\n" 
"sample->result.shadingNormal = bsdf->hitPoint.shadeN;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_MATERIAL_ID)\n" 
"// Initialize image maps page pointer table\n" 
"INIT_IMAGEMAPS_PAGES\n" 
"sample->result.materialID = BSDF_GetMaterialID(bsdf\n" 
"MATERIALS_PARAM);\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_OBJECT_ID)\n" 
"sample->result.objectID = BSDF_GetObjectID(bsdf, sceneObjs);\n" 
"#endif\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_UV)\n" 
"sample->result.uv = bsdf->hitPoint.uv;\n" 
"#endif\n" 
"}\n" 
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n" 
"// Check if it is a light source (note: I can hit only triangle area light sources)\n" 
"if (BSDF_IsLightSource(bsdf)) {\n" 
"__global GPUTaskDirectLight *taskDirectLight = &tasksDirectLight[gid];\n" 
"// Initialize image maps page pointer table\n" 
"INIT_IMAGEMAPS_PAGES\n" 
"DirectHitFiniteLight(\n" 
"taskDirectLight->lastBSDFEvent,\n" 
"&taskState->throughput,\n" 
"rayHits[gid].t, bsdf, taskDirectLight->lastPdfW,\n" 
"&sample->result\n" 
"LIGHTS_PARAM);\n" 
"}\n" 
"#endif\n" 
"// Check if this is the last path vertex (but not also the first)\n" 
"//\n" 
"// I handle as a special case when the path vertex is both the first\n" 
"// and the last: I do direct light sampling without MIS.\n" 
"taskState->state = (sample->result.lastPathVertex && !sample->result.firstPathVertex) ?\n" 
"MK_SPLAT_SAMPLE : MK_DL_ILLUMINATE;\n" 
"}\n" 
"//------------------------------------------------------------------------------\n" 
"// Evaluation of the Path finite state machine.\n" 
"//\n" 
"// From: MK_RT_DL\n" 
"// To: MK_SPLAT_SAMPLE or MK_GENERATE_NEXT_VERTEX_RAY\n" 
"//------------------------------------------------------------------------------\n" 
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths_MK_RT_DL(\n" 
"KERNEL_ARGS\n" 
") {\n" 
"const size_t gid = get_global_id(0);\n" 
"// Read the path state\n" 
"__global GPUTask *task = &tasks[gid];\n" 
"__global GPUTaskState *taskState = &tasksState[gid];\n" 
"PathState pathState = taskState->state;\n" 
"if (pathState != MK_RT_DL)\n" 
"return;\n" 
"//--------------------------------------------------------------------------\n" 
"// Start of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"__global GPUTaskDirectLight *taskDirectLight = &tasksDirectLight[gid];\n" 
"// Initialize image maps page pointer table\n" 
"INIT_IMAGEMAPS_PAGES\n" 
"\n"  
"//--------------------------------------------------------------------------\n" 
"// End of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"float3 connectionThroughput = WHITE;\n" 
"#if defined(PARAM_HAS_PASSTHROUGH) || defined(PARAM_HAS_VOLUMES)\n" 
"const bool continueToTrace =\n" 
"Scene_Intersect(\n" 
"#if defined(PARAM_HAS_VOLUMES)\n" 
"&directLightVolInfos[gid],\n" 
"&task->tmpHitPoint,\n" 
"#endif\n" 
"#if defined(PARAM_HAS_PASSTHROUGH)\n" 
"taskDirectLight->rayPassThroughEvent,\n" 
"#endif\n" 
"&rays[gid], &rayHits[gid], &task->tmpBsdf,\n" 
"&connectionThroughput, WHITE,\n" 
"NULL,\n" 
"// BSDF_Init parameters\n" 
"meshDescs,\n" 
"sceneObjs,\n" 
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n" 
"meshTriLightDefsOffset,\n" 
"#endif\n" 
"vertices,\n" 
"#if defined(PARAM_HAS_NORMALS_BUFFER)\n" 
"vertNormals,\n" 
"#endif\n" 
"#if defined(PARAM_HAS_UVS_BUFFER)\n" 
"vertUVs,\n" 
"#endif\n" 
"#if defined(PARAM_HAS_COLS_BUFFER)\n" 
"vertCols,\n" 
"#endif\n" 
"#if defined(PARAM_HAS_ALPHAS_BUFFER)\n" 
"vertAlphas,\n" 
"#endif\n" 
"triangles\n" 
"MATERIALS_PARAM\n" 
");\n" 
"VSTORE3F(connectionThroughput * VLOAD3F(taskDirectLight->illumInfo.lightRadiance.c), taskDirectLight->illumInfo.lightRadiance.c);\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_IRRADIANCE)\n" 
"VSTORE3F(connectionThroughput * VLOAD3F(taskDirectLight->illumInfo.lightIrradiance.c), taskDirectLight->illumInfo.lightIrradiance.c);\n" 
"#endif\n" 
"#else\n" 
"const bool continueToTrace = false;\n" 
"#endif\n" 
"const bool rayMiss = (rayHits[gid].meshIndex == NULL_INDEX);\n" 
"// If continueToTrace, there is nothing to do, just keep the same state\n" 
"if (!continueToTrace) {\n" 
"__global Sample *sample = &samples[gid];\n" 
"if (rayMiss) {\n" 
"// Nothing was hit, the light source is visible\n" 
"__global BSDF *bsdf = &taskState->bsdf;\n" 
"if (!BSDF_IsShadowCatcher(bsdf MATERIALS_PARAM)) {\n" 
"const float3 lightRadiance = VLOAD3F(taskDirectLight->illumInfo.lightRadiance.c);\n" 
"SampleResult_AddDirectLight(&sample->result, taskDirectLight->illumInfo.lightID,\n" 
"BSDF_GetEventTypes(bsdf\n" 
"MATERIALS_PARAM),\n" 
"VLOAD3F(taskState->throughput.c), lightRadiance,\n" 
"1.f);\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_IRRADIANCE)\n" 
"// The first path vertex is not handled by AddDirectLight(). This is valid\n" 
"// for irradiance AOV only if it is not a SPECULAR material.\n" 
"//\n" 
"// Note: irradiance samples the light sources only here (i.e. no\n" 
"// direct hit, no MIS, it would be useless)\n" 
"if ((sample->result.firstPathVertex) && !(BSDF_GetEventTypes(bsdf\n" 
"MATERIALS_PARAM) & SPECULAR)) {\n" 
"const float3 irradiance = (M_1_PI_F * fabs(dot(\n" 
"VLOAD3F(&bsdf.hitPoint.shadeN.x),\n" 
"VLOAD3F(&rays[gid].d.x)))) *\n" 
"VLOAD3F(taskDirectLight->illumInfo.lightIrradiance.c);\n" 
"VSTORE3F(irradiance, sample->result.irradiance.c);\n" 
"}\n" 
"#endif\n" 
"}\n" 
"taskDirectLight->isLightVisible = true;\n" 
"}\n" 
"// Check if this is the last path vertex\n" 
"if (sample->result.lastPathVertex)\n" 
"pathState = MK_SPLAT_SAMPLE;\n" 
"else\n" 
"pathState = MK_GENERATE_NEXT_VERTEX_RAY;\n" 
"// Save the state\n" 
"taskState->state = pathState;\n" 
"}\n" 
"#if defined(PARAM_HAS_PASSTHROUGH)\n" 
"else {\n" 
"// I generate a new random variable starting from the previous one. I'm\n" 
"// not really sure about the kind of correlation introduced by this\n" 
"// trick.\n" 
"taskDirectLight->rayPassThroughEvent = fabs(taskDirectLight->rayPassThroughEvent - .5f) * 2.f;\n" 
"}\n" 
"#endif\n" 
"}\n" 
"//------------------------------------------------------------------------------\n" 
"// Evaluation of the Path finite state machine.\n" 
"//\n" 
"// From: MK_DL_ILLUMINATE\n" 
"// To: MK_DL_SAMPLE_BSDF or MK_GENERATE_NEXT_VERTEX_RAY\n" 
"//------------------------------------------------------------------------------\n" 
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths_MK_DL_ILLUMINATE(\n" 
"KERNEL_ARGS\n" 
") {\n" 
"const size_t gid = get_global_id(0);\n" 
"// Read the path state\n" 
"__global GPUTask *task = &tasks[gid];\n" 
"__global GPUTaskState *taskState = &tasksState[gid];\n" 
"if (taskState->state != MK_DL_ILLUMINATE)\n" 
"return;\n" 
"//--------------------------------------------------------------------------\n" 
"// Start of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"const uint pathVertexCount = taskState->pathVertexCount;\n" 
"__global BSDF *bsdf = &taskState->bsdf;\n" 
"__global Sample *sample = &samples[gid];\n" 
"__global float *sampleData = Sampler_GetSampleData(sample, samplesData);\n" 
"__global float *sampleDataPathBase = Sampler_GetSampleDataPathBase(sample, sampleData);\n" 
"#if (PARAM_SAMPLER_TYPE != 0)\n" 
"// Used by Sampler_GetSamplePathVertex() macro\n" 
"__global float *sampleDataPathVertexBase = Sampler_GetSampleDataPathVertex(\n" 
"sample, sampleDataPathBase, pathVertexCount);\n" 
"#endif\n" 
"// Read the seed\n" 
"Seed seedValue = task->seed;\n" 
"// This trick is required by Sampler_GetSample() macro\n" 
"Seed *seed = &seedValue;\n" 
"__global GPUTaskDirectLight *taskDirectLight = &tasksDirectLight[gid];\n" 
"// Initialize image maps page pointer table\n" 
"INIT_IMAGEMAPS_PAGES\n" 
"\n"  
"//--------------------------------------------------------------------------\n" 
"// End of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"// It will set eventually to true if the light is visible\n" 
"taskDirectLight->isLightVisible = false;\n" 
"if (!BSDF_IsDelta(bsdf\n" 
"MATERIALS_PARAM) &&\n" 
"DirectLight_Illuminate(\n" 
"#if defined(PARAM_HAS_INFINITELIGHTS)\n" 
"worldCenterX, worldCenterY, worldCenterZ, worldRadius,\n" 
"#endif\n" 
"#if (PARAM_TRIANGLE_LIGHT_COUNT > 0)\n" 
"&task->tmpHitPoint,\n" 
"#endif\n" 
"Sampler_GetSamplePathVertex(pathVertexCount, IDX_DIRECTLIGHT_X),\n" 
"Sampler_GetSamplePathVertex(pathVertexCount, IDX_DIRECTLIGHT_Y),\n" 
"Sampler_GetSamplePathVertex(pathVertexCount, IDX_DIRECTLIGHT_Z),\n" 
"#if defined(PARAM_HAS_PASSTHROUGH)\n" 
"Sampler_GetSamplePathVertex(pathVertexCount, IDX_DIRECTLIGHT_W),\n" 
"#endif\n" 
"VLOAD3F(&bsdf->hitPoint.p.x), &taskDirectLight->illumInfo\n" 
"LIGHTS_PARAM)) {\n" 
"// I have now to evaluate the BSDF\n" 
"taskState->state = MK_DL_SAMPLE_BSDF;\n" 
"} else {\n" 
"// No shadow ray to trace, move to the next vertex ray\n" 
"// however, I have to Check if this is the last path vertex\n" 
"taskState->state = (sample->result.lastPathVertex) ? MK_SPLAT_SAMPLE : MK_GENERATE_NEXT_VERTEX_RAY;\n" 
"}\n" 
"//--------------------------------------------------------------------------\n" 
"// Save the seed\n" 
"task->seed = seedValue;\n" 
"}\n" 
"//------------------------------------------------------------------------------\n" 
"// Evaluation of the Path finite state machine.\n" 
"//\n" 
"// From: MK_DL_SAMPLE_BSDF\n" 
"// To: MK_GENERATE_NEXT_VERTEX_RAY or MK_RT_DL or MK_SPLAT_SAMPLE\n" 
"//------------------------------------------------------------------------------\n" 
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths_MK_DL_SAMPLE_BSDF(\n" 
"KERNEL_ARGS\n" 
") {\n" 
"const size_t gid = get_global_id(0);\n" 
"// Read the path state\n" 
"__global GPUTaskState *taskState = &tasksState[gid];\n" 
"PathState pathState = taskState->state;\n" 
"if (pathState != MK_DL_SAMPLE_BSDF)\n" 
"return;\n" 
"//--------------------------------------------------------------------------\n" 
"// Start of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"const uint pathVertexCount = taskState->pathVertexCount;\n" 
"__global Sample *sample = &samples[gid];\n" 
"// Initialize image maps page pointer table\n" 
"INIT_IMAGEMAPS_PAGES\n" 
"\n"  
"//--------------------------------------------------------------------------\n" 
"// End of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"if (DirectLight_BSDFSampling(\n" 
"&tasksDirectLight[gid].illumInfo,\n" 
"rays[gid].time, sample->result.lastPathVertex, taskState->pathVertexCount,\n" 
"&taskState->bsdf,\n" 
"&rays[gid]\n" 
"LIGHTS_PARAM)) {\n" 
"#if defined(PARAM_HAS_PASSTHROUGH)\n" 
"__global float *sampleData = Sampler_GetSampleData(sample, samplesData);\n" 
"__global float *sampleDataPathBase = Sampler_GetSampleDataPathBase(sample, sampleData);\n" 
"#if (PARAM_SAMPLER_TYPE != 0)\n" 
"// Used by Sampler_GetSamplePathVertex() macro\n" 
"__global float *sampleDataPathVertexBase = Sampler_GetSampleDataPathVertex(\n" 
"sample, sampleDataPathBase, pathVertexCount);\n" 
"#endif\n" 
"__global GPUTask *task = &tasks[gid];\n" 
"Seed seedValue = task->seed;\n" 
"// This trick is required by Sampler_GetSample() macro\n" 
"Seed *seed = &seedValue;\n" 
"// Initialize the pass-through event for the shadow ray\n" 
"tasksDirectLight[gid].rayPassThroughEvent = Sampler_GetSamplePathVertex(pathVertexCount, IDX_DIRECTLIGHT_A);\n" 
"\n"  
"// Save the seed\n" 
"task->seed = seedValue;\n" 
"#endif\n" 
"#if defined(PARAM_HAS_VOLUMES)\n" 
"// Make a copy of current PathVolumeInfo for tracing the\n" 
"// shadow ray\n" 
"directLightVolInfos[gid] = pathVolInfos[gid];\n" 
"#endif\n" 
"// I have to trace the shadow ray\n" 
"taskState->state = MK_RT_DL;\n" 
"} else {\n" 
"// No shadow ray to trace, move to the next vertex ray\n" 
"// however, I have to check if this is the last path vertex\n" 
"taskState->state = (sample->result.lastPathVertex) ? MK_SPLAT_SAMPLE : MK_GENERATE_NEXT_VERTEX_RAY;\n" 
"}\n" 
"}\n" 
"//------------------------------------------------------------------------------\n" 
"// Evaluation of the Path finite state machine.\n" 
"//\n" 
"// From: MK_GENERATE_NEXT_VERTEX_RAY\n" 
"// To: MK_SPLAT_SAMPLE or MK_RT_NEXT_VERTEX\n" 
"//------------------------------------------------------------------------------\n" 
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths_MK_GENERATE_NEXT_VERTEX_RAY(\n" 
"KERNEL_ARGS\n" 
") {\n" 
"const size_t gid = get_global_id(0);\n" 
"// Read the path state\n" 
"__global GPUTask *task = &tasks[gid];\n" 
"__global GPUTaskState *taskState = &tasksState[gid];\n" 
"PathState pathState = taskState->state;\n" 
"if (pathState != MK_GENERATE_NEXT_VERTEX_RAY)\n" 
"return;\n" 
"//--------------------------------------------------------------------------\n" 
"// Start of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"uint pathVertexCount = taskState->pathVertexCount;\n" 
"__global BSDF *bsdf = &taskState->bsdf;\n" 
"__global Sample *sample = &samples[gid];\n" 
"__global float *sampleData = Sampler_GetSampleData(sample, samplesData);\n" 
"__global float *sampleDataPathBase = Sampler_GetSampleDataPathBase(sample, sampleData);\n" 
"#if (PARAM_SAMPLER_TYPE != 0)\n" 
"// Used by Sampler_GetSamplePathVertex() macro\n" 
"__global float *sampleDataPathVertexBase = Sampler_GetSampleDataPathVertex(\n" 
"sample, sampleDataPathBase, pathVertexCount);\n" 
"#endif\n" 
"// Read the seed\n" 
"Seed seedValue = task->seed;\n" 
"// This trick is required by Sampler_GetSample() macro\n" 
"Seed *seed = &seedValue;\n" 
"// Initialize image maps page pointer table\n" 
"INIT_IMAGEMAPS_PAGES\n" 
"__global Ray *ray = &rays[gid];\n" 
"\n"  
"//--------------------------------------------------------------------------\n" 
"// End of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"// Sample the BSDF\n" 
"float3 sampledDir;\n" 
"float lastPdfW;\n" 
"float cosSampledDir;\n" 
"BSDFEvent event;\n" 
"float3 bsdfSample;\n" 
"if (BSDF_IsShadowCatcher(bsdf MATERIALS_PARAM) && tasksDirectLight[gid].isLightVisible) {\n" 
"bsdfSample = BSDF_ShadowCatcherSample(bsdf,\n" 
"&sampledDir, &lastPdfW, &cosSampledDir, &event\n" 
"MATERIALS_PARAM);\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_ALPHA)\n" 
"if (sample->result.firstPathVertex) {\n" 
"// In this case I have also to set the value of the alpha channel to 0.0\n" 
"sample->result.alpha = 0.f;\n" 
"}\n" 
"#endif\n" 
"} else {\n" 
"bsdfSample = BSDF_Sample(bsdf,\n" 
"Sampler_GetSamplePathVertex(pathVertexCount, IDX_BSDF_X),\n" 
"Sampler_GetSamplePathVertex(pathVertexCount, IDX_BSDF_Y),\n" 
"&sampledDir, &lastPdfW, &cosSampledDir, &event, ALL\n" 
"MATERIALS_PARAM);\n" 
"sample->result.passThroughPath = false;\n" 
"}\n" 
"// Russian Roulette\n" 
"const bool rrEnabled = (pathVertexCount >= PARAM_RR_DEPTH);\n" 
"const float rrProb = rrEnabled ? RussianRouletteProb(bsdfSample) : 1.f;\n" 
"const bool rrContinuePath = !rrEnabled || !(rrProb < Sampler_GetSamplePathVertex(pathVertexCount, IDX_RR));\n" 
"// Max. path depth\n" 
"const bool maxPathDepth = (pathVertexCount >= PARAM_MAX_PATH_DEPTH);\n" 
"const bool continuePath = !Spectrum_IsBlack(bsdfSample) && rrContinuePath && !maxPathDepth;\n" 
"if (continuePath) {\n" 
"float3 throughputFactor = WHITE;\n" 
"// RR increases path contribution\n" 
"throughputFactor /= rrProb;\n" 
"// PDF clamping (or better: scaling)\n" 
"const float pdfFactor = (event & SPECULAR) ? 1.f : min(1.f, lastPdfW / PARAM_PDF_CLAMP_VALUE);\n" 
"throughputFactor *= bsdfSample * pdfFactor;\n" 
"VSTORE3F(throughputFactor * VLOAD3F(taskState->throughput.c), taskState->throughput.c);\n" 
"#if defined(PARAM_FILM_CHANNELS_HAS_IRRADIANCE)\n" 
"// This is valid for irradiance AOV only if it is not a SPECULAR material and\n" 
"// first path vertex. Set or update sampleResult.irradiancePathThroughput\n" 
"if (sample->result.firstPathVertex) {\n" 
"if (!(BSDF_GetEventTypes(&taskState->bsdf\n" 
"MATERIALS_PARAM) & SPECULAR))\n" 
"VSTORE3F(M_1_PI_F * fabs(dot(\n" 
"VLOAD3F(&bsdf->hitPoint.shadeN.x),\n" 
"sampledDir)) / rrProb,\n" 
"sample->result.irradiancePathThroughput.c);\n" 
"else\n" 
"VSTORE3F(BLACK, sample->result.irradiancePathThroughput.c);\n" 
"} else\n" 
"VSTORE3F(throughputFactor * VLOAD3F(sample->result.irradiancePathThroughput.c), sample->result.irradiancePathThroughput.c);\n" 
"#endif\n" 
"#if defined(PARAM_HAS_VOLUMES)\n" 
"// Update volume information\n" 
"PathVolumeInfo_Update(&pathVolInfos[gid], event, bsdf\n" 
"MATERIALS_PARAM);\n" 
"#endif\n" 
"Ray_Init2(ray, VLOAD3F(&bsdf->hitPoint.p.x), sampledDir, ray->time);\n" 
"++pathVertexCount;\n" 
"sample->result.firstPathVertex = false;\n" 
"sample->result.lastPathVertex = (pathVertexCount == PARAM_MAX_PATH_DEPTH);\n" 
"if (sample->result.firstPathVertex)\n" 
"sample->result.firstPathVertexEvent = event;\n" 
"taskState->pathVertexCount = pathVertexCount;\n" 
"tasksDirectLight[gid].lastBSDFEvent = event;\n" 
"tasksDirectLight[gid].lastPdfW = lastPdfW;\n" 
"#if defined(PARAM_HAS_PASSTHROUGH)\n" 
"// This is a bit tricky. I store the passThroughEvent in the BSDF\n" 
"// before of the initialization because it can be use during the\n" 
"// tracing of next path vertex ray.\n" 
"// This sampleDataPathVertexBase is used inside Sampler_GetSamplePathVertex() macro\n" 
"__global float *sampleDataPathVertexBase = Sampler_GetSampleDataPathVertex(\n" 
"sample, sampleDataPathBase, pathVertexCount);\n" 
"taskState->bsdf.hitPoint.passThroughEvent = Sampler_GetSamplePathVertex(pathVertexCount, IDX_PASSTHROUGH);\n" 
"#endif\n" 
"pathState = MK_RT_NEXT_VERTEX;\n" 
"} else\n" 
"pathState = MK_SPLAT_SAMPLE;\n" 
"// Save the state\n" 
"taskState->state = pathState;\n" 
"//--------------------------------------------------------------------------\n" 
"// Save the seed\n" 
"task->seed = seedValue;\n" 
"}\n" 
"//------------------------------------------------------------------------------\n" 
"// Evaluation of the Path finite state machine.\n" 
"//\n" 
"// From: MK_SPLAT_SAMPLE\n" 
"// To: MK_NEXT_SAMPLE\n" 
"//------------------------------------------------------------------------------\n" 
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths_MK_SPLAT_SAMPLE(\n" 
"KERNEL_ARGS\n" 
") {\n" 
"const size_t gid = get_global_id(0);\n" 
"// Read the path state\n" 
"__global GPUTask *task = &tasks[gid];\n" 
"__global GPUTaskState *taskState = &tasksState[gid];\n" 
"PathState pathState = taskState->state;\n" 
"if (pathState != MK_SPLAT_SAMPLE)\n" 
"return;\n" 
"//--------------------------------------------------------------------------\n" 
"// Start of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"__global Sample *sample = &samples[gid];\n" 
"__global float *sampleData = Sampler_GetSampleData(sample, samplesData);\n" 
"// Read the seed\n" 
"Seed seedValue = task->seed;\n" 
"\n"  
"//--------------------------------------------------------------------------\n" 
"// End of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"// Initialize Film radiance group pointer table\n" 
"__global float *filmRadianceGroup[PARAM_FILM_RADIANCE_GROUP_COUNT];\n" 
"#if defined(PARAM_FILM_RADIANCE_GROUP_0)\n" 
"filmRadianceGroup[0] = filmRadianceGroup0;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_RADIANCE_GROUP_1)\n" 
"filmRadianceGroup[1] = filmRadianceGroup1;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_RADIANCE_GROUP_2)\n" 
"filmRadianceGroup[2] = filmRadianceGroup2;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_RADIANCE_GROUP_3)\n" 
"filmRadianceGroup[3] = filmRadianceGroup3;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_RADIANCE_GROUP_4)\n" 
"filmRadianceGroup[4] = filmRadianceGroup4;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_RADIANCE_GROUP_5)\n" 
"filmRadianceGroup[5] = filmRadianceGroup5;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_RADIANCE_GROUP_6)\n" 
"filmRadianceGroup[6] = filmRadianceGroup6;\n" 
"#endif\n" 
"#if defined(PARAM_FILM_RADIANCE_GROUP_7)\n" 
"filmRadianceGroup[7] = filmRadianceGroup7;\n" 
"#endif\n" 
"if (PARAM_SQRT_VARIANCE_CLAMP_MAX_VALUE > 0.f) {\n" 
"// Radiance clamping\n" 
"VarianceClamping_Clamp(&sample->result, PARAM_SQRT_VARIANCE_CLAMP_MAX_VALUE\n" 
"FILM_PARAM);\n" 
"}\n" 
"Sampler_SplatSample(&seedValue, sample, sampleData\n" 
"FILM_PARAM);\n" 
"taskStats[gid].sampleCount += 1;\n" 
"// Save the state\n" 
"taskState->state = MK_NEXT_SAMPLE;\n" 
"//--------------------------------------------------------------------------\n" 
"// Save the seed\n" 
"task->seed = seedValue;\n" 
"}\n" 
"//------------------------------------------------------------------------------\n" 
"// Evaluation of the Path finite state machine.\n" 
"//\n" 
"// From: MK_NEXT_SAMPLE\n" 
"// To: MK_GENERATE_CAMERA_RAY\n" 
"//------------------------------------------------------------------------------\n" 
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths_MK_NEXT_SAMPLE(\n" 
"KERNEL_ARGS\n" 
") {\n" 
"const size_t gid = get_global_id(0);\n" 
"// Read the path state\n" 
"__global GPUTask *task = &tasks[gid];\n" 
"__global GPUTaskState *taskState = &tasksState[gid];\n" 
"PathState pathState = taskState->state;\n" 
"if (pathState != MK_NEXT_SAMPLE)\n" 
"return;\n" 
"//--------------------------------------------------------------------------\n" 
"// Start of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"__global Sample *sample = &samples[gid];\n" 
"__global float *sampleData = Sampler_GetSampleData(sample, samplesData);\n" 
"// Read the seed\n" 
"Seed seedValue = task->seed;\n" 
"//--------------------------------------------------------------------------\n" 
"// End of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"Sampler_NextSample(&seedValue, sample, sampleData);\n" 
"// Save the state\n" 
"taskState->state = MK_GENERATE_CAMERA_RAY;\n" 
"//--------------------------------------------------------------------------\n" 
"// Save the seed\n" 
"task->seed = seedValue;\n" 
"}\n" 
"//------------------------------------------------------------------------------\n" 
"// Evaluation of the Path finite state machine.\n" 
"//\n" 
"// From: MK_GENERATE_CAMERA_RAY\n" 
"// To: MK_RT_NEXT_VERTEX\n" 
"//------------------------------------------------------------------------------\n" 
"__kernel __attribute__((work_group_size_hint(64, 1, 1))) void AdvancePaths_MK_GENERATE_CAMERA_RAY(\n" 
"KERNEL_ARGS\n" 
") {\n" 
"const size_t gid = get_global_id(0);\n" 
"// Read the path state\n" 
"__global GPUTask *task = &tasks[gid];\n" 
"__global GPUTaskState *taskState = &tasksState[gid];\n" 
"PathState pathState = taskState->state;\n" 
"if (pathState != MK_GENERATE_CAMERA_RAY)\n" 
"return;\n" 
"//--------------------------------------------------------------------------\n" 
"// Start of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"__global Sample *sample = &samples[gid];\n" 
"__global float *sampleData = Sampler_GetSampleData(sample, samplesData);\n" 
"// Read the seed\n" 
"Seed seedValue = task->seed;\n" 
"__global Ray *ray = &rays[gid];\n" 
"\n"  
"//--------------------------------------------------------------------------\n" 
"// End of variables setup\n" 
"//--------------------------------------------------------------------------\n" 
"GenerateEyePath(&tasksDirectLight[gid], taskState, sample, sampleData, camera,\n" 
"filmWidth, filmHeight,\n" 
"filmSubRegion0, filmSubRegion1, filmSubRegion2, filmSubRegion3,\n" 
"#if defined(PARAM_USE_FAST_PIXEL_FILTER)\n" 
"pixelFilterDistribution,\n" 
"#endif\n" 
"ray, &seedValue);\n" 
"// taskState->state is set to RT_NEXT_VERTEX inside GenerateEyePath()\n" 
"// Re-initialize the volume information\n" 
"#if defined(PARAM_HAS_VOLUMES)\n" 
"PathVolumeInfo_Init(&pathVolInfos[gid]);\n" 
"#endif\n" 
"//--------------------------------------------------------------------------\n" 
"// Save the seed\n" 
"task->seed = seedValue;\n" 
"}\n" 
; } } 
