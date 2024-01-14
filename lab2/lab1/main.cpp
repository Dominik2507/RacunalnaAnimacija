#include <stdio.h>
#include <iostream>
#include <GL/glut.h>
#include <soil.h>
//#include <SOIL2/SOIL2.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept> 
#include <cmath>
#include <random>

GLuint window;
GLuint width = 400, height = 400;

typedef struct _Ociste {
	GLdouble	x;
	GLdouble	y;
	GLdouble	z;
} Ociste;

struct Point {
	float x;
	float y;
	float z;

	Point operator*(float scalar) const {
		return { x * scalar, y * scalar, z * scalar };
	}

	Point operator-(Point a) const {
		return { x - a.x, y - a.y, z - a.z };
	}

	Point operator+(Point a) const {
		return { x + a.x, y + a.y, z + a.z };
	}
};

Point operator*(float scalar, const Point& point) {
	return point * scalar;
}

Point operator-(Point a, Ociste b) {
	return Point(a.x - b.x, a.y - b.y, a.z - b.z);
}
struct Particle {
	Point position;
	Point normal;
	Point velocity;
	float age;
	float lifespan;
};

Ociste	ociste = { 0.0, 0.0, 10.0 };
Point center = { 0 ,0 ,0 };
Point worldUp = { 0.0f, 1.0f, 0.0f };

GLuint textureID;
std::vector<Particle> particles;

float timer = 0.0;

Point spawnPosition = center;
float simSpeed = 0.01;
float lifespanMultiplyer = 5.0f;
float velocityMultiplyer = 2.0f;
const int maxParticles = 70;
bool useBillboard = true;

//*********************************************************************************
//	Function Prototypes.
//*********************************************************************************

void myDisplay();
void myReshape(int width, int height);
void myMouse(int button, int state, int x, int y);
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);
void redisplay_all(void);

void InitializeParticleSystem(void);
void UpdateParticles(void);
void DrawParticles(void);
void LoadTexture(void);
void TurnToOciste(Particle* particle);

Point crossProduct(Point v1, Point v2);
float dot(Point a, Point b);
Point normalize(float x, float y, float z);
Point normalize(Point p);
float norm(Point p);
float norm(Ociste p);
//*************************
//**************************

void redisplay_all(void)
{
	glutSetWindow(window);
	myReshape(width, height);
	glutPostRedisplay();
}

//*********************************************************************************
//	Glavni program.
//*********************************************************************************

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	// postavljanje dvostrukog spremnika za prikaz (zbog titranja)
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);

	window = glutCreateWindow("Sustav cestica");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);
	glutMouseFunc(myMouse);
	glutKeyboardFunc(myKeyboard);

	printf("esc: izlaz iz programa\n");
	InitializeParticleSystem();

	redisplay_all();
	glutMainLoop();
	return 0;
}

//*********************************************************************************
//	Osvjezavanje prikaza. 
//*********************************************************************************

void myDisplay(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DrawParticles();

	glutSwapBuffers();
	redisplay_all();
}

//*********************************************************************************
//	Promjena velicine prozora.
//	Funkcija gluPerspective i gluLookAt se obavlja 
//		transformacija pogleda i projekcija
//*********************************************************************************

void myReshape(int w, int h)
{
	width = w; height = h;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);        // aktivirana matrica projekcije
	glLoadIdentity();
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);		         // boja pozadine - bijela
	glClear(GL_COLOR_BUFFER_BIT);				     // brisanje zaslona
	gluPerspective(45.0, (float)width / height, 0.1, 100.0); // kut pogleda, x/y, prednja i straznja ravnina odsjecanja

	gluLookAt(ociste.x, ociste.y, ociste.z, center.x, center.y, center.z, 0.0, 1.0, 0.0);

	glMatrixMode(GL_MODELVIEW);    
	glLoadIdentity();
	glColor3ub(0, 0, 255);
}


#pragma region SustavCestica

void InitializeParticleSystem() {
	LoadTexture();
	particles.clear();
	for (int i = 0; i < maxParticles; ++i) {
		Particle particle;

		particle.position.x = center.x;  
		particle.position.y = center.y;
		particle.position.z = center.z;

		particle.normal = Point(0, 0, 1);

		particle.velocity.x = ((float)(rand() % 200) / 100.0 - 1) * velocityMultiplyer;
		particle.velocity.y = ((float)(rand() % 200) / 100.0 - 1) * velocityMultiplyer;
		particle.velocity.z = ((float)(rand() % 200) / 100.0 - 1) * velocityMultiplyer;

		particle.age = 0.0;
		particle.lifespan = lifespanMultiplyer * ((float)(rand() % 100) / 100.0); 
		particles.push_back(particle);
	}
}

void SortParticles() {
	for (int i = 0; i < maxParticles-1; i++) {
		float max = -1.0f;
		int index = 0;
		for (int j = i; j < maxParticles; j++) {
			if (norm(particles[j].position - ociste) > max) {
				max = norm(particles[j].position - ociste);
				index = j;
			}
		}
		if (max >= 0) {
			Particle temp = particles[i];
			particles[i] = particles[index];
			particles[index] = temp;
		}
	}
}

