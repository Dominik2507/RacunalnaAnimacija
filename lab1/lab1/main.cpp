//*************************************************************************************************************
//	Crtanje tijela
//	Tijelo se crta naredbom glutWireCube (velicina)
//	
//	Zadatak: Treba ucitati tijelo zapisano u *.obj, sjencati i ukloniti staznje poligone
//	S tastature l - pomicanje ocista po x osi +
//				k - pomicanje ocista po x osi +
//              r - pocetni polozaj
//              esc izlaz iz programa
//*************************************************************************************************************

#include <stdio.h>
#include <iostream>
#include <GL/glut.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept> 
#include <cmath>

Assimp::Importer importer;
//const aiScene* scene = importer.ReadFile("tetrahedron.obj", aiProcess_Triangulate);
const aiScene* scene = importer.ReadFile("aircraft747.obj", aiProcess_Triangulate);

//*********************************************************************************
//	Pokazivac na glavni prozor i pocetna velicina.
//*********************************************************************************

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
};

Ociste	ociste = { 0.0720305, 6.24087, -5.10637 };
Point center = { 0 ,0 ,0 };

std::vector<Point> points;

float timer = 0.0;
float speed = 1;

std::vector<Point> Bspline;
std::vector<Point> tangents;
std::vector<Point> normals;
std::vector<Point> binormals;


//*********************************************************************************
//	Function Prototypes.
//*********************************************************************************

void myDisplay();
void myReshape(int width, int height);
void myMouse(int button, int state, int x, int y);
void myKeyboard(unsigned char theKey, int mouseX, int mouseY);
void myObject();
void redisplay_all(void);
void loadPointsFromFile(const std::string& filename);
void drawBSpline();
void calcBSpline();
std::vector<std::vector<float>> dot(std::vector<std::vector<float>> a, std::vector<std::vector<float>> b);
std::vector<float> crossProduct(const std::vector<float>& v1, const std::vector<float>& v2);
Point crossProduct(Point v1, Point v2);
std::vector<std::vector<float>> inverseMatrix3x3(const std::vector<std::vector<float>>& M);
float dot(Point a, Point b);
Point normalize(float x, float y, float z);
Point normalize(Point p);
float norm(Point p);
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
	//loadPointsFromFile("example1.txt");
	loadPointsFromFile("spiral.txt");
	
	calcBSpline();

	window = glutCreateWindow("Tijelo");
	glutReshapeFunc(myReshape);
	glutDisplayFunc(myDisplay);
	glutMouseFunc(myMouse);
	glutKeyboardFunc(myKeyboard);
	printf("Tipka: l - pomicanje ocista po x os +\n");
	printf("Tipka: k - pomicanje ocista po x os -\n");
	printf("Tipka: r - pocetno stanje\n");
	printf("esc: izlaz iz programa\n");

	

	redisplay_all();
	glutMainLoop();
	return 0;
}

//*********************************************************************************
//	Osvjezavanje prikaza. 
//*********************************************************************************

void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawBSpline();
	myObject();
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

	glColor3ub(0, 0, 255);
	glMatrixMode(GL_MODELVIEW);        
}

//*********************************************************************************
//	Crta moj objekt. Ovdje treba naciniti prikaz ucitanog objekta.
//*********************************************************************************

GLfloat* getDCM(int index = 0) {
	
	//std::cout << norm(tangents[index]) << ", " << norm(normals[index]) << ", " << norm(binormals[index]) << std::endl;
	
	std::vector<std::vector<float>> R_local_to_global =
	{
		{tangents[index].x, normals[index].x, binormals[index].x},
		{tangents[index].y, normals[index].y, binormals[index].y},
		{tangents[index].z, normals[index].z, binormals[index].z},
	};

	std::vector<std::vector<float>> R_global_to_local = inverseMatrix3x3(R_local_to_global);

	GLfloat* rotationMatrix = new GLfloat[16];

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			rotationMatrix[i * 4 + j] = R_global_to_local[i][j];
		}
	}
	rotationMatrix[15] = 1.0f;

	return rotationMatrix;
}


