
#include <iostream>

#include "Profile.h"


EncodingProfile::ProfileToStringMap EncodingProfile::profile2string = {
		{PointerEncoding, "PointerEncoding"},
		{GEPExpansion, "GEPExpansion"},
		{CheckBeforeDecode, "CheckBeforeDecode"},
		{AccumulateBeforeDecode, "AccumulateBeforeDecode"},
		{CheckAfterDecode, "CheckAfterDecode"},
		{PinChecks, "PinChecks"},
		{AccumulateChecks, "AccumulateChecks"},
		{IgnoreAccumulateOverflow, "IgnoreAccumulateOverflow"},
		{DuplicateLoad, "DuplicateLoad"},
		{CheckAfterStore, "CheckAfterStore"}
};

EncodingProfile::OperationToStringMap EncodingProfile::operation2string = {
		{Arithmetic, "Arithmetic"},
		{Bitwise, "Bitwise"},
		{Comparison, "Comparison"},
		{GEP, "GEP"},
		{Load, "Load"},
		{Store, "Store"},
		{Memory, "Memory"}
};

EncodingProfile::PositionToStringMap EncodingProfile::position2string = {
		{Before, "Before"},
		{After, "After"}
};

bool EncodingProfile::hasProfile(Profile p) {
	return profiles.find(p) != profiles.end();
}

void EncodingProfile::addProfile(Profile p) {
  if (!hasProfile(p)) {
    profiles.insert(p);
  }
}

bool EncodingProfile::hasOperation(Operation op) {
	return operations.find(op) != operations.end();
}

bool EncodingProfile::hasOperationWithPosition(Operation op, Position pos) {
	return hasOperation(op) &&
			operations[op].find(pos) != operations[op].end();
}

void EncodingProfile::addOperationWithPosition(Operation op, Position pos) {
  if (!hasOperationWithPosition(op, pos)) {
    operations[op].insert(pos);
  }
}

bool EncodingProfile::checksDecode() {
  return profiles.count(EncodingProfile::CheckBeforeDecode) ||
      profiles.count(EncodingProfile::AccumulateBeforeDecode) ||
      profiles.count(EncodingProfile::CheckAfterDecode);
}

void EncodingProfile::print() {
	printProfiles();
	printOperations();
}

void EncodingProfile::printProfiles() {
	for (auto it = profiles.begin(); it != profiles.end(); it++) {
		std::cout << profile2string[*it] << std::endl;
	}

}

void EncodingProfile::printOperations() {
	for (auto it = operations.begin(); it != operations.end(); it++) {
		std::cout << operation2string[it->first] << ": ";

		for (auto iit = operations[it->first].begin(); iit != operations[it->first].end(); iit++) {
			std::cout << position2string[*iit] << " ";
		}
		std::cout << std::endl;
	}
}
