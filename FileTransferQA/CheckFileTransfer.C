#include <odbc++/connection.h>
#include <odbc++/drivermanager.h>
#include <odbc++/resultset.h>
#include <odbc++/statement.h>

#include <algorithm>
#include <filesystem>
#include <set>
#include <string>

#include <boost/format.hpp>

R__LOAD_LIBRARY(libodbc++.so)

namespace
{

  //! subsystem characterization, used for locating files on disk, db, etc.
  struct subsystem_info_t
  {
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

  //! streamer
  std::ostream& operator << (std::ostream& o, const subsystem_info_t& subsystem )
  {
    o << "{" << subsystem.path << ", " << subsystem.subsystem << ", " << subsystem.host << "}";
    return o;
  }

  // hardcoded list of all tracking subsystems
  static const subsystem_info_t::list subsystems = {
    // TPC subsystems
    {"tpc", "TPC", "ebdc00" },
    {"tpc", "TPC", "ebdc01" },
    {"tpc", "TPC", "ebdc02" },
    {"tpc", "TPC", "ebdc03" },
    {"tpc", "TPC", "ebdc04" },
    {"tpc", "TPC", "ebdc05" },
    {"tpc", "TPC", "ebdc06" },
    {"tpc", "TPC", "ebdc07" },
    {"tpc", "TPC", "ebdc08" },
    {"tpc", "TPC", "ebdc09" },
    {"tpc", "TPC", "ebdc10" },
    {"tpc", "TPC", "ebdc11" },
    {"tpc", "TPC", "ebdc12" },
    {"tpc", "TPC", "ebdc13" },
    {"tpc", "TPC", "ebdc14" },
    {"tpc", "TPC", "ebdc15" },
    {"tpc", "TPC", "ebdc16" },
    {"tpc", "TPC", "ebdc17" },
    {"tpc", "TPC", "ebdc18" },
    {"tpc", "TPC", "ebdc19" },
    {"tpc", "TPC", "ebdc20" },
    {"tpc", "TPC", "ebdc21" },
    {"tpc", "TPC", "ebdc22" },
    {"tpc", "TPC", "ebdc23" },
    {"tpc", "TPC", "ebdc23" },

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
    {"MVTX", "MVTX", "mvtx5" }
  };

  //! file information (from DB)
  struct fileinfo_t
  {
    std::string filename;
    bool in_hpss = false;
    bool in_sdcc = false;
  };

  //! strong ordering
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

  //! streamer
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

    //! streamer
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

  //! database connection
  std::unique_ptr<odbc::Connection> dbConnection;

  // verbosity
  static constexpr int verbosity = 0;

  // files missing from database
  filename_set_t files_missing_from_db;

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
std::pair<int,int> get_run_segment( const std::string& filename )
{
  // get segment number from filename
  std::regex regex("-0*(\\d+)-(\\d+)\\.evt");
  std::smatch match;
  if( std::regex_search(filename, match, regex) )
  {
    return std::make_pair(std::stoi(match[1]),std::stoi(match[2]));
  } else {
    return {-1,-1};
  }
}

//_________________________________________________________
std::string get_basefilename( const subsystem_info_t& subsystem = default_subsystem, const std::string& runtype = default_runtype )
{
  // base filename depends on the subsystem type unfortunately
  if( subsystem.subsystem == "TPC" || subsystem.subsystem == "TPOT" )
  {
    return subsystem.subsystem+"_"+subsystem.host+"*_"+runtype;
  } else if( subsystem.subsystem == "MVTX" || subsystem.subsystem == "INTT" ) {
    return runtype+"_"+subsystem.host;
  } else {
    return {};
  }
}

//_________________________________________________________
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

    const std::string file_selection_pattern = path+"/"+get_basefilename(subsystem,runtype)+"-*-*.evt";
    const std::string command = ssh_command+" -x 'ls "+file_selection_pattern+"'";
    out.merge(read_files(command));

  } else {

    for( const auto& runnumber:runnumbers )
    {
      const std::string file_selection_pattern = path+"/"+get_basefilename(subsystem,runtype)+(boost::format("-%08i-*.evt")%runnumber).str();
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

    const std::string file_selection_pattern = path+"/"+get_basefilename(subsystem,runtype)+"-*-*.evt";
    const std::string command = "ls " + file_selection_pattern;
    out.merge(read_files(command));

  } else {

    for( const auto& runnumber:runnumbers )
    {
      const std::string file_selection_pattern = path+"/"+get_basefilename(subsystem,runtype)+(boost::format("-%08i-*.evt")%runnumber).str();

      if( verbosity )
      { std::cout << "get_files_from_lustre - file_selection_pattern: " << file_selection_pattern << std::endl; }

      const std::string command = "ls " + file_selection_pattern;
      out.merge(read_files(command));
    }

  }

  return out;
}

//_________________________________________
bool connect_db()
{
 try
  {
    dbConnection.reset( odbc::DriverManager::getConnection("daq", "", "") );
  }
  catch (odbc::SQLException &e)
  {
    std::cerr << "test_db_connection - Database connection failed: " << e.getMessage() << std::endl;
    return false;
  }
  return true;
}


//_________________________________________________________
//! list all runnumbers from DB after a given day
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
//! list all files matching run numbers from daq database
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
      "hostname LIKE '%' || '"+subsystem.host+"' || '%' "
      "AND runtype='"+runtype+"' "
      "AND brtimestamp>'2025-01-01'";

