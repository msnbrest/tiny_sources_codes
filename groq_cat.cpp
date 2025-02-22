#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sys/stat.h>
#include <chrono>
#include <ctime>
#include <cctype>

namespace fs = std::filesystem;

// créé par msnbrest le 20250222, aidé plusieurs fois par Groq : https://console.groq.com/playground



// Fonction pour parcourir les fichiers et créer le fichier de cache
void create_cache(const char* outputFilename){
	std::string ignored= "";
	std::ofstream cacheFile(outputFilename);

	if( cacheFile.is_open() ){
		// Parcourir les fichiers dans le dossier actuel et ses sous-dossiers
		for( const auto& entry : fs::recursive_directory_iterator(".") ){
			if (entry.is_regular_file()) {
				std::string filename = entry.path().string();
				std::string extension = entry.path().extension().string();

				// Vérifier si le fichier est un .exe, .svg ou .pdf
				if(
					extension == ".php" ||
					extension == ".html" ||
					extension == ".js" ||
					extension == ".css"
				){
					// Ouvrir le fichier et lire son contenu
					std::ifstream file(filename);
					if (file.is_open()) {
						std::string content((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));
						std::string lower_str= "";
						std::for_each(content.begin(), content.end(), [&](char c) { lower_str += std::tolower(c); });
						cacheFile << "=== " << 	filename << std::endl << lower_str << std::endl << "\n\n\n" << std::endl;
						file.close();
					} else {
						std::cerr << "Erreur lors de l'ouverture du fichier " << filename << std::endl;
					}
				} else {
					ignored += "\nignored: "+filename+"\n";
				}
			}
		}

		cacheFile << ignored;
		cacheFile.close();
		std::cout << "Cache OK" << std::endl;
	} else {
		std::cerr << "Erreur creation cache !" << std::endl;
	}
}



bool need_new(const char* outputFilename){

	struct stat attrib;

	if( stat(outputFilename, &attrib) != 0 ){   return true;   } // exist pas

	auto limit = std::chrono::seconds(9 * 24 * 60 * 60); // 9 jours
	auto noww = std::chrono::system_clock::now(); // now
	auto edit = std::chrono::system_clock::from_time_t(attrib.st_mtime); // last edit
	auto delay= std::chrono::duration_cast<std::chrono::seconds>( noww - edit );

	if( delay > limit ){   return true;   } // ancien

	return false;
}



std::string pardon(){

	std::string phrase;

	std::cout << "Que chercher :" << std::endl;
	std::getline(std::cin, phrase);
	return phrase;
}



std::string trim_tab_spa(const std::string& str){ // retire les tab et espaces en début de chaine
    size_t pos = 0;
    while( pos < str.size() && ( str.at(pos)==' ' || str.at(pos)=='	' ) ){
        ++pos;
    }
    return str.substr(pos);
}



void chercher( const char* path, std::string voulu ){

	if( voulu=="" ){
		std::cout << "(vide) Bye." << std::endl;
		return;
	}

    std::ifstream fichier(path);

    if( fichier.is_open() ){

        std::string a_line;
		std::string simple_part1; // memoriser ligne actuelle pour si besoin prochain tour
		bool need_part3= false; // besoin de voir la ligne suivante
		uint32_t deja1= 4294967295; // memo line desinnee
		uint32_t deja2= 4294967295; // memo line desinnee
		uint32_t deja3= 4294967295; // memo line desinnee
		std::string filename;
		// 30 à 37 = noir,rouge,vert,jaune,bleu,mauve,cyan,blanc.   1;30 à 1;37 is bold.   0;40 à 0;47 is background   1;40 à 1;47 bold background
		std::string red= "\033[41m"; // text red
		std::string ooo= "\033[40m"; // text red
		std::string cya= "\033[36m"; // text cyan
		std::string wit= "\033[0m"; // text white
		uint32_t ind= 1;
		uint16_t ss= voulu.size(); // search size
		std::string cc = red + voulu + wit; // content colored

        while( std::getline(fichier, a_line) ){ // parfois dessiner ancienne car partie3.  ensuite si, dessiner avec couleurs.

			if( a_line.compare(0, 4, "=== ") == 0 ){ // -1 voudrait dire faux.   ligne inutile car titre
				filename= ooo + a_line + " L " + wit + "\n\n";
				ind= 1;
				deja1= 4294967295;
				deja2= 4294967295;
				deja3= 4294967295;
				continue;
			}
			std::string actualle= trim_tab_spa(a_line);
			if( need_part3 ){ // dessiner si besoin precedente ligne
				if( ind!=deja1 && ind!=deja2 && ind!=deja3 ){
					if( actualle.find( voulu ) != std::string::npos ){
						actualle= actualle.replace(actualle.find( voulu ), ss, cc);
					}
					std::cout << filename << cya << ind << ":\n" << wit << actualle << "\n" << std::endl;
					filename= "";
					deja1= ind;
				}
				need_part3= false;
			}

			if( actualle.find(voulu) != std::string::npos ){ // afficher chaque ligne contenant voulu
				if( (ind-1)!=deja1 && (ind-1)!=deja2 && (ind-1)!=deja3 ){
					if( simple_part1.find( voulu ) != std::string::npos ){
						simple_part1= simple_part1.replace(simple_part1.find( voulu ), ss, cc);
					}
					std::cout << filename << cya << (ind-1) << ":\n" << wit << simple_part1 << std::endl;
					filename= "";
					deja3= (ind-1);
				}
				if( ind!=deja1 && ind!=deja2 && ind!=deja3 ){
					std::cout << filename << cya << ind << ":\n" << wit << actualle.replace(actualle.find( voulu ), ss, cc) << std::endl;
					filename= "";
					deja2= ind;
				}
				// --- n-1
				need_part3= true;
            }
			simple_part1= actualle;

            ind++;
        }
        fichier.close();
    }else{
        std::cout << "Cache indisponible" << std::endl;
    }
}



int main() {

	const char* path = "perime_2024.cache";

	if( need_new(path) ){
		std::cout << "Recreation cache..." << std::endl;
		create_cache("perime_2024.cache");
	}

	chercher( path, pardon() );

	return 0;
}
