#include "Tree.h"
#include "glm/gtc/matrix_transform.hpp"
#include <iostream>

/// TreeBranch Class Functions

void TreeBranch::AddAxillaryBuds(const Bud& sourceBud, const int numBuds, const float internodeLength) {
    // Create a temporary list of Buds that will be inserted in this branch's list of buds
    std::vector<Bud> newBuds = std::vector<Bud>();

    // Direction in which growth occurs
    const glm::vec3 newShootGrowthDir = glm::normalize(sourceBud.naturalGrowthDir + OPTIMAL_GROWTH_DIR_WEIGHT * sourceBud.optimalGrowthDir + TROPISM_DIR_WEIGHT * TROPISM_VECTOR);

    // Axillary bud orientation: Golden angle of 137.5 about the growth axis
    glm::vec3 crossVec = (std::abs(glm::dot(newShootGrowthDir, WORLD_UP_VECTOR)) > 0.99f) ? glm::vec3(1.0f, 0.0f, 0.0f) : WORLD_UP_VECTOR; // avoid glm::cross returning a nan or 0-vector
    const glm::quat branchQuat = glm::angleAxis(glm::radians(22.5f), glm::normalize(glm::cross(newShootGrowthDir, crossVec)));
    const glm::mat4 budRotMat = glm::toMat4(branchQuat);

    // Direction in which the bud itself is oriented
    glm::vec3 budGrowthDir = glm::normalize(glm::vec3(budRotMat * glm::vec4(newShootGrowthDir, 0.0f)));

    // Buds will be inserted @ current terminal bud pos + (float)b * branchGrowthDir * internodeLength
    Bud& terminalBud = buds[buds.size() - 1]; // last bud is always the terminal bud
    for (int b = 0; b < numBuds; ++b) {
        // Account for golden angle here
        const float rotAmt = 137.5f * (float)((buds.size() + b) /** (axisOrder + 1)*/);
        const glm::quat branchQuatGoldenAngle = glm::angleAxis(glm::radians(rotAmt), newShootGrowthDir);
        const glm::mat4 budRotMatGoldenAngle = glm::toMat4(branchQuatGoldenAngle);
        const glm::vec3 budGrowthGoldenAngle = glm::normalize(glm::vec3(budRotMatGoldenAngle * glm::vec4(budGrowthDir, 0.0f)));
        
        // Special measure taken:
        // If this is the first bud among the buds to be added, give it the internode length of the the terminal bud.
        // But, if this is the first time the terminal bud is growing, make the internode length 0 instead. The bud shouldn't grow at all.
        const float internodeLengthChecked = (buds.size() == 1) ? ((b == 0) ? 0.0f : internodeLength) : ((b == 0) ? terminalBud.internodeLength : internodeLength);
        newBuds.emplace_back(terminalBud.point + (float)b * newShootGrowthDir * internodeLength, budGrowthGoldenAngle, glm::vec3(0.0f),
                             0.0f, 0.0f, 0.0f, -1, internodeLengthChecked, 0.0f, 0, AXILLARY, DORMANT);
    }
    // Update terminal bud position
    terminalBud.point = terminalBud.point + (float)(numBuds) * newShootGrowthDir * internodeLength;
    terminalBud.internodeLength = internodeLength;
    buds.insert(buds.begin() + buds.size() - 1, newBuds.begin(), newBuds.end());
}


/// Tree Class Functions

