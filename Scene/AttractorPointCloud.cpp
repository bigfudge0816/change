#include "Globals.h"
#include "AttractorPointCloud.h"

void AttractorPointCloud::GeneratePointsInUnitCube(unsigned int numPoints) {
    #ifdef ENABLE_DEBUG_OUTPUT
    auto start = std::chrono::system_clock::now();
    #endif
    for (unsigned int i = 0; i < numPoints; ++i) {
        const glm::vec3 p = glm::vec3(dis(rng), dis(rng), dis(rng));
        minPoint.x = std::min(minPoint.x, p.x);
        minPoint.y = std::min(minPoint.y, p.y);
        minPoint.z = std::min(minPoint.z, p.z);
        maxPoint.x = std::max(maxPoint.x, p.x);
        maxPoint.y = std::max(maxPoint.y, p.y);
        maxPoint.z = std::max(maxPoint.z, p.z);
        points.emplace_back(AttractorPoint(p));
    }
    #ifdef ENABLE_DEBUG_OUTPUT
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Elapsed time for Attractor Point Cloud Generation: " << elapsed_seconds.count() << "s\n";
    std::cout << "Number of Attractor Points Generated: " << points.size() << "\n\n";
    #endif
    create();
}

void AttractorPointCloud::GeneratePoints(unsigned int numPoints) {
    #ifdef ENABLE_DEBUG_OUTPUT
    auto start = std::chrono::system_clock::now();
    #endif
    //boundingMesh.LoadFromFile("OBJs/helixRot.obj");
	
    for (unsigned int i = 0; i < numPoints; ++i) {
        const glm::vec3 p = glm::vec3(dis(rng), dis(rng)+0.5, dis(rng)); // these scales are hard coded for the helixRot mesh

		minPoint.x = std::min(minPoint.x, p.x);
		minPoint.y = std::min(minPoint.y, p.y);
		minPoint.z = std::min(minPoint.z, p.z);
		maxPoint.x = std::max(maxPoint.x, p.x);
		maxPoint.y = std::max(maxPoint.y, p.y);
		maxPoint.z = std::max(maxPoint.z, p.z);
		points.emplace_back(AttractorPoint(p));
		        //// Intersect with mesh
        //if (boundingMesh.Contains(p)/*p.x * p.x + p.z * p.z < p.y * p.y*/) {
        //   
        //}
    }

    #ifdef ENABLE_DEBUG_OUTPUT
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Elapsed time for Attractor Point Cloud Generation: " << elapsed_seconds.count() << "s\n";
    std::cout << "Number of Attractor Points Generated: " << points.size() << "\n\n";
    #endif
    create();
}

// Generate points 
void AttractorPointCloud::GeneratePointsGivenSketchPoints(unsigned int numPoints, const std::vector<glm::vec3>& sketchPoints, const float brushRadius) {
    #ifdef ENABLE_DEBUG_OUTPUT
    auto start = std::chrono::system_clock::now();
    #endif
    for (unsigned int i = 0; i < numPoints; ++i) {
        const glm::vec3 p = glm::vec3(dis(rng) * 5.0, dis(rng) * 5.0f, dis(rng) * 5.0f);

        // Check if the point lies within at least one of the brush locations, assuming spheres for now (TODO: more brush types / cylinders?)
        bool contained = false;

        for (unsigned int sp = 0; sp < (unsigned int)sketchPoints.size(); ++sp) {
            if (glm::length(p - sketchPoints[sp]) < brushRadius) { // if the point is inside the sphere
                contained = true;
                break;
            }
        }

        if (contained) {
            minPoint.x = std::min(minPoint.x, p.x);
            minPoint.y = std::min(minPoint.y, p.y);
            minPoint.z = std::min(minPoint.z, p.z);
            maxPoint.x = std::max(maxPoint.x, p.x);
            maxPoint.y = std::max(maxPoint.y, p.y);
            maxPoint.z = std::max(maxPoint.z, p.z);
            points.emplace_back(AttractorPoint(p));
        }
    }
    #ifdef ENABLE_DEBUG_OUTPUT
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    std::cout << "Elapsed time for Attractor Point Cloud Generation: " << elapsed_seconds.count() << "s\n";
    std::cout << "Number of Attractor Points Generated: " << points.size() << "\n\n";
    #endif
    create();
}

void AttractorPointCloud::create() {
    // Indices
    genBufIdx();

    // Create an indices vector
    std::vector<unsigned int> indices = std::vector<unsigned int>();
    for (int i = 0; i < points.size(); ++i) {
        indices.emplace_back(i);
    }

    count = (int)indices.size();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // Positions
    std::vector<glm::vec3> actualPoints = std::vector <glm::vec3>();
    for (int i = 0; i < points.size(); ++i) {
        actualPoints.emplace_back(points[i].point);
    }
    genBufPos();
    glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * actualPoints.size(), actualPoints.data(), GL_STATIC_DRAW);
}
