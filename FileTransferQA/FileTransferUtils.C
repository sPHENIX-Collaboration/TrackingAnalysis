#ifndef FileTransferUtil_C
#define FileTransferUtil_C

#include <odbc++/connection.h>
#include <odbc++/drivermanager.h>
#include <odbc++/resultset.h>
#include <odbc++/statement.h>

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>

#include <boost/format.hpp>
#include <boost/algorithm/string/case_conv.hpp>

//! subsystem characterization, used for locating files on disk, db, etc.
struct subsystem_info_t
{
  //! constructor
  subsystem_info_t() = default;

  //! constructor
  subsystem_info_t( const std::string& _path, const std::string& _subsystem, const std::string& _host ):
    path( _path ),
    subsystem( _subsystem ),
    host( _host )
  {}

  std::string path;
  std::string subsystem;
  std::string host;

  using list = std::vector<subsystem_info_t>;
};

//! three-way operator for subsystem_info_t
auto operator <=> (const subsystem_info_t& lhs, const subsystem_info_t& rhs )
{
  if( lhs.path != rhs.path ) return lhs.path <=> rhs.path;
  else if( lhs.subsystem != rhs.subsystem ) return lhs.subsystem <=> rhs.subsystem;
  else return lhs.host <=> rhs.host;
}

//! streamer for subsystem_info_t
std::ostream& operator << (std::ostream& o, const subsystem_info_t& subsystem )
{
  o << "{" << subsystem.path << ", " << subsystem.subsystem << ", " << subsystem.host << "}";
  return o;
}

// hardcoded list of all tracking subsystems
static const subsystem_info_t::list default_subsystems =
{
  // TPC subsystems
  {"tpc", "TPC", "ebdc00_0" },
  {"tpc", "TPC", "ebdc01_0" },
  {"tpc", "TPC", "ebdc02_0" },
  {"tpc", "TPC", "ebdc03_0" },
  {"tpc", "TPC", "ebdc04_0" },
  {"tpc", "TPC", "ebdc05_0" },
  {"tpc", "TPC", "ebdc06_0" },
  {"tpc", "TPC", "ebdc07_0" },
  {"tpc", "TPC", "ebdc08_0" },
  {"tpc", "TPC", "ebdc09_0" },
  {"tpc", "TPC", "ebdc10_0" },
  {"tpc", "TPC", "ebdc11_0" },
  {"tpc", "TPC", "ebdc12_0" },
  {"tpc", "TPC", "ebdc13_0" },
  {"tpc", "TPC", "ebdc14_0" },
  {"tpc", "TPC", "ebdc15_0" },
  {"tpc", "TPC", "ebdc16_0" },
  {"tpc", "TPC", "ebdc17_0" },
  {"tpc", "TPC", "ebdc18_0" },
  {"tpc", "TPC", "ebdc19_0" },
  {"tpc", "TPC", "ebdc20_0" },
  {"tpc", "TPC", "ebdc21_0" },
  {"tpc", "TPC", "ebdc22_0" },
  {"tpc", "TPC", "ebdc23_0" },

  // TPC subsystems
  {"tpc", "TPC", "ebdc00_1" },
  {"tpc", "TPC", "ebdc01_1" },
  {"tpc", "TPC", "ebdc02_1" },
  {"tpc", "TPC", "ebdc03_1" },
  {"tpc", "TPC", "ebdc04_1" },
  {"tpc", "TPC", "ebdc05_1" },
  {"tpc", "TPC", "ebdc06_1" },
  {"tpc", "TPC", "ebdc07_1" },
  {"tpc", "TPC", "ebdc08_1" },
  {"tpc", "TPC", "ebdc09_1" },
  {"tpc", "TPC", "ebdc10_1" },
  {"tpc", "TPC", "ebdc11_1" },
  {"tpc", "TPC", "ebdc12_1" },
  {"tpc", "TPC", "ebdc13_1" },
  {"tpc", "TPC", "ebdc14_1" },
  {"tpc", "TPC", "ebdc15_1" },
  {"tpc", "TPC", "ebdc16_1" },
  {"tpc", "TPC", "ebdc17_1" },
  {"tpc", "TPC", "ebdc18_1" },
  {"tpc", "TPC", "ebdc19_1" },
  {"tpc", "TPC", "ebdc20_1" },
  {"tpc", "TPC", "ebdc21_1" },
  {"tpc", "TPC", "ebdc22_1" },
  {"tpc", "TPC", "ebdc23_1" },

  // TPOT subsystems
  {"TPOT", "TPOT", "ebdc39" },

  // INTT subsystem
  {"INTT", "INTT", "intt0" },
  {"INTT", "INTT", "intt1" },
  {"INTT", "INTT", "intt2" },
  {"INTT", "INTT", "intt3" },
  {"INTT", "INTT", "intt4" },
  {"INTT", "INTT", "intt5" },
  {"INTT", "INTT", "intt6" },
  {"INTT", "INTT", "intt7" },

  // MVTX subsystems
  {"MVTX", "MVTX", "mvtx0" },
  {"MVTX", "MVTX", "mvtx1" },
  {"MVTX", "MVTX", "mvtx2" },
  {"MVTX", "MVTX", "mvtx3" },
  {"MVTX", "MVTX", "mvtx4" },
  {"MVTX", "MVTX", "mvtx5" },

  // EMCAL
  {"emcal", "EMCAL", "seb00" },
  {"emcal", "EMCAL", "seb01" },
  {"emcal", "EMCAL", "seb02" },
  {"emcal", "EMCAL", "seb03" },
  {"emcal", "EMCAL", "seb04" },
  {"emcal", "EMCAL", "seb05" },
  {"emcal", "EMCAL", "seb06" },
  {"emcal", "EMCAL", "seb07" },
  {"emcal", "EMCAL", "seb08" },
  {"emcal", "EMCAL", "seb09" },
  {"emcal", "EMCAL", "seb10" },
  {"emcal", "EMCAL", "seb11" },
  {"emcal", "EMCAL", "seb12" },
  {"emcal", "EMCAL", "seb13" },
  {"emcal", "EMCAL", "seb14" },
  {"emcal", "EMCAL", "seb15" },

  // HCAL
  {"HCal", "HCAL", "seb16" },
  {"HCal", "HCAL", "seb17" },

  // MBD
  {"mbd", "MBD", "seb18" },

  // ZDC
  {"ZDC", "ZDC", "seb20" },

  // GL1
  {"GL1", "GL1", "gl1daq" }
};

