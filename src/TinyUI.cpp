//
// Created by 19450 on 2024/3/13.
//

#include "TinyUI.h"
#include "imgui/imgui.h"
#include "tool/tool.h"
#include "tool/loader.h"
#include "instance/Mesh.h"
#include "material/RoughConductor.h"
#include "material/RoughDielectric.h"
#include "ResourceManager.h"

bool TinyUI::showUI = true;
Instance* TinyUI::selectedInstance = nullptr;
std::set<Instance*> TinyUI::toopenList;
std::set<Instance*> TinyUI::toeraseList;

void TinyUI::init(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void TinyUI::update(Scene *scene, float fps) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow(&showUI);
    if (showUI) {
        ImGui::Begin("Hierarchy", &showUI);
        insert_instance_Hierarchy(scene);
        ImGui::End();

        ImGui::Begin("Editor", &showUI);
        if(selectedInstance) insert_instance_Editor(selectedInstance);
        ImGui::End();

        ImGui::Begin("Settings", &showUI);
        ImGui::Text(str_format("FPS: %.2f", fps).c_str());
        insert_configuration();
        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void TinyUI::terminate() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void TinyUI::insert_instance_Hierarchy(Instance *u) {
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if(selectedInstance == u) node_flags |= ImGuiTreeNodeFlags_Selected;
    if(u->children.size() == 0) node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

    if(toopenList.count(u)) {
        toopenList.erase(u);
        ImGui::SetNextItemOpen(true);
    }

    bool node_open = ImGui::TreeNodeEx((void*)u, node_flags, u->name.c_str());
    if(ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) selectedInstance = u;

    if (ImGui::BeginPopupContextItem()) {
        if(ImGui::Button("Import")) {
            string dir = show_file_open_dialog();
            Instance *nw = AssimpLoader::load_model(dir);
            if(nw) {
                u->add_child(nw);
                ResourceManager::manager->reload_scene(Scene::main_scene);
            }
            ImGui::CloseCurrentPopup();
        }
        if(ImGui::Button("Delete")) {
            selectInstance(nullptr);
            toeraseList.insert(u);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if(node_open) {
        for(auto *cd: u->children) {
            if(toeraseList.count(cd)) {
                toeraseList.erase(cd);
                delete cd;
            } else {
                insert_instance_Hierarchy(cd);
            }
        }
        ImGui::TreePop();
    }
}

void TinyUI::insert_instance_Editor(Instance *u) {
    ImGui::SeparatorText(u->name.c_str());
    u->transform.insert_gui();
    if(u->mesh) {
        ImGui::SeparatorText("Mesh");
        ImGui::BulletText(str_format("%u triangles", u->mesh->triangles.size()).c_str());

        if(u->material) {
            ImGui::SeparatorText("Material");

            static ImGuiComboFlags flags = ImGuiComboFlags_HeightSmall | ImGuiComboFlags_PopupAlignLeft;

            const char* items[] = { "RoughConductor", "RoughDielectric"};
            int item_current_idx = u->material->material_type() - 1;
            const char* combo_preview_value = items[item_current_idx];

            if (ImGui::BeginCombo("Type", combo_preview_value, flags)) {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                    const bool is_selected = (item_current_idx == n);

                    if (ImGui::Selectable(items[n], is_selected)) {
                        if(n == item_current_idx) continue;
                        else {
                            // update material;
                            delete u->material;

                            if(n == 0) u->material = new RoughConductor();
                            else if(n == 1) u->material = new RoughDielectric();
                            else assert(false);

                            item_current_idx = n;
                        }
                    }

                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            u->material->insert_gui();
        }
    }
}

void TinyUI::insert_configuration() {
    ImGui::SeparatorText("Config");

    ImGui::Checkbox("Use TAA", &Config::useTAA);
    ImGui::Checkbox("Use TemporalFilter", &Config::useTemporalFilter);
    ImGui::Checkbox("Use StaticBlender", &Config::useStaticBlender);
    ImGui::SliderInt("Filter Level", &Config::filterLevel, 0, 5);
    ImGui::SliderInt("Samples per pixel", &Config::SPP, 1, 16);
}

void TinyUI::selectInstance(Instance *instance) {
    if(selectedInstance == instance) {
        selectedInstance = nullptr;
        return;
    }
    selectedInstance = instance;
    if(instance == nullptr) return;
    Instance *ptr = instance;
    while(ptr) {
        toopenList.insert(ptr);
        ptr = ptr->get_parent();
    }
}