void Tree::IterateGrowth(KdTree& currentKdTree, std::vector<AttractorPoint>& attractorPoints,glm::vec3& minAttrPt, glm::vec3& maxAttrPt, TreeParameters& treeParams, bool useGPU) {
    
    ResetState(useGPU);               // Prepare all data to be iterated over again, e.g. set accumQ / resourceBH for all buds back to 0
    
    for (int n = 0; n < treeParams.numSpaceColonizationIterations; ++n) {
        didUpdate = false;
        #ifdef ENABLE_DEBUG_OUTPUT
        std::cout << "Iteration #: " << n << std::endl;
        #endif

        #ifdef ENABLE_DEBUG_OUTPUT
        auto start = std::chrono::system_clock::now();
        #endif
        PerformSpaceColonization(currentKdTree,attractorPoints, minAttrPt, maxAttrPt, treeParams, useGPU); // 1. Compute Q (presence of space/light) and optimal growth direction using space colonization
        #ifdef ENABLE_DEBUG_OUTPUT
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::time_t end_time = std::chrono::system_clock::to_time_t(end);
        std::cout << "Elapsed time for Space Colonization: " << elapsed_seconds.count() << "s\n";
        #endif

        #ifdef ENABLE_DEBUG_OUTPUT
        start = std::chrono::system_clock::now();
        #endif
        ComputeBHModelBasipetalPass(treeParams);             // 2. Using BH Model, flow resource basipetally and then acropetally
        ComputeBHModelAcropetalPass(treeParams);
        #ifdef ENABLE_DEBUG_OUTPUT
        end = std::chrono::system_clock::now();
        elapsed_seconds = end - start;
        end_time = std::chrono::system_clock::to_time_t(end);
        std::cout << "Elapsed time for Computing BH Model (both passes): " << elapsed_seconds.count() << "s\n";
        #endif

        #ifdef ENABLE_DEBUG_OUTPUT
        start = std::chrono::system_clock::now();
        #endif
        AppendNewShoots(n, treeParams);                         // 3. Add new shoots using the resource computed in previous step
        #ifdef ENABLE_DEBUG_OUTPUT
        end = std::chrono::system_clock::now();
        elapsed_seconds = end - start;
        end_time = std::chrono::system_clock::to_time_t(end);
        std::cout << "Elapsed time for Appending New Shoots: " << elapsed_seconds.count() << "s\n";
        #endif
        
        #ifdef ENABLE_DEBUG_OUTPUT
        start = std::chrono::system_clock::now();
        #endif
        ResetState(useGPU);                      // 4. Prepare all data to be iterated over again, e.g. set accumQ / resourceBH for all buds back to 0
        #ifdef ENABLE_DEBUG_OUTPUT
        end = std::chrono::system_clock::now();
        elapsed_seconds = end - start;
        end_time = std::chrono::system_clock::to_time_t(end);
        std::cout << "Elapsed time for State Resetting: " << elapsed_seconds.count() << "s\n\n";
        #endif
    }

    #ifdef ENABLE_DEBUG_OUTPUT
    auto start = std::chrono::system_clock::now();
    #endif
    ComputeBranchRadii(treeParams);
    #ifdef ENABLE_DEBUG_OUTPUT
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Elapsed time for Computing Branch Radii: " << elapsed_seconds.count() << "s\n";
    #endif
}

void Tree::PerformSpaceColonization(KdTree& currentKdTree,std::vector<AttractorPoint>& attractorPoints, glm::vec3& minAttrPt, glm::vec3& maxAttrPt, TreeParameters& treeParams, bool useGPU) {
    /*#ifdef ENABLE_DEBUG_OUTPUT
    auto start = std::chrono::system_clock::now();
    #endif*/
    //RemoveAttractorPoints(attractorPoints);
    /*#ifdef ENABLE_DEBUG_OUTPUT
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Total Elapsed time for Attractor Point Removal: " << elapsed_seconds.count() << "s\n";
    #endif*/
	
    if (attractorPoints.size() == 0) { std::cout << "attractorPoints.size() == 0" << std::endl; return; }

    #ifdef ENABLE_DEBUG_OUTPUT
    auto start = std::chrono::system_clock::now();
    #endif
    if (useGPU) {
		//PerformSpaceColonizationGPU(currentKdTree, attractorPoints, minAttrPt, maxAttrPt, reconstructUniformGrid, resetAttrPtState);
		return;
    } else {
		#ifdef ENABLE_DEBUG_OUTPUT
		auto start = std::chrono::system_clock::now();
		#endif
		RemoveAttractorPoints(currentKdTree);
		#ifdef ENABLE_DEBUG_OUTPUT
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::time_t end_time = std::chrono::system_clock::to_time_t(end);
		std::cout << "Total Elapsed time for RemoveAttractorPoints: " << elapsed_seconds.count() << "s\n";
		#endif
		PerformSpaceColonizationCPU(currentKdTree,treeParams);
    }
    #ifdef ENABLE_DEBUG_OUTPUT
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Total Elapsed time for Actual Space Col: " << elapsed_seconds.count() << "s\n";
    #endif
}

