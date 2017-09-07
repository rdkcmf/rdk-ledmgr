#ifndef LEDMGR_H
#define LEDMGR_H
#include "ledmgrbase.hpp"
#include "fp_profile.hpp"

//TODO (OEM): OEM vendors are to implement the functions required for their platform.

class ledMgr : public ledMgrBase
{
        private:
        static ledMgr m_singleton;

        public:
        ledMgr();
        static ledMgr &getInstance();
        virtual void handleModeChange(unsigned int mode);
        virtual void handleGatewayConnectionEvent(unsigned int state, unsigned int error);
        virtual void handleKeyPress(int key_code, int key_type);
};

#endif /*LEDMGR_H*/

