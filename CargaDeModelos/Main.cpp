#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Camera.h"
#include "shaderClass.h"
#include "Model.h"
#include "AABB.h"
#include "AABBRenderer.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// Change the window size here
const unsigned int SCR_WIDTH = 2000;
const unsigned int SCR_HEIGHT = 1200;

enum class AppState {
    MENU,
    INSTRUCCIONES,
    CARGA,
    JUEGO,
    GANASTE,
    SALIR
};

// Forward declaration of the function to avoid "identifier not found" error
void RenderAppState(AppState& state, GLFWwindow* window, bool& showMenu);

int main()
{
    // 1. Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Remove automatic maximization to respect SCR_WIDTH/SCR_HEIGHT
    // glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto", NULL, NULL);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; return -1; }
    glfwMakeContextCurrent(window);

    // --- IMGUI INITIALIZATION ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Capture the mouse for 360° rotation (only once at the start)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 2. Load GLAD
    if (!gladLoadGL()) { std::cerr << "Failed to initialize GLAD\n"; return -1; }

    // 3. Global parameters
    glEnable(GL_DEPTH_TEST);

    // 4. Camera
    Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(-30.0f, 1.0f, 150.0f)); // More to the left

    // 5. Shaders
    Shader objectShader("default.vert", "default.frag");
    Shader lampShader("light.vert", "light.frag");

    // 6. Models
    Model room("room/scene.gltf", 0.0f, 0.0f, 90.0f, 0.0f, 0.0, 0.0f, 18.0f, 8.0f, 8.0f);
    Model Personaje("personaje/scene.gltf", 0.0f, 0.0f, 90.0f, 0.0f, 8.0, 0.0f, 1.0f, 1.0f, 1.0f);
    Model LamparaPapel("lampara_papel/scene.gltf", 0.0f, 0.0f, 90.0f, 0.0f, 100.0, 0.0f, 1.0f, 1.0f, 1.0f);
    Model LamparaPapel2("lampara_papel/scene.gltf", 0.0f, 0.0f, 90.0f, -100.0f, 100.0, 0.0f, 1.0f, 1.0f, 1.0f);
    Model F1("letra_f/scene.gltf", 0.0f, 0.0f, 90.0f, 30.0f, -20.0, -10.0f, 0.3f, 0.3f, 0.3f);
    Model U1("letra_u/scene.gltf", 0.0f, 0.0f, 90.0f, -100.0f, -25.0, -40.0f, 0.3f, 0.3f, 0.3f);
    Model F2("letra_f/scene.gltf", 180.0f, 0.0f, 90.0f, -150.0f, -15.0, -40.0f, 0.5f, 0.5f, 0.5f);
    Model U2("letra_u/scene.gltf", 0.0f, 0.0f, 90.0f, -70.0f, -18.0, -5.0f, 0.5f, 0.5f, 0.5f);

    // --- AABB definition for room, character and lamps ---
    AABB aabbRoom{ glm::vec3(-90, -10, -90), glm::vec3(90, 90, 90) };
    AABB aabbPersonaje{ glm::vec3(-5, 0, -5), glm::vec3(5, 20, 5) };
    AABB aabbLampara1{ glm::vec3(-5, 90, -5), glm::vec3(5, 110, 5) };
    AABB aabbLampara2{ glm::vec3(-105, 90, -5), glm::vec3(-95, 110, 5) };

    // Renderer for AABB
    Shader lineShader("default_line.vert", "default_line.frag");
    AABBRenderer aabbRenderer;

    // 7. Light parameters
    glm::vec3 lightPos = glm::vec3(0.0f, 100.0f, 0.0f);
    glm::vec4 lightColor = glm::vec4(1.0f);

    // --- Variables to move and rotate the character ---
    glm::vec3 personajePos = glm::vec3(0.0f, 1.0f, 0.0f);
    float personajeYaw = 0.0f;
    float personajeSpeed = 2.0f;
    float mouseSensitivity = 0.09f;
    static double lastX = SCR_WIDTH / 2.0;
    static bool firstMouse = true;

    // Variables to control the appearance/disappearance of U1/U2 and F1/F2
    static bool u1Eliminado = false;
    static bool f1Eliminado = false;
    glm::vec3 u1Pos = glm::vec3(-121.42f, -30.0f, -51.02f);
    glm::vec3 f1Pos = glm::vec3(17.08f, -30.0f, -21.50f);
    float radioTrigger = 2.0f;

    AppState state = AppState::MENU;
    bool showMenu = true;

    float winTimer = 0.0f;
    bool winTriggered = false;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // More pleasant background color
        glClearColor(0.12f, 0.13f, 0.15f, 1.0f);
        camera.Inputs(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- GAME LOGIC ONLY IF STATE IS JUEGO ---
        if (state == AppState::JUEGO) {
            if (!u1Eliminado && glm::distance(personajePos, u1Pos) < radioTrigger) {
                u1Eliminado = true;
            }
            if (!f1Eliminado && glm::distance(personajePos, f1Pos) < radioTrigger) {
                f1Eliminado = true;
            }
            objectShader.Activate();
            camera.updateMatrix(45.0f, 0.1f, 1000.0f);
            glUniform3f(glGetUniformLocation(objectShader.ID, "objectColor"), 1.0f, 1.0f, 1.0f);
            if (!f1Eliminado) F1.Draw(objectShader, camera);
            if (!u1Eliminado) U1.Draw(objectShader, camera);
            glUniform3f(glGetUniformLocation(objectShader.ID, "objectColor"), 1.0f, 0.0f, 0.0f);
            if (f1Eliminado) F2.Draw(objectShader, camera);
            if (u1Eliminado) U2.Draw(objectShader, camera);
            glUniform3f(glGetUniformLocation(objectShader.ID, "objectColor"), 1.0f, 1.0f, 1.0f);
            room.Draw(objectShader, camera);
            Personaje.Draw(objectShader, camera);

            lampShader.Activate();
            camera.updateMatrix(45.0f, 0.1f, 1000.0f);
            glUniformMatrix4fv(glGetUniformLocation(lampShader.ID, "model"),
                1, GL_FALSE, glm::value_ptr(
                    glm::translate(glm::mat4(1.0f), lightPos)
                ));
            glUniform4fv(glGetUniformLocation(lampShader.ID, "lightColor"),
                1, glm::value_ptr(lightColor));
            LamparaPapel.Draw(lampShader, camera);
            LamparaPapel2.Draw(lampShader, camera);

            lineShader.Activate();
            camera.updateMatrix(45.0f, 0.1f, 1000.0f);
            AABB aabbPersonajeActual = aabbPersonaje;
            aabbPersonajeActual.min += personajePos;
            aabbPersonajeActual.max += personajePos;
            aabbRenderer.draw(aabbRoom, lineShader, camera, glm::vec3(1, 1, 1), glm::mat4(1.0f));
            aabbRenderer.draw(aabbPersonajeActual, lineShader, camera, glm::vec3(1, 0, 0), glm::mat4(1.0f));

            AABB aabbCam{
                camera.Position - glm::vec3(2, 5, 2),
                camera.Position + glm::vec3(2, 5, 2)
            };
            bool colision = false;
            if (checkAABBCollision(aabbCam, aabbRoom)) colision = true;
            if (checkAABBCollision(aabbCam, aabbPersonaje)) colision = true;
            if (checkAABBCollision(aabbCam, aabbLampara1) || checkAABBCollision(aabbCam, aabbLampara2)) colision = true;
            camera.SetAllowMovement(!colision);

            bool colisionW = false, colisionA = false, colisionD = false;
            glm::vec3 nextPos;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                nextPos = camera.Position + camera.speed * camera.Orientation;
                AABB aabbNext{ nextPos - glm::vec3(2, 5, 2), nextPos + glm::vec3(2, 5, 2) };
                if (checkAABBCollision(aabbNext, aabbRoom) || checkAABBCollision(aabbNext, aabbPersonaje) || checkAABBCollision(aabbNext, aabbLampara1) || checkAABBCollision(aabbNext, aabbLampara2))
                    colisionW = true;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                nextPos = camera.Position + camera.speed * -glm::normalize(glm::cross(camera.Orientation, camera.Up));
                AABB aabbNext{ nextPos - glm::vec3(2, 5, 2), nextPos + glm::vec3(2, 5, 2) };
                if (checkAABBCollision(aabbNext, aabbRoom) || checkAABBCollision(aabbNext, aabbPersonaje) || checkAABBCollision(aabbNext, aabbLampara1) || checkAABBCollision(aabbNext, aabbLampara2))
                    colisionA = true;
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                nextPos = camera.Position + camera.speed * glm::normalize(glm::cross(camera.Orientation, camera.Up));
                AABB aabbNext{ nextPos - glm::vec3(2, 5, 2), nextPos + glm::vec3(2, 5, 2) };
                if (checkAABBCollision(aabbNext, aabbRoom) || checkAABBCollision(aabbNext, aabbPersonaje) || checkAABBCollision(aabbNext, aabbLampara1) || checkAABBCollision(aabbNext, aabbLampara2))
                    colisionD = true;
            }
            camera.allowMovement = !(colisionW || colisionA || colisionD);

            float deltaTime = 0.016f;
            glm::vec3 forward = glm::vec3(sin(glm::radians(personajeYaw)), 0, cos(glm::radians(personajeYaw)));
            float roomMinX = -154.527f;
            float roomMaxX = 99.6282f;
            float roomMinZ = -55.0215f;
            float roomMaxZ = 62.5086f;
            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
            glm::vec3 nextPersonajePos = personajePos;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                nextPersonajePos += forward * personajeSpeed * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                nextPersonajePos -= forward * personajeSpeed * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                nextPersonajePos -= right * personajeSpeed * deltaTime;
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                nextPersonajePos += right * personajeSpeed * deltaTime;
            }
            if (nextPersonajePos.x > roomMinX && nextPersonajePos.x < roomMaxX &&
                nextPersonajePos.z > roomMinZ && nextPersonajePos.z < roomMaxZ) {
                personajePos = nextPersonajePos;
            }
            static bool mouseFree = false;
            if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !mouseFree) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                mouseFree = true;
            }
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && mouseFree) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                mouseFree = false;
                firstMouse = true;
            }
            if (!mouseFree) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                int centerX = SCR_WIDTH / 2;
                int centerY = SCR_HEIGHT / 2;
                if (firstMouse) {
                    glfwSetCursorPos(window, centerX, centerY);
                    lastX = centerX;
                    firstMouse = false;
                }
                float xoffset = float(xpos - lastX);
                lastX = xpos;
                personajeYaw -= xoffset * mouseSensitivity;
                glfwSetCursorPos(window, centerX, centerY);
                lastX = centerX;
            }
            glm::mat4 personajeModel = glm::translate(glm::mat4(1.0f), personajePos)
                * glm::rotate(glm::mat4(1.0f), glm::radians(personajeYaw), glm::vec3(0, 1, 0));
            room.Draw(objectShader, camera);
            Personaje.Draw(objectShader, camera, personajeModel);
            float camDist = 70.0f;
            float camHeight = 80.0f;
            glm::vec3 camBehind = personajePos - forward * camDist + glm::vec3(0, camHeight, 0);
            camera.Position = camBehind;
            glm::vec3 lookAtOffset(0, 30.0f, 0);
            camera.Orientation = glm::normalize((personajePos + lookAtOffset) - camera.Position);
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                state = AppState::MENU;
                showMenu = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            std::cout << "Posición del personaje: (" << personajePos.x << ", " << personajePos.y << ", " << personajePos.z << ")\r";
            static bool esquinaActivada[4] = { false, false, false, false };
            glm::vec3 esquinasMesa[] = {
                glm::vec3(-45.93f, 1.0f, -17.37f),
                glm::vec3(-45.84f, 1.0f, 26.98f),
                glm::vec3(49.69f, 1.0f, 26.84f),
                glm::vec3(51.60f, 1.0f, -18.95f)
            };
            for (int i = 0; i < 4; ++i) {
                if (!esquinaActivada[i] && glm::distance(personajePos, esquinasMesa[i]) < 1.0f) {
                    personajePos.y -= 50.0f;
                    esquinaActivada[i] = true;
                }
            }
            static bool estabaDentroMesa = false;
            float mesaMinX = -45.93f;
            float mesaMaxX = 51.60f;
            float mesaMinZ = -18.95f;
            float mesaMaxZ = 26.98f;
            float margen = 1.0f;
            bool dentroMesa = (personajePos.x > mesaMinX - margen && personajePos.x < mesaMaxX + margen &&
                personajePos.z > mesaMinZ - margen && personajePos.z < mesaMaxZ + margen);
            if (estabaDentroMesa && !dentroMesa) {
                personajePos.y = -30.0f;
            }
            estabaDentroMesa = dentroMesa;

            // --- WIN LOGIC ---
            if (f1Eliminado && u1Eliminado && !winTriggered) {
                winTriggered = true;
                winTimer = 0.0f;
            }
            if (winTriggered && state == AppState::JUEGO) {
                winTimer += deltaTime;
                if (winTimer > 5.0f) { // Wait 5 seconds
                    state = AppState::GANASTE;
                    winTriggered = false;
                }
            }
        }

        // --- IMGUI FRAME ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        RenderAppState(state, window, showMenu);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // At the end, delete both shaders
    objectShader.Delete();
    lampShader.Delete();
    lineShader.Delete();

    // --- IMGUI CLEANUP ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

}

