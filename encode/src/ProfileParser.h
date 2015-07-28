#ifndef __PROFILE_PARSER_H__
#define __PROFILE_PARSER_H__

#include <set>
#include <map>
#include <string>


class ProfileParser {
public:
	enum Profile {
		PointerEncoding,
		/*GEPExpansion,*/
		CheckAfterDecode,
		PinChecks,
		DuplicateLoad,
		CheckAfterStore
	};
	typedef std::map<std::string, Profile> StringToProfileMap;
	static StringToProfileMap string2profile;

	typedef std::map<Profile, std::string> ProfileToStringMap;
	static ProfileToStringMap profile2string;


	enum Operation {
		Arithmetic,
		Bitwise,
		Comparison,
		GEP,
		Memory,
		Call
	};
	typedef std::map<std::string, Operation> StringToOperationMap;
	static StringToOperationMap string2operation;

	typedef std::map<Operation, std::string> OperationToStringMap;
	static OperationToStringMap operation2string;


	enum Position {
		Before,
		After
	};
	typedef std::map<std::string, Position> StringToPositionMap;
	static StringToPositionMap string2position;

	typedef std::map<Position, std::string> PositionToStringMap;
	static PositionToStringMap position2string;


	ProfileParser() {};

	void parseFile(std::ifstream &);

	bool hasProfile(Profile);
	bool hasOperation(Operation);
	bool hasOperationWithPosition(Operation, Position);

	void print();
	void printProfiles();
	void printOperations();
private:
	std::set<Profile> profiles;
	std::map<Operation, std::set<Position>> operations;

};

#endif /* __PROFILE_PARSER_H__ */
