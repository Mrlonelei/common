#include "stdafx.h"

#include "comm.h"

std::ofstream __debug_file;

using namespace Common;

class CommandNotifier : public ICommandNotifier
{
public:
	CommandNotifier(DWORD tid)
        : _tid(tid)
	{}

private:
	virtual void OnCommand() override {
        ::PostThreadMessage(_tid, WM_USER + 0, 0, 0);
	}

private:
    DWORD _tid;
};

int main()
{
	CComm comm;
	CComm::s_setting_comm settings;
	CommandNotifier notifier(GetCurrentThreadId());

	comm.open(1);

	settings.baud_rate = CBR_9600;
	settings.databit = 8;
	settings.parity = NOPARITY;
	settings.stopbit = ONESTOPBIT;

	comm.setting_comm(&settings);
	comm.begin_threads();
	comm.set_notifier(&notifier);

	MSG msg;
	while (::GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_USER + 0) {
            while (Command* bpCmd = comm.get_command()) {
                if (bpCmd->type == CommandType::kErrorMessage) {
                    auto pCmd = static_cast<Command_ErrorMessage*>(bpCmd);
                    fprintf(stderr, "����%s(%d).\n", pCmd->what.c_str(), pCmd->code);
                }
                else if (bpCmd->type == CommandType::kUpdateCounter) {
                    int nRead, nWritten, nQueued;
                    comm.get_counter(&nRead, &nWritten, &nQueued);
                    fprintf(stdout, "״̬�����ռ���:%u,���ͼ���:%u,�ȴ�����:%u", nRead, nWritten, nQueued);
                }

                delete bpCmd;
            }
        }
	}

	return 0;
}
