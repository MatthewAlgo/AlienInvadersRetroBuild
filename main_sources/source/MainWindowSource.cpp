#include "alieninvadersretro/MainWindowHeader.h"
#include "alieninvadersretro/AnimationWindow.h"
#include "alieninvadersretro/BoomBox.h"
#include "alieninvadersretro/VirtualWindow.h"
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <stdlib.h>
#include <iostream>

#pragma region MAINCLASS_FUNC_IMPLEMENTATIONS
void MatthewsNamespace::MainWindowClass::MainWindowThreadExecution(TripleItemHolder<sf::RenderWindow, sf::Thread, VirtualWindowClass>& ITEM_HOLDER) {
	sf::WindowHandle handle = ITEM_HOLDER.getA()->getSystemHandle(); // Use the handle with OS specific functions
	// Main Window Settings
	ITEM_HOLDER.getA()->setActive(true);
	ITEM_HOLDER.getA()->setVerticalSyncEnabled(true);
	ITEM_HOLDER.getA()->setFramerateLimit(60);

	std::unique_ptr<DoubleItemHolder<sf::RenderWindow, VirtualWindowClass>> CurrentHolder = std::make_unique<DoubleItemHolder<sf::RenderWindow, VirtualWindowClass>>(WindowPointer, this);
	RenderTextures(*CurrentHolder.get());

	// Display main Window and reset the IMGUI delta clock
	// We need to initiate the IMGUI context
	ImGuiRenderer = std::make_unique<ImGUIRenderer>(ITEM_HOLDER.getA()); 
	ImGuiRenderer->getDeltaClock()->restart();

	while (ITEM_HOLDER.getA()->isOpen()) {
		sf::Event* Event = new sf::Event();
		
		while (ITEM_HOLDER.getA()->pollEvent(*Event)) {
			
			// Event handling for IMGUi
			ImGuiRenderer->ToBeCalledAfterEventHandling(Event);
			
			if (Event->type == sf::Event::Closed) {
				BoomBox::IS_MUSIC_ENABLED = false; // Disable music
				BoomBox::LocalDJ->SOUND_MAIN.stop();
				BoomBox::LocalDJ->MainThemeSound.stop();
				sf::sleep(sf::Time(sf::seconds(1))); // Sleep for 1s
				delete this->ParticleGenerator; // Delete the random particles generator
				ITEM_HOLDER.getA()->close(); // Deletes the animation window
				exit(EXIT_SUCCESS);
			}
			else if (Event->type == sf::Event::MouseButtonReleased) {
				std::unique_ptr<sf::Mouse> MyMouse = std::make_unique<sf::Mouse>();

				// 630-250 - BR first button | 340-150 - TL first button
				if (MyMouse.get()->getPosition(*WindowPointer).x >= 340 && MyMouse.get()->getPosition(*WindowPointer).x <= 630
					&& MyMouse.get()->getPosition(*WindowPointer).y >= 150 && MyMouse.get()->getPosition(*WindowPointer).y <= 250) {

					// The first (start) button is pressed -> Launch the game (animation window) and player stats window
					if (AnimationWindow::ANIMATION_INSTANCES == 0) {
						if (BoomBox::getMainTheme()->getStatus() == sf::SoundSource::Status::Playing) {
							BoomBox::getMainTheme()->pause();
						}
						MatthewsNamespace::AnimationWindow* MyMainWindow
							= new MatthewsNamespace::AnimationWindow("AlienInvasionRetro", 1000, 700);
						BoomBox::WindowSoundEffect();
					}
					else {
						// Cannot open window due to too many instances
						BoomBox::WrongSelectionEffect();
					}
				}
				// 760-380 - BR second button | 200-280 - TL first button
				if (MyMouse.get()->getPosition(*WindowPointer).x >= 200 && MyMouse.get()->getPosition(*WindowPointer).x <= 760
					&& MyMouse.get()->getPosition(*WindowPointer).y >= 280 && MyMouse.get()->getPosition(*WindowPointer).y <= 380) {
					// Open The BoomBoxWindow
					if (BoomBox::BOOMBOX_INSTANCES == 0) {
						MatthewsNamespace::BoomBox* MyBoomBox = new MatthewsNamespace::BoomBox("BoomBox - Retro", 500, 500);
						BoomBox::WindowSoundEffect();
					}
					else {
						// Cannot open boombox due to too many instances
						BoomBox::WrongSelectionEffect();
					}
				}

			}
			else if (Event->type == sf::Event::KeyPressed) {
				if (Event->key.code == sf::Keyboard::Escape) { // Exits on ESC pressed
					BoomBox::IS_MUSIC_ENABLED = false; // Disable music
					BoomBox::LocalDJ->SOUND_MAIN.stop();
					BoomBox::LocalDJ->MainThemeSound.stop();
					sf::sleep(sf::Time(sf::seconds(1))); // Sleep for 1s
					delete this->ParticleGenerator; // Delete the random particles generator
					ITEM_HOLDER.getA()->close(); // Deletes the animation window

					exit(EXIT_SUCCESS);
				}
			}
			else if (Event->type == sf::Event::TextEntered) {
				if (Event->text.unicode < 128)
					break;
			}
		}
		// Check For BoomBox Status
		if ((BoomBox::IS_MUSIC_ENABLED == 1) && ((BoomBox::getMainTheme()->getStatus() == sf::SoundSource::Status::Paused)
			|| (BoomBox::getMainTheme()->getStatus() == sf::SoundSource::Status::Stopped)) && (AnimationWindow::ANIMATION_INSTANCES == 0)){
				BoomBox::StartMainThemeSong();
		}
		else {
			if (AnimationWindow::ANIMATION_INSTANCES == 1) {
				BoomBox::getMainTheme()->stop();
			}
		}
		std::free(Event);

		MainWindowClass* MyWindowVirt = dynamic_cast<MainWindowClass*>(ITEM_HOLDER.getC()); // Polymorphic conversion
		MatthewsNamespace::MainWindowClass::DrawInsideMainWindow(ITEM_HOLDER.getA(), ITEM_HOLDER.getB(), MyWindowVirt);
		MyWindowVirt = NULL; delete MyWindowVirt;
	}
}
void MatthewsNamespace::MainWindowClass::DrawInsideMainWindow(sf::RenderWindow* WINDOW, sf::Thread* WINTHREAD, MatthewsNamespace::MainWindowClass* C) {
	WINDOW->clear(sf::Color::Red);
	WINDOW->draw(BackGround->SPRITE);

	// Draw the buttons
	WINDOW->draw(MenuBox1.SPRITE);
	WINDOW->draw(MenuBox2.SPRITE);
	WINDOW->draw(TextBTN1);
	WINDOW->draw(TextBTN2);

	// For the particle generator
	ParticleGenerator->Generate();

	ParticleGenerator->InLoopForParticles(WINDOW);
	ParticleGenerator->ClearMemory(WINDOW);

	// Draw the greeting text for the main window
	WINDOW->draw(GreetingText);

	// Draw IMGUI Elements
	std::vector<std::string> ScoresVector = C->RawFileData;
	ImGuiRenderer->ToBeCalledForDrawingWindowElements(ScoresVector);
	ImGuiRenderer->RenderImguiContents();
	WINDOW->display();
}
void MatthewsNamespace::MainWindowClass::RenderTextures(DoubleItemHolder<sf::RenderWindow, VirtualWindowClass> ITEM_HOLDER) {
	// Inside a separate thread -> Background
	BackGround = std::make_unique<ImageToBeDrawn>();
	BackGround->TEXTURE.loadFromFile("GameAddicted.jpg");
	BackGround->SPRITE.setTexture(BackGround->TEXTURE);
	BackGround->SPRITE.setScale(0.5, 0.5);

	// Inside a separate thread -> Menu Button
	// First Button
	MenuBox1.TEXTURE.loadFromFile("Rounded-Button.png");
	MenuBox1.SPRITE.setTexture(MenuBox1.TEXTURE);
	MenuBox1.SPRITE.setPosition(WWidth / 3, WHeight / 10);
	MenuBox1.SPRITE.setScale(0.5, 0.5);

	// Second Button
	MenuBox2.TEXTURE.loadFromFile("Rounded-Button.png");
	MenuBox2.SPRITE.setTexture(MenuBox2.TEXTURE);
	MenuBox2.SPRITE.setPosition(WWidth / 5.4, WHeight / 2.75);
	MenuBox2.SPRITE.setScale(1, 0.5);

	// Render Font for text
	GlobalWindowFont.loadFromFile("Fonts/Emulogic.ttf");
	GreetingText.setFont(GlobalWindowFont);
	GreetingText.setString("Alien Invasion - Retro");
	GreetingText.setCharacterSize(24);
	GreetingText.setFillColor(sf::Color::Yellow);
	GreetingText.setStyle(sf::Text::Bold);
	GreetingText.setPosition(WWidth / 5, WHeight / 100);

	// For the first button
	TextBTN1.setFont(GlobalWindowFont);
	TextBTN1.setString("Start");
	TextBTN1.setCharacterSize(24);
	TextBTN1.setFillColor(sf::Color::Blue);
	TextBTN1.setStyle(sf::Text::Bold);
	TextBTN1.setPosition(WWidth / 2.40, WHeight / 2.75);

	// For the second button
	TextBTN2.setFont(GlobalWindowFont);
	TextBTN2.setString("Music / Sound Toggle");
	TextBTN2.setCharacterSize(24);
	TextBTN2.setFillColor(sf::Color::Blue);
	TextBTN2.setStyle(sf::Text::Bold);
	TextBTN2.setPosition(WWidth / 4.25, WHeight / 1.5);


}
void MatthewsNamespace::MainWindowClass::ScoresLoaderLocal(std::string FileName){
	// Load the scores from the file
	std::ifstream ScoresFile(FileName);
	if (ScoresFile.is_open()) {
		std::string Line;
		while (std::getline(ScoresFile, Line)) {
			// Now we need to split the line into the name and the score
			std::stringstream ss(Line);
			std::string Name, Score;
			std::getline(ss, Name, ' ');
			std::getline(ss, Score, ' ');
			// Now we need to convert the score to an integer
			int ScoreInt = std::stoi(Score);
			// Now we need to add the name and the score to the vector;
			PlayerInfo TempPair = {std::pair<std::string, int>(Name, ScoreInt)};
			PlayerInfoList.push_back(TempPair);
		}
		ScoresFile.close();
	}
	else {
		std::cout << "Unable to open file" << std::endl;
	}
}

std::vector<std::string> MatthewsNamespace::MainWindowClass::RawFileReader(std::string FileName) {
	std::vector<std::string> FileLines;
	std::ifstream File(FileName);
	if (File.is_open()) {
		std::string Line;
		while (std::getline(File, Line)) {
			FileLines.push_back(Line);
		}
		File.close();
	}
	else {
		std::cout << "Unable to open file" << std::endl;
	}
	return FileLines;
}

void MatthewsNamespace::MainWindowClass::ScoresSaverLocal(std::string FileName){
	// Save the scores to the file
	std::ofstream ScoresFile(FileName);
	if (ScoresFile.is_open()) {
		// Write the Current Best Score
	}
	else {
		std::cout << "Unable to open file" << std::endl;
	}
}

// Get best game score from a file
int MatthewsNamespace::MainWindowClass::getBestScoreLocal(std::string FileName) {
	std::ifstream MyFile(FileName);
	int BestScore;
	MyFile >> BestScore;
	MyFile.close();
	return BestScore;
}

#pragma endregion MAINCLASS_FUNC_IMPLEMENTATIONS
