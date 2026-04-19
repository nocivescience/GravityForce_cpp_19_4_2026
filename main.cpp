#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>

const float G = 9.81f;
const float RHO_AIRE = 1.225f;
const float RHO_AGUA = 1000.0f;
const float ESCALA = 100.0f; 

struct Cuerpo {
    sf::CircleShape shape;
    sf::Vector2f vel{0, 0};
    float radioMetros;
    float densidadObjeto;
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

    void actualizar(float dt, float nivelAgua, float alturaVentana) {
        bool enAgua = (shape.getPosition().y > nivelAgua);
        float rhoFluido = enAgua ? RHO_AGUA : RHO_AIRE;

        // --- FÍSICA DE CAÍDA ---
        float peso = masa * G;
        float empuje = rhoFluido * volumen * G;
        float area = 3.1415f * (radioMetros * radioMetros);
        float vY_metros = vel.y / ESCALA;
        float Cd = 0.47f;
        
        // Fuerza de arrastre (siempre opuesta a la velocidad)
        float fArrastre = 0.5f * rhoFluido * (vY_metros * vY_metros) * Cd * area;
        if (vel.y > 0) fArrastre = -fArrastre;

        float fTotal = peso - empuje + (vel.y != 0 ? fArrastre : 0);
        float accY = fTotal / masa;

        vel.y += accY * ESCALA * dt;
        shape.move({0, vel.y * dt});

        // --- LÓGICA DE REBOTE EN EL FONDO ---
        float limiteInferior = alturaVentana - shape.getRadius();
        
        if (shape.getPosition().y > limiteInferior) {
            // Reposicionar para que no se "entierre" en el suelo
            shape.setPosition({shape.getPosition().x, limiteInferior});

            // Coeficiente de restitución base (rebote)
            // En agua rebotan mucho menos (0.2) que en aire (0.7)
            float e = enAgua ? 0.2f : 0.7f;

            // El tamaño también afecta: esferas más grandes pierden más energía en el fluido
            // Reducimos el rebote un poco más basándonos en el radio
            float factorTamano = 1.0f - (radioMetros * 2.0f); 
            if (factorTamano < 0.1f) factorTamano = 0.1f;

            // Invertir velocidad y aplicar pérdidas
            vel.y = -vel.y * e * factorTamano;

            // Umbral de parada: si la velocidad es muy baja, detener para evitar micro-rebotes
            if (std::abs(vel.y) < 10.0f) vel.y = 0;
        }
    }
};

int main() {
    // SFML 3 usa sf::VideoMode({ancho, alto})
    sf::RenderWindow window(sf::VideoMode({800, 800}), "Simulador de Rebote en Fluidos");
    sf::Clock clock;

    float nivelAgua = 450.f;
    std::vector<Cuerpo> esferas;

    // 1. Acero (Pequeña, muy densa) -> Rebota un poco más en el fondo del agua
    esferas.push_back(Cuerpo(15.f, 7800.f, sf::Color::Red,    sf::Vector2f({200.f, 50.f})));
    esferas.push_back(Cuerpo(30.f, 1200.f, sf::Color::Yellow, sf::Vector2f({400.f, 50.f})));
    esferas.push_back(Cuerpo(45.f, 2700.f, sf::Color::Cyan,   sf::Vector2f({600.f, 50.f})));

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }

        float dt = clock.restart().asSeconds();
        if (dt > 0.02f) dt = 0.02f; // Limitar DT para estabilidad física

        window.clear(sf::Color(20, 20, 25));

        // Dibujar Agua
        sf::RectangleShape rectAgua({800, 350});
        rectAgua.setPosition({0, nivelAgua});
        rectAgua.setFillColor(sf::Color(0, 80, 255, 100));
        window.draw(rectAgua);

        for (auto& e : esferas) {
            e.actualizar(dt, nivelAgua, 800.f);
            window.draw(e.shape);
        }

        window.display();
    }
    return 0;
}