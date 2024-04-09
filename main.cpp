#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>


// Function to keep enemy within window bounds
void keepEnemyInBounds(sf::Sprite& enemy, const sf::RenderWindow& window) {
    const auto halfY = window.getSize().y / 2;
    if (enemy.getPosition().x < 0 || enemy.getPosition().x > window.getSize().x - enemy.getGlobalBounds().width) {
        enemy.setPosition(static_cast<float>(std::rand() % window.getSize().x), enemy.getPosition().y);
    }

    if (enemy.getPosition().y < 0 || enemy.getPosition().y > halfY) {
        enemy.setPosition(enemy.getPosition().x, static_cast<float>(std::rand() % halfY));
    }
}

// Function to respawn enemy within window bounds
void respawnEnemy(sf::Sprite& enemy, const sf::RenderWindow& window) {
    enemy.setPosition(
        static_cast<float>(std::rand() % static_cast<unsigned int>(window.getSize().x)),
        static_cast<float>(std::rand() % static_cast<unsigned int>(window.getSize().y / 2))
    );
}

void resetGameState(sf::RenderWindow& window, int& score, int& lives, bool& gameOver, bool& missionComplete, sf::Sprite& spaceship, std::vector<sf::Sprite>& enemies, std::vector<sf::Sprite>& bullets, sf::Clock& gameTimer, sf::Texture& lifeTexture, std::vector<sf::Sprite>& lifeSprites) {
   
    score = 0;
    lives = 3;
     
    gameOver = false;
    missionComplete = false;
     
    spaceship.setPosition(350, 500);
     
    enemies.clear();
    bullets.clear();

    // Reset life sprites
    lifeSprites.clear();
    for (int i = 0; i < lives; ++i) {
        sf::Sprite lifeSprite(lifeTexture);
        lifeSprite.setPosition(10.f + i * (lifeTexture.getSize().x + 10), 40.f);
        lifeSprites.push_back(lifeSprite);
    }

    gameTimer.restart();
}


