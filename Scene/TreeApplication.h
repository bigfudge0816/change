#pragma once

#include "Globals.h"
#include "../OpenGL/ShaderProgram.h"
#include "Tree.h"
#include "AttractorPointCloud.h"
#include "../kd_tree.h"
#include "Camera.h"

class TreeApplication {
private:
    TreeParameters treeParameters;
    std::vector<Tree> sceneTrees; // trees in the scene
    std::vector<AttractorPointCloud> sceneAttractorPointClouds; // attractor point clouds in the scene
    std::vector<glm::vec3> currentSketchPoints; // the sketch points in screen space of the current sketch stroke
	std::vector<KdTree> sceneKdTrees;
	
    // App management variables
    int currentlySelectedTreeIndex;
    int currentlySelectedAttractorPointCloudIndex;
	int currentlySelectedKdTreeIndex;

public:
	std::vector<Point> global_pcd;
    TreeApplication() : currentlySelectedTreeIndex(-1), currentlySelectedAttractorPointCloudIndex(-1) {
        treeParameters = TreeParameters();
        std::vector<Tree> sceneTrees = std::vector<Tree>();
        std::vector<AttractorPointCloud> sceneAttractorPointClouds = std::vector<AttractorPointCloud>();
		
		std::vector<KdTree> sceneKdTrees = std::vector<KdTree>();
    }

    void DestroyTrees() {
        for (unsigned int t = 0; t < (unsigned int)sceneTrees.size(); ++t) {
            sceneTrees[t].DestroyMeshes();
        }
    }
    void DestroyAttractorPointClouds() {
        for (unsigned int ap = 0; ap < (unsigned int)sceneAttractorPointClouds.size(); ++ap) {
            sceneAttractorPointClouds[ap].destroy();
        }
    }

    // Scene Editing Functions
    void AddTreeToScene() {
        sceneTrees.emplace_back(Tree());
        currentlySelectedTreeIndex = (int)(sceneTrees.size()) - 1;
    }
    void AddAttractorPointCloudToScene() {
        sceneAttractorPointClouds.emplace_back(AttractorPointCloud());
        currentlySelectedAttractorPointCloudIndex = (int)(sceneAttractorPointClouds.size()) - 1;
    }
	void AddKdTreeToScene() {
		sceneKdTrees.emplace_back(KdTree());
		currentlySelectedKdTreeIndex = (int)(sceneKdTrees.size()) - 1;
	}
	KdTree& GetSelectedKdTree() {
		if (currentlySelectedKdTreeIndex != -1) {
			return sceneKdTrees[currentlySelectedKdTreeIndex];
		}
		return KdTree(); // a bad temporary tree!
	}

    AttractorPointCloud& GetSelectedAttractorPointCloud() {
        if (currentlySelectedAttractorPointCloudIndex != -1) {
            return sceneAttractorPointClouds[currentlySelectedAttractorPointCloudIndex];
        }
        return AttractorPointCloud(); // a bad temporary attractor point cloud! TODO replace this functionality
    }
    Tree GetSelectedTree() {
        if (currentlySelectedTreeIndex != -1) {
            return sceneTrees[currentlySelectedTreeIndex];
        }
        return Tree(); // a bad temporary tree!
    }
    const Tree& GetSelectedTreeConst() const {
        if (currentlySelectedTreeIndex != -1) {
            return sceneTrees[currentlySelectedTreeIndex];
        }
        return Tree(); // a bad temporary tree!
    }
    const std::vector<glm::vec3>& GetSketchPointsConst() const { return currentSketchPoints; }
    std::vector<glm::vec3>& GetSketchPoints() { return currentSketchPoints; }
    void ClearSketchPoints() { currentSketchPoints.clear(); }

    void IterateSelectedTreeInSelectedAttractorPointCloud();
    void RegrowSelectedTreeInSelectedAttractorPointCloud();
    void ComputeWorldSpaceSketchPoints(const Camera& camera);
    void GenerateSketchAttractorPointCloud();

    TreeParameters& GetTreeParameters() { return treeParameters; }
    //const TreeParameters& GetTreeParametersConst() const { return treeParameters; }

    void ExportTreeAsObj() const { GetSelectedTreeConst().ExportAsObj(); }

    // Functions for drawing the scene
    void DrawAttractorPointClouds(ShaderProgram& sp) {
        for (unsigned int ap = 0; ap < (unsigned int)sceneAttractorPointClouds.size(); ++ap) {
            AttractorPointCloud& currentAPC = sceneAttractorPointClouds[ap];
            if (currentAPC.ShouldDisplay()) {
                sp.Draw(sceneAttractorPointClouds[ap]);
            }
        }
    }
    void DrawTrees(ShaderProgram& sp) {
        for (unsigned int t = 0; t < (unsigned int)sceneTrees.size(); ++t) {
            Tree& currentTree = sceneTrees[t];
            if (currentTree.HasBeenCreated()) {
                sp.setUniformColor("u_color", currentTree.GetBranchColor());
                sp.Draw(currentTree.GetTreeMesh());
                sp.setUniformColor("u_color", currentTree.GetLeafColor());
                sp.Draw(currentTree.GetLeavesMesh());
            }
        }
    }
};
