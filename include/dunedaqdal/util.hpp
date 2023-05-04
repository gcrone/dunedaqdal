#ifndef _dal_util_H_
#define _dal_util_H_

#include <exception>

#include "oksdbinterfaces/Configuration.hpp"
#include "oksdbinterfaces/DalObject.hpp"


namespace dunedaq {

namespace dal {

    // forward declaration

  class BaseApplication;
  class Session;
  class SW_Repository;
  class Tag;
  class Computer;


   /**
     *  \brief  Check if given tag can be used on given computer.
     *  
     *  The algorithm reads platforms compatibility description from session's OnlineSegment.
     *  This allows to describe a host with real hw platform and installed operating system,
     *  and to run on it applications with compatible tags, e.g.:
     *  host with 64 bits SLC5 allows to run applications with x86_64-slc5, i686-slc5 and i686-slc4 tags.
     *
     *  \par Parameters and return value
     *
     *  \param tag             tested tag
     *  \param host            host where the tag is tested
     *  \param session       session defining online segment with compatibility info
     *  \return                true, if tag is compatible
     */

  bool is_compatible(const dunedaq::dal::Tag& tag, const dunedaq::dal::Computer& host, const dunedaq::dal::Session& session);



    /**
     *  \brief Substitute variables from conversion map or from process environment.
     *
     *  Substitute variables using special syntax like ${FOO}, $(BAR), etc.
     *  If substitution %is defined, then the variables are replaced by the corresponding
     *  substitution value. The substitution values are defined either by the
     *  substitution map, or by the process environment.
     *
     *  \par Parameters and return value
     *
     *  \param value            string containing variables to be substituted
     *  \param conversion_map   pointer to conversion map; if null, the process environment %is used
     *  \param beginning        definition of syntax symbols which delimit the beginning of the variable
     *  \param ending           definition of syntax symbols which delimit the ending of the variable
     *  \return                 Returns the result of substitution.
     *
     *  \par Example
     *
     *  If the conversion map has pair "FOO","BAR", then substitute_variables("/home/${FOO}", cmap, "${", "}") returns "/home/BAR".
     *  Otherwise it returns non-changed value "/home/${FOO}".
     *  <BR>
     *  If there %is environment variable "USER" with value "Online", then substitute_variables("/home/$(USER)", 0, "$()") returns "/home/Online".
     *  Otherwise it returns non-changed value "/home/$(USER)".
     */

  std::string substitute_variables(const std::string& value, const std::map<std::string, std::string> * conversion_map, const std::string& beginning, const std::string& end);

    /**
     *  \brief Implements string converter for database parameters.
     *
     *  The class implements dunedaq::oksdbinterfaces::Configuration::AttributeConverter for string %type.
     *  It reads parameters defined for given session object and uses them to
     *  substitute values of database string attributes.
     *
     *  The parameters are stored as a map of substitution keys and values.
     *  If database %is changed, the reset(dunedaq::oksdbinterfaces::Configuration&, const Session&) method needs to be used.
     *
     *  \par Example
     *
     *  The example shows how to use the converter:
     *
     *  <pre><i>
     *
     *  dunedaq::oksdbinterfaces::Configuration db(...);  // some code to build configuration database object
     *
     *  const dunedaq::dal::Session * session = dunedaq::dal::get_session(db, session_name);
     *  if(session) {
     *    db.register_converter(new dunedaq::dal::SubstituteVariables(db, *session));
     *  }
     *
     *  </i></pre>
     *
     */

  class SubstituteVariables : public dunedaq::oksdbinterfaces::Configuration::AttributeConverter<std::string> {

    public:


        /** Build converter object. **/

      SubstituteVariables(const Session& p)
      {
        reset (p);
      }


        /** Method to reset substitution map in case of database changes. **/

      void reset(const Session&);


        /** Implementation of convert method. **/

      virtual void convert(std::string& value, const dunedaq::oksdbinterfaces::Configuration& conf, const dunedaq::oksdbinterfaces::ConfigObject& obj, const std::string& attr_name);


        /** Destroy conversion map. **/

      virtual ~SubstituteVariables() {;}


        /** Return conversion map **/

      const std::map<std::string, std::string> * get_conversion_map() const {return &m_cvt_map;}


    private:

      std::map<std::string, std::string> m_cvt_map;
  };


    /**
     *  \brief Get session object.
     *
     *  The algorithm %is searching the session object by given name.
     *  If the name %is empty, then the algorithm takes the name from
     *  the TDAQ_SESSION environment variable.<BR>
     *
     *  The last parameter of the algorithm can be used to optimise performance
     *  of the DAL in case if a database server config implementation %is used.
     *  The parameter defines how many layers of objects referenced by given 
     *  session object should be read into client's config cache together with
     *  session object during single network operation. For example:
     *  - if the parameter %is 0, then only session object %is read;
     *  - if the parameter %is 1, then session and first layer segment objects are read;
     *  - if the parameter %is 2, then session, segments of first and second layers, and application/resources of first layer segments objects are read;
     *  - if the parameter %is 10, then mostly probable all objects referenced by given session object are read.<BR>
     *
     *  The parameters of the algorithm are:
     *  \param conf      the configuration object with loaded database
     *  \param name      the name of the session to be loaded (if empty, TDAQ_SESSION variable %is used)
     *  \param rlevel    optional parameter to optimise performance ("the references level")
     *  \param rclasses  optional parameter to optimise performance ("names of classes which objects are cached")
     *
     *  \return Returns the pointer to the session object if found, or 0.
     */

  const dunedaq::dal::Session * get_session(dunedaq::oksdbinterfaces::Configuration& conf, const std::string& name, unsigned long rlevel = 10, const std::vector<std::string> * rclasses = nullptr);


    /**
     *  \brief Get used software repositories.
     *
     *  The algorithm %is searching the sw repositories used by given session,
     *  checking all active segments and applications.
     *
     *  The method throws dunedaq::dal::AlgorithmError exception in case of logical problems found in database
     *  (such as circular dependencies between segments, resources or repositories).
     *
     *  The parameters of the algorithm are:
     *  \param p the session object
     *
     *  \return The used repositories
     */

  std::set<const dunedaq::dal::SW_Repository *> get_used_repositories(const dunedaq::dal::Session& p);


    /**
     *  \brief Add into CLASSPATH JARs defined by JarFile objects.
     *
     *  The function iterates all SW objects of given repository and tests found JarFile objects.
     *  For each JarFile it checks if corresponding JAR file exists in repository root, patch or installation areas.
     *  First readable jar file is added to the class path.
     *
     *  @param[in] rep               the repository with JarFile objects
     *  @param[in] repository_root   the session's repository root
     *  @param[in,out] class_path    the value of class path
     *
     *  @throw dunedaq::dal::NoJarFile is thrown when the jar file is not found or not readable.
     */

  void add_classpath(const dunedaq::dal::SW_Repository& rep, const std::string& repository_root, std::string& class_path);


    /**
     * \brief Get OKS GIT version for running session.
     *
     * The method extracts GIT version for running session configuration reading value from the RunParams information server.
     * If not set, it tries to extract the value from the TDAQ_DB_VERSION process environment.
     *
     * \param session the name of the session
     * \return the configuration version
     *
     *  @throw dunedaq::oksdbinterfaces::NotFound is thrown if case if session or information repository does not exist
     *  @throw dunedaq::oksdbinterfaces::Exception is thrown if case of problems
     */

  std::string
  get_config_version(const std::string& session);


    /**
     * \brief Set new OKS GIT version for running session.
     *
     * The method writes version on the RunParams information server and reloads with this version RDB and RDB_RW servers.
     * In initial session only RDB_INITIAL server is reloaded.
     *
     * \param session the name of the session
     * \param version the configuration version
     * \param reload if true, send reload command to RDB and RDB_RW servers
     *
     *  @throw dunedaq::oksdbinterfaces::NotFound is thrown if case if session or information repository does not exist
     *  @throw dunedaq::oksdbinterfaces::Exception is thrown if case of problems
     */

  void
  set_config_version(const std::string& session, const std::string& version, bool reload);

} // namespace dal