//! file information (from DB)
struct fileinfo_t
{
  std::string filename;
  bool in_hpss = false;
  bool in_sdcc = false;
};

//! three-way operator for fileinfo_t
auto operator <=> (const fileinfo_t& lhs, const fileinfo_t& rhs )
{
  if( lhs.filename != rhs.filename ) return lhs.filename <=> rhs.filename;
  else if( lhs.in_hpss != rhs.in_hpss ) return lhs.in_hpss <=> rhs.in_hpss;
  else return lhs.in_sdcc <=> rhs.in_sdcc;
}

//! streamer
std::ostream& operator << (std::ostream& o, const fileinfo_t& fileinfo )
{
  o << fileinfo.filename << " in_hpss: " << fileinfo.in_hpss << " in_sdcc: " << fileinfo.in_sdcc;
  return o;
}

using runnumber_set_t = std::set<int>;
using filename_set_t = std::set<std::string>;
using fileinfo_set_t = std::set<fileinfo_t>;

//! default subsystem
subsystem_info_t default_subsystem( "TPOT", "TPOT", "ebdc39" );
std::string default_runtype = "physics";

//! generic streamer for std::set
template<class T>
  std::ostream& operator << (std::ostream& o, const std::set<T>& input )
{
  if( input.empty() ) o << "{}";
  else
  {

    o << "[" << input.size() << "] { " << std::endl;
    for( const auto& i:input )
    { o << "  " << i << std::endl; }
    o << "}";
  }
  return o;
}

//! specialized streamer for runnumbers
template<>
  std::ostream& operator << (std::ostream& o, const std::set<int>& input )
{
  static constexpr int maxcount = 10;
  if( input.empty() ) o << "{}";
  else
  {

    o << "[" << input.size() << "] { ";

    if( input.size() >= maxcount ) { std::cout << endl; }

    int counter = 0;
    for( const auto& i:input )
    {

      if( counter > 0 ) { o << ", "; }
      o << i;

      if( ++counter == maxcount )
      {
        counter =0;
        o << std::endl;
      }
    }
    o << " }";
  }
  return o;
}

