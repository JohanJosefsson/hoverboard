#define JJ_FULLSCREEN

#define LENGTH (10000)
#define POS_BRAKE_LIMIT_PERCENTAGE (95)
#define NEG_BRAKE_LIMIT_PERCENTAGE (95)
#define SPEED_ADJUSTMENT_FACTOR (0.3)
#define TREES_PER_K (25)
#define SPEED_WHEN_COLLIDING (40.0)

struct {
	int a; int b; int c; int d;
}prsp = {
	85, 50, 50,0
};


/*



backlog
=======


start/finish line

snow spray in correct directions
handle irregular tick times





done:
=====
piste
more visible snow spray
ini file
remove singleton pattern
filter calibration, calibration
speed factor
try hockey stop on input angle

speed dependent snow spray
shutdown bug



*/
#define _CRT_SECURE_NO_WARNINGS
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <list>
#define _USE_MATH_DEFINES 
#include <math.h>
#include <cassert>
#include <array>
#include "ParticleSystem.hpp"


#define maxTimePerFrame (sf::seconds(1.f / (80.0)))
// TODO should be minTime? There should be a max too...

class ViewManager
{
	bool isPov_;
	ViewManager() :isPov_(false) {};

	static ViewManager * instance_;
public:
	void setPov(bool pov)
	{
		isPov_ = pov;
	}
	bool isPov() const { return isPov_; }
	static ViewManager * getInstance()
	{
		if (!instance_) {
			instance_ = new ViewManager();
		}
		return instance_;
	}
};
ViewManager * ViewManager::instance_;

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
		bkgCol = sf::Color::White;// White;
	}
	sf::Texture getStarTexture() const { return starTexture; }
	sf::Texture getStoneTexture() const { return stoneTexture; }
	sf::Texture getTuftTexture() const { return tuftTexture; }

	sf::Color getBkgCol() const { return bkgCol; }
};

/* Once created, this class returns the parameter values necessary for creating a main window */
class Screen
{
	static int width_;
	static int height_;
	sf::VideoMode videoMode_;
	sf::String title_;
	int style_;
public:
	int getWidth() const { return width_; };
	int getHeight() const { return height_; };
	sf::VideoMode getVideoMode() const  { return videoMode_; };
	sf::String getTitle() const { return title_; };
	int getStyle() const { return style_; };
	Screen() {		
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
		videoMode_ = sf::VideoMode(mode.width, mode.height, desktop.bitsPerPixel);
		title_ = sf::String("Full screen");
		style_ = sf::Style::Fullscreen;
		const int screen_w = mode.width;
		const int screen_h = mode.height;
#else
		videoMode_ = sf::VideoMode(700, 400);
		title_ = sf::String("Oh yeah!");
		style_ = sf::Style::Default;
		const int screen_w = 700;
		const int screen_h = 400;
#endif
		width_ = screen_w;
		height_ = screen_h; 
	}
};
int Screen::width_ = 0;
int Screen::height_ = 0;



/* Helps with transformation from logical coordinates to onscreen coordinates */
class CoordsTransform
{
	float w_; // screen width
	float h_; // screen height
	//CoordsTransform(); // do not use TODO
	float y0_; // screen position of logical y=0 

	void transform(const float x, const float y, float& xs, float& ys, float& scale)
	{
		// https://gamedev.stackexchange.com/questions/24957/doing-an-snes-mode-7-affine-transform-effect-in-pygame

	// center-of-the screen coordinates

	// x coordinate
		const float p = w_ / 4;
		const float q = w_ / 4;// 500;
		float z = (-1)*y;
		float xc = p * x / z;

		// y coordinate
		float yc = 0.0;

		// scaling
		scale = q / z;			
		xs = xc + w_ / 2.0;
		ys = 	yc + h_ / 2.0;
	}

public:

	bool isVisible(float x, float y)
	{
		if (ViewManager::getInstance()->isPov())
		{
			return (y < 0.0);
		}
		else {
			return true;
		}
	}

	CoordsTransform() {};
	CoordsTransform(int w, int h, int y0)
	{
		w_ = static_cast<float>(w);
		h_ = static_cast<float>(h);
		y0_ = static_cast<float>(y0);
	}


