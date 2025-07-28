#include "FileTransferUtils.C"

R__LOAD_LIBRARY(libodbc++.so)

namespace RawDataCatalog
{

  //! database connection
  std::unique_ptr<odbc::Connection> dbConnection;

  //! connect to daq database
  bool connect_db()
  {
    try
    {
      // dbConnection.reset( odbc::DriverManager::getConnection("daq", "", "") );
      dbConnection.reset( odbc::DriverManager::getConnection("RawdataCatalog", "sphnxhpssdbmaster", "") );
    }
    catch (odbc::SQLException &e)
    {
      std::cerr << "connect_db - Database connection failed: " << e.getMessage() << std::endl;
      return false;
    }
    return true;
  }

}

// start_date
// date should look like
// XXXX-XX-XX
// as in 2025-07-21
//
//_________________________________________________________
void CheckMissingFiles(const std::string& start_date="2025-07-21")
{
  // get list of runs
  const auto runnumbers = DBUtils::get_runnumbers_from_db( start_date );
  // const runnumber_set_t runnumbers = {69746};

  // files missing from DB
  using filename_map_t = std::map<subsystem_info_t, filename_set_t>;
  filename_map_t missing_files_from_db;

  RawDataCatalog::connect_db();
  return;
  // loop over runnumbers
  for( const auto runnumber: runnumbers )
  {
    std::cout << "runnumber: " << runnumber << std::endl;

    // loop over subsystems
    for( size_t i = 0; i<default_subsystems.size(); ++i )
    {
      // get subsystem
      const auto& subsystem = default_subsystems[i];

      // get the files from database
      const auto daqdb_files = DBUtils::get_files_from_db( {runnumber}, subsystem );

      if( verbosity )
      { std::cout << "subsystem: " << subsystem << " files: " << daqdb_files << std::endl << std::endl; }

      // do nothing if daqdb_files is empty
      if( daqdb_files.empty() ) continue;

      // get max segment and expected filenames
      const auto max_segment = Utils::get_segment( std::max_element( daqdb_files.begin(), daqdb_files.end(),
        []( const fileinfo_t& first, const fileinfo_t& second )
        { return Utils::get_segment(first.filename) < Utils::get_segment(second.filename); } )->filename );
      const auto expected_filenames = Utils::get_expected_filenames( runnumber, max_segment+1, subsystem );

      // look for missing filenames in the database by comparing expected to daqdb files
      for( const auto& filename:expected_filenames)
      {
        if( std::find_if(daqdb_files.begin(),daqdb_files.end(), [&filename](const fileinfo_t& fileinfo)
          { return Utils::get_local_filename(fileinfo.filename)==filename; } ) == daqdb_files.end())
        { missing_files_from_db[subsystem].insert( filename ); }
      }

    }

  }

  // print summary
  std::cout << std::endl;
  std::cout << "start date: " << start_date << std::endl << std::endl;

  // print files not found in the database
  for( const auto& [subsystem, filenames]:missing_files_from_db )
  {
    std::cout << "syubsystem: " << subsystem << " files missing from db: " << filenames << std::endl;

    // loop over files, see if they exist at SDCC
    for( const auto& filename:filenames )
    {
      const auto lustre_filename = Utils::get_lustre_filename( filename, subsystem );
      const bool file_exists = filesystem::exists(lustre_filename);
      std::cout << "lustre_filename: " << lustre_filename << " esists: " << file_exists << std::endl;
    }
  }

}
