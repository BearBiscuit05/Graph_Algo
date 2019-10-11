//
// Created by Thoh Testarossa on 2019-03-12
//

#pragma once

#ifndef GRAPH_ALGO_PAGERANKGPU_H
#define GRAPH_ALGO_PAGERANKGPU_H

#include "PageRank.h"
#include "../../include/GPUconfig.h"
#include "kernel_src/PageRankGPU_kernel.h"

template <typename VertexValueType, typename MessageValueType>
class PageRankGPU : public PageRank<VertexValueType, MessageValueType>
{
public:
    PageRankGPU();

    void Init(int vCount, int eCount, int numOfInitV) override;
    void GraphInit(Graph<VertexValueType> &g, std::set<int> &activeVertices, const std::vector<int> &initVList) override;
    void Deploy(int vCount, int eCount, int numOfInitV) override;
    void Free() override;

    int MSGApply_array(int vCount, int eCount, Vertex *vSet, int numOfInitV, const int *initVSet, VertexValueType *vValues, MessageValueType *mValues) override;
    int MSGGenMerge_array(int vCount, int eCount, const Vertex *vSet, const Edge *eSet, int numOfInitV, const int *initVSet, const VertexValueType *vValues, MessageValueType *mValues) override;

protected:
    int vertexLimit;
    int mPerMSGSet;
    int ePerEdgeSet;

    VertexValueType *vValueSet; //all value size = vCount
    double *d_vValueSet;        //limit size = vertexLimit

    int *mDstSet;  //limist size = max(mPerMSGSet, ePerEdgeSet)
    int *d_mDstSet;//limist size = max(mPerMSGSet, ePerEdgeSet)
    PRA_MSG *mValueSet;
    PRA_MSG *d_mValueSet; //limist size = max(mPerMSGSet, ePerEdgeSet)

    Vertex *d_vSet; //limit size = vertexLimit
    Edge *d_eGSet;  //limit size = ePerEdgeSet

    PRA_MSG *mMergedMSGValueSet; //eCount
    PRA_MSG *mTransformedMergedMSGValueSet; //limist size = max(mPerMSGSet, ePerEdgeSet)
    PRA_MSG *d_mTransformedMergedMSGValueSet; //limist size = max(mPerMSGSet, ePerEdgeSet)

private:
    auto MSGGenMerge_GPU_MVCopy(Vertex *d_vSet, const Vertex *vSet,
                                double *d_vValues, const double *vValues,
                                PRA_MSG *d_mTransformedMergedMSGValueSet,
                                PRA_MSG *mTransformedMergedMSGValueSet,
                                int vGCount, int eGCount);

    auto MSGApply_GPU_VVCopy(Vertex *d_vSet, Vertex *vSet,
                             double *d_vValues, double *vValues,
                             int vGCount);
};

#endif //GRAPH_ALGO_PAGERANKGPU_H