// verbosity
static constexpr int verbosity = 0;

namespace DBUtils
{

  //! database connection
  std::unique_ptr<odbc::Connection> dbConnection;

  //! connect to daq database
  bool connect_db()
  {
    try
    {
      dbConnection.reset( odbc::DriverManager::getConnection("daq", "", "") );
    }
    catch (odbc::SQLException &e)
    {
      std::cerr << "connect_db - Database connection failed: " << e.getMessage() << std::endl;
      return false;
    }
    return true;
  }

  //_________________________________________________________
  //! list all runnumbers from DB after a given timestamp
  runnumber_set_t get_runnumbers_from_db( const std::string& timestamp, const std::string& runtype = default_runtype )
  {
    if( !(dbConnection || connect_db()) )
    {
      std::cout << "get_runnumbers_from_db - cannot connect to db" << std::endl;
      return {};
    }

    std::unique_ptr<odbc::Statement> stmt( dbConnection->createStatement() );

    // get run of type runtype, with length > 5 minutes and found in DB since timestamp
    const std::string sql =
      "SELECT runnumber "
      "FROM run "
      "WHERE runtype='"+runtype+"' "
      "AND EXTRACT(EPOCH FROM (ertimestamp - brtimestamp)) > 300 " // five minute runs
      "AND brtimestamp>='"+timestamp+"';";

    runnumber_set_t runnumbers;
    std::unique_ptr<odbc::ResultSet> resultSet( stmt->executeQuery(sql) );
    while( resultSet && resultSet->next() )
    {
      const auto runnumber = resultSet->getInt("runnumber");
      if( runnumber > 0 ) { runnumbers.insert(runnumber); }
    }

    return runnumbers;
  }

  //_________________________________________________________
  //! list all files matching selection from daq database
  fileinfo_set_t get_files_from_db( const std::string& selection )
  {
    if( !(dbConnection || connect_db()) )
    {
      std::cout << "get_files_from_db - cannot connect to db" << std::endl;
      return {};
    }

    fileinfo_set_t out;
    std::unique_ptr<odbc::Statement> stmt( dbConnection->createStatement() );

    // create request
    const std::string sql =
      "SELECT run.runnumber, filename, transferred_to_hpss,transferred_to_sdcc "
      "FROM run,filelist "
      "WHERE run.runnumber=filelist.runnumber "
      "AND " + selection + ";";

    // request
    std::unique_ptr<odbc::ResultSet> resultSet( stmt->executeQuery(sql) );
    while( resultSet && resultSet->next() )
    {
      out.insert( {
        .filename=resultSet->getString("filename"),
          .in_hpss= resultSet->getBoolean("transferred_to_hpss"),
          .in_sdcc= resultSet->getBoolean("transferred_to_sdcc") } );
    }
    return out;
  }

  //_________________________________________________________
  //! list all files matching run number runtype and subsystem from daq database
  fileinfo_set_t get_files_from_db(
    const runnumber_set_t runnumbers = {},
    const subsystem_info_t& subsystem = default_subsystem,
    const std::string& runtype = default_runtype
    )
  {
    fileinfo_set_t out;

    if( runnumbers.empty() )
    {

      // create selection, adding host, runtype, and selecting only run3 Au-Au runs
      const std::string selection =
        "filename LIKE '%' || '"+subsystem.host+"' || '%' "
        "AND runtype='"+runtype+"' "
        "AND brtimestamp>'2025-01-01'";

      out.merge( get_files_from_db(selection) );

    } else {

      // create selection, adding host, runtype, and selecting only run3 Au-Au runs
      for( const auto& runnumber:runnumbers )
      {
        const std::string selection =
          "run.runnumber = "+std::to_string(runnumber)+
          " AND filename LIKE '%' || '"+subsystem.host+"' || '%';";
        out.merge( get_files_from_db(selection) );
      }

    }

    return out;
  }
}

namespace Utils
{

