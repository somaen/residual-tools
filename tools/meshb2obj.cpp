#include <fstream>
#include <string>
#include <iostream>

using namespace std;

int main(int argc, char **argv) {
	if(argc<2){
		std::cout << "Error: filename not specified" << std::endl;
		return 0;
	}
	std::string filename=argv[1];
	
	std::fstream file(filename.c_str(), std::ios::in | std::ios::binary);
	
	if (!file.is_open()){
		std::cout << "Unable to open file " << filename << std::endl;
		return 0;
	}
	int strLength = 0;
	
	file.read((char*)&strLength,4);
	
	char* readString = new char[64];
	file.read(readString,strLength);
	
	// Then a list of textures 48 bytes later
	int numTextures = 0;
	file.seekg(48,ios::cur);
	file.read((char*)&numTextures,4);
	
	char **texNames = new char*[numTextures];
	for(int i=0;i<numTextures;i++){
		file.read((char*)&strLength,4);
		texNames[i] = new char[strLength];
		file.read(texNames[i],strLength);
		// Every texname seems to be followed by 4 0-bytes
		file.seekg(4,ios::cur);
	}
	for(int i=0;i<numTextures;i++){
		std::cout << "# TexName " << texNames[i] << std::endl;
	}
	// 4 unknown bytes - usually with value 19
	file.seekg(4,ios::cur);

	// Should create an empty mtl
	std::cout << "mtllib quit.mtl"<<std::endl<<"o Arrow"<<std::endl;
	// Vertices
	int numVertices;
	file.read((char*)&numVertices,4);
	std::cout << "#File has " << numVertices << " Vertices" << std::endl;
	
	float x = 0, y = 0, z = 0;
	
	for (int i = 0; i < numVertices; ++i) {
		file.read((char *)&x, 4);
		file.read((char *)&y, 4);
		file.read((char *)&z, 4);
		std::cout << "v " << x << " " << y << " " << z << std::endl;
	}
	
	for (int i = 0; i < numVertices; ++i) {
		file.read((char *)&x, 4);
		file.read((char *)&y, 4);
		file.read((char *)&z, 4);
		std::cout << "vn " << x << " " << y << " " << z << std::endl;
	}
	file.seekg(numVertices * 12,ios::cur);
	// Actually, this file has 6*4*numVertices floats in this block.
	std::cout<<"usemtl (null)"<<std::endl;
	// And then another block of unknowns
	// Faces
	// The head of this section needs quite a bit of rechecking
	int numFaces = 0;
	file.read((char *) &numFaces, 4);
	std::cout << "# NumFaces: " << numFaces << std::endl;

	int faceLength = 0;
	
	int unknown;
	int numBones = 0;
	
	int totaltFaceOffset = 0;
	
	file.read((char *)&unknown, 4);
	file.read((char *)&strLength, 4);
	std::cout << "# Unknown: " << unknown << std::endl;
	std::cout << "# meshNum: " << strLength << std::endl;
	
	for(int j=0;j<numFaces;j++){
		file.read((char*)&faceLength,4);
		std::cout << "# FaceLength: " << faceLength << std::endl;
		short x = 0, y = 0, z = 0;
		cout << "g " << j << endl;
		for (int i = 0; i < faceLength; i+=3) {
			file.read((char *)&x, 2);
			file.read((char *)&y, 2);
			file.read((char *)&z, 2);
			++x;
			++y;
			++z;
			std::cout << "f " << x << "//" << x << " " << y << "//" << y << " " << z << "//" << z << std::endl;
			totaltFaceOffset += 6;
		}
		// 12 bytes of unknown
		file.read((char *)&unknown, 4);
		file.read((char *)&numBones, 4);
		file.read((char *)&strLength, 4);
		std::cout << "# Unknown: " << unknown << std::endl;
		std::cout << "# numBones: " << numBones << std::endl;
		std::cout << "# meshNum: " << strLength << std::endl;
		//file.seekg(12,ios::cur);
	}
	// Although, the last run ends with the lead-in for the bones.

	std::cout << "# Bytes read from Faces: " << totaltFaceOffset << std::endl;
	std::cout << "# numBones: " << numBones << std::endl;
	
	char **boneNames = new char*[numBones];
	for(int i=0;i<numBones;i++) 
	{
		if(i)
			file.read((char*)&strLength, 4);
		boneNames[i] = new char[strLength];
		file.read(boneNames[i],strLength);
		//file.seekg(4,ios::cur);
	}
	for(int i=0;i<numBones;i++){
		std::cout << "# BoneName " << boneNames[i] << std::endl;
	}
	// Then an int, that is exactly 12 times the size of the rest of the file.
	int numVals = 0;
	file.read((char *) &numVals, 4);
	std::cout << "# NumFloats: " << numVals << std::endl;
	for (int i = 0; i < numVals; i++) {
		int x = 0, y = 0, z = 0;
		file.read((char *)&x, 4);
		file.read((char *)&y, 4);
		file.read((char *)&z, 4);
		std::cout << "#? " << x << " " << y << " " << z << std::endl;
	}
}