void Tree::PerformSpaceColonizationCPU(KdTree& currentKdTree,TreeParameters& treeParams) {

	for (unsigned int br = 0; br < (unsigned int)branches.size(); ++br) {
		std::vector<Bud>& buds = branches[br].buds;
		for (unsigned int bu = 0; bu < (unsigned int)buds.size(); ++bu) {
			if (buds[bu].internodeLength > 0.0f && buds[bu].fate == DORMANT) {
				Point temp;
				std::vector<Point> out;
				float perceptionR = 6 * buds[bu].internodeLength;
				temp.x = buds[bu].point.x;
				temp.y = buds[bu].point.y;
				temp.z = buds[bu].point.z;
				
				searchRadiusNeighbor(&currentKdTree, temp, perceptionR, out,false);
				for (int i = 0; i < out.size(); i++)
				{
					glm::vec3 budToPtDir = glm::vec3(out[i].x, out[i].y, out[i].z) - buds[bu].point;
					budToPtDir = glm::normalize(budToPtDir);
					const float dotProd = glm::dot(budToPtDir, buds[bu].naturalGrowthDir);
					if (dotProd > std::abs(treeParams.perceptionCosTheta))
					{
						buds[bu].numNearbyAttrPts++;
						buds[bu].optimalGrowthDir += budToPtDir;
						buds[bu].environmentQuality = 1.0f;
					}
					else
					{
						undoRemoved(&currentKdTree, out[i]);
					}
				}
				buds[bu].optimalGrowthDir = buds[bu].numNearbyAttrPts > 0 ? glm::normalize(buds[bu].optimalGrowthDir) : glm::vec3(0.0f);
			}
		}
	} 
}

void Tree::PerformSpaceColonizationGPU(KdTree& currentKdTree,std::vector<AttractorPoint>& attractorPoints, glm::vec3& minAttrPt, glm::vec3& maxAttrPt, bool& reconstructUniformGrid, bool& resetAttrPtState) {
    // Assemble array of buds
    std::vector<Bud> buds = std::vector<Bud>(); // TODO replace this vector
    for (unsigned int br = 0; br < (unsigned int)branches.size(); ++br) {
        const std::vector<Bud> branchBuds = branches[br].GetBuds();
        for (unsigned int bu = 0; bu < branchBuds.size(); ++bu) {
            buds.emplace_back(branchBuds[bu]);
        }
    }

    Bud* budArray = new Bud[buds.size()];
    for (int i = 0; i < buds.size(); ++i) {
        budArray[i] = buds[i];
    }

    // Need to make sure that the grid bounds contain all currently existing buds
    glm::vec3& minGridPoint = minAttrPt;
    glm::vec3& maxGridPoint = maxAttrPt;
    for (unsigned int br = 0; br < (unsigned int)branches.size(); ++br) {
        const std::vector<Bud>& branchBuds = branches[br].GetBuds();
        for (unsigned int bu = 0; bu < branchBuds.size(); ++bu) {
            const Bud& currentBud = branchBuds[bu];
            if (currentBud.point.x < minGridPoint.x) {
                minGridPoint.x = currentBud.point.x;
                reconstructUniformGrid = true;
            }
            if (currentBud.point.y < minGridPoint.y) {
                minGridPoint.y = currentBud.point.y;
                reconstructUniformGrid = true;
            }
            if (currentBud.point.z < minGridPoint.z) {
                minGridPoint.z = currentBud.point.z;
                reconstructUniformGrid = true;
            }
            if (currentBud.point.x > maxGridPoint.x) {
                maxGridPoint.x = currentBud.point.x;
                reconstructUniformGrid = true;
            }
            if (currentBud.point.y > maxGridPoint.y) {
                maxGridPoint.y = currentBud.point.y;
                reconstructUniformGrid = true;
            }
            if (currentBud.point.z > maxGridPoint.z) {
                maxGridPoint.z = currentBud.point.z;
                reconstructUniformGrid = true;
            }
        }
    }
    reconstructUniformGrid = true; // why does this FIX ITTTT
    //std::cout << "reconstruct grid after changing points: " << reconstructUniformGrid << std::endl;
    const int maxGridSideLength = (int)std::ceil(std::abs(std::fmax(std::fmax(maxGridPoint.x - minGridPoint.x, maxGridPoint.y - minGridPoint.y), maxGridPoint.z - minGridPoint.z)));
    const float gridCellWidth = maxGridSideLength / (float)UNIFORM_GRID_CELL_COUNT;
    const int numTotalGridCells = UNIFORM_GRID_CELL_COUNT * UNIFORM_GRID_CELL_COUNT * UNIFORM_GRID_CELL_COUNT;

    /*TreeApp::PerformSpaceColonizationParallel(budArray, (int)buds.size(), attractorPoints.data(), (int)attractorPoints.size(),
                                              UNIFORM_GRID_CELL_COUNT, numTotalGridCells, minGridPoint, gridCellWidth, reconstructUniformGrid, resetAttrPtState);*/
    // Copy bud info back to the tree
    int budCounter = 0;
    for (unsigned int br = 0; br < (unsigned int)branches.size(); ++br) {
        std::vector<Bud>& branchBuds = branches[br].buds;
        for (unsigned int bu = 0; bu < branchBuds.size(); ++bu) {
            branchBuds[bu] = budArray[bu + budCounter];
        }
        budCounter += (unsigned int)branchBuds.size();
    }
}

