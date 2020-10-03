#ifndef PJSIPLOGWRITER_HPP
#define PJSIPLOGWRITER_HPP
#include <pjsua2.hpp>
#include <iostream>

using namespace pj;
using namespace std;

class pjsipLogWriter : public LogWriter
{
public:
    pjsipLogWriter();
    ~pjsipLogWriter();
    void write(const LogEntry &entry)
    {
        std::cout << entry.msg << std::endl;
    }
};

void utilLogWrite(int prmLevel,
                  const string &prmSender,
                  const string &prmMsg);
                  
void pj_log_write(int level, const char *sender,
                          const char *format, ...);

#define _PJSIP_DEBUG
#ifdef _PJSIP_DEBUG
#define PJSIP_ENTER_DEBUG()     std::cout << "Enter " << __func__ << std::endl
#define PJSIP_LEAVE_DEBUG()     std::cout << "Leave " << __func__ << std::endl
#define PJSIP_ERROR_DEBUG(m)    std::cout << "ERROR " << __func__ << ": " << m << std::endl
#define PJSIP_MSG_DEBUG(m)      std::cout << "======MSG: " << __func__ << ": " << m << std::endl
#else
#define PJSIP_ENTER_DEBUG()     
#define PJSIP_LEAVE_DEBUG() 
#define PJSIP_ERROR_DEBUG(m)  
#define PJSIP_MSG_DEBUG(m)  
#endif

#endif // PJSIPLOGWRITER_HPP