  ERS_DECLARE_ISSUE(
    dal,
    AlgorithmError,
    ,
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    BadVariableUsage,
    AlgorithmError,
    message,
    ,
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    BadApplicationInfo,
    AlgorithmError,
    "Failed to retrieve information for Application \'" << app_id << "\' from the database: " << message,
    ,
    ((std::string)app_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    BadSessionID,
    AlgorithmError,
    "There is no session object with UID = \"" << name << '\"',
    ,
    ((std::string)name)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    SegmentDisabled,
    AlgorithmError,
    "Cannot get information about applications because the segment is disabled",
    ,
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    BadProgramInfo,
    AlgorithmError,
    "Failed to retrieve information for Program \'" << prog_id << "\' from the database: " << message,
    ,
    ((std::string)prog_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    BadHost,
    AlgorithmError,
    "Failed to retrieve application \'" << app_id << "\' from the database: " << message,
    ,
    ((std::string)app_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    NoDefaultHost,
    AlgorithmError,
    "Failed to find default host for segment \'" << seg_id << "\' " << message,
    ,
    ((std::string)seg_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    NoTemplateAppHost,
    AlgorithmError,
    "Both session default and segment default hosts are not defined for template application \'" << app_id << "\' from segment \'" << seg_id << "\' (will use localhost, that may cause problems presenting info in IGUI for distributed session).",
    ,
    ((std::string)app_id)
    ((std::string)seg_id)
  )


  ERS_DECLARE_ISSUE_BASE(
    dal,
    BadTag,
    AlgorithmError,
    "Failed to use tag \'" << tag_id << "\' because: " << message,
    ,
    ((std::string)tag_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    BadSegment,
    AlgorithmError,
    "Invalid Segment \'" << seg_id << "\' because: " << message,
    ,
    ((std::string)seg_id)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    GetTemplateApplicationsOfSegmentError,
    AlgorithmError,
    "Failed to get template applications of \'" << name << "\' segment" << message,
    ,
    ((std::string)name)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    BadTemplateSegmentDescription,
    AlgorithmError,
    "Bad configuration description of template segment \'" << name << "\': " << message,
    ,
    ((std::string)name)
    ((std::string)message)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    CannotGetApplicationObject,
    AlgorithmError,
    "Failed to get application object from name: " << reason,
    ,
    ((std::string)reason)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    CannotFindSegmentByName,
    AlgorithmError,
    "Failed to find segment object \'" << name << "\': " << reason,
    ,
    ((std::string)name)
    ((std::string)reason)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    NotInitedObject,
    AlgorithmError,
    "The " << item << " object " << obj << " was not initialized",
    ,
    ((const char *)item)
    ((void *)obj)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    NotInitedByDalAlgorithm,
    AlgorithmError,
    "The " << obj_id << '@' << obj_class << " object " << address << " was not initialized by DAL algorithm " << algo,
    ,
    ((std::string)obj_id)
    ((std::string)obj_class)
    ((void*)address)
    ((const char *)algo)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    CannotCreateSegConfig,
    AlgorithmError,
    "Failed to create config for segment \'" << name << "\': " << reason,
    ,
    ((std::string)name)
    ((std::string)reason)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    CannotGetParents,
    AlgorithmError,
    "Failed to get parents of \'" << object << '\'',
    ,
    ((std::string)object)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    FoundCircularDependency,
    AlgorithmError,
    "Reach maximum allowed recursion (" << limit << ") during calculation of " << goal << "; possibly there is circular dependency between these objects: " << objects,
    ,
    ((unsigned int)limit)
    ((const char *)goal)
    ((std::string)objects)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    NoJarFile,
    AlgorithmError,
    "Cannot find jar file \'" << file << "\' described by \'" << obj_id << '@' << obj_class << "\' that is part of \'" << rep_id << '@' << rep_class << '\'',
    ,
    ((std::string)file)
    ((std::string)obj_id)
    ((std::string)obj_class)
    ((std::string)rep_id)
    ((std::string)rep_class)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    DuplicatedApplicationID,
    AlgorithmError,
    "Two applications have equal IDs:\n  1) " << first << "\n  2) " << second,
    ,
    ((std::string)first)
    ((std::string)second)
  )

  ERS_DECLARE_ISSUE_BASE(
    dal,
    SegmentIncludedMultipleTimes,
    AlgorithmError,
    "The segment \"" << segment << "\" is included by:\n  1) " << first << "\n  2) " << second,
    ,
    ((std::string)segment)
    ((std::string)first)
    ((std::string)second)
  )

} // namespace dunedaq

#endif