	float getScreenX(float x, float y)
	{
		if(!ViewManager::getInstance()->isPov())return x + w_ / 2;
		float xs, ys, scale;
		transform(x, y, xs, ys, scale);
		return xs;
	}
	float getScreenY(float x, float y)
	{
		if (!ViewManager::getInstance()->isPov())return y *(-1) + y0_;
		float xs, ys, scale;
		transform(x, y, xs, ys, scale);
		return ys;
	}
	float getScale(float x, float y)
	{
		if (!ViewManager::getInstance()->isPov())return 1.0;
		float xs, ys, scale;
		transform(x, y, xs, ys, scale);
		return scale;
	}

	float gety0() const { return y0_; };


	void setPov(bool pov)
	{
		ViewManager::getInstance()->setPov(pov);
	}

};



/* Encapsulates calls to the SFML joystick */
class JoyS {
	int get_js()
	{
		int js = 0;
		sf::Joystick::update();
		while (js < MAX_JS + 1)
		{
			if (sf::Joystick::isConnected(js)) {
				break;
			}
			js++;
		}
		if (js > MAX_JS) {
		}
		return js;
	}
	int js;
	float jsx;
	static const int MAX_JS = 7;
	bool is_connected;

public:
	JoyS() : is_connected(false), jsx(0.0) {};
	bool connect()
	{
		js = get_js();
		if (js > MAX_JS) {
			// No joystick
			is_connected = false;
		} else {
			is_connected = true;
		}
		return is_connected;
	}
#if 0
	static void reconnect()
	{

		if (js > MAX_JS || !sf::Joystick::isConnected(js)) {
			js = get_js();
		}
	}
#endif
	float get()
	{
		if (is_connected)
		{
			jsx = sf::Joystick::getAxisPosition(js, sf::Joystick::X);
		}
		return jsx;
	}
#if 0
	void fullLeft()
	{
		jsx = 15.0;// JS_MAX;
	}
	void fullRight()
	{
		jsx = -15.0;// JS_MIN;
	}
	void straightDown()
	{
		jsx = 0.0;
	}
#endif
} ;

class Text
{
	sf::Font font_;
	sf::Text text_;
	static const int SZ_BUF = 200;
	char textBuf_[SZ_BUF];
	void init()
	{

		//if (!font_.loadFromFile("C:\\Windows\\fonts\\arial.ttf"))
		if (!font_.loadFromFile("C:\\Windows\\fonts\\DejaVuSansMono.ttf"))
		{
			printf("Error loading font.\n");
			exit(1);
		}
		// select the font
		text_.setFont(font_);
		text_.setCharacterSize(24 * 4); // in pixels, not points!
		text_.setFillColor(sf::Color::Cyan);
		text_.setOutlineColor(sf::Color::Blue);
		text_.setOutlineThickness(0.7);
		textBuf_[0] = '\0';
	}

public:
	Text()
	{
		init();
		//text_.setPosition(0.0, 0.0);
	}
	Text(int pos)
	{
		init();
		text_.setPosition(0.0, pos);
	}
	~Text() {};
	char * getBuf() { return textBuf_; }

	void draw(sf::RenderWindow& bkg)
	{
		text_.setString(textBuf_);
		bkg.draw(text_);
	}
};



/* Keeps info about the physical board (joystick) */
class Board
{
	float js_max;
	float js_min;
	static Board * instance_;
	JoyS joyS;
	Text text;
	Text oText;
	bool observability;
	bool is_connected;
	const static int N = 20; // filter depth
	std::array<float, N> a; // filter for calibration
	int ai; // index within the array a
	void print()
	{
		std::cout << "MAX: " << js_max << " MIN: " << js_min << std::endl;
	}

	float toNorm(float js) const
	{
		return 2.0 * js / (js_max - js_min);
	};

	sf::Time calibration_time;
	bool is_calibrating;
	float ncval;
	float norm;
	float js;

	int writetofile(float min, float max)
	{
		int num;
		FILE *fptr;
		fptr = fopen("calibration.txt", "w");

		if (fptr == NULL)
		{
			printf("Error!");
			exit(1);
		}

		fprintf(fptr, "%f %f", min, max);
		fclose(fptr);

		return 0;
	}

