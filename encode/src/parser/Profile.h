#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <set>
#include <map>
#include <string>


class EncodingProfile {
public:
	enum Profile {
		PointerEncoding,
		GEPExpansion,
		CheckBeforeDecode,
		AccumulateBeforeDecode,
		CheckAfterDecode,
		PinChecks,
		AccumulateChecks,
		IgnoreAccumulateOverflow,
		DuplicateLoad,
		CheckAfterStore
	};
	typedef std::map<Profile, std::string> ProfileToStringMap;
	static ProfileToStringMap profile2string;


	enum Operation {
		Arithmetic,
		Bitwise,
		Comparison,
		GEP,
		Load,
		Store,
		Memory,
		Call
	};
	typedef std::map<Operation, std::string> OperationToStringMap;
	static OperationToStringMap operation2string;


	enum Position {
		Before,
		After
	};
	typedef std::map<Position, std::string> PositionToStringMap;
	static PositionToStringMap position2string;


	EncodingProfile() {};

	bool hasProfile(Profile);
	bool hasOperation(Operation);
	bool hasOperationWithPosition(Operation, Position);
	bool checksDecode();

	void addProfile(Profile p);
	void addOperationWithPosition(Operation, Position);

	void print();
	void printProfiles();
	void printOperations();
private:
	std::set<Profile> profiles;
	std::map<Operation, std::set<Position>> operations;

};

#endif /* __PROFILE_H__ */
