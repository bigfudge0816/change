#pragma once

#include "../imgui.h"
#include "../imgui_impl_glfw.h"
#include "../imgui_impl_opengl3.h"

struct GLFWwindow;
class TreeApplication;

class UIManager {
public:
    UIManager(GLFWwindow* window) { ImguiSetup(window); }
    void ImguiSetup(GLFWwindow* window);
    void ImguiNewFrame() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
    }
    void RenderImgui() {
        ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    void DestroyImgui() {
		ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
    }
    void HandleInput(TreeApplication& treeApp);
};