void myObject()
{
	
	glColor3f(0.0, 0.0, 1.0);
	
	timer = timer + speed;
	int index = static_cast<int>(floor(timer)) % Bspline.size();
	glPushMatrix();
	glLoadIdentity();
	//glTranslatef(center.x, center.y, center.z);

	Point currentPoint = Bspline[index];

	GLfloat* DCM = getDCM(index);

	//KUTEVI ROTACIJA
	Point s = { 0.0, 0.0, 1.0 };
	Point e = tangents[index];
	Point os = crossProduct(s, e);
	os = normalize(os);
	float angle = acos(dot(s, e) / (norm(s) * norm(e)));
	float angleDegrees = angle * 180.0 / (3.1415);
	
	
	glTranslatef(currentPoint.x, currentPoint.y, currentPoint.z);


	glMultMatrixf(DCM);
	//glRotatef(angleDegrees, os.x, os.y, os.z);
	

	if (scene) {
		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[i];

			glBegin(GL_TRIANGLES);
			for (unsigned int j = 0; j < mesh->mNumFaces; j++) {
				aiFace face = mesh->mFaces[j];
				for (unsigned int k = 0; k < face.mNumIndices; k++) {
					unsigned int vertexIndex = face.mIndices[k];
					aiVector3D vertex = mesh->mVertices[vertexIndex];
					glVertex3f(vertex.x, vertex.y, vertex.z);
				}
			}
			glEnd();
		}
	}

	glPopMatrix();

	delete[] DCM;

}


#pragma region drawingFunctionComponents



void drawPoints() {
	glPointSize(5.0);
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_POINTS);
	for (const auto& point : points) {
		glVertex3f(point.x, point.y, point.z);
	}
	glEnd();
}

void drawPointConnections() {
	glColor3f(1.0, 0.0, 0.0);

	glBegin(GL_LINES);
	for (size_t i = 0; i < points.size() - 1; i++) {
		Point p1 = points[i];
		Point p2 = points[i + 1];
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p2.x, p2.y, p2.z);
	}
	glEnd();
}

void drawTangents(int step = 1, float width = 1.0){
	glColor3f(1.0, 0.0, 0.0);

	glBegin(GL_LINES);
	for (size_t i = 0; i < tangents.size(); i += step) {
		Point p1 = Bspline[i];
		Point p2 = tangents[i];

		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p1.x + width * p2.x,p1.y + width * p2.y,p1.z + width * p2.z);
	}
	glEnd();
}

void drawNormals(int step = 1, float width = 1.0) {
	glColor3f(0.0, 0.0, 1.0);

	glBegin(GL_LINES);
	for (size_t i = 0; i < normals.size(); i += step) {
		Point p1 = Bspline[i];
		Point p2 = normals[i];

		
		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p1.x + width * p2.x, p1.y + width * p2.y, p1.z + width * p2.z);
	}
	glEnd();
}

void drawBinormals(int step = 1, float width = 1.0) {
	glColor3f(0.0, 1.0, 1.0);

	glBegin(GL_LINES);
	for (size_t i = 0; i < binormals.size(); i += step) {
		Point p1 = Bspline[i];
		Point p2 = binormals[i];


		glVertex3f(p1.x, p1.y, p1.z);
		glVertex3f(p1.x + width * p2.x, p1.y + width * p2.y, p1.z + width * p2.z);
	}
	glEnd();
}

void drawBSpline() {

	//drawPoints();
	//drawPointConnections();
	
	drawTangents(tangents.size() / 20, 5.0);
	//drawNormals(normals.size() / 20, 5.0);
	//drawBinormals(binormals.size() / 20, 5.0);

	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINE_STRIP);

	for (const auto& point : Bspline) {
		glVertex3f(point.x, point.y, point.z);
	}
	
	glEnd();
	
}

#pragma endregion

