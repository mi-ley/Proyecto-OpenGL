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

const unsigned int SCR_WIDTH = 2000;
const unsigned int SCR_HEIGHT = 1800;

int main()
{
    // 1. Inicializar GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto", NULL, NULL);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; return -1; }
    glfwMakeContextCurrent(window);

    // Captura el mouse para rotación 360° (solo una vez al inicio)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // 2. Cargar GLAD
    if (!gladLoadGL()) { std::cerr << "Failed to initialize GLAD\n"; return -1; }

    // 3. Parámetros globales
    glEnable(GL_DEPTH_TEST);

    // 4. Cámara
    Camera camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(-30.0f, 1.0f, 150.0f)); // Más a la izquierda

// 5. Cargar ambos shaders
     // 5. Shaders
    Shader objectShader("default.vert", "default.frag");
    Shader lampShader("light.vert", "light.frag");

    // 6. Modelos
    Model room("room/scene.gltf", /* rotation */ 0.0f, 0.0f, 90.0f, /*translation */ 0.0f, 0.0, 0.0f, /* Scale */ 18.0f, 8.0f, 8.0f);
    Model Personaje("personaje/scene.gltf", /* rotation */ 0.0f, 0.0f, 90.0f, /*translation */ 0.0f, 8.0, 0.0f, /* Scale */ 1.0f, 1.0f, 1.0f);
    Model LamparaPapel("lampara_papel/scene.gltf", /* rotation */ 0.0f, 0.0f, 90.0f, /*translation */ 0.0f, 100.0, 0.0f, /* Scale */ 1.0f, 1.0f, 1.0f);
    Model LamparaPapel2("lampara_papel/scene.gltf", /* rotation */ 0.0f, 0.0f, 90.0f, /*translation */ -100.0f, 100.0, 0.0f, /* Scale */ 1.0f, 1.0f, 1.0f);

    // --- Definición de AABB para room, personaje y lámparas ---
    // (Valores aproximados, deben ajustarse según el modelo real)
    AABB aabbRoom { glm::vec3(-90, -10, -90), glm::vec3(90, 90, 90) };
    AABB aabbPersonaje { glm::vec3(-5, 0, -5), glm::vec3(5, 20, 5) };
    AABB aabbLampara1 { glm::vec3(-5, 90, -5), glm::vec3(5, 110, 5) };
    AABB aabbLampara2 { glm::vec3(-105, 90, -5), glm::vec3(-95, 110, 5) };
    
    // Renderer para AABB
    Shader lineShader("default_line.vert", "default_line.frag");
    AABBRenderer aabbRenderer;

    // 7. Parámetros de luz
    glm::vec3 lightPos = glm::vec3(0.0f, 100.0f, 0.0f);
    glm::vec4 lightColor = glm::vec4(1.0f);

    // --- Variables para mover y rotar al personaje ---
    glm::vec3 personajePos = glm::vec3(0.0f, 1.0f, 0.0f); // Posición inicial igual a la traslación del modelo
    float personajeYaw = 0.0f; // Yaw inicial (mirando hacia adelante en Z+)
    float personajeSpeed = 2.0f; // Velocidad de movimiento
    float mouseSensitivity = 0.09f; // Sensibilidad más baja para giro natural
    static double lastX = SCR_WIDTH / 2.0; // Para el seguimiento del mouse
    static bool firstMouse = true;

    // Bucle principal
    while (!glfwWindowShouldClose(window))
    {
        // Input + limpieza
        camera.Inputs(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- OBJECT SHADER ---
        objectShader.Activate();
        // Sends camMatrix, lightPos, lightColor and camPos
        camera.updateMatrix(45.0f, 0.1f, 1000.0f);

        // Draw room and character with Phong lighting
        room.Draw(objectShader, camera);
        Personaje.Draw(objectShader, camera);

        // --- LAMP SHADER ---
        lampShader.Activate();
        // Sends the same camMatrix
        camera.updateMatrix(45.0f, 0.1f, 1000.0f);
        // Sends model=translation to lightPos and lightColor
        glUniformMatrix4fv(glGetUniformLocation(lampShader.ID, "model"),
            1, GL_FALSE, glm::value_ptr(
                glm::translate(glm::mat4(1.0f), lightPos)
            ));
        glUniform4fv(glGetUniformLocation(lampShader.ID, "lightColor"),
            1, glm::value_ptr(lightColor));

        LamparaPapel.Draw(lampShader, camera);
		LamparaPapel2.Draw(lampShader, camera);

        // --- DRAW AABB ---
        // Update projection matrix before drawing lines
        lineShader.Activate();
        camera.updateMatrix(45.0f, 0.1f, 1000.0f);
        // Update character's AABB to current position
        AABB aabbPersonajeActual = aabbPersonaje;
        aabbPersonajeActual.min += personajePos;
        aabbPersonajeActual.max += personajePos;
        // Room (white)
        aabbRenderer.draw(aabbRoom, lineShader, camera, glm::vec3(1,1,1), glm::mat4(1.0f));
        // Character (red)
        aabbRenderer.draw(aabbPersonajeActual, lineShader, camera, glm::vec3(1,0,0), glm::mat4(1.0f));

        // --- COLLISIONS ---
        // Camera AABB (you can adjust the size)
        AABB aabbCam{
            camera.Position - glm::vec3(2, 5, 2),
            camera.Position + glm::vec3(2, 5, 2)
        };
        bool colision = false;
        if (checkAABBCollision(aabbCam, aabbRoom)) colision = true;
        if (checkAABBCollision(aabbCam, aabbPersonaje)) colision = true;
        if (checkAABBCollision(aabbCam, aabbLampara1) || checkAABBCollision(aabbCam, aabbLampara2)) colision = true;
        camera.SetAllowMovement(!colision);

        // --- ADVANCED COLLISIONS BY DIRECTION ---
        bool colisionW = false, colisionA = false, colisionD = false;
        glm::vec3 nextPos;
        // Forward (W)
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            nextPos = camera.Position + camera.speed * camera.Orientation;
            AABB aabbNext{ nextPos - glm::vec3(2, 5, 2), nextPos + glm::vec3(2, 5, 2) };
            if (checkAABBCollision(aabbNext, aabbRoom) || checkAABBCollision(aabbNext, aabbPersonaje) || checkAABBCollision(aabbNext, aabbLampara1) || checkAABBCollision(aabbNext, aabbLampara2))
                colisionW = true;
        }
        // Left (A)
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            nextPos = camera.Position + camera.speed * -glm::normalize(glm::cross(camera.Orientation, camera.Up));
            AABB aabbNext{ nextPos - glm::vec3(2, 5, 2), nextPos + glm::vec3(2, 5, 2) };
            if (checkAABBCollision(aabbNext, aabbRoom) || checkAABBCollision(aabbNext, aabbPersonaje) || checkAABBCollision(aabbNext, aabbLampara1) || checkAABBCollision(aabbNext, aabbLampara2))
                colisionA = true;
        }
        // Right (D)
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            nextPos = camera.Position + camera.speed * glm::normalize(glm::cross(camera.Orientation, camera.Up));
            AABB aabbNext{ nextPos - glm::vec3(2, 5, 2), nextPos + glm::vec3(2, 5, 2) };
            if (checkAABBCollision(aabbNext, aabbRoom) || checkAABBCollision(aabbNext, aabbPersonaje) || checkAABBCollision(aabbNext, aabbLampara1) || checkAABBCollision(aabbNext, aabbLampara2))
                colisionD = true;
        }
        // Allow only movements that do not collide
        camera.allowMovement = !(colisionW || colisionA || colisionD);

        // --- INPUT AND CHARACTER MOVEMENT ---
        float deltaTime = 0.016f; // Approximate, ideally use real time
        glm::vec3 forward = glm::vec3(sin(glm::radians(personajeYaw)), 0, cos(glm::radians(personajeYaw)));
        // --- Character movement with custom room collision ---
        float roomMinX = -154.527f;
        float roomMaxX = 99.6282f;
        float roomMinZ = -55.0215f;
        float roomMaxZ = 62.5086f;
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
        glm::vec3 nextPersonajePos = personajePos;
        // Move forward
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            nextPersonajePos += forward * personajeSpeed * deltaTime;
        }
        // Move backward
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            nextPersonajePos -= forward * personajeSpeed * deltaTime;
        }
        // Move left
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            nextPersonajePos -= right * personajeSpeed * deltaTime;
        }
        // Move right
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            nextPersonajePos += right * personajeSpeed * deltaTime;
        }
        // Only allow movement if the new position is inside the custom AABB
        if (nextPersonajePos.x > roomMinX && nextPersonajePos.x < roomMaxX &&
            nextPersonajePos.z > roomMinZ && nextPersonajePos.z < roomMaxZ) {
            personajePos = nextPersonajePos;
        }
        // --- Collision with room ---
        //AABB aabbPersonajeMove = aabbPersonaje;
        //aabbPersonajeMove.min += nextPersonajePos - personajePos;
        //aabbPersonajeMove.max += nextPersonajePos - personajePos;
        //if (checkAABBCollision(aabbRoom, aabbPersonajeMove)) {
        //    personajePos = nextPersonajePos;
        //}
        // --- Character rotation with mouse (natural direction, right-right) ---
        // --- Character and camera rotation whenever the mouse moves (robust, no click needed) ---
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
            lastX = centerY;
        }

        // --- Update character model ---
        glm::mat4 personajeModel = glm::translate(glm::mat4(1.0f), personajePos)
            * glm::rotate(glm::mat4(1.0f), glm::radians(personajeYaw), glm::vec3(0, 1, 0));

        // --- DRAW CHARACTER IN NEW POSITION AND ROTATION ---
        // Room
        room.Draw(objectShader, camera);
        // Character with custom matrix
        Personaje.Draw(objectShader, camera, personajeModel);

        // --- CAMERA FOLLOWS CHARACTER (third person, behind and well rotated, look up) ---
        float camDist = 70.0f; // Distance behind
        float camHeight = 80.0f; // Height
        glm::vec3 camBehind = personajePos - forward * camDist + glm::vec3(0, camHeight, 0);
        camera.Position = camBehind;
        glm::vec3 lookAtOffset(0, 30.0f, 0); // Look up 30 units in Y
        camera.Orientation = glm::normalize((personajePos + lookAtOffset) - camera.Position);

        // --- Liberar el mouse al presionar ESC ---
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        // --- Imprimir las coordenadas mundiales del personaje en consola ---
        std::cout << "Posición del personaje: (" << personajePos.x << ", " << personajePos.y << ", " << personajePos.z << ")\r";

        // --- Checar si el personaje llega a las esquinas de la mesa y bajar Y solo una vez por esquina ---
        static bool esquinaActivada[4] = {false, false, false, false};
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

        // --- Bajar Y solo al salir de la mesa (AABB), no al entrar ni mientras esté adentro ---
        static bool estabaDentroMesa = false;
        // Definir el AABB de la mesa usando las esquinas
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

        // Swap buffers and events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // At the end, delete both shaders
    objectShader.Delete();
    lampShader.Delete();
    lineShader.Delete(); // Added to delete line shader

    // Terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

}