	bool readfromfile(float& min, float& max)
	{
		int num;
		FILE *fptr;

		if ((fptr = fopen("calibration.txt", "r")) == NULL) {
			printf("Could not open file");

			// Program exits if the file pointer returns NULL.
			// exit(1);
			return false;
		}

		fscanf(fptr, "%f %f", &min, &max);

		printf("Value (from file) of min,max=%f %f\n", min, max);
		fclose(fptr);

		return true;
	}




public:

	Board() : js_max(13), js_min(-15), is_calibrating(false), ncval(0.0), norm(0.0), text(200), oText(100), observability(false)
	{
		is_connected = joyS.connect();
		float f1, f2;
		if (readfromfile(f1, f2))
		{
			js_min = f1;
			js_max = f2;
		}

	};


	float get()
	{
		
		float ret;
		if (is_connected)
		{
			js = joyS.get();
			norm = toNorm(js);
			if (norm > 1.0)norm = 1.0;
			if (norm < -1.0)norm = -1.0;
			ret = norm;
		}
		else {
			ret = ncval;
		}
		return ret;
	}
	float getNorm() const { return norm; };
	void set(float val)
	{
		ncval = val;
	}
	void setObservability(bool obs)
	{
		observability = obs;
	}
	void tick(sf::Time dTime)
	{

		if (observability)
		{
			sprintf(oText.getBuf(), "% 4.2f % 4.2f % 4.2f % 4.2f", js_min, js_max, js, norm);
		}
		else {
			sprintf(oText.getBuf(), "");
		}
		if (!is_calibrating) return;

		// Calibrating...

// calibrating with filter
		float j = joyS.get();
		a[ai] = j;
		ai++;
		ai %= N;
		float sum = 0.0;
		for (int i = 0; i < N; i++)
		{
			float e = a[((ai - i) + N) % N];
			sum += e;
		}
		float f = sum / N;

		std::cout << f << std::endl;
		if (f > js_max) {  js_max = f; std::cout << "          max" << std::endl;};
		if (f < js_min) { js_min = f;  std::cout << "     min" << std::endl;};
		//print();
		sprintf(text.getBuf(), "Calibrating... ");

		// Time to end calibration?
		calibration_time += dTime;
		if (calibration_time > sf::seconds(15))
		{
			is_calibrating = false;
			sprintf(text.getBuf(), "");
			writetofile(js_min, js_max);
		}

	}
	void draw(sf::RenderWindow& bkg)
	{
		text.draw(bkg);
		oText.draw(bkg);
	}
	void startCalibration()
	{
		calibration_time = sf::Time::Zero;
		is_calibrating = true;
		js_max = 1.0;
		js_min = -1.0;
	}

};

/* Demo thing that let a ball float over the screen */
class Ball
{
	float x;
	float y;
	sf::CircleShape circle;
public:
	Ball() : x(100.0), circle(50.0) {};
	void move(float playerFwd, float playerSidews)
	{
		y += playerFwd;
		x -= playerSidews;
		if (y > 1000.0) y = 0.0;
		circle.setPosition(x, y);
	}
	void draw(sf::RenderWindow& bkg) {

		bkg.draw(circle);
	}
} Ball;

class Tree
{
	float x;
	float y;
	sf::Texture texture;
	sf::Sprite sprite;
	int w_;
	int h_;
	Tree();
	CoordsTransform ct_;
	int toggle;
public:
	Tree(int w, int h, CoordsTransform ct) : x(100.0), y(0.0), toggle(0)
	{
		w_ = w;
		h_ = h;
		ct_ = ct;
		if (!texture.loadFromFile("../res/spruce.png"))
		{
			printf("Error loading pic!\n");
			exit(1);
		}
		sprite.setTexture(texture);
		sprite.setOrigin(sprite.getLocalBounds().width / 2, sprite.getLocalBounds().height/2);
	};
	void move(float playerFwd, float playerSidews)
	{
		y += playerFwd;
		x += playerSidews;
		if (y > h_ / 2 ) {
			y = (-1)*  h_ * 3.0 / 2;
			x = rand()%(w_/2) - w_/2;
		}
	}