float Tree::ComputeQAccumRecursive(TreeParameters& treeParams,TreeBranch& branch) {
    float accumQ = 0.0f;
    for (int bu = (int)branch.buds.size() - 1; bu >= 0; --bu) {
        Bud& currentBud = branch.buds[bu];
        switch (currentBud.type) {
        case TERMINAL:
            accumQ += currentBud.environmentQuality;
            break;
        case AXILLARY:
        {
            switch (currentBud.fate) {
            case DORMANT:
                accumQ += currentBud.environmentQuality;
                break;
            case FORMED_BRANCH:
                accumQ += ComputeQAccumRecursive(treeParams,branches[currentBud.formedBranchIndex]);
                break;
            case FORMED_FLOWER: // double check if we include the resource in this case TODO
                accumQ += ComputeQAccumRecursive(treeParams,branches[currentBud.formedBranchIndex]);
                break;
            default: // includes ABORT case - ignore this bud
                break;
            }
            break;
        }
        }
        currentBud.accumEnvironmentQuality = accumQ;
    }
    return accumQ;
}
void Tree::ComputeBHModelBasipetalPass(TreeParameters& treeParams) {
    // TODO: This is a little inefficient and I should eventually memoize the information so we don't have to recompute branches.
    ComputeQAccumRecursive(treeParams,branches[0]); // ignore return value
}

// make this a non-member helper function in the cpp file
void Tree::ComputeResourceFlowRecursive(TreeParameters& treeParams,TreeBranch& branch, float resource) {
    for (unsigned int bu = 0; bu < (unsigned int)branch.buds.size(); ++bu) {
        Bud& currentBud = branch.buds[bu];
        switch (currentBud.type) {
        case TERMINAL:
            currentBud.resourceBH = resource;
            break;
        case AXILLARY:
            switch (currentBud.fate) {
            case DORMANT:
                currentBud.resourceBH = resource;
                break;
                // Have to scope w/ brackets for nontrivial cases, apparently: https://stackoverflow.com/questions/10381144/error-c2361-initialization-of-found-is-skipped-by-default-label
            case FORMED_BRANCH: { // It is assumed that these buds always occur at the 0th index in the vector
                TreeBranch& axillaryBranch = branches[currentBud.formedBranchIndex];
                const float Qm = branch.buds[bu + 1].accumEnvironmentQuality; // Q on main axis
                const float Ql = axillaryBranch.buds[1].accumEnvironmentQuality; // Q on axillary axis
                const float denom = treeParams.BHLambda * Qm + (1.0f - treeParams.BHLambda) * Ql;
                currentBud.resourceBH = resource * (treeParams.BHLambda * Qm) / denom; // formula for main axis
                ComputeResourceFlowRecursive(treeParams,axillaryBranch, resource * (1.0f - treeParams.BHLambda) * Ql / denom); // call this function on the axillary branch with the other formula
                resource = currentBud.resourceBH; // Resource reaching the remaining buds in this branch have the attenuated resource
                break;
            }
            case FORMED_FLOWER:
                currentBud.resourceBH = 0.0f;
                break;
            default: // FORMED_FLOWER or ABORT
                currentBud.resourceBH = 0.0f;
                break;

            }
            break;
        }
    }
}
void Tree::ComputeBHModelAcropetalPass(TreeParameters& treeParams) { // Recursive like basipetal pass, but will definitely need to memoize or something
                                     // pass in the first branch and the base amount of resource (v)
    ComputeResourceFlowRecursive(treeParams,branches[0], (branches[0].buds[0].type == TERMINAL) ? branches[0].buds[0].accumEnvironmentQuality * 1.0f : branches[0].buds[0].accumEnvironmentQuality * treeParams.BHAlpha);
}

