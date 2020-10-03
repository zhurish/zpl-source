#include "pjsiplogwriter.hpp"

using namespace pj;
using namespace std;

pjsipLogWriter::pjsipLogWriter()
{
}
pjsipLogWriter::~pjsipLogWriter()
{
}

void utilLogWrite(int prmLevel,
                  const string &prmSender,
                  const string &prmMsg)
{
    Endpoint::instance().utilLogWrite(prmLevel, prmSender, prmMsg);
}

void pj_log_write(int level, const char *sender,
                          const char *format, ...)
{
#if PJ_LOG_MAX_LEVEL >= 1
    va_list arg;
    va_start(arg, format);
    pj_log(sender, level, format, arg );
    va_end(arg);
#endif
}

/*
void utilLogWrite(int prmLevel,
                  const string &prmSender,
                  const char *prmMsg)
{
    Endpoint::instance().utilLogWrite(prmLevel, prmSender, prmMsg);
}
*/