	void draw(sf::RenderWindow& bkg) {
		if (ct_.isVisible(x, y)) {
			sprite.setScale(ct_.getScale(x, y), ct_.getScale(x, y));
			sprite.setPosition(ct_.getScreenX(x, y), ct_.getScreenY(x, y));
			bkg.draw(sprite);
		}
	}
};






class Race
{

	//static const int LENGTH = 5000;
	float accFwd_;
	sf::Time accTime;
	Text text;

	int w_;
	int h_;
	CoordsTransform ct_;
	sf::RectangleShape rectangle_;
	bool showLine_;
	float offset_;
	bool finish_ = false;
	void resetLine()
	{
		if (showLine_) return;
		offset_ = accFwd_ + h_;
		showLine_ = true;
	}

	void reset()
	{
		accFwd_ = 0.0;
		accTime = sf::Time::Zero;
		//resetLine();
		finish_ = false;
	}


public:
	Race(int w, int h, CoordsTransform ct)
	{
		w_ = w;
		h_ = h;
		ct_ = ct;
		rectangle_ = sf::RectangleShape(sf::Vector2f(w_, 10.0));
		rectangle_.setFillColor(sf::Color::Red);
		reset();
	};
	void move(float playerFwd, float playerSidews, sf::Time dTime)
	{
		if(!finish_)
			accTime += dTime;
		accFwd_ += playerFwd;
		if (accFwd_ + h_ > LENGTH)
		{
			resetLine();
		}
		if (accFwd_ > LENGTH)
		{
			finish_ = true;
			//std::cout << "FINISH"<< std::endl;
		}
		sprintf(text.getBuf(), " %2.2f", accTime.asSeconds());
	}

	void draw(sf::RenderWindow& bkg) {
		if (showLine_ && !ViewManager::getInstance()->isPov())
		{
			rectangle_.setPosition(0.0, ct_.getScreenY(100.0, accFwd_ - offset_));
			bkg.draw(rectangle_);
		}
		text.draw(bkg);
	}

	void start()
	{
		reset();
	}
};


















class Trees
{
	sf::Texture texture;
	int w_;
	int h_;
	Trees();
	CoordsTransform ct_;
	class aTree {
	public:
		float x;
		float y;
		sf::Sprite sprite;
		aTree() : sprite() {}
		bool move(float playerFwd, float playerSidews, float y0, int w)
		{
			bool ret = true;
			x += playerSidews;
			y += playerFwd;
			// Let the trees wrap sideways
			if (x > (w + sprite.getLocalBounds().width) / 2) {
				x -= w + sprite.getLocalBounds().width;
			}
			if (x < ((-1)*w - sprite.getLocalBounds().width) / 2) {
				x += w + sprite.getLocalBounds().width;
			}
			if (y > y0 + sprite.getLocalBounds().height / 2)
			{
				// return false if the tree has left the screen
				ret = false;
			}
			return ret;
		}
		bool isColliding()
		{
#if 0
			// right, left , upper, lower edges
			float r = sprite.getLocalBounds().width / 2;
			float l = (-1)*sprite.getLocalBounds().width / 2;
			float u = sprite.getLocalBounds().height / 2;
			float lw = (-1)*sprite.getLocalBounds().height / 2;
			return(x < r && x > l && y < u && y > lw);
#endif
			//const float r = 10.0;
			//return(x < r && x >(-1)*r && y < r && y > (-1)*r);

			// right, left , upper, lower edges
			float r = sprite.getLocalBounds().width / 2;
			float l = (-1)*sprite.getLocalBounds().width / 2;
			float u = sprite.getLocalBounds().height / 2;
			float lw = (-1)*sprite.getLocalBounds().height / 2;
			const float s = 0.5;
			return(x < s*r && x > s*l && y < s*u && y > s*lw);



		}
	};
	std::list<aTree> treeList;
public:
	Trees(int w, int h, CoordsTransform ct)
	{
		w_ = w;
		h_ = h;
		ct_ = ct;
		if (!texture.loadFromFile("../res/spruce.png"))
		{
			printf("Error loading pic!\n");
			exit(1);
		}
	}

	void plantTree(float x)
	{
		aTree t;
		t.sprite.setTexture(texture);
		t.sprite.setOrigin(t.sprite.getLocalBounds().width / 2, t.sprite.getLocalBounds().height/2);
		t.x = x;
		t.y = (-1)*h_ + ct_.gety0() - t.sprite.getLocalBounds().height / 2;
		treeList.push_front(t);
	};
	bool isColliding()
	{
		for (auto &t : treeList)
		{
			if (t.isColliding()) return true;
		}
		return false;
	}