// Determine whether to grow new shoots and their length(s)
void Tree::AppendNewShoots(int n,  TreeParameters& treeParams) {
    const unsigned int numBranches = (unsigned int)branches.size();
    for (unsigned int br = 0; br < numBranches; ++br) {
        TreeBranch& currentBranch = branches[br];
        std::vector<Bud>& buds = currentBranch.buds;
        const unsigned int numBuds = (unsigned int)buds.size();
        for (unsigned int bu = 0; bu < numBuds; ++bu) {
            Bud& currentBud = buds[bu];
            const int numMetamers = static_cast<int>(std::floor(currentBud.resourceBH));
            if (numMetamers > 0) {
                const float metamerLength = currentBud.resourceBH / (float)numMetamers * treeParams.internodeScale;
                switch (currentBud.type) {
                case TERMINAL: {
                    didUpdate = true;
                    currentBranch.AddAxillaryBuds(currentBud, numMetamers, metamerLength);
                    break;
                }
                case AXILLARY: {
                    if (currentBud.fate == DORMANT) {
                        didUpdate = true;
                        TreeBranch newBranch = TreeBranch(currentBud.point, currentBud.naturalGrowthDir, branches[br].axisOrder + 1, br);
                        newBranch.AddAxillaryBuds(currentBud, numMetamers, metamerLength);
                        branches.emplace_back(newBranch);
                        currentBud.fate = FORMED_BRANCH;
                        currentBud.formedBranchIndex = (int)branches.size() - 1;
                    }
                    break;
                }
                }
            }
        }
    }
}

// Using the "pipe model" described in the paper, compute the radius of each branch
float Tree::ComputeBranchRadiiRecursive(TreeBranch& branch,  TreeParameters& treeParams) {
    float branchRadius = treeParams.minimumBranchRadius;
    for (int bu = (int)branch.buds.size() - 1; bu >= 0; --bu) {
        Bud& currentBud = branch.buds[bu];
        switch (currentBud.type) {
        case TERMINAL:
            break;
        case AXILLARY: {
            switch (currentBud.fate) {
            case DORMANT:
                branchRadius += std::pow(currentBud.branchRadius, PIPE_EXPONENT);
                break;
            case FORMED_BRANCH:
                branchRadius = std::pow(std::pow(branchRadius, PIPE_EXPONENT) + std::pow(ComputeBranchRadiiRecursive(branches[currentBud.formedBranchIndex], treeParams), PIPE_EXPONENT), 1.0f / PIPE_EXPONENT);
                break;
            case FORMED_FLOWER:
                // don't change radius for now
                break;
            default: // includes the ABORT case of bud fate
                break;
            }
            break;
        }
        }
        currentBud.branchRadius = std::min(branchRadius, treeParams.maximumBranchRadius);
    }
    return branchRadius;
}

void Tree::ComputeBranchRadii( TreeParameters& treeParams) {
    ComputeBranchRadiiRecursive(branches[0], treeParams); // ignore return value
}

void Tree::ResetState(bool useGPU) {
    for (unsigned int br = 0; br < (unsigned int)branches.size(); ++br) {
        std::vector<Bud>& buds = branches[br].buds;
        for (int bu = 0; bu < buds.size(); ++bu) {
            Bud& currentBud = buds[bu];
            currentBud.accumEnvironmentQuality = 0.0f;
            currentBud.environmentQuality = 0.0f;
            currentBud.numNearbyAttrPts = 0;
            currentBud.optimalGrowthDir = glm::vec3(0.0f);
            currentBud.resourceBH = 0.0f;
        }
    }

    /*if (!useGPU) {
        for (unsigned int ap = 0; ap < (unsigned int)attractorPoints.size(); ++ap) {
            AttractorPoint& currentAttrPt = attractorPoints[ap];
            currentAttrPt.nearestBudDist2 = 9999999.0f;
            currentAttrPt.nearestBudBranchIdx = -1;
            currentAttrPt.nearestBudIdx = -1;
        }
    }*/
}

