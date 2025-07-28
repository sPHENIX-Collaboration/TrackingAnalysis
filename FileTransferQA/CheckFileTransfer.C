#include "FileTransferUtils.C"

R__LOAD_LIBRARY(libodbc++.so)

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

// start_date
// date should look like
// XXXX-XX-XX
// as in 2025-07-21
//
//_________________________________________________________
void CheckFileTransfer(const std::string& start_date="2025-07-21")
{
  // get list of runs
  const auto runnumbers = DBUtils::get_runnumbers_from_db( start_date );
  // const runnumber_set_t runnumbers = {69746};

  // map runnumber with missing subsystems
  std::map<int, subsystem_info_t::list> missing;
  std::map<int, subsystem_info_t::list> missing_first_segment;

  // runnumbers with completes DB
  runnumber_set_t incomplete_db_runs;
  runnumber_set_t complete_db_runs;

  // set to true to produce QA page
  const bool generate_qa_page = true;

  // set to true to verify if transfered files are effectively on disk
  const bool check_db_consistency = false;
  fileinfo_set_t inconsistent_files;

  // files missing from DB
  filename_set_t missing_files_from_db;

  // loop over runnumbers
  for( const auto runnumber: runnumbers )
  {
    if( verbosity )
    { std::cout << "runnumber: " << runnumber << std::endl; }

    // create histograms
    std::unique_ptr<TH1> h_expected(create_subsytems_histogram( "h_expected", "total number of segments", default_subsystems ) );
    std::unique_ptr<TH1> h_ref(create_subsytems_histogram( "h_ref", "total number of segments", default_subsystems ) );
    std::unique_ptr<TH1> h_transfered(create_subsytems_histogram( "h_transfered", "transfered segments", default_subsystems ) );
    std::unique_ptr<TH1> h_transfered_first_segment(create_subsytems_histogram( "h_first_segment_transfered", "first segment transfered", default_subsystems ) );

    // counters
    unsigned int n_segments_expected = 0;
    unsigned int n_first_segment_expected = 0;

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
      const auto daqdb_files = DBUtils::get_files_from_db( {runnumber}, subsystem );
      h_ref->Fill(i, daqdb_files.size());

      if( verbosity )
      { std::cout << "subsystem: " << subsystem << " files: " << daqdb_files << std::endl << std::endl; }

      // do nothing if daqdb_files is empty
      if( daqdb_files.empty() ) continue;

      // get max segment and expected filenames
      const auto max_segment = Utils::get_segment( std::max_element( daqdb_files.begin(), daqdb_files.end(),
        []( const fileinfo_t& first, const fileinfo_t& second )
        { return Utils::get_segment(first.filename) < Utils::get_segment(second.filename); } )->filename );
      const auto expected_filenames = Utils::get_expected_filenames( runnumber, max_segment+1, subsystem );

      n_segments_expected += expected_filenames.size();
      n_first_segment_expected += std::count_if( expected_filenames.begin(), expected_filenames.end(),
        []( const std::string& filename ) { return Utils::get_segment(filename)==0; } );

      h_expected->Fill(i, expected_filenames.size());

      // print
      if( verbosity )
      { std::cout << "subsystem: " << subsystem << " expected files: " << expected_filenames << std::endl << std::endl; }

      // look for missing filenames in the database by comparing expected to daqdb files
      std::copy_if( expected_filenames.begin(), expected_filenames.end(), std::inserter(missing_files_from_db,missing_files_from_db.end()),
        [&daqdb_files](const std::string& filename ){
          return std::find_if(daqdb_files.begin(),daqdb_files.end(), [&filename](const fileinfo_t& fileinfo)
          { return Utils::get_local_filename(fileinfo.filename)==filename; } ) == daqdb_files.end(); });

      // loop over files check if requested segments have been transfered
      bool transferred = true;
      bool transferred_first_segment = true;
      for( const auto& file_info:daqdb_files )
      {
        const bool is_first_segment = (Utils::get_segment(file_info.filename) == 0);

        // increment counters
        ++n_segments_total;
        if( is_first_segment ) { ++n_first_segment_total; }

        if( check_db_consistency && file_info.in_sdcc )
        {
          const auto lustre_filename = Utils::get_lustre_filename( Utils::get_local_filename(file_info.filename), subsystem );
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
          h_transfered->Fill(i);
          if( is_first_segment )
          { h_transfered_first_segment->Fill(i); }

          // increment counters
          ++n_segments_transfered;
          if( is_first_segment ) { ++n_first_segment_transfered; }

        }

      }

      if(!transferred) { missing[runnumber].emplace_back(subsystem); }
      if(!transferred_first_segment) { missing_first_segment[runnumber].emplace_back(subsystem); }

    }

    if( n_segments_total == n_segments_expected )
    {
      complete_db_runs.insert(runnumber);
    } else {
      incomplete_db_runs.insert(runnumber);
    }

    if( generate_qa_page )
    {
      // make canvas and save
      std::unique_ptr<TCanvas> cv( new TCanvas( "cv", "cv", 1200, 1200 ) );
      cv->Divide(1,2 );

      // adjust pad dimensions
      cv->GetPad(1)->SetPad(0, 0.3, 1, 1);
      cv->GetPad(2)->SetPad(0, 0, 1, 0.3);

      // status histogram
      cv->cd(1);
      h_expected->SetStats(0);
      h_expected->SetTitle(Form( "File transfer status for run %i", runnumber ));
      h_expected->SetFillStyle(1001);
      h_expected->SetFillColor(kYellow-10);
      h_expected->SetMinimum(0.5);
      h_expected->GetXaxis()->SetLabelSize(0.03);
      h_expected->Draw("hist");

      h_ref->SetFillStyle(1001);
      h_ref->SetFillColor(kYellow-9);
      h_ref->Draw("hist same");

      h_transfered->SetFillStyle(1001);
      h_transfered->SetFillColor(kGreen-8);
      h_transfered->Draw("hist same");

      h_transfered_first_segment->SetFillStyle(1001);
      h_transfered_first_segment->SetFillColor(kGreen-5);
      h_transfered_first_segment->Draw("hist same");

      // legend
      auto legend = new TLegend( 0.7, 0.7, 0.95, 0.85, "", "NDC" );
      legend->SetFillStyle(0);
      legend->AddEntry( h_expected.get(), "expected files", "f" );
      legend->AddEntry( h_ref.get(), "files in DB", "f" );
      legend->AddEntry( h_transfered.get(), "transfered", "f" );
      legend->AddEntry( h_transfered_first_segment.get(), "first segment transfered", "f" );
      legend->Draw();

      gPad->SetBottomMargin(0.15);
      gPad->SetLogy();

      // summary
      cv->cd(2);
      std::unique_ptr<TPaveText> text( new TPaveText(0.1,0.1,0.9,0.9, "NDC" ) );
      text->SetFillColor(0);
      text->SetFillStyle(0);
      text->SetBorderSize(0);
      text->SetTextAlign(11);

      text->AddText( "File transfer summary:" );

      if( n_segments_total == n_segments_expected )
      {
        text->AddText( Form("Number of files in db: %i, expected: %i - good",n_segments_total, n_segments_expected ))->SetTextColor(kGreen+1);
      } else {
        text->AddText( Form("Number of files in db: %i, expected: %i - bad",n_segments_total, n_segments_expected ))->SetTextColor(kRed+1);
      }

      if( n_first_segment_total == n_first_segment_expected )
      {
        text->AddText( Form("Number of first segment files in db: %i, expected: %i - good",n_first_segment_total, n_first_segment_expected ))->SetTextColor(kGreen+1);
      } else {
        text->AddText( Form("Number of first segment files in db: %i, expected: %i - bad",n_first_segment_total, n_first_segment_expected ))->SetTextColor(kRed+1);
      }


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

  }

  // print summary
  std::cout << std::endl;
  std::cout << "start date: " << start_date << std::endl << std::endl;
  std::cout << "runnumbers: " << runnumbers << std::endl << std::endl;

  // list of runs for which the db seems complete
  std::cout << "runs with complete db files: " << complete_db_runs << std::endl << std::endl;

  // list of runs for which the db seems complete
  std::cout << "runs with incomplete db files: " << incomplete_db_runs << std::endl << std::endl;

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

  // print files not found in the database
  std::cout << "files missing from db: " << missing_files_from_db << std::endl << std::endl;

  if( check_db_consistency )
  { std::cout << "files marked as transfered but not found on lustre: " << inconsistent_files << std::endl << std::endl; }

}