void calcBSpline() {
	float center_x = 0;
	float center_y = 0;
	float center_z = 0;

	for (const auto& point : points) {
		center_x += point.x;
		center_y += point.y;
		center_z += point.z;
	}

	center.x = center_x / points.size();
	center.y = center_y / points.size();
	center.z = center_z / points.size();


	std::vector<std::vector<float>> B = {
		{-1.0, 3.0, -3.0, 1.0},
		{3.0, -6.0, 3.0, 0.0},
		{-3.0, 0.0, 3.0, 0.0},
		{1.0, 4.0, 1.0, 0.0}
	};

	std::vector<std::vector<float>> B_tangent = {
		{-1.0, 3.0, -3.0, 1.0},
		{2.0, -4.0, 2.0, 0.0},
		{-1.0, 0.0, 1.0, 0.0}
	};

	std::vector<std::vector<float>> B_normal = {
		{-1.0, 3.0, -3.0, 1.0},
		{1.0, -2.0, 1.0, 0.0}
	};

	for (size_t i = 0; i < points.size() - 3; i++) {
		Point p1 = points[i];
		Point p2 = points[i + 1];
		Point p3 = points[i + 2];
		Point p4 = points[i + 3];

		for (float t = 0.0; t <= 1.0; t += 0.01) {

			// BSPLINE
			std::vector<std::vector<float>> T = { { (t * t * t) / 6.0f, (t * t) / 6.0f, t / 6.0f, 1.0f / 6.0f } };
			std::vector<std::vector<float>> R = {
				{p1.x, p1.y, p1.z},
				{p2.x, p2.y, p2.z},
				{p3.x, p3.y, p3.z},
				{p4.x, p4.y, p4.z}
			};


			std::vector<std::vector<float>> p_t = dot(dot(T, B), R);
			float x = p_t[0][0];
			float y = p_t[0][1];
			float z = p_t[0][2];
			Point newPoint = { x,y,z };
			Bspline.push_back(newPoint);

			// TANGENTS
			std::vector<std::vector<float>> T_tangent = {
				{ (t * t) / 2.0f, t / 2.0f, 1.0f / 2.0f}
			};

			std::vector<std::vector<float>> p_tangent = dot(dot(T_tangent, B_tangent), R);
			float x_tangent = p_tangent[0][0];
			float y_tangent = p_tangent[0][1];
			float z_tangent = p_tangent[0][2];
			Point newPoint_tangent = normalize(x_tangent, y_tangent, z_tangent);
			tangents.push_back(newPoint_tangent);

			// NORMALS
			std::vector<std::vector<float>> T_normal = { { t, 1.0f} };

			std::vector<std::vector<float>> p_2ndDev = dot(dot(T_normal, B_normal), R);

			std::vector<float> p_normal = crossProduct(p_tangent[0], p_2ndDev[0]);
			float x_normal = p_normal[0];
			float y_normal = p_normal[1];
			float z_normal = p_normal[2];
			Point newPoint_normal = normalize(x_normal, y_normal, z_normal);
			normals.push_back(newPoint_normal);

			// BINORMALS
			std::vector<float> p_binormal = crossProduct(p_tangent[0], p_normal);
			float x_binormal = p_binormal[0];
			float y_binormal = p_binormal[1];
			float z_binormal = p_binormal[2];
			Point newPoint_binormal = normalize( x_binormal, y_binormal, z_binormal );
			binormals.push_back(newPoint_binormal);

			if (abs(dot(newPoint_tangent, newPoint_normal)) > 0.01 )  {
				std::cerr << "NISU OKOMITI normala i tangenta" << dot(newPoint_tangent, newPoint_normal) << std::endl;
			}
			if (abs(dot(newPoint_tangent, newPoint_binormal)) > 0.01) {
				std::cerr << "NISU OKOMITI binormala i tangenta" << dot(newPoint_tangent, newPoint_binormal) <<  std::endl;
			}
			if (abs(dot(newPoint_normal, newPoint_binormal)) > 0.01) {
				std::cerr << "NISU OKOMITI normala i binormala" << dot(newPoint_normal, newPoint_binormal) << std::endl;
			}
		}
	}
}

void loadPointsFromFile(const std::string& filename) {
	std::ifstream file(filename.c_str());

	if (!file.is_open()) {
		std::cerr << "Neuspješno otvaranje datoteke: " << filename << std::endl;
		return;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream ss(line);
		Point point;
		char comma;

		if (ss >> point.x >> comma >> point.y >> comma >> point.z) {
			points.push_back(point);
		}
		else {
			std::cerr << "Neuspješno čitanje točke: " << line << std::endl;
		}
	}

	file.close();
}

#pragma region Controls

//*********************************************************************************
//	Mis.
//*********************************************************************************

void myMouse(int button, int state, int x, int y)
{
	//	Desna tipka - brise canvas. 
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		ociste.x = 0;
		redisplay_all();
	}
}

//*********************************************************************************
//	Tastatura tipke - esc - izlazi iz programa.
//*********************************************************************************

float azimuth = 0.0;
float elevation = 0.0;

float angleIncrement = 0.05;
float radiusIncrement = 1.0;
float radius = sqrt((ociste.x - center.x) * (ociste.x - center.x) + (ociste.y - center.y) * (ociste.y - center.y) + (ociste.z - center.z) * (ociste.z - center.z));