	void clear()
	{
		treeList.clear();
	}

	void move(float playerFwd, float playerSidews)
	{
		// https://stackoverflow.com/questions/596162/can-you-remove-elements-from-a-stdlist-while-iterating-through-it
		for (auto i = treeList.begin(); i != treeList.end();)
		{	
			if((*i).move(playerFwd, playerSidews, ct_.gety0(), w_))
			{
				i++;
			}
			else {
				i = treeList.erase(i);
			}
		}
	}
	void draw(sf::RenderWindow& bkg) {
		for (auto &t : treeList)
		{
			if (ct_.isVisible(t.x, t.y))
			{
				t.sprite.setScale(ct_.getScale(t.x, t.y), ct_.getScale(t.x, t.y));
				t.sprite.setPosition(ct_.getScreenX(t.x, t.y), ct_.getScreenY(t.x, t.y));

				bkg.draw(t.sprite);
			}
		}
	}
};


class Piste
{
	int w_;
	Trees trees;
	float accFwd;
	float accSidews;
//	static int const TREES_PER_K = 25;
	int nPlanted; // Number of planted trees
	// How many trees should be planted?
	int getNPlants()
	{
		int d;
		int nShould = (accFwd * TREES_PER_K) / 1000.0;
		d =  nShould - nPlanted;
		nPlanted = nShould;
		return d;
	}
public:
	Piste(int w, int h, CoordsTransform ct) : trees(w, h, ct), w_(w) {
		clear();
	};
	void move(float playerFwd, float playerSidews)
	{
		float cur = accFwd;
		accFwd += playerFwd;
		accSidews += playerSidews;
#if 0
		if ( (static_cast<int>(cur) / 500) < (static_cast<int>(accFwd) / 500) )
		{
			//trees.plantTree(0.0);
			trees.plantTree(200.0 + accSidews);
			trees.plantTree(-200.0 + accSidews);
		}
#endif

		int n = getNPlants();
		const int pistw = w_*700/2000;
		for (int i = 0; i < n; i++)
		{
			//int x = rand() % (w_ - pistw) + pistw / 2;
			//trees.plantTree(x + accSidews);
			float center = 2000.0 * sin(2 * M_PI / 10000 * accFwd);
			int x = center + rand() % (w_ - pistw) + pistw / 2;
			trees.plantTree(x + accSidews);
		}
		trees.move(playerFwd, playerSidews);
	}
	void draw(sf::RenderWindow& bkg)
	{
		trees.draw(bkg);
	}
	bool isColliding()
	{
		return trees.isColliding();
	}
	void clear()
	{
		trees.clear();

		accFwd = 0.0;
		accSidews = 0.0;
		nPlanted = 0;

	}
};







class Stars
{
	sf::Texture texture;
	static const int NUM_OF_STARS = 5;
	struct StarCoord {
		int x;
		int y;
		sf::Sprite sprite;
	}stars[NUM_OF_STARS];
	float x;
	float y;
	int w_;
	int h_;
	Stars(); // do not
public:
	Stars(int w, int h):x(0.0), y(0.0)
	{
		w_ = w;
		h_ = h;
		if (!texture.loadFromFile("../res/snowstar.png"))
		{
			printf("Error loading pic!\n");
			exit(1);
		}
		for (int i = 0; i < NUM_OF_STARS; i++)
		{
			stars[i].x = static_cast<float>(rand() % w_);
			stars[i].y = static_cast<float>(rand() % h_);
			stars[i].sprite.setTexture(texture);
		}
	}
	void move(float playerFwd, float playerSidews)
	{
		
		y -= playerFwd;
		x += playerSidews;
		x = fmod(x + w_, w_);
		y = fmod(y + h_, h_);
	}
	void draw(sf::RenderWindow& bkg) {
		for (int i = 0; i < NUM_OF_STARS; i++)
		{
			float spriteX = stars[i].x + x;
			spriteX = fmod(spriteX, w_);
			float spriteY = stars[i].y + y;
			spriteY = fmod(spriteY, h_);
			stars[i].sprite.setPosition(spriteX, spriteY);
			bkg.draw(stars[i].sprite);
		}
	}
};


