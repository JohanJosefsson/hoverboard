#define _CRT_SECURE_NO_WARNINGS
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <list>

#define JJ_FULLSCREEN

#include <cassert>

#define TPS (40)
#define timePerFrame (sf::seconds(1.f / (TPS*1.f)))

class HbCoords
{
	static int screen_w;
	static int screen_h;
	static int margin;
	float x;
	float y;
public:
	static void init(int w, int h, int m);
	static void setPos(sf::Sprite &s, float x, float y);

};
void HbCoords::init(int w, int h, int m) {
	screen_w = w;
	screen_h = h;
	margin = m;
}
int HbCoords::screen_w;
int HbCoords::screen_h;
int HbCoords::margin;
void HbCoords::setPos(sf::Sprite &s, float x, float y)
{
	s.setOrigin(s.getLocalBounds().width / 2, s.getLocalBounds().height / 2);
	float x1 = x + screen_w / 2;
	float y1 = screen_h - margin - y;
	s.setPosition(x1, y1);
}

class GraphicData
{
	sf::Texture starTexture;
	sf::Texture stoneTexture;
	sf::Texture tuftTexture;
	sf::Color bkgCol;
public:
	GraphicData()
	{
		if (!starTexture.loadFromFile("../res/star.png"))
		{
			printf("Error loading pic!\n");
			exit(1);
		}
		if (!stoneTexture.loadFromFile("../res/stone.png"))
		{
			printf("Error loading pic!\n");
			exit(1);
		}
		if (!tuftTexture.loadFromFile("../res/tuft.png"))
		{
			printf("Error loading pic!\n");
			exit(1);
		}
		bkgCol = sf::Color(169, 246, 164);
	}
	sf::Texture getStarTexture() const { return starTexture; }
	sf::Texture getStoneTexture() const { return stoneTexture; }
	sf::Texture getTuftTexture() const { return tuftTexture; }

	sf::Color getBkgCol() const { return bkgCol; }
};



class Screen
{
	static int width_;
	static int height_;
public:
	static int getWidth() { return width_; };
	static int getHeight() { return height_; };
	static void init(int width, int height) { width_ = width; height_ = height; }
};
int Screen::width_ = 0;
int Screen::height_ = 0;

class Thing
{
	sf::Texture texture;
	sf::Sprite sprite;
	float x;
	float y;
	int picIndex;
	Thing * next;
	bool isHit;
public:
	enum Kind { NOTHING, TUFT, STAR, STONE };
private:
	enum Kind kind;
public:
	Thing(float x, float y, Kind kind, GraphicData &graphicData) :x(x), y(y), next(0), kind(kind), isHit(false) {
		switch (kind) {

		case STAR:
			texture = graphicData.getStarTexture();
			break;
		case STONE:
			texture = graphicData.getStoneTexture();
			break;
		case TUFT:
			texture = graphicData.getTuftTexture();
			break;
		}

		sprite.setTexture(texture);
	}
	bool move(float playerFwd, float playerSidews) {
		x -= playerSidews;
		y -= playerFwd;
		HbCoords::setPos(sprite, x, y);
		return (y > -100.0);
	}
	void draw(sf::RenderWindow& bkg) {
		bkg.draw(sprite);
	}
	enum Kind hitsBoard()
	{
		if (!isHit && kind != NOTHING && abs(y) < sprite.getLocalBounds().height / 2 && abs(x) < sprite.getLocalBounds().width / 2)
		{
			isHit = true;
			return kind;
		}
		else
			return NOTHING;
	}
};

class
{
	std::list<Thing*> things;

public:
	void add(Thing * p)
	{
		things.push_front(p);
	}
	void move(float playerFwd, float playerSidews)
	{
		std::list<Thing *>::iterator it = things.begin();
		while (it != things.end()) {
			if (!(*it)->move(playerFwd, playerSidews))
			{
				delete *it;
				it = things.erase(it);
				continue;
			}
			it++;
		};
	}

	void draw(sf::RenderWindow& bkg) {

		for (std::list<Thing *>::iterator it = things.begin(); it != things.end(); it++) {
			(*it)->draw(bkg);
		}
	}

	Thing::Kind const hitsBoard() {
		for (std::list<Thing *>::iterator it = things.begin(); it != things.end(); it++) {
			Thing::Kind kind = (*it)->hitsBoard();
			if (Thing::Kind::NOTHING != kind)return kind;
		}
		return Thing::Kind::NOTHING;
	}
} ThingKeeper;


class Rider {

	sf::Texture texture;
	sf::Sprite sprite;
	sf::Texture texture_q;
	sf::Sprite sprite_q;
	sf::Texture boardTexture;
	sf::Sprite boardSprite;

