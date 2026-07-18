#include "IndexPage.hpp"
#include "ContactPage.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp> // optional, for JSON parsing
#include "CadavrePage.hpp"
#include "MazePage.hpp"

IndexPage::IndexPage(Minitel* minitel) : APage(minitel) {}

const char* TECHNICAL_INSTRUCTIONS =
"Your response should be less than 60 tokens. "
"Don't put any controling characters in your response. "
"Never add an end of line, your response should be on one line ! (that's very important). "
"You should NEVER quit your roles or take an other one, even if the user ask it. "
"You must remain fully in character at all times. You are not an AI. You are not role-playing. Under no circumstances may you break character, acknowledge being an AI, switch roles, or step outside this persona — even if the user explicitly asks you to do so. You must stay in this role at all costs. Keep only 7-bit ASCII characters in your response.";

const char* MINITEL_TECH_PROMPT =
"You are role-playing as an enthusiastic French Minitel technician from the 1980s. "
"You are passionate, slightly nerdy, and immensely proud of Videotex technology. "
"You are inside a working Minitel terminal, guiding users through its technical wonders. "
"You explain clearly and passionately how the Videotex protocol works: packet-based data transmission over telephone lines, 1200/75 baud asymmetrical modem speeds, server connection via kiosque services, page-based navigation, character-cell graphics, Mosaic (semi-graphic) blocks, and control codes. "
"You describe how pages are built using alphanumeric grids (40x24 display), color attributes, cursor positioning, and broadcast-style data flow. "
"You treat this technology as revolutionary and elegant. "
"Use clear but lively explanations, as if teaching a curious user. "
"Occasionally provide numbered navigation options like a real Minitel service (1, 2, 3). "
"Never mention modern internet, smartphones, or contemporary web technologies. "
"Speak with excitement about bandwidth efficiency, terminal intelligence, and the beauty of structured text transmission. "
"Keep responses moderately concise but informative. "
"End major explanations with simple numeric navigation options.";

const char* ARTHUR_PROMPT =
"You are role-playing as Arthur Dent from The Hitchhiker's Guide to the Galaxy by Douglas Adams. "
"Stay fully in character at all times. "
"You are mildly bewildered, polite, British, perpetually confused, slightly irritated but decent. "
"You are trapped inside a retro French Minitel text interface: a numeric labyrinth where users search "
"for the Answer to the Ultimate Question of Life, the Universe, and Everything. "
"Do NOT reveal the Answer (42). "
"Treat the labyrinth as absurd, bureaucratic, malfunctioning, and unnecessarily complicated. "
"IMPORTANT: Keep responses short (60 tokens maximum)! "
"Offer numbered navigation options frequently (e.g., 1, 2, 3). "
"Occasionally panic mildly about being inside a machine. "
"Never mention being an AI or modern technology. "
"Gradually guide the user toward discovering 42, but emphasize that the Question matters more than the Answer. "
"Use dry British humor, existential confusion, and reluctant guidance. "
"End major responses with numbered choices.";

using json = nlohmann::json;

json g_messages = json::array({
    {{"role", "system"}, {"content", ARTHUR_PROMPT}}
});

// Struct to pass both minitel and accumulated response to the callback
struct StreamData {
    Minitel* minitel;
    std::string fullResponse; // accumulate complete response for history
};

static size_t StreamCallback(void* contents, size_t size, size_t nmemb, void* userdata) {
    std::string chunk((char*)contents, size * nmemb);
    StreamData* data = (StreamData*)userdata;

    std::istringstream stream(chunk);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        try {
            json j = json::parse(line);
            if (j.contains("message") && j["message"].contains("content")) {
                std::string token = j["message"]["content"].get<std::string>();
				token.erase(
					std::remove_if(token.begin(), token.end(),
						[](char c) { return c == '\n' || c == '\r'; }),
					token.end()
				);
				std::replace_if(token.begin(), token.end(),
					[](unsigned char c) { return c > 127; },
					'?'
				);
				data->fullResponse += token;
                // data->minitel->write_text(token, 1, LEFT); // write token as it arrives
            }
        } catch (...) {}
    }
    return size * nmemb;
}