class Player
{
	float speed_;

	sf::RectangleShape board_;

	float c_; // normalized input signal
	float angle_; // Direction of motion [-90.0, 90.0]
	float fwd_;
	float sw_; // sideways
	sf::Texture texture_;
	sf::Sprite sprite_;

	ParticleSystem particles;
	sf::Time acc_time_;
	bool colliding_;

public:
	Player(CoordsTransform ct) :speed_(120.0), c_(0.0), board_(sf::Vector2f(20.0, 100.0)), particles(5000), acc_time_(sf::Time::Zero) {
		board_.setPosition(ct.getScreenX(0.0, 0.0), ct.getScreenY(0.0, 0.0));
		board_.setOrigin(10.0, 50.0);
		board_.setFillColor(sf::Color::Yellow);
		board_.setOutlineColor(sf::Color::Red);
		board_.setOutlineThickness(2.0);
		if (!texture_.loadFromFile("../res/skier.png"))
		{
			printf("Error loading pic!\n");
			exit(1);
		}
		sprite_.setTexture(texture_);
		sprite_.setOrigin(sprite_.getLocalBounds().width / 2, sprite_.getLocalBounds().height);
		sprite_.setPosition(ct.getScreenX(0.0, 0.0), ct.getScreenY(0.0, 0.0));

		// create the particle system
		particles.setEmitter(sf::Vector2f(ct.getScreenX(0.0, 0.0), ct.getScreenY(0.0, 0.0)));

	};
	void tick(sf::Time dTime, float c)
	{
		c_ =c;
		acc_time_ += dTime;

		float c_angle = c_ * 90.0;
		float diff = c_angle - angle_;

	angle_ += diff * dTime.asSeconds() * 2.0;

		// hockey break
		//std::cout << angle_ << std::endl;
		//if (abs(angle_) > 85.0)


		if (c > POS_BRAKE_LIMIT_PERCENTAGE/100.0 || c < (-1)*(NEG_BRAKE_LIMIT_PERCENTAGE/100.0))
		{
			//speed_ -= 500.0 * dTime.asSeconds();
			speed_ *= pow(1/(5.0), dTime.asSeconds());


			//std::cout << "HOCKEY" << std::endl;
			particles.setIntensity((20 * dTime.asSeconds() / maxTimePerFrame.asSeconds()) * (speed_ / 1000.0)); // TODO time dependant... (speed dependant)
		}
		// Friction:
		speed_ -= 250.0 * dTime.asSeconds()* (1/(40.0)/maxTimePerFrame.asSeconds());

		// Gravity:
		speed_ += cos(angle_ * M_PI / 180.0) * 15;
		// Don't go backwards
		if (speed_ < 0.0) speed_ = 0.0;

		// or are we colliding?
		if (colliding_)
		{
			speed_ = SPEED_WHEN_COLLIDING;
		}

		//speed_ = 200.0;


		float step = SPEED_ADJUSTMENT_FACTOR * speed_ * dTime.asSeconds();
		fwd_ = step * cos(angle_ * M_PI / 180.0);
		sw_ = step * sin(angle_ * M_PI / 180.0);



		particles.update(dTime);


	};

	void draw(sf::RenderWindow& bkg) {
		if (!ViewManager::getInstance()->isPov())
		{
			board_.setRotation(angle_);
			bkg.draw(particles);
			bkg.draw(board_);
			bkg.draw(sprite_);
		}
	}

	float getSideMove() const {	return sw_; };
	float getFwdMove() const { return fwd_; };
	float getAngle() const { return angle_; };
	float getSpeed() const { return speed_; };
	void collide(bool c)
	{
		colliding_ = c;
		//speed_ = 10;
	}
};
class Background
{