void UpdateParticles() {
	for (auto& particle : particles) {
		particle.age += simSpeed;

		particle.position.x += particle.velocity.x * simSpeed;
		particle.position.y += particle.velocity.y * simSpeed;
		particle.position.z += particle.velocity.z * simSpeed;



		if (particle.age > particle.lifespan) {

			particle.position.x = spawnPosition.x;
			particle.position.y = spawnPosition.y;
			particle.position.z = spawnPosition.z;

			particle.normal = Point(0, 0, 1);

			particle.velocity.x = ((float)(rand() % 200) / 100.0 - 1) * velocityMultiplyer;
			particle.velocity.y = ((float)(rand() % 200) / 100.0 - 1) * velocityMultiplyer;
			particle.velocity.z = ((float)(rand() % 200) / 100.0 - 1) * velocityMultiplyer;
			particle.lifespan = lifespanMultiplyer * ((float)(rand() % 100) / 100.0);
			particle.age = 0.0;
		}

		TurnToOciste(&particle);
	}

	SortParticles();
}

void TurnToOciste(Particle* particle) {
	Point position = particle->position;

	Point newNormal = normalize(ociste.x - position.x, ociste.y - position.y, ociste.z - position.z);

	particle->normal = newNormal;
}


void DrawParticles() {
	
	UpdateParticles();
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glBegin(GL_QUADS);
	

	for (const auto& particle : particles) {
		glColor3f(1.0, 1.0, 1.0);

		float size = 0.5f * (particle.lifespan - particle.age) / particle.lifespan;
		if (useBillboard) {
			Point look = normalize(center - ociste);
			Point forward = particle.normal;
			Point right = normalize(crossProduct(look, worldUp));
			Point up =  -1 * normalize(crossProduct(right, particle.normal));

			Point bottomLeft = particle.position - right * size - size * up;
			Point bottomRight = particle.position + size * right - size * up;
			Point topRight = particle.position + size * right + size * up;
			Point topLeft = particle.position - size * right + size * up;

			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(bottomLeft.x, bottomLeft.y, bottomLeft.z);

			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(bottomRight.x, bottomRight.y, bottomRight.z);

			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(topRight.x, topRight.y, topRight.z);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(topLeft.x, topLeft.y, topLeft.z);
		}
		else {
			glTexCoord2f(0.0f, 0.0f);
			glVertex3f(particle.position.x - size, particle.position.y - size, particle.position.z);

			glTexCoord2f(1.0f, 0.0f);
			glVertex3f(particle.position.x + size, particle.position.y - size, particle.position.z);

			glTexCoord2f(1.0f, 1.0f);
			glVertex3f(particle.position.x + size, particle.position.y + size, particle.position.z);

			glTexCoord2f(0.0f, 1.0f);
			glVertex3f(particle.position.x - size, particle.position.y + size, particle.position.z);
		}
	}

	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void LoadTexture() {
	
	textureID = SOIL_load_OGL_texture
	(
		"./flame.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
	);

	if (0 == textureID)
	{
		printf("SOIL loading error: '%s'\n", SOIL_last_result());
	}
}


#pragma endregion

#pragma region Controls

void myActiveMouseMotion(int x, int y) {
	float widthAtCenter = norm(ociste) * tanf((45.0f / 2.0f) *(3.1415 / 180));
	spawnPosition.x = 2 * ((float)x / width - 0.5) * widthAtCenter;
	spawnPosition.y = -2 * ((float)y / height - 0.5) * widthAtCenter;
	spawnPosition.z = 0;
	std::cout << x << ", " << y << ":" << spawnPosition.x << ", " << spawnPosition.y << std::endl;;
	redisplay_all();
}

void myMouse(int button, int state, int x, int y)
{
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		redisplay_all();
	}
	else if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			ociste = Ociste(0, 0, 10);
			myActiveMouseMotion(x, y);
			glutMotionFunc(myActiveMouseMotion);
		}
		else {
			spawnPosition = center;
			glutMotionFunc(nullptr);
		}
	}
	else {
		spawnPosition = center;
	}
}


void myKeyboard(unsigned char theKey, int mouseX, int mouseY) {
	switch (theKey) {
	case 'w':
		ociste = Ociste(0, 0, 10);
		break;
	case 'a':
		ociste = Ociste(10, 0, 0);
		break;
	case 's':
		ociste = Ociste(0, 0, -10);
		break;
	case 'd':
		ociste = Ociste(-10, 0, 0);
		break;
	case 'q':
		ociste = Ociste(10, 10, 0);
		break;
	case 'e':
		ociste = Ociste(0, 10, 10);
		break;
	case 'b':
		useBillboard = !useBillboard;
		break;
	case 27:
		exit(0);
		break;
	}

	redisplay_all();
}
#pragma endregion

#pragma region Utils
Point normalize(float x, float y, float z) {
	float length = std::sqrt(x * x + y * y + z * z);
	if (length == 0.0f) {
		return { 0.0f, 0.0f, 0.0f };
	}

	float invLength = 1.0f / length;
	return { x * invLength, y * invLength, z * invLength };
}

Point normalize(Point p) {
	float x = p.x;
	float y = p.y;
	float z = p.z;

	float length = std::sqrt(x * x + y * y + z * z);
	if (length == 0.0f) {
		return { 0.0f, 0.0f, 0.0f };
	}

	float invLength = 1.0f / length;
	return { x * invLength, y * invLength, z * invLength };
}

float norm(Point p) {
	return std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

float norm(Ociste p) {
	return std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
}

Point crossProduct(Point v1, Point v2) {
	Point result;

	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;

	return result;
}

float dot(Point a, Point b) {
	float result = a.x * b.x + a.y * b.y + a.z * b.z;
	return result;
}
#pragma endregion
