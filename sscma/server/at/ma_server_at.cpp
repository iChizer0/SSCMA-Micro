#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <stack>

#include "ma_server_at.h"

#include "callback/common.hpp"
#include "callback/model.hpp"
#include "callback/resource.hpp"

namespace ma {

using namespace ma::server::callback;

// static constexpr char TAG[] = "ma::server::ATServer";

ATService::ATService(const std::string& name,
                     const std::string& desc,
                     const std::string& args,
                     ATServiceCallback cb)
    : name(name), desc(desc), args(args), cb(cb) {
    argc = std::count(args.begin(), args.end(), ',') + 1;
}


ma_err_t ATServer::addService(ATService& service) {
    for (auto it = m_services.begin(); it != m_services.end(); ++it) {
        if (it->name == service.name) {
            return MA_EEXIST;
        }
    }
    m_services.push_back(service);
    return MA_OK;
}

ATServer::ATServer(Encoder* encoder) : m_encoder(*encoder) {
    m_thread = new Thread("ATServer", ATServer::threadEntryStub);
    MA_ASSERT(m_thread);
}

ATServer::ATServer(Encoder& encoder) : m_encoder(encoder) {

    m_thread = new Thread("ATServer", ATServer::threadEntryStub);
    MA_ASSERT(m_thread);
}

void ATServer::threadEntryStub(void* arg) {
    ATServer* server = static_cast<ATServer*>(arg);
    server->threadEntry();
}

void ATServer::threadEntry() {
    char* buf = static_cast<char*>(ma_malloc(MA_SEVER_AT_CMD_MAX_LENGTH + 1));
    std::memset(buf, 0, MA_SEVER_AT_CMD_MAX_LENGTH + 1);
    while (true) {
        for (auto& transport : static_resource->transports) {
            if (*transport) {
                int len = transport->receiveUntil(buf, MA_SEVER_AT_CMD_MAX_LENGTH, 0x0D);
                if (len > 1) {
                    execute(buf, transport);
                }
                len = transport->receiveUntil(buf, MA_SEVER_AT_CMD_MAX_LENGTH, 0x0A);
                if (len > 1) {
                    execute(buf, transport);
                }
            }
        }
        Thread::sleep(Tick::fromMicroseconds(20));
    }
    ma_free(buf);
}

ma_err_t ATServer::init() {

    ma_err_t err = MA_OK;

    static_resource->init();

    this->addService("ID?",
                     "Get device ID",
                     "",
                     [](std::vector<std::string> args, Transport& transport, Encoder& encoder) {
                         static_resource->executor->submit([cmd = std::move(args[0]),
                                                              &transport,
                                                              &encoder](const std::atomic<bool>&) {
                             get_device_id(cmd, transport, encoder);
                         });
                         return MA_OK;
                     });

    this->addService("NAME?",
                     "Get device name",
                     "",
                     [](std::vector<std::string> args, Transport& transport, Encoder& encoder) {
                         static_resource->executor->submit([cmd = std::move(args[0]),
                                                              &transport,
                                                              &encoder](const std::atomic<bool>&) {
                             get_device_name(cmd, transport, encoder);
                         });
                         return MA_OK;
                     });

    this->addService("STAT?",
                     "Get device status",
                     "",
                     [](std::vector<std::string> args, Transport& transport, Encoder& encoder) {
                         static_resource->executor->submit([cmd = std::move(args[0]),
                                                              &transport,
                                                              &encoder](const std::atomic<bool>&) {
                             get_device_status(cmd, transport, encoder);
                         });
                         return MA_OK;
                     });

    this->addService("VER?",
                     "Get device version",
                     "",
                     [](std::vector<std::string> args, Transport& transport, Encoder& encoder) {
                         static_resource->executor->submit([cmd = std::move(args[0]),
                                                              &transport,
                                                              &encoder](const std::atomic<bool>&) {
                             get_version(cmd, transport, encoder, MA_AT_API_VERSION);
                         });
                         return MA_OK;
                     });

    this->addService("RST",
                     "Reset device",
                     "",
                     [](std::vector<std::string> args, Transport& transport, Encoder& encoder) {
                         static_resource->executor->submit(
                             [cmd = std::move(args[0]), &transport, &encoder](
                                 const std::atomic<bool>&) { });
                         return MA_OK;
                     });

    this->addService(
        "BREAK",
        "Stop all running tasks",
        "",
        [](std::vector<std::string> args, Transport& transport, Encoder& encoder) {
            if (static_resource->is_ready.load()) [[likely]] {
                static_resource->executor->submit(
                    [cmd = std::move(args[0]), &transport, &encoder](const std::atomic<bool>&) {
                        static_resource->current_task_id.fetch_add(1);
                        break_task(cmd, transport, encoder);
                    });
            }
            return MA_OK;
        });

    this->addService("YIELD",
                     "Yield running tasks for a period time",
                     "TIME_S",
                     [](std::vector<std::string> args, Transport& Transport, Encoder& Encoder) {
                         static_resource->executor->submit(
                             [cmd    = std::move(args[0]),
                              period = std::atoi(args[1].c_str()),
                              &Transport,
                              &Encoder](const std::atomic<bool>& stop_token) mutable {
                                 while (stop_token.load(std::memory_order_seq_cst) && period > 0) {
                                     Thread::yield();
                                     period -= 1;
                                 }
                             });
                         return MA_OK;
                     });

    this->addService(
        "LED",
        "Set LED status",
        "ENABLE/DISABLE",
        [](std::vector<std::string> args, Transport& transport, Encoder& encoder) {
            static_resource->executor->submit(
                [cmd = std::move(args[0]), sta = std::atoi(args[1].c_str()), &transport, &encoder](
                    const std::atomic<bool>&) { task_status(cmd, sta, transport, encoder); });

            return MA_OK;
        });

    this->addService("MODELS?",
                     "Get available models",
                     "",
                     [](std::vector<std::string> args, Transport& transport, Encoder& encoder) {
                         static_resource->executor->submit([cmd = std::move(args[0]),
                                                              &transport,
                                                              &encoder](const std::atomic<bool>&) {
                             get_available_models(cmd, transport, encoder);
                         });
                         return MA_OK;
                     });

    this->addService("MODEL?",
                     "Get current model",
                     "",
                     [](std::vector<std::string> args, Transport& transport, Encoder& encoder) {
                         static_resource->executor->submit([cmd = std::move(args[0]),
                                                              &transport,
                                                              &encoder](const std::atomic<bool>&) {
                             get_model_info(cmd, transport, encoder);
                         });
                         return MA_OK;
                     });

    this->addService("MODEL",
                     "Set current model",
                     "MODEL_ID",
                     [](std::vector<std::string> args, Transport& transport, Encoder& encoder) {
                         static_resource->executor->submit([cmd      = std::move(args[0]),
                                                              model_id = std::atoi(args[1].c_str()),
                                                              &transport,
                                                              &encoder](const std::atomic<bool>&) {
                             set_model(cmd, transport, encoder, model_id);
                         });
                         return MA_OK;
                     });


    return err;
}

ma_err_t ATServer::start() {

    static_resource->executor->submit(
        [](const std::atomic<bool>&) { static_resource->is_ready.store(true); });

    m_thread->start(this);

    return MA_OK;
}

ma_err_t ATServer::stop() {
    return MA_OK;
}

ma_err_t ATServer::addService(const std::string& name,
                              const std::string& desc,
                              const std::string& args,
                              ATServiceCallback cb) {
    ATService service(name, desc, args, cb);
    return addService(service);
}

ma_err_t ATServer::execute(std::string line, Transport& transport) {
    ma_err_t ret = MA_OK;
    std::string name;
    std::string args;

    line.erase(std::remove_if(line.begin(), line.end(), [](char c) { return !std::isprint(c); }),
               line.end());

    // find first '=' in AT command (<name>=<args>)
    size_t pos = line.find_first_of("=");
    if (pos != std::string::npos) {
        name = line.substr(0, pos);
        args = line.substr(pos + 1, line.size());
    } else {
        name = line;
    }

    std::transform(name.begin(), name.end(), name.begin(), ::toupper);

    // check if name is valid (starts with "AT+")
    if (name.rfind("AT+", 0) != 0) {
        m_encoder.begin(MA_MSG_TYPE_EVT, MA_EINVAL, "AT", "Uknown command: " + name);
        m_encoder.end();
        transport.send(reinterpret_cast<const char*>(m_encoder.data()), m_encoder.size());
        return MA_EINVAL;
    }

    name = name.substr(3, name.size());  // remove "AT+" command prefix

    // find command tag (everything after last '@'), then remove the tag to get the AT command
    // name
    size_t cmd_body_pos    = name.rfind('@');
    cmd_body_pos           = cmd_body_pos != std::string::npos ? cmd_body_pos + 1 : 0;
    std::string target_cmd = name.substr(cmd_body_pos, name.size());

    // find command in cmd list
    auto it = std::find_if(m_services.begin(), m_services.end(), [&](const ATService& c) {
        return c.name.compare(target_cmd) == 0;
    });

    if (it == m_services.end()) [[unlikely]] {
        m_encoder.begin(MA_MSG_TYPE_EVT, MA_EINVAL, "AT", "Uknown command: " + name);
        m_encoder.end();
        transport.send(reinterpret_cast<const char*>(m_encoder.data()), m_encoder.size());
        return MA_EINVAL;
    }

    // split args by ','
    std::vector<std::string> argv;
    argv.push_back(name);
    std::stack<char> stk;
    size_t index = 0;
    size_t size  = args.size();

    while (index < size &&
           argv.size() < it->argc + 1u) {  // while not reach end of args and not enough args
        char c = args.at(index);
        if (c == '\'' || c == '"') [[unlikely]] {  // if current char is a quote
            stk.push(c);                           // push the quote to stack
            std::string arg;
            while (++index < size &&
                   stk.size()) {     // while not reach end of args and stack is not empty
                c = args.at(index);  // get next char
                if (c == stk.top())  // if the char matches the top of stack, pop the stack
                    stk.pop();
                else if (c != '\\') [[likely]]  // if the char is not a backslash, append it to
                    arg += c;
                else if (++index < size)
                    [[likely]]  // if the char is a backslash, get next char, append it to arg
                    arg += args.at(index);
            }
            argv.push_back(std::move(arg));
        } else if (c == '-' || std::isdigit(c)) {  // if current char is a digit or a minus sign
            size_t prev = index;
            while (++index < size)  // while not reach end of args
                if (!std::isdigit(args.at(index)))
                    break;  // if current char is not a digit, break
            argv.push_back(args.substr(prev, index - prev));  // append the number string to argv
        } else
            ++index;  // if current char is not a quote or a digit, skip it
    }
    argv.shrink_to_fit();

    if (argv.size() < it->argc) [[unlikely]] {
        m_encoder.begin(
            MA_MSG_TYPE_EVT, MA_EINVAL, "AT", "Command " + name + " got wrong arguments");
        m_encoder.end();
        transport.send(reinterpret_cast<const char*>(m_encoder.data()), m_encoder.size());
        return MA_EINVAL;
    }

    ret = it->cb(std::move(argv), transport, m_encoder);

    return ret;
}

ma_err_t ATServer::execute(std::string line, Transport* transport) {
    return execute(line, *transport);
}

}  // namespace ma