// Remove all attractor points that are too close to buds
void Tree::RemoveAttractorPoints(KdTree&currentKdTree) {
    // 1. Remove all attractor points that are too close to any bud
	std::vector<Point> out;
	for (unsigned int br = 0; br < (unsigned int)branches.size(); ++br) {
	    const std::vector<Bud>& buds = branches[br].buds;
	    for (unsigned int bu = 0; bu < (unsigned int)buds.size(); ++bu) {
			Point temp;
			float occupancyR = 2.26 * buds[bu].internodeLength;
			temp.x = buds[bu].point.x;
			temp.y = buds[bu].point.y;
			temp.z = buds[bu].point.z;
			searchRadiusNeighbor(&currentKdTree, temp, occupancyR, out,true);
	    }
	}
    //for (unsigned int br = 0; br < (unsigned int)branches.size(); ++br) {
    //    const std::vector<Bud>& buds = branches[br].buds;
    //    for (unsigned int bu = 0; bu < (unsigned int)buds.size(); ++bu) {

    //        auto attrPtIter = attractorPoints.begin();
    //        while (attrPtIter != attractorPoints.end()) {
    //            const Bud& currentBud = buds[bu];
    //            const float budToPtDist = glm::length2(attrPtIter->point - currentBud.point);
    //            if (budToPtDist < 5.1f * currentBud.internodeLength * currentBud.internodeLength) { // ~2x internode length - use distance squared
    //                attrPtIter = attractorPoints.erase(attrPtIter); // This attractor point is close to the bud, remove it
    //            } else {
    //                ++attrPtIter;
    //            }
    //        }
    //    }
    //}
}

