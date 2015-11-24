#include <iostream>

#include "procargs.h"
#include "guestptimg.h"
#include "ptcpustate.h"

int main(int argc, char* argv[], char* envp[])
{
	GuestPTImg	*gs;
	const char 	*saveas;
	pid_t		pid;
	bool		do_attach;

	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << "program_path <args>\n";
		return -1;
	}

	do_attach = getenv("GUEST_ATTACH") != nullptr;

	pid = do_attach
		? atoi(getenv("GUEST_ATTACH"))
		: GuestPTImg::createChild(argc - 1, argv + 1, envp);
	PTCPUState::registerCPUs(pid);

	if (do_attach) {
		auto pa = new ProcArgs(pid);
		gs = GuestPTImg::createAttached<GuestPTImg>(pid, pa->getArgv());
	} else {
		gs = GuestPTImg::create<GuestPTImg>(pid, argv + 1);
	}

	if ((saveas = getenv("GUEST_SAVEAS"))) {
		std::cerr << "Saving as " << saveas << "...\n";
		gs->save(saveas);
	} else {
		std::cerr << "Saving...\n";
		gs->save();
	}

	delete gs;

	return 0;
}