int main() {
    sf::Clock explosionTimer;
    bool isDamaged = false;
    sf::Clock damageClock;
    const sf::Time damageDuration = sf::seconds(2.5f);

    bool startGameOverFadeIn = false;
    sf::Clock gameOverFadeClock;
    sf::Uint8 gameOverAlpha = 0;
    const float fadeDuration = 2.5f;

    sf::RenderWindow window(sf::VideoMode(800, 600), "Defender of the Jungle");

    sf::Texture mainMenuTexture;
    if (!mainMenuTexture.loadFromFile("assets/mainMenu.png")) {
        return -1;
    }

    sf::Sprite mainMenu(mainMenuTexture);

    float mainMenuScale = std::min(
        static_cast<float>(window.getSize().x) / mainMenuTexture.getSize().x,
        static_cast<float>(window.getSize().y) / mainMenuTexture.getSize().y
    );
    mainMenu.setScale(mainMenuScale, mainMenuScale);

    float centeredX = (window.getSize().x - (mainMenuTexture.getSize().x * mainMenuScale)) / 2;
    float centeredY = (window.getSize().y - (mainMenuTexture.getSize().y * mainMenuScale)) / 2;
    mainMenu.setPosition(centeredX, centeredY);

    sf::Font font;
    if (!font.loadFromFile("assets/Sansation_Bold.ttf")) {
        return -1;
    }

    sf::Text startText("Press Enter to Start", font, 24);
    startText.setFillColor(sf::Color::White);
    startText.setPosition(
        window.getSize().x / 2 - startText.getLocalBounds().width / 2,
        window.getSize().y - 60
    );

    sf::Texture spaceshipTexture, enemyShipTexture, enemyUFOTexture;
    sf::Texture playerDamagedTexture, playerLeftTexture, playerRightTexture;
    sf::Texture laserGreenShotTexture, laserRedShotTexture;
    sf::Texture shieldTexture, lifeTexture;
    sf::Texture meteorBigTexture, meteorSmallTexture;
    sf::Texture backgroundTexture;

    if (!spaceshipTexture.loadFromFile("assets/player.png") ||
        !enemyShipTexture.loadFromFile("assets/enemyShip.png") ||
        !enemyUFOTexture.loadFromFile("assets/enemyUFO.png") ||
        !playerDamagedTexture.loadFromFile("assets/playerDamaged.png") ||
        !playerLeftTexture.loadFromFile("assets/playerLeft.png") ||
        !playerRightTexture.loadFromFile("assets/playerRight.png") ||
        !laserGreenShotTexture.loadFromFile("assets/laserGreenShot.png") ||
        !laserRedShotTexture.loadFromFile("assets/laserRedShot.png") ||
        !shieldTexture.loadFromFile("assets/shield.png") ||
        !lifeTexture.loadFromFile("assets/life.png") ||
        !backgroundTexture.loadFromFile("assets/jungle.png")) {

        return -1;
    }

    sf::Sprite spaceship(spaceshipTexture);
    sf::Sprite enemyShip(enemyShipTexture);
    sf::Sprite enemyUFO(enemyUFOTexture);
    sf::Sprite playerDamaged(playerDamagedTexture);
    sf::Sprite playerLeft(playerLeftTexture);
    sf::Sprite playerRight(playerRightTexture);
    sf::Sprite laserGreenShot(laserGreenShotTexture);
    sf::Sprite laserRedShot(laserRedShotTexture);
    sf::Sprite shield(shieldTexture);
    sf::Sprite life(lifeTexture);
    sf::Sprite meteorBig(meteorBigTexture);
    sf::Sprite meteorSmall(meteorSmallTexture);
    sf::Sprite background(backgroundTexture);

    std::vector<sf::Sprite> enemies;

    sf::Texture pauseTexture;
    if (!pauseTexture.loadFromFile("assets/Pause.png")) {
        // Handle error loading texture
        return -1;
    }

    sf::Sprite pauseSprite(pauseTexture);
    sf::Clock hitDelayClock;
    const sf::Time hitDelayDuration = sf::seconds(2.0f);

    bool isGamePaused = false;
    sf::Text pauseText("Press P to resume", font, 32);
    pauseText.setFillColor(sf::Color::White);
    pauseText.setPosition(window.getSize().x - pauseText.getLocalBounds().width - 10, 10);

  
    sf::Text pauseMessage("Press P to Pause", font, 32);
    pauseMessage.setFillColor(sf::Color::White);
    pauseMessage.setPosition(window.getSize().x - pauseMessage.getLocalBounds().width - 10, 10);
     
    sf::Texture laserEnemyShotTexture;
    if (!laserEnemyShotTexture.loadFromFile("assets/laserRed.png")) {
        return -1; 
    }

  
    std::vector<sf::Sprite> enemyLasers;

    
    const float enemyLaserInterval = 1.4f;
    sf::Clock enemyLaserClock;
 
    background.setScale(
        static_cast<float>(window.getSize().x) / static_cast<float>(backgroundTexture.getSize().x),
        static_cast<float>(window.getSize().y) / static_cast<float>(backgroundTexture.getSize().y)
    );
    
    spaceship.setPosition(350, 500);

    sf::Text scoreText("Score: 0", font, 24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(10.f, 10.f);

    int lives = 3;

    std::vector<sf::Sprite> lifeSprites;
    for (int i = 0; i < lives; ++i) {
        sf::Sprite lifeSprite(lifeTexture);
        lifeSprite.setPosition(10.f + i * (lifeTexture.getSize().x + 10), 40.f);
        lifeSprites.push_back(lifeSprite);
    }

 
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

  
    const float enemySpawnInterval = 1.f; 
    sf::Clock enemySpawnClock;

    const float spaceshipSpeed = 300.f; 
    const float enemySpeed = 100.f; 
    sf::Clock clock;
    sf::Clock respawnClock;


    std::vector<sf::Sprite> bullets;

    bool gameOver = false;

    int score = 0;

    bool isSpacebarReleased = true;
    bool showMainMenu = true; 

    sf::Texture gameOverTexture;
    if (!gameOverTexture.loadFromFile("assets/gameOver.png")) {
        std::cerr << "Failed to load Game Over texture." << std::endl;
        return -1;
    }

    // Initialize Game Over sprite
    sf::Sprite gameOverSprite(gameOverTexture);

    // Calculate the scale to preserve aspect ratio and fill the screen for game over sprite
    float gameOverScale = std::max(
        static_cast<float>(window.getSize().x) / gameOverTexture.getSize().x,
        static_cast<float>(window.getSize().y) / gameOverTexture.getSize().y
    );

    gameOverSprite.setScale(gameOverScale, gameOverScale);

    // Reuse centeredX and centeredY for positioning the game over sprite
    centeredX = (window.getSize().x - gameOverTexture.getSize().x * gameOverScale) / 2;
    centeredY = (window.getSize().y - gameOverTexture.getSize().y * gameOverScale) / 2;
    gameOverSprite.setPosition(centeredX, centeredY);

    // Initialize final score text
    sf::Text finalScoreText;
    finalScoreText.setFont(font); 
    finalScoreText.setCharacterSize(50);
    finalScoreText.setFillColor(sf::Color::Black);  
    finalScoreText.setStyle(sf::Text::Bold);

    sf::Clock gameTimer;
    const sf::Time gameDuration = sf::seconds(30); // 30 seconds

    // Initialize timer text
    sf::Text timerText("", font, 24);
    timerText.setFillColor(sf::Color::White);
    timerText.setPosition(window.getSize().x - timerText.getLocalBounds().width - 10, 0);

    // Declare a texture and sprite for the Mission Complete screen
    sf::Texture missionCompleteTexture;
    sf::Sprite missionCompleteSprite;

    // Load the texture (add this to the texture loading section)
    if (!missionCompleteTexture.loadFromFile("assets/missionCompleted.png")) {
        return -1;
    }

    // Create the sprite (add this to the sprite initialization section)
    missionCompleteSprite.setTexture(missionCompleteTexture);
    missionCompleteSprite.setScale(
        static_cast<float>(window.getSize().x) / missionCompleteTexture.getSize().x,
        static_cast<float>(window.getSize().y) / missionCompleteTexture.getSize().y
    );

    bool missionComplete = false;
    bool gameStarted = false; 


// Game loop
    while (window.isOpen()) {
        sf::Time deltaTime = clock.restart();
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (missionComplete && sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
                window.close(); 
            }
           
            if ((missionComplete || gameOver) && sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
                missionComplete = false;
                gameOver = false;
                gameStarted = true; 

               
                score = 0;
                lives = 3;
                enemies.clear();
                bullets.clear();
                enemyLasers.clear();
                spaceship.setPosition(350, 500); 
                lifeSprites.clear();
                for (int i = 0; i < lives; ++i) {
                    sf::Sprite lifeSprite(lifeTexture);
                    lifeSprite.setPosition(10.f + i * (lifeTexture.getSize().x + 10), 40.f);
                    lifeSprites.push_back(lifeSprite);
                }
                gameTimer.restart(); 
                continue; 
            }

            if (!gameStarted && event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                gameStarted = true;

                score = 0;
                lives = 3;
                enemies.clear();
                bullets.clear();
                enemyLasers.clear();
                gameTimer.restart();
                spaceship.setPosition(350, 500);
            }

            if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Space) {
                isSpacebarReleased = true;
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
                isGamePaused = !isGamePaused;
            }

            if (isGamePaused) {
                // Check for 'R' key press to restart the game
                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R) {
                   
                    isGamePaused = false; 
                    gameStarted = true; 

                    lives = 3;
                    score = 0;
                    spaceship.setPosition(350, 500); 
                    spaceship.setColor(sf::Color(255, 255, 255, 255)); 
                    enemies.clear();
                    bullets.clear();
                    enemyLasers.clear();
                    lifeSprites.clear();
                    for (int i = 0; i < lives; ++i) {
                        sf::Sprite lifeSprite(lifeTexture);
                        lifeSprite.setPosition(10.f + i * (lifeTexture.getSize().x + 10), 40.f);
                        lifeSprites.push_back(lifeSprite);
                    }
                 

                    gameTimer.restart();

                    isDamaged = false;
                    break; 


                }


                if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Q) {
                    window.close(); 
                    break;
                }
            }
        }

        if (!gameStarted) {
            window.clear();
            window.draw(mainMenu);
            window.draw(startText);
            window.display();
            continue; 
        }

        if (isGamePaused) {
            window.clear();
            pauseSprite.setScale(
                static_cast<float>(window.getSize().x) / pauseTexture.getSize().x,
                static_cast<float>(window.getSize().y) / pauseTexture.getSize().y
            );
            window.draw(pauseSprite); 
            window.display();
            continue;
        }

       
        if (lives < lifeSprites.size()) {
            lifeSprites.pop_back();
        }

        if (lives <= 1) {
            spaceship.setTexture(playerDamagedTexture);
        }
        else {
            spaceship.setTexture(spaceshipTexture); 
        }

        if (isDamaged && damageClock.getElapsedTime() > damageDuration) {
            spaceship.setColor(sf::Color(255, 255, 255, 255)); 
            isDamaged = false;
            std::cout << "Fade effect ended, spaceship vulnerable again." << std::endl;
        }

    
        sf::Time timeElapsed = gameTimer.getElapsedTime();
        sf::Time timeRemaining = gameDuration - timeElapsed;

       
        if (timeRemaining.asSeconds() <= 0 && !missionComplete) {
            missionComplete = true;

        }

        int remainingMinutes = static_cast<int>(timeRemaining.asSeconds()) / 60;
        int remainingSeconds = static_cast<int>(timeRemaining.asSeconds()) % 60;
        std::ostringstream timeStream;
        timeStream << "Time: "
            << std::setw(2) << std::setfill('0') << remainingMinutes << ":"
            << std::setw(2) << std::setfill('0') << remainingSeconds;
        timerText.setString(timeStream.str());

        timerText.setPosition(window.getSize().x - timerText.getLocalBounds().width - 10, 70.f); 

        // Shooting mechanics
        if (isSpacebarReleased && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            sf::Sprite bullet(laserGreenShotTexture);
            bullet.setPosition(spaceship.getPosition().x + spaceship.getGlobalBounds().width / 2 - bullet.getGlobalBounds().width / 2, spaceship.getPosition().y);
            bullets.push_back(bullet);
            isSpacebarReleased = false;
            std::cout << "Shot fired!" << std::endl;
        }

        // Move bullets
        for (size_t i = 0; i < bullets.size(); ) {
            bullets[i].move(0.f, -0.7); 

            if (bullets[i].getPosition().y < 0) {
                bullets.erase(bullets.begin() + i);
            }
            else {
                ++i;
            }
        }

        // Spaceship movement with boundary checks and visual feedback
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && spaceship.getPosition().x > 0) {
            spaceship.setTexture(playerLeftTexture);
            spaceship.move(-spaceshipSpeed * deltaTime.asSeconds(), 0.f);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && spaceship.getPosition().x < window.getSize().x - spaceship.getGlobalBounds().width) {
            spaceship.setTexture(playerRightTexture);
            spaceship.move(spaceshipSpeed * deltaTime.asSeconds(), 0.f);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && spaceship.getPosition().y > 0) {
            spaceship.move(0.f, -spaceshipSpeed * deltaTime.asSeconds());
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && spaceship.getPosition().y < window.getSize().y - spaceship.getGlobalBounds().height) {
            spaceship.move(0.f, spaceshipSpeed * deltaTime.asSeconds());
        }

        // Enemy movement and respawn logic
        if (enemySpawnClock.getElapsedTime().asSeconds() > enemySpawnInterval) {
            sf::Sprite newEnemy(std::rand() % 2 == 0 ? enemyShipTexture : enemyUFOTexture);
            respawnEnemy(newEnemy, window);
            enemies.push_back(newEnemy);
            enemySpawnClock.restart();
        }

        for (size_t i = 0; i < enemies.size(); ++i) {
            auto& enemy = enemies[i];
            float xMove = (std::rand() % 3 - 1) * enemySpeed * deltaTime.asSeconds();
            float yMove = (std::rand() % 3 - 1) * enemySpeed * deltaTime.asSeconds();
            enemy.move(xMove, yMove);
            keepEnemyInBounds(enemy, window);

            // Check for collision with bullets
            for (size_t j = 0; j < bullets.size(); ) {
                if (bullets[j].getGlobalBounds().intersects(enemy.getGlobalBounds())) {
                    
                    bullets.erase(bullets.begin() + j);
                    enemies.erase(enemies.begin() + i);
                    score += 100; 

                }
                else {
                    ++j;
                }
            }
        }

        if (lives <= 0 && !gameOver) {
            gameOver = true; 
            spaceship.setPosition(-100, -100);
            for (auto& enemy : enemies) {
                enemy.setPosition(-100, -100);
            }
        } 
        if (!gameOver) {
            for (auto& enemy : enemies) {
                // Check if the current enemy should shoot
                float randomFactor = static_cast<float>(std::rand() % 1000) / 1000.0f; 
                if (enemyLaserClock.getElapsedTime().asSeconds() + randomFactor > enemyLaserInterval) {
                    sf::Sprite enemyLaser(laserEnemyShotTexture);
                    enemyLaser.setPosition(enemy.getPosition().x + enemy.getGlobalBounds().width / 2 - enemyLaser.getGlobalBounds().width / 2, enemy.getPosition().y);
                    enemyLasers.push_back(enemyLaser);
                    enemyLaserClock.restart();
                }
            }
        }

        // Move enemy lasers
        for (auto& laser : enemyLasers) {
            laser.move(0, 300.0f * deltaTime.asSeconds()); 
        }

        // Enemy laser collision
        for (size_t i = 0; i < enemyLasers.size();) {
            if (!isDamaged && spaceship.getGlobalBounds().intersects(enemyLasers[i].getGlobalBounds())) {
                isDamaged = true;
                damageClock.restart();
                spaceship.setColor(sf::Color(255, 255, 255, 128));
                lives -= 1;
                std::cout << "Spaceship hit by laser! Lives remaining: " << lives << std::endl;

                enemyLasers.erase(enemyLasers.begin() + i);

                if (lives <= 0) {
                    gameOver = true;
                    spaceship.setPosition(-100, -100);
                    gameTimer.restart();
                    break;
                }
            }
            else {
                ++i;
            }
        }

        // Check for collisions with enemy ships
        for (auto& enemy : enemies) { 
            if (!isDamaged && spaceship.getGlobalBounds().intersects(enemy.getGlobalBounds())) {
                isDamaged = true;  
                damageClock.restart();
                spaceship.setColor(sf::Color(255, 255, 255, 128));  
                lives -= 1;  

                // Move the enemy off-screen or respawn
                enemy.setPosition(-100, -100);

                if (lives <= 0) {
                    gameOver = true;
                    spaceship.setPosition(-100, -100);
                    break;
                }
            }
        }

        // Check for player damage and game over condition
        for (auto& enemy : enemies) {
            if (!isDamaged && spaceship.getGlobalBounds().intersects(enemy.getGlobalBounds())) {
                lives -= 1;
                if (lives <= 0) {
                   
                    gameOver = true;
                    spaceship.setPosition(-100, -100);
                    for (auto& enemy : enemies) {
                        enemy.setPosition(-100, -100);
                    }
                    break;
                }
                enemy.setPosition(-100, -100);
            }
        }

        scoreText.setString("Score: " + std::to_string(score));

        if (gameOver && !startGameOverFadeIn) {
            startGameOverFadeIn = true;
            gameOverFadeClock.restart();
        }

        if (startGameOverFadeIn) {
            float time = gameOverFadeClock.getElapsedTime().asSeconds();
            gameOverAlpha = static_cast<sf::Uint8>(255 * std::min(time / fadeDuration, 1.0f));
            gameOverSprite.setColor(sf::Color(255, 255, 255, gameOverAlpha));

            
            if (time >= fadeDuration) {
                // Handle the "Press Enter to Restart" logic here
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
                   
                    startGameOverFadeIn = false;
                    gameOverAlpha = 0;
                    gameOverSprite.setColor(sf::Color(255, 255, 255, gameOverAlpha));
                    gameTimer.restart();
                }
            }
        }

        if (isGamePaused) {
            window.draw(pauseText);
        }

        // Modify the timer checking section
        if (timeRemaining.asSeconds() <= 0 && !gameOver) {
            gameOver = true;
            missionComplete = true;
        }


        if (missionComplete) {
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter) {
                   
                    missionComplete = false;
                    gameStarted = false;
                    score = 0;
                    lives = 3;

                    gameTimer.restart();
                }
                else if (event.key.code == sf::Keyboard::Q) {
                    window.close();
                }
            }
        }

        window.clear();
        window.draw(background);

        if (isGamePaused) {
            window.clear();
            window.draw(pauseText);
            window.display();
            continue; 
        }

        if (!isGamePaused) {
            window.draw(pauseMessage);
        }

        for (const auto& laser : enemyLasers) {
            window.draw(laser);
        }

        if (gameOver) {
           
            if (startGameOverFadeIn) {
                window.draw(gameOverSprite); 
            }
        }


        if (missionComplete) {
            // Update the final score text
            finalScoreText.setString("Final Score: " + std::to_string(score));
            finalScoreText.setPosition(
                (window.getSize().x - finalScoreText.getLocalBounds().width) / 2.f,
                (window.getSize().y - finalScoreText.getLocalBounds().height) / 1.05f
            );
            window.draw(missionCompleteSprite);
            window.draw(finalScoreText);
            window.display();

            continue;
        }

        if (!gameOver) {
            // Draw game objects only if the game is not over
            window.draw(spaceship);
            for (const auto& enemy : enemies) {
                window.draw(enemy);
            }
            for (const auto& bullet : bullets) {
                window.draw(bullet);
            }
            for (const auto& lifeSprite : lifeSprites) {
                window.draw(lifeSprite);
            }
            window.draw(scoreText);
        }
       

        if (!gameOver && !isGamePaused) {
            window.draw(timerText);
        }

        window.display();

        if (gameOver) {
            // Wait for player input to restart the game
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
               
                lives = 3;
                score = 0;
                gameOver = false;
                spaceship.setPosition(350, 500);// Reset spaceship position
                spaceship.setColor(sf::Color(255, 255, 255, 255)); // Reset spaceship color to normal
                // Clear enemies and bullets
                enemies.clear();
                bullets.clear();
                // Reset life sprites
                lifeSprites.clear();
                for (int i = 0; i < lives; ++i) {
                    sf::Sprite lifeSprite(lifeTexture);
                    lifeSprite.setPosition(10.f + i * (lifeTexture.getSize().x + 10), 40.f);
                    lifeSprites.push_back(lifeSprite);
                }

             
                isDamaged = false; // Also reset the damage state
            }
        }
    }
    return 0;
}
