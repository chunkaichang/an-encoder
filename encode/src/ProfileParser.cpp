
#include <fstream>
#include <iostream>

#include "ProfileParser.h"


ProfileParser::StringToProfileMap ProfileParser::string2profile = {
		{"PointerEncoding", PointerEncoding},
		/*{"GEPExpansion", GEPExpansion},*/
		{"CheckAfterDecode", CheckAfterDecode},
		{"PinChecks", PinChecks},
		{"DuplicateLoad", DuplicateLoad},
		{"CheckAfterStore", CheckAfterStore}
};

ProfileParser::ProfileToStringMap ProfileParser::profile2string = {
		{PointerEncoding, "PointerEncoding"},
		/*{GEPExpansion, "GEPExpansion"},*/
		{CheckAfterDecode, "CheckAfterDecode"},
		{PinChecks, "PinChecks"},
		{DuplicateLoad, "DuplicateLoad"},
		{CheckAfterStore, "CheckAfterStore"}
};

ProfileParser::StringToOperationMap ProfileParser::string2operation = {
		{"Arithmetic", Arithmetic},
		{"Bitwise", Bitwise},
		{"Comparison", Comparison},
		{"GEP", GEP},
		{"Memory", Memory}
};

ProfileParser::OperationToStringMap ProfileParser::operation2string = {
		{Arithmetic, "Arithmetic"},
		{Bitwise, "Bitwise"},
		{Comparison, "Comparison"},
		{GEP, "GEP"},
		{Memory, "Memory"}
};

ProfileParser::StringToPositionMap ProfileParser::string2position = {
		{"Before", Before},
		{"After", After}
};

ProfileParser::PositionToStringMap ProfileParser::position2string = {
		{Before, "Before"},
		{After, "After"}
};

void ProfileParser::parseFile(std::ifstream &ifs) {
	std::string s;

	while ((ifs >> s) && !ifs.eof()) {
		// Check if 's' is a profile:
		{
			auto it = string2profile.find(s);
			if (it != string2profile.end()) {
				this->profiles.insert(it->second);
				continue;
			}
		}
		// Check if 's' is an operation:
		{
			auto it = string2operation.find(s);
			if (it != string2operation.end()) {
				auto op = it->second;

				std::string line;
				while ((ifs >> line) && line.compare("end") && !ifs.eof()) {
					auto it = string2position.find(line);
					if (it == string2position.end())
						// error:
						break;

					operations[op].insert(it->second);
				}
				continue;
			}
		}
		// Any other value of 's' is an error:
		std::cerr << "Token '" << s << "' not recognized." << std::endl;
	}
	return;
}

bool ProfileParser::hasProfile(Profile p) {
	return profiles.find(p) != profiles.end();
}

bool ProfileParser::hasOperation(Operation op) {
	return operations.find(op) != operations.end();
}

bool ProfileParser::hasOperationWithPosition(Operation op, Position pos) {
	return hasOperation(op) &&
			operations[op].find(pos) != operations[op].end();
}

void ProfileParser::print() {
	printProfiles();
	printOperations();
}

void ProfileParser::printProfiles() {
	for (auto it = profiles.begin(); it != profiles.end(); it++) {
		std::cout << profile2string[*it] << std::endl;
	}

}

void ProfileParser::printOperations() {
	for (auto it = operations.begin(); it != operations.end(); it++) {
		std::cout << operation2string[it->first] << ": ";

		for (auto iit = operations[it->first].begin(); iit != operations[it->first].end(); iit++) {
			std::cout << position2string[*iit] << ", ";
		}
		std::cout << std::endl;
	}
}
/*
int main() {
	std::ifstream ifs("test.ep");
	ProfileParser pp;

	pp.parseFile(ifs);
	pp.print();
	std::cout << pp.hasOperationWithPosition(ProfileParser::Arithmetic, ProfileParser::Before) << std::endl;
	std::cout << pp.hasOperationWithPosition(ProfileParser::Arithmetic, ProfileParser::After) << std::endl;
	std::cout << pp.hasOperationWithPosition(ProfileParser::Memory, ProfileParser::After) << std::endl;

}*/