	// create an empty shape
	sf::ConvexShape convex2;
	int w_;
	int h_;


public:
	Background(int w, int h)
	{
		w_ = w;
		h_ = h;
		convex2.setPointCount(4);
		//const float A = ((100 - prsp.a)/100.0 )* h_;
		const float A = h_;
		const float B1 = ((100 - prsp.b)/100.0 / 2) * w_;
		const float B2 = w_ - B1;
		const float C = (100 -prsp.c)/100.0 * h_;

		// define the points
		// Paint in screen ref coordinates
		convex2.setPoint(0, sf::Vector2f(0, C));
		convex2.setPoint(1, sf::Vector2f(w_, C));
		convex2.setPoint(2, sf::Vector2f(w_, h_));
		convex2.setPoint(3, sf::Vector2f(0, h_));

		//convex.setFillColor(sf::Color(100, 250, 50));
		convex2.setFillColor(sf::Color::White);
	}
	void draw(sf::RenderWindow& bkg)
	{
		if (ViewManager::getInstance()->isPov())
		{
			bkg.clear(sf::Color::Blue);
			bkg.draw(convex2);
		}
		else {
			bkg.clear(sf::Color::White);
		}
	}
};




int main()
{
	Board board;
	GraphicData graphicData;

	float jsx = 0.0; //TODO

	Screen screen;
	sf::RenderWindow window(screen.getVideoMode(), screen.getTitle(), screen.getStyle());

	window.setMouseCursorVisible(false);
	window.setVerticalSyncEnabled(true);

	CoordsTransform ct(screen.getWidth(), screen.getHeight(), 200);


	sf::Clock clock; // starts the clock
	sf::Time timeSinceLastUpdate = sf::Time::Zero;

	Background background(screen.getWidth(), screen.getHeight());
	Stars stars(screen.getWidth(), screen.getHeight());
	Race race(screen.getWidth(), screen.getHeight(), ct);
	Player player(ct);
	Text text;
	//Tree tree(screen.getWidth(), screen.getHeight(), ct);
	Piste piste(screen.getWidth(), screen.getHeight(), ct);

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
				}
			}
		}
		//window.clear(graphicData.getBkgCol());
		background.draw(window);

		// draw everything here...
		// window.draw(...);

		//Ball.draw(window);

		race.draw(window);
		player.draw(window);
		text.draw(window);
		board.draw(window);
		//tree.draw(window);
		piste.draw(window);
		if (!ViewManager::getInstance()->isPov()) { stars.draw(window); }

		// end the current frame
		window.display();

		// Check the clock and update if time is due
		//
		timeSinceLastUpdate += clock.restart();
		while (timeSinceLastUpdate > maxTimePerFrame)
		{
			sf::Time dTime = timeSinceLastUpdate;
			timeSinceLastUpdate = sf::Time().Zero;

			// Update stuff...

			float rsw = player.getSideMove();
			float rfwd = player.getFwdMove();

			player.tick(dTime, jsx);

			race.move(rfwd, rsw, dTime);
			Ball.move(rfwd, rsw);
			stars.move(rfwd, rsw);
			//sprintf(text.getBuf(), "%d", static_cast<int>(player.getAngle()));
			board.tick(dTime);
			//tree.move(rfwd, rsw);
			piste.move(rfwd, rsw);
			if (piste.isColliding())
			{
				//piste.clear();
				player.collide(true);
			}
			else {
				player.collide(false);
			}
		}
//		JoyS::reconnect();
		//jsx = joyS.get();
		jsx = board.get();
		int printjsx = (int)jsx;
		//sprintf(text.getBuf(), "%d %f", printjsx, jsx);
		//sprintf(text.getBuf(), " %4i   (% 3i)   %.2f", (int)player.getSpeed(), (int)player.getAngle(), board.getNorm());
		//std::cout << jsx << std::endl;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			//
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
			board.set(0.0);
#if 0
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
			{
				std::cout << "UP" << std::endl;
			} else {
				std::cout << "up" << std::endl;
			}
#endif
		}

		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
				ViewManager::getInstance()->setPov(true);
		}

		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
			ViewManager::getInstance()->setPov(false);
		}


		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::O)) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
			{
				board.setObservability(false);
			}
			else {
				board.setObservability(true);
			}
		}

		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
			board.set(0.0);
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
			{
				board.set(-0.99);
			}
			else {
				board.set(-0.7);
			}
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::RShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
			{
				board.set(0.99);
			}
			else {
				board.set(0.7);
			}
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
			board.startCalibration();
		}
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			race.start();
			piste.clear();
		}

	}
	return 0;
}