void Tree::create() {
    // Flush currently stored mesh
    treeMesh.clearData();
    leavesMesh.clearData();

    // Vectors containing branch geometry - many transformed versions of branchMesh all unioned together
    std::vector<glm::vec3> branchPoints = std::vector<glm::vec3>();
    std::vector<glm::vec3> branchNormals = std::vector<glm::vec3>();
    std::vector<unsigned int> branchIndices = std::vector<unsigned int>();
    // Retrieve branchMesh data
    const std::vector<glm::vec3>& branchMeshPoints = branchMesh.GetPositions();
    const std::vector<glm::vec3>& branchMeshNormals = branchMesh.GetNormals();
    const std::vector<unsigned int>& branchMeshIndices = branchMesh.GetIndices();

    // Do the same for all leaves in the tree
    std::vector<glm::vec3> leafPoints = std::vector<glm::vec3>();
    std::vector<glm::vec3> leafNormals = std::vector<glm::vec3>();
    std::vector<unsigned int> leafIndices = std::vector<unsigned int>();
    // Retrieve leafMesh data
    const std::vector<glm::vec3>& leafMeshPoints = leafMesh.GetPositions();
    const std::vector<glm::vec3>& leafMeshNormals = leafMesh.GetNormals();
    const std::vector<unsigned int>& leafMeshIndices = leafMesh.GetIndices();

    for (int br = 0; br < branches.size(); ++br) {
        const std::vector<Bud>& buds = branches[br].GetBuds();
        int bu = 1;
        for (; bu < buds.size(); ++bu) {
            const Bud& currentBud = buds[bu];
            const glm::vec3& internodeEndPoint = currentBud.point; // effectively, just the position of the bud at the end of the current internode

            // Compute the transformation for the current internode
            glm::vec3 branchAxis = glm::normalize(internodeEndPoint - buds[bu - 1].point);
            const float angle = std::acos(glm::dot(branchAxis, WORLD_UP_VECTOR));
            glm::mat4 branchTransform;
            if (angle > 0.01f) {
                const glm::vec3 axis = glm::normalize(glm::cross(WORLD_UP_VECTOR, branchAxis));
                const glm::quat branchQuat = glm::angleAxis(angle, axis);
                branchTransform = glm::toMat4(branchQuat); // initially just a rotation matrix, eventually stores the entire transformation
            }
            else { // if it's pretty much straight up, call it straight up
                branchTransform = glm::mat4(1.0f);
            }

            // Compute the translation component, placing the mesh at the halfway point
            const glm::vec3 translation = internodeEndPoint - 0.5f * branchAxis * currentBud.internodeLength;

            // Create an overall transformation matrix of translation and rotation and scale
            branchTransform = glm::translate(glm::mat4(1.0f), translation) * branchTransform * glm::scale(glm::mat4(1.0f), glm::vec3(currentBud.branchRadius * 0.05f, currentBud.internodeLength * 0.5f, currentBud.branchRadius * 0.05f));
            
            std::vector<glm::vec3> branchMeshPointsTrans = std::vector<glm::vec3>();
            std::vector<glm::vec3> branchMeshNormalsTrans = std::vector<glm::vec3>();
            for (int i = 0; i < branchMeshPoints.size(); ++i) {
                branchMeshPointsTrans.emplace_back(glm::vec3(branchTransform * glm::vec4(branchMeshPoints[i], 1.0f)));
                const glm::vec3 transformedNormal = glm::normalize(glm::vec3(glm::inverse(glm::transpose(branchTransform)) * glm::vec4(branchMeshNormals[i], 0.0f)));
                branchMeshNormalsTrans.emplace_back(transformedNormal);
            }

            std::vector<unsigned int> branchMeshIndicesNew = std::vector<unsigned int>();
            for (int i = 0; i < branchMeshIndices.size(); ++i) {
                const unsigned int size = (unsigned int)branchPoints.size();
                branchMeshIndicesNew.emplace_back(branchMeshIndices[i] + size); // Offset this set of indices by the # of positions
            }

            branchPoints.insert(branchPoints.end(), branchMeshPointsTrans.begin(), branchMeshPointsTrans.end());
            branchNormals.insert(branchNormals.end(), branchMeshNormalsTrans.begin(), branchMeshNormalsTrans.end());
            branchIndices.insert(branchIndices.end(), branchMeshIndicesNew.begin(), branchMeshIndicesNew.end());

            /// Compute transformation(s) for leaves

            if (currentBud.type == AXILLARY && currentBud.fate != FORMED_BRANCH /* && branches[br].GetAxisOrder() > 1*/) {
                const float leafScale = 0.05f * currentBud.internodeLength / currentBud.branchRadius; // Joe's made-up heuristic
                if (leafScale < 0.01) { break; }
                std::vector<glm::vec3> leafMeshPointsTrans = std::vector<glm::vec3>();
                std::vector<glm::vec3> leafMeshNormalsTrans = std::vector<glm::vec3>();
                const glm::mat4 leafTransform = glm::translate(glm::mat4(1.0f), internodeEndPoint) * glm::toMat4(glm::angleAxis(std::acos(glm::dot(currentBud.naturalGrowthDir, WORLD_UP_VECTOR)), glm::normalize(glm::cross(WORLD_UP_VECTOR, currentBud.naturalGrowthDir))));
                for (int i = 0; i < leafMeshPoints.size(); ++i) {
                    leafMeshPointsTrans.emplace_back(glm::vec3(leafTransform * glm::vec4(leafMeshPoints[i] * leafScale, 1.0f)));
                    const glm::vec3 transformedNormal = glm::normalize(glm::vec3(glm::inverse(glm::transpose(leafTransform)) * glm::vec4(leafMeshNormals[i], 0.0f)));
                    leafMeshNormalsTrans.emplace_back(transformedNormal);
                }

                std::vector<unsigned int> leafIndicesNew = std::vector<unsigned int>();
                for (int i = 0; i < leafMeshIndices.size(); ++i) {
                    const unsigned int size = (unsigned int)leafPoints.size();
                    leafIndicesNew.emplace_back(leafMeshIndices[i] + size); // Offset this set of indices by the # of positions
                }
                
                leafPoints.insert(leafPoints.end(), leafMeshPointsTrans.begin(), leafMeshPointsTrans.end());
                leafNormals.insert(leafNormals.end(), leafMeshNormalsTrans.begin(), leafMeshNormalsTrans.end());
                leafIndices.insert(leafIndices.end(), leafIndicesNew.begin(), leafIndicesNew.end());
            }
        }
    }
    treeMesh.AddPositions(branchPoints);
    treeMesh.AddNormals(branchNormals);
    treeMesh.AddIndices(branchIndices);
    leavesMesh.AddPositions(leafPoints);
    leavesMesh.AddNormals(leafNormals);
    leavesMesh.AddIndices(leafIndices);
    treeMesh.create();
    leavesMesh.create();
    hasBeenCreated = true;
}