	int screen_w;
	int screen_h;
	Rider() {}
	int angle;
	float dy;
	bool q;
	static const float speed;
	static const float pi;
	int board;
	float sideMove;
	float fwdMove;
public:
	Rider(int screen_w, int screen_h) :screen_w(screen_w), screen_h(screen_h), angle(0), dy(0.0), q(false), board(0) {
		Filter.init();
		if (!texture.loadFromFile("../res/rider.png"))
		{
			printf("Error loading pic!\n");
			exit(1);
		}
		sprite.setTexture(texture);
		HbCoords::setPos(sprite, 0, sprite.getLocalBounds().height / 2);

		if (!texture_q.loadFromFile("../res/rider_q.png"))
		{
			printf("Error loading pic!\n");
			exit(1);
		}
		sprite_q.setTexture(texture_q);
		HbCoords::setPos(sprite_q, 0, sprite_q.getLocalBounds().height / 2);

		if (!boardTexture.loadFromFile("../res/board.png"))
		{
			printf("Error loading pic!\n");
			exit(1);
		}
		boardSprite.setTexture(boardTexture);
		HbCoords::setPos(boardSprite, 0, 0);
	}

	void draw(sf::RenderWindow& bkg)
	{
		if (!q)
			bkg.draw(sprite);
		else
			bkg.draw(sprite_q);
		boardSprite.setRotation(angle);
		bkg.draw(boardSprite);

	}
	void setAngle(int angle) { this->angle = angle; }
private:
	class {
		int tim_ms;
		int ix;
		static const int n = (1000 * TPS) / 1000;
		float values[n];
	public:
		void init() { ix = 0; for (int i = 0; i < n; i++) { values[i] = 0.0; } }
		float value(float in)
		{
			assert(ix >= 0 && ix < n);
			values[ix] = in;
			float sum = 0.0;
			for (int i = 0; i < n; i++)
			{
				sum += values[(ix - i + n) % n];
				assert(((ix - i + n) % n) >= 0 && ((ix - i + n) % n) < n);
			}
			ix = (++ix) % n;
			return sum / n;
		}
	}Filter;


public:
	void tick() {
		switch (board) {

		default:
		case 0:
		{
			double a1 = angle*0.5;
			double a3 = (-0.001)*(angle*angle*angle);

			sideMove = a1 + a3;


			if (angle > 9) {
				sideMove = 5 + (angle - 9)*0.5;
			}
			if (angle < -9) {
				sideMove = -5 + (angle + 9)*0.5;
			}
		}

		fwdMove = speed * 3 /* 2*2 */;

		break;
		case 1:
			sideMove = angle;
			fwdMove = speed;
			break;
		case 2:
			sideMove = speed * tan(2.0*angle*pi / 180.0);
			fwdMove = speed;
			break;
		case 3:
			if (angle > 10)sideMove = speed / 2.0;
			else if (angle < -10)sideMove = (-1)*speed / 2.0;
			else sideMove = 0;
			fwdMove = speed;
			break;

		case 4:
			//
			fwdMove = speed;
			sideMove = 0.5*Filter.value(angle);
			break;
		}

	}
	float getSideMove() const { return sideMove; }
	float getFwdMove() const { return fwdMove; }

	void changeQ() { q = !q; }

	void setBoard(int b) { board = b; }

};
const float Rider::speed = 2.5;
const float Rider::pi = 3.141592654f;


int get_js()
{
	int js = 0;
	sf::Joystick::update();
	while (js < 8)
	{
		if (sf::Joystick::isConnected(js)) {
			break;
		}
		js++;
	}
	if (js > 7) {
	}
	return js;
}


class StarSrc
{
	int cnt;
	GraphicData graphicData;
public:
	StarSrc(GraphicData &graphicData) :cnt(0) { this->graphicData = graphicData; };
	void tick()
	{

		const int spread = 200;
		if (cnt % 5 == 0) ThingKeeper.add(new Thing(rand() % 4000 - 2000, 800, Thing::Kind::TUFT, graphicData));
		if (cnt % (2 * TPS) == 0)ThingKeeper.add(new Thing(rand() % spread - spread / 2, 800, Thing::Kind::STAR, graphicData));
		if ((rand() % 100) < 40 && (cnt + TPS) % (2 * TPS) == 0)ThingKeeper.add(new Thing(rand() % 600 - 300, 800, Thing::Kind::STONE, graphicData));
		cnt++;
	}
	void move(float playerFwd, float playerSidews)
	{
	}
};