void myKeyboard(unsigned char theKey, int mouseX, int mouseY) {
	switch (theKey) {
	case 'a':
		azimuth += angleIncrement;
		break;

	case 'd':
		azimuth -= angleIncrement;
		break;

	case 'w':
		elevation += angleIncrement;
		break;

	case 's':
		elevation -= angleIncrement;
		break;

	case '+':
		radius += radiusIncrement;
		break;

	case '-':
		radius -= radiusIncrement;
		break;

	case 'r':
		azimuth = 0.0;
		elevation = 0.0;
		radius = 15.0;
		break;

	case 27:
		exit(0);
		break;
	}

	ociste.x = center.x + radius * sin(elevation) * sin(azimuth);
	ociste.y = center.y + radius * cos(elevation);
	ociste.z = center.z + radius * sin(elevation) * cos(azimuth);

	std::cout << ociste.x << ", " << ociste.y << ", " << ociste.z << std::endl;
	redisplay_all();
}

#pragma endregion

#pragma region HelperFunctions


std::vector<std::vector<float>> dot(std::vector<std::vector<float>> a, std::vector<std::vector<float>> b) {
	if (a[0].size() != b.size()) {
		std::cerr << "Krive dimenzije matrica: " << a.size() << "x" << a[0].size() << ", " << b.size() << "x" << b[0].size() << std::endl;
		throw std::runtime_error("Error");
	}

	int rowsA = a.size();
	int colsA = a[0].size();
	int colsB = b[0].size();

	std::vector<std::vector<float>> result(rowsA, std::vector<float>(colsB, 0.0));

	for (int i = 0; i < rowsA; i++) {
		for (int j = 0; j < colsB; j++) {
			for (int k = 0; k < colsA; k++) {
				result[i][j] += a[i][k] * b[k][j];
			}
		}
	}

	return result;
}

float dot(Point a, Point b) {
	float result = a.x * b.x + a.y * b.y + a.z * b.z;
	return result;
}

std::vector<float> crossProduct(const std::vector<float>& v1, const std::vector<float>& v2) {
	if (v1.size() != 3 || v2.size() != 3) {
		std::cerr << "Vektori nisu dobrih dimenzija." << v1.size() << ", "<< v2.size() << std::endl;
		throw std::runtime_error("Error");
	}

	std::vector<float> result(3);

	result[0] = v1[1] * v2[2] - v1[2] * v2[1];
	result[1] = v1[2] * v2[0] - v1[0] * v2[2]; 
	result[2] = v1[0] * v2[1] - v1[1] * v2[0];

	return result;
}

Point crossProduct(Point v1, Point v2) {
	Point result;

	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;

	return result;
}

float determinant2x2(float a, float b, float c, float d) {
	return a * d - b * c;
}

std::vector<std::vector<float>> inverseMatrix3x3(const std::vector<std::vector<float>>& M) {
	if (M.size() != 3 || M[0].size() != 3) {
		std::cerr << "Matrica mora biti 3x3." << std::endl;
		throw std::runtime_error("Error");
	}

	std::vector<std::vector<float>> result(3, std::vector<float>(3));

	float det = 
		  M[0][0] * determinant2x2(M[1][1], M[1][2], M[2][1], M[2][2])
		- M[0][1] * determinant2x2(M[1][0], M[1][2], M[2][0], M[2][2])
		+ M[0][2] * determinant2x2(M[1][0], M[1][1], M[2][0], M[2][1]);

	if (det == 0.0) {
		std::cerr << "Determinanta matrice je 0, inverz ne postoji." << std::endl;
		throw std::runtime_error("Error");
	}

	result[0][0] =  determinant2x2(M[1][1], M[1][2], M[2][1], M[2][2]) / det;
	result[0][1] = -determinant2x2(M[0][1], M[0][2], M[2][1], M[2][2]) / det;
	result[0][2] =  determinant2x2(M[0][1], M[0][2], M[1][1], M[1][2]) / det;

	result[1][0] = -determinant2x2(M[1][0], M[1][2], M[2][0], M[2][2]) / det;
	result[1][1] =  determinant2x2(M[0][0], M[0][2], M[2][0], M[2][2]) / det;
	result[1][2] = -determinant2x2(M[0][0], M[0][2], M[1][0], M[1][2]) / det;

	result[2][0] =  determinant2x2(M[1][0], M[1][1], M[2][0], M[2][1]) / det;
	result[2][1] = -determinant2x2(M[0][0], M[0][1], M[2][0], M[2][1]) / det;
	result[2][2] =  determinant2x2(M[0][0], M[0][1], M[1][0], M[1][1]) / det;

	return result;
}

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


#pragma endregion