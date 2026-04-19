#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

const float G = 9.81f;
const float RHO_AIRE = 1.225f;
const float RHO_AGUA = 1000.0f;
const float ESCALA = 100.0f; // 100px = 1 metro

struct Cuerpo {
    sf::CircleShape shape;
    sf::Vector2f vel{0, 0};
    float radioMetros;
    float densidadObjeto; // kg/m^3 (Acero: 7800, Madera: 600, Aluminio: 2700)
    float masa;
    float volumen;

    Cuerpo(float rPx, float dens, sf::Color col, sf::Vector2f pos) {
        radioMetros = rPx / ESCALA;
        densidadObjeto = dens;
        volumen = (4.0f / 3.0f) * 3.1415f * std::pow(radioMetros, 3);
        masa = densidadObjeto * volumen;

        shape.setRadius(rPx);
        shape.setOrigin({rPx, rPx});
        shape.setFillColor(col);
        shape.setPosition(pos);
    }

    void actualizar(float dt, float nivelAgua) {
        bool enAgua = (shape.getPosition().y > nivelAgua);
        float rhoFluido = enAgua ? RHO_AGUA : RHO_AIRE;

        // 1. Peso (hacia abajo)
        float peso = masa * G;

        // 2. Empuje de Arquímedes (hacia arriba)
        // Solo depende del volumen sumergido y la densidad del fluido
        float empuje = rhoFluido * volumen * G;

        // 3. Arrastre (resistencia al movimiento)
        float Cd = 0.47f; // Coeficiente para esfera
        float area = 3.1415f * std::pow(radioMetros, 2);
        float vY = vel.y / ESCALA;
        float arrastre = 0.5f * rhoFluido * (vY * vY) * Cd * area;
        if (vY < 0) arrastre *= -1; // Invertir si sube

        // Fuerza Total y Aceleración
        float fTotal = peso - empuje - (vel.y > 0 ? arrastre : -arrastre);
        float accY = fTotal / masa;

        vel.y += accY * G * dt; // Aplicamos aceleración
        shape.move({0, vel.y * dt});
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 800}), "Simulador de Densidades SFML 3");
    sf::Clock clock;

    float nivelAgua = 400.f;
    std::vector<Cuerpo> esferas;

    // Diferentes materiales (Radio px, Densidad kg/m3, Color, Posición)
    esferas.emplace_back(20, 7800, sf::Color::Red,    sf::Vector2f(200, 50)); // Acero (Cae rápido siempre)
    esferas.emplace_back(25, 2700, sf::Color::Cyan,   sf::Vector2f(400, 50)); // Aluminio (Se frena notablemente)
    esferas.emplace_back(30, 1100, sf::Color::Yellow, sf::Vector2f(600, 50)); // Casi agua (Cae muy lento en líquido)

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.1f) dt = 0.1f; // Evitar saltos por lag

        window.clear(sf::Color(30, 30, 30));

        // Dibujar Agua
        sf::RectangleShape rectAgua({800, 400});
        rectAgua.setPosition({0, nivelAgua});
        rectAgua.setFillColor(sf::Color(0, 0, 255, 80));
        window.draw(rectAgua);

        for (auto& e : esferas) {
            e.actualizar(dt, nivelAgua);
            window.draw(e.shape);
        }

        window.display();
    }
}