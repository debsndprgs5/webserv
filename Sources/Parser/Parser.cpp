#include "../../Includes/Parser.hpp"
#include "../../Includes/Process.hpp"
// ------------------------------------------------------------------
// Main  && parsing entry point
// ------------------------------------------------------------------

void printLocation(const LocationConfig &loc, int indent = 0)
{
    // make an indent string to keep track of the layer we're in
    std::string indentStr(indent, ' ');
    std::cout << indentStr << "Location Match: " << loc._location_match << "\n";
    std::cout << indentStr << "Path: " << loc._path << "\n";
    std::cout << indentStr << "Root: " << loc._root << "\n";
    std::cout << indentStr << "Name: " << loc._name << "\n";
    std::cout << indentStr << "Auto Index: " << (loc._auto_index ? "true" : "false") << "\n";
    std::cout << indentStr << "alias: " << loc._alias << "\n";
    if (!loc._methods.empty()) {
        std::cout << indentStr << "Allowed Methods: ";
        for (size_t i = 0; i < loc._methods.size(); ++i)
            std::cout << loc._methods[i] << " ";
        std::cout << "\n";
    }
    // add other member vars for debug if needed

    // Check if the loc has nested locations
    if (!loc._nested_locations.empty()) {
        std::cout << indentStr << "Nested Locations:\n";
        // For each nested location you call the function back recursively and add 4 spaces to indentstr 
        for (size_t i = 0; i < loc._nested_locations.size(); ++i) {
            printLocation(loc._nested_locations[i], indent + 4);
        }
    }
}

// Print all nested locations -> entrypoint for recursive ft
void printAllLocations(const std::vector<LocationConfig>& locations, int indent = 0)
{
    for (size_t i = 0; i < locations.size(); ++i)
        printLocation(locations[i], indent);
}


int main(int argc, char **argv)
{
	if(argc != 2)
		ExitWithMessage(1, "Wrong Numbers of Arguments, only one config file");

    std::ifstream infile(argv[1]);
    if (!infile) {
        std::cerr << "Error: config file not found" << std::endl;
        return 1;
    }

    std::vector<ServerConfig> servers;
    HttpConfig httpConfig;
    std::string line;

    while (std::getline(infile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        // Detect server block.
        if (line.find("server") == 0) {
            // Check for opening brace.
            if (line.find("{") == std::string::npos) {
                if (std::getline(infile, line)) {
                    if (trim(line) != "{") {
                        std::cerr << "Warning: Expected opening brace '{' after server, block ignored." << std::endl;
                        continue;
                    }
                } else {
                    std::cerr << "Warning: Unexpected EOF after server." << std::endl;
                    break;
                }
            }
            ServerConfig server = parseServer(infile);
            servers.push_back(server);
        }
        // Detect http block.
        else if (line.find("http") == 0) {
            if (line.find("{") == std::string::npos) {
                if (std::getline(infile, line)) {
                    if (trim(line) != "{") {
                        std::cerr << "Warning: Expected opening brace '{' after http, block ignored." << std::endl;
                        continue;
                    }
                } else {
                    std::cerr << "Warning: Unexpected EOF after http." << std::endl;
                    break;
                }
            }
            httpConfig = parseHttp(infile);
        }
        // Other lines are ignored.
        else {
            std::cerr << "Warning: Unknown block or extra line ignored: " << line << std::endl;
        }
    }

    // // Output configuration for verification.
    // std::cout << "Number of servers found: " << servers.size() << "\n";
    // for (size_t i = 0; i < servers.size(); ++i) {
    //     std::cout << "Server " << i + 1 << ":\n";
    //     std::cout << "  server_name: ";
    //     for (std::vector<std::string>::iterator it = servers[i]._server_name.begin(); it != servers[i]._server_name.end(); ++it)
    //         std::cout << *it << " ";
    //     std::cout << "\n";
    //     std::cout << "  listen ports: ";
    //     for (std::vector<int>::iterator it = servers[i]._listen.begin(); it != servers[i]._listen.end(); ++it)
    //         std::cout << *it << " ";
    //     std::cout << "  Ip address: " << " | " << servers[i]._ipAdr<< std::endl;
    //     std::cout << "\n";
    //     std::cout << "  root: " << servers[i]._root << "\n";
    //     std::cout << "  client_max_body_size: " << servers[i]._client_max_body_size << "\n";
    //     std::cout << "  Number of locations: " << servers[i]._location.size() << "\n";
    //     printAllLocations(servers[i]._location);
    //     std::cout << "\n";
    // }

    // std::cout << "HTTP Configuration:\n";
    // std::cout << "  index: ";
    // for (std::vector<std::string>::iterator it = httpConfig._index.begin(); it != httpConfig._index.end(); ++it)
    //     std::cout << *it << " ";
    // std::cout << "\n";
    // std::cout << "  client_max_body_size: " << httpConfig._client_max_body_size << "\n";


	//Start of processing
		Process Proc;
		if(Proc.start(servers)){
			return 1;
		}
		Proc.mainLoop();
	
    return 0;
}

