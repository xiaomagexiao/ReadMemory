//
// Created by MaMa on 2017/4/15.
//

#include "StackCallBack.h"

#include <sstream>
#include <iomanip>

#include <unwind.h>
#include <dlfcn.h>

namespace {
    struct BacktraceState {
        void **current;
        void **end;
    };

    static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context *context, void *arg) {
        BacktraceState *state = static_cast<BacktraceState *>(arg);
        uintptr_t pc = _Unwind_GetIP(context);
        if (pc) {
            if (state->current == state->end) {
                return _URC_END_OF_STACK;
            } else {
                *state->current++ = reinterpret_cast<void *>(pc);
            }
        }
        return _URC_NO_REASON;
    }

}

size_t StackCallBack::captureBacktrace(void **buffer, size_t max) {
    BacktraceState state = {buffer, buffer + max};
    _Unwind_Backtrace(unwindCallback, &state);
    return state.current - buffer;
}

void StackCallBack::dumpBacktrace(std::ostream &os, void **buffer, size_t count) {
    for (size_t idx = 0; idx < count; ++idx) {
        const void *addr = buffer[idx];
        const char *symbol = "";

        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname) {
            symbol = info.dli_sname;
        }

        os << "  #" << std::setw(2) << idx << ": " << addr << "  " << symbol << "\n";
    }
}

void StackCallBack::backtraceToLogcat() {
    const size_t max = 30;
    void *buffer[max];
    std::ostringstream oss;
    dumpBacktrace(oss, buffer, captureBacktrace(buffer, max));
    cout << oss.str().c_str() << endl;
}