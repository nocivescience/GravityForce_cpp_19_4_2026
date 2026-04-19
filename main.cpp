#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

// Constantes globales
const float GRAVEDAD = 9.81f;
const float DENSIDAD_AIRE = 1.225f;
const float DENSIDAD_LIQUIDO = 1000.0f; // Agua
const float ESCALA = 100.0f; // 100 píxeles = 1 metro

class Cuerpo {
public:
    sf::CircleShape shape;
    sf::Vector2f velocidad{0.f, 0.f};
    float masa;
    float radio;

    Cuerpo(float r, sf::Color color, float m, sf::Vector2f pos) : radio(r), masa(m) {
        shape.setRadius(radio);
        shape.setOrigin({radio, radio}); // Origen al centro
        shape.setFillColor(color);
        shape.setPosition(pos);
    }

    void actualizar(float dt, float nivelAgua) {
        // Determinar en qué medio estamos
        bool enAgua = (shape.getPosition().y + radio > nivelAgua);
        float rho = enAgua ? DENSIDAD_LIQUIDO : DENSIDAD_AIRE;
        
        // 1. Fuerza de Gravedad
        float Fg = masa * GRAVEDAD * ESCALA;

        // 2. Fuerza de Arrastre (Simplificada)
        float Cd = 0.47f; // Esfera
        float area = (radio / ESCALA); // En metros
        float v = velocidad.y / ESCALA;
        float Fd = 0.5f * rho * (v * v) * Cd * area * (v > 0 ? 1 : -1);

        // 3. Empuje (Arquímedes)
        float Fe = 0.f;
        if (enAgua) {
            float volumen = (4.f/3.f) * 3.1415f * std::pow(radio/ESCALA, 3);
            Fe = rho * GRAVEDAD * volumen * ESCALA;
        }

        // Aceleración a = F / m
        float aceleracionY = (Fg - Fd - Fe) / masa;
        
        velocidad.y += aceleracionY * dt;
        shape.move({0.f, velocidad.y * dt});
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 800}), "Simulador de Caida Libre - SFML 3");
    sf::Clock clock;

    float nivelAgua = 500.f;
    sf::RectangleShape agua(sf::Vector2f(800.f, 300.f));
    agua.setPosition({0.f, nivelAgua});
    agua.setFillColor(sf::Color(0, 100, 255, 100));

    std::vector<Cuerpo> cuerpos;
    cuerpos.emplace_back(15.f, sf::Color::Red, 2.0f, sf::Vector2f(200.f, 50.f));   // Pesado
    cuerpos.emplace_back(25.f, sf::Color::Green, 0.5f, sf::Vector2f(400.f, 50.f)); // Ligero y grande
    cuerpos.emplace_back(10.f, sf::Color::Yellow, 5.0f, sf::Vector2f(600.f, 50.f)); // Muy denso

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        float dt = clock.restart().asSeconds();

        window.clear(sf::Color(200, 230, 255)); // Cielo
        window.draw(agua);

        for (auto& c : cuerpos) {
            c.actualizar(dt, nivelAgua);
            window.draw(c.shape);
        }

        window.display();
    }
    return 0;
}