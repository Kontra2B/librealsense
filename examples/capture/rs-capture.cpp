// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.

#include <chrono>
#include <iostream>
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <map>
#include "example.hpp"          // Include short list of convenience functions for rendering
#include "librealsense2/h/rs_types.h"

// Capture Example demonstrates how to
// capture depth and color video streams and render them to the screen
int main(int argc, char * argv[]) try
{
    std::map<int, std::pair<rs2_metadata_type, rs2_metadata_type>> last;
    rs2::log_to_file(RS2_LOG_SEVERITY_DEBUG, "rs-capture.log");
    // Create a simple OpenGL window for rendering:
    // window app(1280, 720, "RealSense Capture Example");

    // Declare depth colorizer for pretty visualization of depth data
    // rs2::colorizer color_map;
    // Declare rates printer for showing streaming rates of the enabled streams.
    // rs2::rates_printer printer;

    // Declare RealSense pipeline, encapsulating the actual device and sensors
    rs2::pipeline pipe;
    rs2::config cfg;

    // Start streaming with default recommended configuration
    // The default video configuration contains Depth and Color streams
    // If a device is capable to stream IMU data, both Gyro and Accelerometer are enabled by default
    cfg.enable_stream(RS2_STREAM_DEPTH);
    cfg.enable_stream(RS2_STREAM_COLOR);
    cfg.enable_stream(RS2_STREAM_INFRARED);
    // Or enable all camera streams
    // cfg.enable_all_streams();

    if (argc > 1) {
        cfg.enable_device(argv[1]);
    }
    pipe.start(cfg);

    int idx = 0;
    while (true) // Application still alive?
    {
        rs2::frameset data = pipe.wait_for_frames();
        // rs2::frameset data = pipe.wait_for_frames(4000000000).apply_filter(printer).     // Print each enabled stream frame rate
		// apply_filter(color_map);   // Find and colorize the depth data
        // std::cout << std::endl << "----- CLOCK:" << std::chrono::steady_clock::now().time_since_epoch().count() << "ns" << std::endl;
	const size_t nf = data.size();
        
	rs2_metadata_type tmin = 0.0, tmax = 0.0;
	// std::cout << std::endl << "WOJTEK: " << tmin << '/' << tmax << ' ' << sizeof(tmin);
	std::cout << std::endl << idx << '/' << nf << '.' << std::fixed << std::setprecision(3);
	data.foreach_rs([&last, &idx, &tmin, &tmax] (const rs2::frame& f) {
		static int ne = 0;
		auto fn = f.get_profile().stream_name().substr(0, 1);
		auto fi = f.get_profile().stream_index();
		auto fu = f.get_profile().unique_id();
		rs2_metadata_type ufc;
		rs2_metadata_type ft;
		// rs2_time_t ft;
		try {
			ufc = f.get_frame_metadata(RS2_FRAME_METADATA_FRAME_COUNTER);
			ft = f.get_frame_metadata(RS2_FRAME_METADATA_FRAME_TIMESTAMP);
			// ft = f.get_timestamp();
		} catch(...) { return; }
		// auto fr = f.get_frame_metadata(RS2_FRAME_METADATA_CRC);
		//char path[64];
		//sprintf(path, "S%d-%d.bin", (int)fs, (int)idx);
		std::cout << '\t' << fn << fu << ':' << ufc << '/' << ft;
		try {
			const auto& ofs = last.at(fu);
			if (!(ufc > ofs.first && ft > ofs.second)) {
				std::cout << " !!!/" << ++ne << ' ' << fn << ':' << ofs.first << '/' << ofs.second;
				if (ne > 10) exit(EXIT_FAILURE);
			}
			else if (ufc > ofs.first + 1)
				std::cout << ' ' << '+' << ufc - ofs.first -1;
			std::cout << '/' << ft - ofs.second;
		} catch(...) {}
		if (ft > tmax) tmax = ft;
		if (tmin == 0.0 || ft < tmin) tmin = ft;
		last[fu] = {ufc, ft};
                //auto fo = fopen(path, "wb+");
                //fwrite(fd, f.get_data_size(), 1, fo);
                //fclose(fo);
        });
	std::cout << '\t' << '\t' << tmin << '/' << tmax << '/' << tmax - tmin;
        idx++;

        // The show method, when applied on frameset, break it to frames and upload each frame into a gl textures
        // Each texture is displayed on different viewport according to it's stream unique id
        // app.show(data);
    }

    return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
    std::cerr << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