  //_________________________________________________________
  std::pair<int,int> get_run_segment( const std::string& filename )
  {
    // get segment number from filename
    std::regex regex("-0*(\\d+)-(\\d+)\\.(evt|prdf)");
    std::smatch match;
    if( std::regex_search(filename, match, regex) )
    {
      return std::make_pair(std::stoi(match[1]),std::stoi(match[2]));
    } else {
      return {-1,-1};
    }
  }

  //_________________________________________________________
  int get_runnumber( const std::string& filename )
  { return get_run_segment(filename).first; }

  //_________________________________________________________
  int get_segment( const std::string& filename )
  { return get_run_segment(filename).second; }

  //_________________________________________________________
  // get raw data file extension for a given subsystem
  std::string get_extension( const subsystem_info_t& subsystem = default_subsystem)
  {
    if( subsystem.subsystem == "TPC" || subsystem.subsystem == "TPOT" || subsystem.subsystem == "INTT" || subsystem.subsystem == "MVTX" )
    {
      return ".evt";
    } else {
      return ".prdf";
    }
  }

  //_________________________________________________________
  // get raw data base filename for a given subsystem
  std::string get_basefilename( const subsystem_info_t& subsystem = default_subsystem, const std::string& runtype = default_runtype )
  {
    // base filename depends on the subsystem type unfortunately
    if( subsystem.subsystem == "TPC" || subsystem.subsystem == "TPOT" )
    {
      return subsystem.subsystem+"_"+subsystem.host+"_"+runtype;
    } else if( subsystem.subsystem == "GL1" ) {
      return subsystem.subsystem+"_"+runtype+"_"+subsystem.host;
    } else {
      return runtype+"_"+subsystem.host;
    }
  }

  //_________________________________________________________
  // get sybststem from filename
  subsystem_info_t get_subsystem( const std::string& filename )
  {
    {
      // TPOT or TPC file
      // TPC_ebdc23_1_physics-00069746-0036.evt
      std::regex regex("/([A-Za-z]+)/[A-Za-z]+/([A-Za-z]+)_([A-Za-z0-9]+(:?_0|1)?)_");
      std::smatch match;
      if( std::regex_search(filename, match, regex) )
      { return {match[1], match[2], match[3]}; }
    }

    {
      // GL1
      // /GL1/physics/GL1_physics_gl1daq-00069746-0000.evt
      std::regex regex("/GL1/[A-Za-z]+/GL1_[A-Za-z]+_([A-Za-z0-9]+)-");
      std::smatch match;
      if( std::regex_search(filename, match, regex) )
      { return {"GL1", "GL1", match[1]}; }
    }

    {
      // all other subystems
      std::regex regex("/([A-Za-z]+)/[A-Za-z]+/[A-Za-z]+_([A-Za-z0-9]+)-");
      std::smatch match;
      if( std::regex_search(filename, match, regex) )
      { return {match[1], boost::to_upper_copy<std::string>(match[1]), match[2]}; }
    }

    return {};
  }

  //_________________________________________________________
  //! list files from file descripor
  filename_set_t read_files( const std::string& command )
  {
    filename_set_t out;
    char line[512];
    auto tmp = popen(command.c_str(), "r");
    while( fgets( line, 512, tmp ) )
    {
      std::string filename;
      istringstream(line) >> filename;
      out.insert(filename);
    }

    // close on exit
    pclose(tmp);

    return out;
  }

  //_________________________________________________________
  // get the 'local' filenames (with path removed) from input file
  std::string get_local_filename( const std::string& source )
  { return  std::filesystem::path(source).filename().string(); }

  //_________________________________________________________
  // get the 'local' filenames (with path removed) from fileinfo_t
  std::string get_local_filename( const fileinfo_t& source )
  { return  std::filesystem::path(source.filename).filename().string(); }

  //_________________________________________________________
  // get the 'local' filenames (with path removed) from input list
  template< class T>
  filename_set_t get_local_filenames( const std::set<T>& in )
  {
    filename_set_t out;
    std::transform( in.begin(), in.end(), std::inserter(out, out.end() ),get_local_filename);
    return out;
  }

  //_________________________________________________________
  // get runnumbers from input list
  runnumber_set_t get_runnumbers( const filename_set_t& in )
  {

    runnumber_set_t out;
    std::transform( in.begin(), in.end(), std::inserter(out, out.end() ),
      [](const std::string& filename )
    { return get_run_segment(filename).first; });

    return out;
  }

