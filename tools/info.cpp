/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * pvxs is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#include <iostream>
#include <list>

#include <epicsStdlib.h>
#include <epicsGetopt.h>
#include <epicsThread.h>

#include <pvxs/client.h>
#include <pvxs/log.h>
#include "utilpvt.h"

using namespace pvxs;

namespace {

void usage(const char* argv0)
{
    std::cerr<<"Usage: "<<argv0<<" <opts> [pvname ...]\n";
}

}

int main(int argc, char *argv[])
{
    logger_config_env(); // from $PVXS_LOG
    double timeout = 5.0;
    bool verbose = false;

    {
        int opt;
        while ((opt = getopt(argc, argv, "hvdw:")) != -1) {
            switch(opt) {
            case 'h':
                usage(argv[0]);
                return 0;
            case 'v':
                verbose = true;
                break;
            case 'd':
                logger_level_set("pvxs.*", Level::Debug);
                break;
            case 'w':
                if(epicsParseDouble(optarg, &timeout, nullptr)) {
                    std::cerr<<"Invalid timeout value: "<<optarg<<"\n";
                    return 1;
                }
                break;
            default:
                usage(argv[0]);
                std::cerr<<"\nUnknown argument: "<<char(opt)<<std::endl;
                return 1;
            }
        }
    }

    auto ctxt = client::Config::from_env().build();

    if(verbose)
        std::cout<<"Effective config\n"<<ctxt.config();

    std::list<std::shared_ptr<client::Operation>> ops;

    for(auto n : range(optind, argc)) {

        ops.push_back(ctxt.info(argv[n])
                      .result([&argv, n](Value&& prototype) {
                          std::cout<<argv[n]<<"\n"<<prototype;
                      })
                      .exec());
    }

    epicsThreadSleep(timeout);

    return 0;
}