int main()
{
	GraphicData graphicData;

	float starCoord = 0.0;

	StarSrc starSrc(graphicData);

	int js = get_js();

	if (js > 7) {
		// No joystick
	}


	sf::SoundBuffer starBuffer;
	if (!starBuffer.loadFromFile("../res/star.wav"))
		return -1;


	sf::SoundBuffer stoneBuffer;
	if (!stoneBuffer.loadFromFile("../res/stone.wav"))
		return -1;

	sf::Sound starSound;
	starSound.setBuffer(starBuffer);

	sf::Sound stoneSound;
	stoneSound.setBuffer(stoneBuffer);


#ifdef JJ_FULLSCREEN
	// Display the list of all the video modes available for fullscreen
	std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
	for (std::size_t i = 0; i < modes.size(); ++i)
	{
		sf::VideoMode mode = modes[i];
		std::cout << "Mode #" << i << ": "
			<< mode.width << "x" << mode.height << " - "
			<< mode.bitsPerPixel << " bpp" << std::endl;
	}

	sf::VideoMode mode = modes[0];
	// Create a window with the same pixel depth as the desktop
	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	sf::RenderWindow window(sf::VideoMode(mode.width, mode.height, desktop.bitsPerPixel), "SFML window", sf::Style::Fullscreen);
	const int screen_w = mode.width;
	const int screen_h = mode.height;
#else
	sf::RenderWindow window(sf::VideoMode(700, 400), "Hoverboard");
	const int screen_w = 700;
	const int screen_h = 400;
#endif
	HbCoords::init(screen_w, screen_h, 60);

	Screen::init(screen_w, screen_h);

	sf::Clock clock; // starts the clock
	sf::Time timeSinceLastUpdate = sf::Time::Zero;

	window.setMouseCursorVisible(false);


	sf::Font font;
	if (!font.loadFromFile("C:\\Windows\\fonts\\arial.ttf"))
	{
		printf("Error loading font.\n");
		exit(1);
	}


	sf::Text text;

	// select the font
	text.setFont(font);
	text.setCharacterSize(24 * 4); // in pixels, not points!

	text.setOutlineColor(sf::Color::Red);

	int points = 0;
	int ticks = 0;
	int result = 0;

	int printjsx = 0;

	Rider rider(screen_w, screen_h);
	for (int i = 0; i < 20/*50*/; i++)
		ThingKeeper.add(new Thing(rand() % 2000 - 1000, rand() % 800, Thing::Kind::TUFT, graphicData));

	// run the program as long as the window is open
	while (window.isOpen())
	{
		// check all the window's events that were triggered since the last iteration of the loop
		sf::Event event;
		while (window.pollEvent(event))
		{
			// "close requested" event: we close the window
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				switch (event.key.code) {
				case sf::Keyboard::Escape:
					window.close();
					break;
				case sf::Keyboard::Q:
					rider.changeQ();
					break;
				case sf::Keyboard::Num0:
					rider.setBoard(0);
					break;
				case sf::Keyboard::Num1:
					rider.setBoard(1);
					break;
				case sf::Keyboard::Num2:
					rider.setBoard(2);
					break;
				case sf::Keyboard::Num3:
					rider.setBoard(3);
					break;
				case sf::Keyboard::Num4:
					rider.setBoard(4);
					break;
				case sf::Keyboard::Num5:
					rider.setBoard(5);
					break;
				case sf::Keyboard::Num6:
					rider.setBoard(6);
					break;
				case sf::Keyboard::Num7:
					rider.setBoard(7);
					break;
				case sf::Keyboard::Num8:
					rider.setBoard(8);
					break;
				case sf::Keyboard::Num9:
					rider.setBoard(9);
					break;

				case sf::Keyboard::Space:
					points = 0;
					ticks = 0;
					result = 0;
					break;

				}

			}
		}

		// clear the window with black color
		//window.clear(sf::Color::Green);
		// http://www.rapidtables.com/web/color/RGB_Color.htm
		//window.clear(sf::Color(169, 246, 164));
		window.clear(graphicData.getBkgCol());

		// draw everything here...
		// window.draw(...);


		// inside the main loop, between window.clear() and window.display()
		//window.draw(text);
		char textBuf[200];
		if (result == 0 && ticks / TPS >= 30) result = points;
		if (!result) {
			sprintf(textBuf, "%d                    %i", points, ticks / TPS);
		}
		else {
			sprintf(textBuf, "RESULT: %i", result);
		}
		text.setString(textBuf);
		window.draw(text);

		ThingKeeper.draw(window);
		rider.draw(window);

		// end the current frame
		window.display();

		// Check the clock and update if time is due
		//
		timeSinceLastUpdate += clock.restart();
		while (timeSinceLastUpdate > timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;

			// Update stuff...

			if (points > 0)ticks++;

			starSrc.tick();

			rider.tick();
			float rsw = rider.getSideMove();
			float rfwd = rider.getFwdMove();
			ThingKeeper.move(rfwd, rsw);
			starSrc.move(rfwd, rsw);

			starCoord -= rsw;
			if (abs(starCoord) > 500)starCoord = 0.0;

			Thing::Kind hit = ThingKeeper.hitsBoard();
			if (hit != Thing::Kind::NOTHING)printf("hit=%d\n", hit);
			switch (hit)
			{
			case Thing::Kind::STAR:
				starSound.play();
				points++;
				break;
			case Thing::Kind::STONE:
				stoneSound.play();
				points = ((points -= 3) < 0) ? 0 : points;
				break;

			}

		}

		if (js > 7 || !sf::Joystick::isConnected(js)) {
			js = get_js();
		}

		float jsx = sf::Joystick::getAxisPosition(js, sf::Joystick::X);
		rider.setAngle((-1)*jsx);
		printjsx = (int)jsx;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		{
			rider.setAngle(20);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		{
			rider.setAngle(-20);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			rider.setAngle(0);
		}
	}
	return 0;
}
