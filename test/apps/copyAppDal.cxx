#include "logging/Logging.hpp"

#include "oksdbinterfaces/Configuration.hpp"
#include "oksdbinterfaces/Schema.hpp"
#include "coredal/Component.hpp"
#include "coredal/DaqApplication.hpp"
#include "coredal/DaqModule.hpp"
#include "coredal/Segment.hpp"
#include "coredal/Session.hpp"

#include <iostream>
#include <string>

using namespace dunedaq;

void listApps(const coredal::Session* session) {
  for (auto app : session->get_all_applications()) {
    std::cout << "Application: " << app->UID();
    if (app->disabled(*session)) {
      std::cout << "<disabled>";
    }
    else {
      auto daqApp = app->cast<coredal::DaqApplication>();
      if (daqApp) {
        std::cout << " Modules:";
        for (auto mod : daqApp->get_contains()) {
          std::cout << " " << mod->UID();
          if (mod->disabled(*session)) {
            std::cout << "<disabled>";
          }
        }
      }
    }
    std::cout << std::endl;
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " session database-file\n";
    return 0;
  }

  dunedaq::logging::Logging::setup();

  std::string dbfile(argv[2]);
  std::string confimpl = "oksconfig:" + dbfile;
  auto confdb = new oksdbinterfaces::Configuration(confimpl);

  std::string sessionName(argv[1]);
  auto session = confdb->get<coredal::Session>(sessionName);

  auto segment = session->get_segment();

  std::string className("DaqApplication");
  for (auto app : segment->get_applications()) {
    auto daqapp = app->cast<dunedaq::coredal::DaqApplication>();
    if (daqapp) {
      std::string uid(app->UID());
      std::cout << "Application " << uid << " read from database: "
                << app << std::endl;
    
      dunedaq::oksdbinterfaces::ConfigObject oldobj;
      confdb->get(className, app->UID(), oldobj);

      dunedaq::oksdbinterfaces::ConfigObject newobj;
      confdb->create(dbfile, className, uid+"-copy", newobj);
      std::cout << "Created obj " << newobj << std::endl;
      auto classInfo=confdb->get_class_info(className);
      for (auto attr : classInfo.p_attributes) {
        if (attr.p_type==dunedaq::oksdbinterfaces::u64_type) {
          uint64_t oldval;
          oldobj.get(attr.p_name, oldval);
          newobj.set_by_val<uint64_t>(attr.p_name, oldval);
        }
        if (attr.p_type==dunedaq::oksdbinterfaces::u32_type) {
          uint32_t oldval;
          oldobj.get(attr.p_name, oldval);
          newobj.set_by_val<uint32_t>(attr.p_name, oldval);
        }
        if (attr.p_type==dunedaq::oksdbinterfaces::string_type) {
          std::string oldval;
          oldobj.get(attr.p_name, oldval);
          newobj.set_by_val<std::string>(attr.p_name, oldval);
        }
      }
      std::string uid2(newobj.UID());
      auto daqapp2 = const_cast<dunedaq::coredal::DaqApplication*>(
        confdb->get<dunedaq::coredal::DaqApplication>(uid2));
      std::cout << "Application " << daqapp2 << " copied from " << daqapp << std::endl;

      std::cout << uid2 << ":"
                << " application_name=" << daqapp2->get_application_name()
                << " commandline_parameters=" << daqapp2->get_commandline_parameters()
                << std::endl;
      daqapp2->set_commandline_parameters("--test");
      std::cout << uid << " dal object:"
                << " application_name=" << daqapp->get_application_name()
                << " commandline_parameters=" << daqapp->get_commandline_parameters()
                << std::endl;
      std::cout << uid2 << " dal object:"
                << " application_name=" << daqapp2->get_application_name()
                << " commandline_parameters=" << daqapp2->get_commandline_parameters()
                << std::endl;
      auto daqapp2a = const_cast<dunedaq::coredal::DaqApplication*>(
        confdb->get<dunedaq::coredal::DaqApplication>(uid2));
      std::cout << uid2 << " 2nd dal object:"
                << " application_name=" << daqapp2a->get_application_name()
                << " commandline_parameters=" << daqapp2a->get_commandline_parameters()
                << std::endl;

      std::string appName;
      oldobj.get("application_name", appName);
      std::string cmdLine;
      oldobj.get("commandline_parameters", cmdLine);
      std::cout << uid << " Configuration object:"
                << " application_name=" << appName
                << " commandline_parameters=" << cmdLine
                << std::endl;

      newobj.get("application_name", appName);
      newobj.get("commandline_parameters", cmdLine);
      std::cout << uid2 << " Configuration object:"
                << " application_name=" << appName
                << " commandline_parameters=" << cmdLine
                << std::endl;


    }
  }


}