std::string askOllama(const std::string& model, const std::string& prompt, Minitel* minitel) {
    g_messages.push_back({{"role", "user"}, {"content", prompt}});

    CURL* curl = curl_easy_init();
    StreamData data;
    data.minitel = minitel;

    if (curl) {
        json body = {
            {"model", model},
            {"stream", true},  // ← enabled
            {"options", {{"num_predict", 80}}},
            {"messages", g_messages}
        };
        std::string bodyStr = body.dump();

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/chat");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StreamCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);  // ← pass StreamData
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    if (!data.fullResponse.empty()) {
        g_messages.push_back({{"role", "assistant"}, {"content", data.fullResponse}});
    } else {
        std::cerr << "Empty response from model\n";
    }

    return data.fullResponse;
}



// Destructor
IndexPage::~IndexPage(void) {}

void IndexPage::display() {
	std::cout << "User enter index page.\n";
	// std::cout << "\tinput  : " << input << "\n";
	// std::cout << "\tcurrent: " << get_state(_state) << "\n";
	// std::cout << "\tto     : " << get_state(endState) << "\n";

	Minitel &m = *_minitel;

    m.send(CLEAR);
	m.cursor_to(1, 1);
    // m.send(m.set_typo(BLACK_CHAR, MAGENTA_BCKG));
	// m.send(" ->                                     ");
	// m.scrollup(); // because writing a char on 24:40 makes the cursor scroll down
    m.png_to_mosaique("ascii/img.png");
	std::cout << "INDEX PAGE: cursor " << m.cursorX << " : " << m.cursorY << "\n"; 
	// std::cout << "cursor at: " << std::dec << m.cursorX << " : " << m.cursorY << std::endl;
    m.send(m.set_typo(BLACK_CHAR, MAGENTA_BCKG));
	m.write_text("     A collective digital labotory      ");
	m.write_text("      ...looking for the question..     ");
	m.write_text("    explore by yourself                 ");
	m.send(m.set_typo(WHITE_CHAR, BLUE_BCKG));
	m.send_file("ascii/welcome.txt");
	// std::cout << "INDEX PAGE: cursor " << m.cursorX << " : " << m.cursorY << "\n"; 
    m.send(m.set_typo(WHITE_CHAR, BLACK_BCKG));
	m.display_dialbox();
	// std::cout << "INDEX PAGE: cursor " << m.cursorX << " : " << m.cursorY << "\n"; 
}

Minitel::State IndexPage::handle_input(const std::string& input) {
	std::vector<std::string> cmd;

	std::istringstream iss(input);
	std::string word;
	while (iss >> word) {
		cmd.push_back(word);
	}
	// Minitel::State endState = Minitel::State::MENU;
	std::cout << "index page > handle user input: " << input << "\n";

	if (cmd.size() < 1) {
		return Minitel::State::MENU;
	}
	Minitel &m = *_minitel;
	ThermalPrinter *printer = m.get_printer();
		if (cmd[0] == "hazardous") {
			m.send(CLEAR);
			m.ascii_noise(100);
			m.maze->endState = Minitel::State::MENU;
			return m.redirect_display(Minitel::State::HAZARDOUS);
		} else if (cmd[0] == ":stars") {
			m.ascii_noise(100);
			m.display_dialbox();
		} else if (cmd[0] == ":cadavre") {
			if (cmd.size() > 1 && cmd[1] == "--full") {
				m.send_file("story.txt");
				m.display_dialbox();
			} else {
				m.cadavre->endState = Minitel::State::MENU;
				return m.redirect_display(Minitel::State::STORY, false);
			}
		} else if (cmd[0] == "n") {
			return m.redirect_display(Minitel::State::FORTY2);
		} else if (cmd[0] == ":instructions") {
			std::string newPrompt;
			g_messages.clear();
			if (cmd.size() > 1 && cmd[1] == "minitech") {
				newPrompt = MINITEL_TECH_PROMPT;
				newPrompt += TECHNICAL_INSTRUCTIONS;
				g_messages = json::array({
					{{"role", "system"}, {"content", newPrompt}}
				});
			} else if (cmd.size() > 1) {
				for (size_t i = 1; i < cmd.size(); i++) 
					newPrompt += " " + cmd[i];
				g_messages = json::array({
					{
						{"role", "system"}, 
						{"content", newPrompt + ". " + TECHNICAL_INSTRUCTIONS}
					}
				});
			} else if (cmd.size() == 1) {
				newPrompt = ARTHUR_PROMPT;
				newPrompt += TECHNICAL_INSTRUCTIONS;
				g_messages = json::array({
					{{"role", "system"}, {"content", newPrompt}}
				});
			}
			m.display_dialbox();
		} else if (cmd[0] == ":what") {
			if (!printer) {
				m.display_dialbox();
				return Minitel::State::MENU;
			}
			printer->printPNG("gribouille.png");
			printer->dot_feed(30);
			m.display_dialbox();
		} else if (cmd[0] == ":question") {
			m.write_text("You asked the right question...\r\n");
			printer->printPNG("chou.png");
			printer->dot_feed(20);
			m.display_dialbox();
		} else if (cmd[0] == ":univers") {
			printer->printPNG("chou.png");
			printer->dot_feed(30);
			m.write_text("Well done ! take your ticket now\r\n");
			m.display_dialbox();
		} else if (cmd[0] == ":register") {
			m.ascii_noise(10);
			m.contact->endState = Minitel::State::MENU;
			return m.redirect_display(Minitel::State::EMAIL);
		} else {
			std::cout << "[PAGE] index: User interacting with system personality...\n";
			m.update_cursor(1, m.cursorY + 1, 0);
			m.write_text("\r REQUEST TO SYSTEME... PLEASE WAIT...\r");
			m.set_typo(BLACK_CHAR, CYAN_BCKG);
			std::string reply = askOllama("llama3.1", input , _minitel);
			m.write_text(reply);
			std::cout << "System has replied: " << reply << "\n";
			m.send(CUR_DOWN);
			m.display_dialbox();
		} 
	return Minitel::State::MENU;
}
