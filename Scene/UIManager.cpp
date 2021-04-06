#include "UIManager.h"

#include "../imgui_internal.h"
#include "../imconfig.h"
#include "TreeApplication.h"

void UIManager::ImguiSetup(GLFWwindow* window) {
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
	io.Fonts->AddFontFromFileTTF("c:/windows/fonts/simhei.ttf", 13.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
}

void UIManager::HandleInput(TreeApplication& treeApp) {
    ImGui::SliderFloat(u8"�ڼ䳤��", &treeApp.GetTreeParameters().internodeScale, 0.0f, 0.1f);
	ImGui::SliderFloat(u8"COS_THETA", &treeApp.GetTreeParameters().perceptionCosTheta, 0.0f, 1.0f);
    ImGui::SliderFloat(u8"��С��֦�뾶", &treeApp.GetTreeParameters().minimumBranchRadius, 0.0f, 1.0f);
    ImGui::SliderFloat(u8"�����֦�뾶", &treeApp.GetTreeParameters().maximumBranchRadius, 0.0f, 10.0f);
	ImGui::SliderFloat(u8"��Դ�����", &treeApp.GetTreeParameters().BHLambda, 0.0f, 1.0f);
    ImGui::SliderInt(u8"��������", &treeApp.GetTreeParameters().numSpaceColonizationIterations, 0, 100);
    ImGui::SliderInt(u8"��Դ������", &treeApp.GetTreeParameters().numAttractorPointsToGenerate, 0, 3000000);
    //ImGui::Checkbox("Enable Debug Output", &treeApp.GetTreeParameters().enableDebugOutput);

    if (ImGui::Button(u8"����")) {
        treeApp.IterateSelectedTreeInSelectedAttractorPointCloud();
    }
    if (ImGui::Button(u8"��������")) {
		treeApp.GetSelectedKdTree() = KdTree();
#ifdef ENABLE_DEBUG_OUTPUT
		auto start = std::chrono::system_clock::now();
#endif
		buildKdTree(&treeApp.GetSelectedKdTree(), treeApp.global_pcd, 0);
        treeApp.RegrowSelectedTreeInSelectedAttractorPointCloud();
#ifdef ENABLE_DEBUG_OUTPUT
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::time_t end_time = std::chrono::system_clock::to_time_t(end);
		std::cout << "����kdtree����ʱ�䣺 " << elapsed_seconds.count() << "s\n";
#endif
    }
    if (ImGui::Button(u8"�����Դ")) {
        treeApp.AddAttractorPointCloudToScene();
		treeApp.AddKdTreeToScene();
        treeApp.GetSelectedAttractorPointCloud().GeneratePoints(treeApp.GetTreeParameters().numAttractorPointsToGenerate);
		for (auto i : treeApp.GetSelectedAttractorPointCloud().GetPoints())
		{
			Point temp;
			temp.x = i.point.x;
			temp.y = i.point.y;
			temp.z = i.point.z;
			treeApp.global_pcd.emplace_back(temp);
		}
		#ifdef ENABLE_DEBUG_OUTPUT
			auto start = std::chrono::system_clock::now();
		#endif
		buildKdTree(&treeApp.GetSelectedKdTree(), treeApp.global_pcd, 0);
		#ifdef ENABLE_DEBUG_OUTPUT
			auto end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - start;
			std::time_t end_time = std::chrono::system_clock::to_time_t(end);
			std::cout << "����kdtree����ʱ�䣺 " << elapsed_seconds.count() << "s\n";
		#endif

        treeApp.GetTreeParameters().reconstructUniformGridOnGPU = false;
    }
    if (ImGui::Button(u8"����/��ʾ����")) {
        treeApp.GetSelectedAttractorPointCloud().ToggleDisplay();
    }
    /*if (ImGui::Button("Export Current Tree as OBJ")) {
        treeApp.ExportTreeAsObj();
    }*/
}