void RenderAppState(AppState& state, GLFWwindow* window, bool& showMenu) {
    switch (state) {
    case AppState::MENU: {
        // Larger window adapted to the new size
        ImVec2 window_size = ImVec2(900, 600);
        ImVec2 window_pos = ImVec2(((float)SCR_WIDTH - window_size.x) * 0.5f, ((float)SCR_HEIGHT - window_size.y) * 0.5f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
        ImGui::Begin("Menú Principal", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

        const char* titulo = "Menú Principal";
        const char* botones[] = { "Play", "Instrucciones", "Salir" };
        int num_botones = 3;
        float font_scale_title = 2.0f;
        float font_scale_button = 1.3f;
        float font_scale_default = 1.0f;
        float button_width = 400.0f;
        float button_height = 60.0f;
        float button_spacing = 40.0f;
        float title_height = ImGui::GetFontSize() * font_scale_title + 10.0f;
        float buttons_height = num_botones * (button_height + button_spacing);
        float total_content_height = title_height + buttons_height;
        float start_y = (window_size.y - total_content_height) * 0.5f;

        ImGui::SetCursorPosY(start_y);
        ImGui::SetWindowFontScale(font_scale_title);
        ImVec2 title_size = ImGui::CalcTextSize(titulo);
        ImGui::SetCursorPosX((window_size.x - title_size.x * font_scale_title) * 0.5f);
        ImGui::Text("%s", titulo);
        ImGui::SetWindowFontScale(font_scale_button);

        for (int i = 0; i < num_botones; ++i) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + button_spacing);
            ImGui::SetCursorPosX((window_size.x - button_width) * 0.5f);
            if (i == 0 && ImGui::Button("Play", ImVec2(button_width, button_height))) {
                std::cout << "Juego iniciado" << std::endl;
                state = AppState::JUEGO;
                showMenu = false;
            }
            else if (i == 1 && ImGui::Button("Instrucciones", ImVec2(button_width, button_height))) {
                state = AppState::INSTRUCCIONES;
            }
            else if (i == 2 && ImGui::Button("Salir", ImVec2(button_width, button_height))) {
                state = AppState::SALIR;
                glfwSetWindowShouldClose(window, true);
            }
        }
        ImGui::SetWindowFontScale(font_scale_default);
        ImGui::End();
        break;
    }
    case AppState::INSTRUCCIONES: {
        ImVec2 window_size = ImVec2(1100, 700);
        ImVec2 window_pos = ImVec2(((float)SCR_WIDTH - window_size.x) * 0.5f, ((float)SCR_HEIGHT - window_size.y) * 0.5f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
        ImGui::Begin("Instrucciones", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

        const char* titulo = "Instrucciones del juego:";
        const char* bullets[] = {
            "Usa WASD para mover al personaje.",
            "Usa el mouse para rotar la cámara.",
            "Recoge las letras para avanzar.",
            "Presiona TAB para liberar el mouse.",
            "Presiona click izquierdo para atrapar el mouse."
        };
        int num_bullets = sizeof(bullets) / sizeof(bullets[0]);
        float font_scale_title = 2.0f;
        float font_scale_bullet = 1.3f;
        float font_scale_default = 1.0f;
        float bullet_spacing = 22.0f * font_scale_bullet;
        float title_height = ImGui::GetFontSize() * font_scale_title + 10.0f;
        float bullets_height = num_bullets * (ImGui::GetFontSize() * font_scale_bullet + bullet_spacing);
        float button_height = 60.0f;
        float button_spacing = 50.0f;
        float total_content_height = title_height + bullets_height + button_height + button_spacing;
        float start_y = (window_size.y - total_content_height) * 0.5f;

        ImGui::SetCursorPosY(start_y);
        ImGui::SetWindowFontScale(font_scale_title);
        ImVec2 title_size = ImGui::CalcTextSize(titulo);
        ImGui::SetCursorPosX((window_size.x - title_size.x * font_scale_title) * 0.5f);
        ImGui::Text("%s", titulo);
        ImGui::SetWindowFontScale(font_scale_bullet);

        for (int i = 0; i < num_bullets; ++i) {
            ImVec2 bullet_size = ImGui::CalcTextSize(bullets[i]);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + bullet_spacing);
            ImGui::SetCursorPosX((window_size.x - bullet_size.x * font_scale_bullet) * 0.5f - 20.0f);
            ImGui::BulletText("%s", bullets[i]);
        }
        ImGui::SetWindowFontScale(font_scale_default);

        ImGui::SetCursorPosY(window_size.y - start_y - button_height);
        ImGui::SetCursorPosX((window_size.x - 400) * 0.5f);
        if (ImGui::Button("Volver al menú", ImVec2(400, 60))) {
            state = AppState::MENU;
        }
        ImGui::End();
        break;
    }
    case AppState::CARGA: {
        ImVec2 window_size = ImVec2(400, 150);
        ImVec2 window_pos = ImVec2((SCR_WIDTH - window_size.x) * 0.5f, (SCR_HEIGHT - window_size.y) * 0.5f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
        ImGui::Begin("Cargando", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
        ImGui::Text("Cargando recursos, por favor espera...");
        ImGui::End();
        break;
    }
    case AppState::JUEGO: {
        // Here you could show HUD, score, etc. If not, leave empty.
        break;
    }
    case AppState::GANASTE: {
        ImVec2 window_size = ImVec2(800, 400);
        ImVec2 window_pos = ImVec2(((float)SCR_WIDTH - window_size.x) * 0.5f, ((float)SCR_HEIGHT - window_size.y) * 0.5f);
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
        ImGui::Begin("Ganaste", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
        float font_scale = 3.0f;
        const char* winText = "¡Ganaste!";
        ImGui::SetCursorPosY((window_size.y - ImGui::GetFontSize() * font_scale - 80.0f) * 0.5f);
        ImGui::SetWindowFontScale(font_scale);
        ImVec2 text_size = ImGui::CalcTextSize(winText);
        ImGui::SetCursorPosX((window_size.x - text_size.x * font_scale) * 0.5f);
        ImGui::Text("%s", winText);
        ImGui::SetWindowFontScale(1.5f);
        ImGui::SetCursorPosY(window_size.y - 120.0f);
        ImGui::SetCursorPosX((window_size.x - 400) * 0.5f);
        if (ImGui::Button("¿Volver al menu?", ImVec2(400, 60))) {
            state = AppState::MENU;
        }
        ImGui::SetWindowFontScale(1.0f);
        ImGui::End();
        break;
    }
    case AppState::SALIR: {
        // No need to render anything, the app will close.
        break;
    }
    }
}