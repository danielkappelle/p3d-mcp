#include "rotary.h"
#include "p3d_connect.h"

std::vector<Rotary*> Rotary::instances = std::vector<Rotary*>();

Rotary::Rotary(int addr, enum EVENT_ID event_id) {
	this->addr = addr;
	this->event_id = event_id;
	Rotary::instances.push_back(this);
	char buf[10];
	sprintf_s(buf, "ROT:%d", addr);
	this->cmd_str = buf;
}

bool Rotary::update(std::string cmd) {
	if (cmd.compare(0, 5, this->cmd_str) == 0) {
		// e.g. ROT:3:UP
		if (cmd.compare(6, 2, "UP") == 0) {
			// Up
			P3dConnect::send_event(this->event_id, MOUSE_FLAG_WHEEL_UP);
		}
		else {
			// Down
			P3dConnect::send_event(this->event_id, MOUSE_FLAG_WHEEL_DOWN);
		}
		return true;
	}
	else {
		return false;
	}
}

void Rotary::update_all(std::string cmd) {
	for (std::vector<Rotary*>::iterator it = Rotary::instances.begin(); it != Rotary::instances.end(); ++it) {
		if ((Rotary*)(*it)->update(cmd)) {
			break;
		}
	}
}