  //_________________________________________________________
  /*
  * generate the list of expected local filenames for a given runumber, subsystem and run type, up to a given segment number
  * this is used to find holes in the recorded file lists
  */
  filename_set_t get_expected_filenames( int runnumber, int max_segment, const subsystem_info_t& subsystem = default_subsystem, const std::string& runtype = default_runtype )
  {
    filename_set_t out;
    for( int segment=0; segment < max_segment; ++segment )
    {
      if( subsystem.subsystem == "TPC" || subsystem.subsystem == "TPOT" ) {
        out.insert(subsystem.subsystem+"_"+subsystem.host+"_"+runtype+(boost::format("-%08i-%04i.evt")%runnumber%segment).str());
      } else if( subsystem.subsystem == "GL1" ) {
        out.insert(subsystem.subsystem+"_"+runtype+"_"+subsystem.host+(boost::format("-%08i-%04i.evt")%runnumber%segment).str());
      } else {
        out.insert(runtype+"_"+subsystem.host+(boost::format("-%08i-%04i%s")%runnumber%segment%get_extension(subsystem)).str());
      }
    }

    return out;
  }

  //_________________________________________________________
  /*
   * Get full path on lustre from local filename, subsystem and run type
   */
  std::string get_lustre_filename( const std::string& local_filename,  const subsystem_info_t& subsystem = default_subsystem, const std::string& runtype = default_runtype )
  {
    static const std::string lustre_path( "/sphenix/lustre01/sphnxpro/physics/" );
    return lustre_path+subsystem.path+"/"+runtype+"/"+local_filename;
  }

  //_________________________________________________________
  //! list all files matching run numbers from bufferbox
  filename_set_t get_files_from_bbox(
    const runnumber_set_t runnumbers = {},
    const subsystem_info_t& subsystem = default_subsystem,
    const std::string& runtype = default_runtype
    )
  {
    std::cout << "get_files_from_bbox" << std::endl;
    filename_set_t out;

    // generate ssh command
    const std::string ssh_command = "ssh -Jcssh01.sdcc.bnl.gov sphnxpro@bbox0.sphenix.bnl.gov";

    // generate path
    const std::string path = "/bbox/bbox*/{a,b}/"+subsystem.path+"/"+runtype+"/";
    if( runnumbers.empty() )
    {

      const std::string file_selection_pattern = path+"/"+get_basefilename(subsystem,runtype)+"-*-*"+get_extension(subsystem);
      const std::string command = ssh_command+" -x 'ls "+file_selection_pattern+"'";
      out.merge(read_files(command));

    } else {

      for( const auto& runnumber:runnumbers )
      {
        const std::string file_selection_pattern = path+"/"+get_basefilename(subsystem,runtype)+(boost::format("-%08i-*%s")%runnumber%get_extension(subsystem).c_str()).str();
        const std::string command = ssh_command+" -x 'ls " + file_selection_pattern + "'";
        out.merge(read_files(command));
      }

    }

    return out;
  }

  //_________________________________________________________
  //! list all files matching run numbers from bufferbox
  filename_set_t get_files_from_lustre(
    const runnumber_set_t runnumbers = {},
    const subsystem_info_t& subsystem = default_subsystem,
    const std::string& runtype = default_runtype
    )
  {
    filename_set_t out;

    // generate path
    const std::string path = "/sphenix/lustre01/sphnxpro/physics/"+subsystem.path+"/"+runtype+"/";
    if( runnumbers.empty() )
    {

      const std::string file_selection_pattern = path+"/"+get_basefilename(subsystem,runtype)+"-*-*"+get_extension(subsystem);
      const std::string command = "ls " + file_selection_pattern;
      out.merge(read_files(command));

    } else {

      for( const auto& runnumber:runnumbers )
      {
        const std::string file_selection_pattern = path+"/"+get_basefilename(subsystem,runtype)+(boost::format("-%08i-*%s")%runnumber%get_extension(subsystem).c_str()).str();

        if( verbosity )
        { std::cout << "get_files_from_lustre - file_selection_pattern: " << file_selection_pattern << std::endl; }

        const std::string command = "ls " + file_selection_pattern;
        out.merge(read_files(command));
      }

    }

    return out;
  }

}

#endif
