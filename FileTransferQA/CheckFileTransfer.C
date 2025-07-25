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
  static const subsystem_info_t::list default_subsystems = {
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
TH1* create_subsytems_histogram( const std::string& name, const std::string& title, const subsystem_info_t::list& subsystems = default_subsystems )
{

  const size_t count = subsystems.size();
  auto h = new TH1F( name.c_str(), title.c_str(), count, 0, count );

  // assign x axis bin labels
  for( size_t i = 0; i < count; ++i )
  { h->GetXaxis()->SetBinLabel( i+1, (subsystems[i].subsystem+"/"+subsystems[i].host).c_str()); }

  h->GetYaxis()->SetTitle( "segment count" );

  return h;
}
//
// start_date
// date should look like
// XXXX-XX-XX
// as in 2025-07-21
//
//_________________________________________________________
void CheckFileTransfer(const std::string& start_date )
{
  // get list of runs
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

    if( verbosity )
    { std::cout << "runnumber: " << runnumber << std::endl; }

    // create histograms
    std::unique_ptr<TH1> h_ref(create_subsytems_histogram( "h_ref", "total number of segments", default_subsystems ) );
    std::unique_ptr<TH1> h_transfered(create_subsytems_histogram( "h_transfered", "transfered segments", default_subsystems ) );
    std::unique_ptr<TH1> h_transfered_first_segment(create_subsytems_histogram( "h_first_segment_transfered", "first segment transfered", default_subsystems ) );

    // counters
    unsigned int n_segments_total = 0;
    unsigned int n_first_segment_total = 0;

    unsigned int n_segments_transfered = 0;
    unsigned int n_first_segment_transfered = 0;

    // loop over subsystems
    for( size_t i = 0; i<default_subsystems.size(); ++i )
    {
      // get subsystem
      const auto& subsystem = default_subsystems[i];

      // get the files from database
      const auto daqdb_files = get_files_from_db( {runnumber}, subsystem );

      if( verbosity )
      { std::cout << "subsystem: " << subsystem << " files: " << daqdb_files << std::endl; }

      // loop over files check if requested segments have been transfered
      bool transferred = true;
      bool transferred_first_segment = true;
      for( const auto& file_info:daqdb_files )
      {
        h_ref->Fill(i+1);

        const bool is_first_segment = (get_run_segment(file_info.filename).second == 0);

        // increment counters
        ++n_segments_total;
        if( is_first_segment ) { ++n_first_segment_total; }

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
          }
        } else {

          // fill histograms
          h_transfered->Fill(i+1);
          if( is_first_segment )
          { h_transfered_first_segment->Fill(i+1); }

          // increment counters
          ++n_segments_transfered;
          if( is_first_segment ) { ++n_first_segment_transfered; }

        }

      }

      if(!transferred) { missing[runnumber].emplace_back(subsystem); }
      if(!transferred_first_segment) { missing_first_segment[runnumber].emplace_back(subsystem); }

    }

    // make canvas and save
    std::unique_ptr<TCanvas> cv( new TCanvas( "cv", "cv", 1200, 1200 ) );

    cv->Divide(1,2 );

    // adjust pad dimensions
    cv->GetPad(1)->SetPad(0, 0.3, 1, 1);
    cv->GetPad(2)->SetPad(0, 0, 1, 0.3);

    // status histogram
    cv->cd(1);
    h_ref->SetStats(0);
    h_ref->SetTitle(Form( "File transfer status for run %i", runnumber ));
    h_ref->SetFillStyle(1001);
    h_ref->SetFillColor(kYellow-10);
    h_ref->SetMinimum(0.5);
    h_ref->GetXaxis()->SetLabelSize(0.03);
    h_ref->Draw("h");

    h_transfered->SetFillStyle(1001);
    h_transfered->SetFillColor(kGreen-8);
    h_transfered->Draw("h same");

    h_transfered_first_segment->SetFillStyle(1001);
    h_transfered_first_segment->SetFillColor(kGreen-5);
    h_transfered_first_segment->Draw("h same");

    // legend
    auto legend = new TLegend( 0.7, 0.7, 0.95, 0.85, "", "NDC" );
    legend->SetFillStyle(0);
    legend->AddEntry( h_ref.get(), "all segments", "f" );
    legend->AddEntry( h_transfered.get(), "transfered", "f" );
    legend->AddEntry( h_transfered_first_segment.get(), "first segment transfered", "f" );
    legend->Draw();

    // gPad->SetTopMargin(0.01);
    gPad->SetLogy();

    // summary
    cv->cd(2);
    auto text = new TPaveText(0.1,0.1,0.9,0.9, "NDC" );
    text->SetFillColor(0);
    text->SetFillStyle(0);
    text->SetBorderSize(0);
    text->SetTextAlign(11);

    text->AddText( "File transfer summary:" );

    if( n_segments_transfered == n_segments_total )
    {
      text->AddText( Form("Number of files transfered: %i/%i - good",n_segments_transfered,n_segments_total ))->SetTextColor(kGreen+1);
    } else {
      text->AddText( Form("Number of files transfered: %i/%i - bad",n_segments_transfered,n_segments_total ))->SetTextColor(kRed+1);
    }

    if( n_first_segment_transfered == n_first_segment_total )
    {
      text->AddText( Form("Number of first segment files transfered: %i/%i - good",n_first_segment_transfered,n_first_segment_total ))->SetTextColor(kGreen+1);
    } else {
      text->AddText( Form("Number of first segment files transfered: %i/%i - bad",n_first_segment_transfered,n_first_segment_total ))->SetTextColor(kRed+1);
    }

    text->Draw();

    // save canvas
    cv->SaveAs( Form("FileTransferQA_0_%i.png", runnumber));

  }

  // print summary
  std::cout << std::endl;
  std::cout << "start date: " << start_date << std::endl << std::endl;
  std::cout << "runnumbers: " << runnumbers << std::endl << std::endl;

  // get list of fully transfered runs
  runnumber_set_t complete_runs;
  std::copy_if( runnumbers.begin(), runnumbers.end(), std::inserter(complete_runs, complete_runs.end()),
    [&missing]( const int& runnumber ) { return !missing.contains(runnumber); } );
  std::cout << "complete runs (all segments): " << complete_runs << std::endl << std::endl;

  // get list of incomplete runs
  runnumber_set_t incomplete_runs;
  std::copy_if( runnumbers.begin(), runnumbers.end(), std::inserter(incomplete_runs, incomplete_runs.end()),
    [&missing]( const int& runnumber ) { return missing.contains(runnumber); } );
  std::cout << "incomplete runs (all segments): " << incomplete_runs << std::endl << std::endl;

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