    out.merge( get_files_from_db(selection) );

  } else {

    // create selection, adding host, runtype, and selecting only run3 Au-Au runs
    for( const auto& runnumber:runnumbers )
    {
      const std::string selection =
        "run.runnumber = "+std::to_string(runnumber)+
        " AND hostname LIKE '%' || '"+subsystem.host+"' || '%';";
      out.merge( get_files_from_db(selection) );
    }

  }

  return out;
}


//_________________________________________________________
// get the 'local' filenames (with path removed) from input list
std::string get_local_filename( const std::string& source )
{ return  std::filesystem::path(source).filename().string(); }

//_________________________________________________________
// get the 'local' filenames (with path removed) from input list
filename_set_t get_local_filenames( const filename_set_t& in )
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
void CheckFileTransfer_from_bbox(
  const runnumber_set_t runnumbers = {},
  const subsystem_info_t& subsystem = default_subsystem,
  const std::string& runtype = default_runtype
)
{

  std::cout << "CheckFileTransfer_from_bbox - subsystem: " << subsystem << std::endl;

  // list of files on buffer boxes and check if transferd
  const auto bbox_files = get_files_from_bbox( runnumbers, subsystem, runtype );

  if( true )
  { std::cout << "CheckFileTransfer_from_bbox - files in bufferbox: " << bbox_files << std::endl; }

  // get corresponding set of runnumbers
  const auto bbox_runnumbers = get_runnumbers(bbox_files);

  // get files from daq database
  const auto daqdb_files = get_files_from_db(bbox_runnumbers, subsystem, runtype);

  if( true )
  { std::cout << "CheckFileTransfer_from_bbox - files in db: " << daqdb_files << std::endl; }

  if( true )
  {
    // compare bbox and database
    filename_set_t found_files;
    filename_set_t missing_files;

    // check transfer status
    for( const auto& source:bbox_files )
    {
      const bool found = std::find_if(daqdb_files.begin(), daqdb_files.end(),
        [&source](const fileinfo_t& fileinfo ) { return fileinfo.filename == source; })
        != daqdb_files.end();

      if( found ) found_files.insert(source);
      else missing_files.insert(source);
    }

    std::cout << "CheckFileTransfer_from_bbox - missing from db: " << missing_files << std::endl;
    files_missing_from_db.merge(missing_files);
  }

  if( true )
  {
    // check whether files have been transfered or not
    // get corresponding lustre files
    const auto lustre_files = get_files_from_lustre(bbox_runnumbers);
    const auto lustre_local_files = get_local_filenames(lustre_files);

    filename_set_t transfered_files;
    filename_set_t missing_files;
    filename_set_t inconsistent_files;

    // check transfer status
    for( const auto& source:bbox_files )
    {
      const bool transfered = (lustre_local_files.find(get_local_filename(source))!=lustre_local_files.end());
      if( transfered ) transfered_files.insert(source);
      else
      {
        missing_files.insert(source);

        // find matching daq_db file info
        const auto iter = std::find_if(daqdb_files.begin(), daqdb_files.end(),
          [&source](const fileinfo_t& fileinfo ) { return fileinfo.filename == source; });
        if( iter != daqdb_files.end() && iter->in_sdcc )
        { inconsistent_files.insert(source); }
      }
    }

    std::cout << "CheckFileTransfer_from_bbox - missing from sdcc: " << missing_files << std::endl;
    std::cout << "CheckFileTransfer_from_bbox - missing and inconsistent with db: " << inconsistent_files << std::endl;

  }
}
//
// start_date
// date should look like
// XXXX-XX-XX
// as in 2025-07-21
// first_segment_only=true - check only if the 0th segment is transferred
//
//_________________________________________________________
void CheckFileTransfer(const std::string& date,
		       const bool first_segment_only = false)
{

  // get list of runs
  static const std::string start_date = date;
  const auto runnumbers = get_runnumbers_from_db( start_date );

  // map runnumber with missing subsystems
  std::map<int, subsystem_info_t::list> missing;
  std::map<int, subsystem_info_t::list> missing_first_segment;


  // set to true to verify if transfered files are effectively on disk
  const bool check_db_consistency = false;
  fileinfo_set_t inconsistent_files;

  // loop over runnumbers
  for( const auto runnumber: runnumbers )
  {

    // if( verbosity )
    { std::cout << "runnumber: " << runnumber << std::endl; }

    // loop over subsystems
    for( const auto& subsystem:subsystems )
    {
      // get the files from database
      const auto& daqdb_files = get_files_from_db( {runnumber}, subsystem );

      if( verbosity )
      { std::cout << "subsystem: " << subsystem << " files: " << daqdb_files << std::endl; }

      // loop over files check if requested segments have been transfered
      bool transferred = true;
      bool transferred_first_segment = true;
      for( const auto& file_info:daqdb_files )
      {
        const bool is_first_segment = (get_run_segment(file_info.filename).second == 0);
        if( first_segment_only && !is_first_segment ) { continue; }

        if( check_db_consistency && file_info.in_sdcc )
        {
          const auto lustre_filename = get_lustre_filename( get_local_filename(file_info.filename), subsystem );
          const bool consistent = filesystem::exists(lustre_filename);
          if( !consistent ) inconsistent_files.insert( file_info );
        }

        // check sdcc transfer status
        if( !file_info.in_sdcc )
        {
          transferred = false;
          if( is_first_segment )
          {
            transferred_first_segment = false;
            if( !check_db_consistency ) { break; }
          }
        }
      }

      if(!transferred) { missing[runnumber].emplace_back(subsystem); }
      if(!transferred_first_segment) { missing_first_segment[runnumber].emplace_back(subsystem); }
    }
  }

  // print summary
  std::cout << std::endl;
  std::cout << "start date: " << start_date << std::endl << std::endl;
  std::cout << "runnumbers: " << runnumbers << std::endl << std::endl;

  // get list of fully transfered runs
  if( !first_segment_only )
  {
    runnumber_set_t complete_runs;
    std::copy_if( runnumbers.begin(), runnumbers.end(), std::inserter(complete_runs, complete_runs.end()),
      [&missing]( const int& runnumber ) { return !missing.contains(runnumber); } );
    std::cout << "Complete runs (all segments): " << complete_runs << std::endl << std::endl;

    // get list of incomplete runs
    runnumber_set_t incomplete_runs;
    std::copy_if( runnumbers.begin(), runnumbers.end(), std::inserter(incomplete_runs, incomplete_runs.end()),
      [&missing]( const int& runnumber ) { return missing.contains(runnumber); } );
    std::cout << "incomplete runs (all segments): " << incomplete_runs << std::endl << std::endl;
  }

  // get list of fully transfered runs (first segment)
  runnumber_set_t complete_runs_first_segment;
  std::copy_if( runnumbers.begin(), runnumbers.end(), std::inserter(complete_runs_first_segment, complete_runs_first_segment.end()),
    [&missing_first_segment]( const int& runnumber ) { return !missing_first_segment.contains(runnumber); } );
  std::cout << "Complete runs (first segment): " << complete_runs_first_segment << std::endl << std::endl;

  // get list of incomplete runs (first segment)
  runnumber_set_t incomplete_runs_first_segment;
  std::copy_if( runnumbers.begin(), runnumbers.end(), std::inserter(incomplete_runs_first_segment, incomplete_runs_first_segment.end()),
    [&missing_first_segment]( const int& runnumber ) { return missing_first_segment.contains(runnumber); } );
  std::cout << "incomplete runs (first segment): " << incomplete_runs_first_segment << std::endl << std::endl;

  // print subsystems for which segments are missing, for each run
  if( verbosity )
  {
    std::cout << "subsystem for which there are missing segments: " << std::endl << std::endl;
    for( const auto& [runnumber, subsystems]:missing )
    {
      std::cout << "runnumber: " << runnumber << " [";
      for( const auto& subsystem:subsystems)
      { std::cout << " " << subsystem.host; };
      std::cout << "]" << std::endl;
    }
  }

  // print subsystems for which segments are missing, for each run
  if( verbosity )
  {
    std::cout << "subsystem for which there are missing first segments: " << std::endl << std::endl;
    for( const auto& [runnumber, subsystems]:missing_first_segment )
    {
      std::cout << "runnumber: " << runnumber << " [";
      for( const auto& subsystem:subsystems)
      { std::cout << " " << subsystem.host; };
      std::cout << "]" << std::endl;
    }
  }

  if( check_db_consistency )
  { std::cout << "files marked as transfered but not found on lustre: " << inconsistent_files << std::endl << std::endl; }

}
