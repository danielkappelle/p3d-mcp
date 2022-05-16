#pragma once
#include "PMDG_777X_SDK.h"
#include "events.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>

class Rotary {
public:
	enum EVENT_ID event_id;
	unsigned int addr;
	std::string cmd_str;

	Rotary(int addr, enum EVENT_ID event_id);

	void static update_all(std::string cmd);

private:
	bool update(std::string cmd);
	static std::vector<Rotary*> instances;
};