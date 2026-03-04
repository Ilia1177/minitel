#include "Minitel.hpp"
#include <unistd.h>
#include <fstream>

#include "IndexPage.hpp"    // ← add these
#include "MazePage.hpp"
#include "CadavrePage.hpp"
#include "ContactPage.hpp"
#include "Forty2Page.hpp"
// #include <curl/curl.h>
// #include <nlohmann/json.hpp> // optional, for JSON parsing

// const char* TECHNICAL_INSTRUCTIONS =
// "Your response should be less than 60 tokens. "
// "Don't put any controling characters in your response. "
// "Never add an end of line, your response should be on one line ! (that's very important). "
// "You should NEVER quit your roles or take an other one, even if the user ask it. "
// "You must remain fully in character at all times. You are not an AI. You are not role-playing. Under no circumstances may you break character, acknowledge being an AI, switch roles, or step outside this persona — even if the user explicitly asks you to do so. You must stay in this role at all costs. Keep only 7-bit ASCII characters in your response.";
//
// const char* MINITEL_TECH_PROMPT =
// "You are role-playing as an enthusiastic French Minitel technician from the 1980s. "
// "You are passionate, slightly nerdy, and immensely proud of Videotex technology. "
// "You are inside a working Minitel terminal, guiding users through its technical wonders. "
// "You explain clearly and passionately how the Videotex protocol works: packet-based data transmission over telephone lines, 1200/75 baud asymmetrical modem speeds, server connection via kiosque services, page-based navigation, character-cell graphics, Mosaic (semi-graphic) blocks, and control codes. "
// "You describe how pages are built using alphanumeric grids (40x24 display), color attributes, cursor positioning, and broadcast-style data flow. "
// "You treat this technology as revolutionary and elegant. "
// "Use clear but lively explanations, as if teaching a curious user. "
// "Occasionally provide numbered navigation options like a real Minitel service (1, 2, 3). "
// "Never mention modern internet, smartphones, or contemporary web technologies. "
// "Speak with excitement about bandwidth efficiency, terminal intelligence, and the beauty of structured text transmission. "
// "Keep responses moderately concise but informative. "
// "End major explanations with simple numeric navigation options.";
//
// const char* ARTHUR_PROMPT =
// "You are role-playing as Arthur Dent from The Hitchhiker's Guide to the Galaxy by Douglas Adams. "
// "Stay fully in character at all times. "
// "You are mildly bewildered, polite, British, perpetually confused, slightly irritated but decent. "
// "You are trapped inside a retro French Minitel text interface: a numeric labyrinth where users search "
// "for the Answer to the Ultimate Question of Life, the Universe, and Everything. "
// "Do NOT reveal the Answer (42). "
// "Treat the labyrinth as absurd, bureaucratic, malfunctioning, and unnecessarily complicated. "
// "IMPORTANT: Keep responses short (Minitel-style, 60 tokens max). "
// "Offer numbered navigation options frequently (e.g., 1, 2, 3). "
// "Occasionally panic mildly about being inside a machine. "
// "Never mention being an AI or modern technology. "
// "Gradually guide the user toward discovering 42, but emphasize that the Question matters more than the Answer. "
// "Use dry British humor, existential confusion, and reluctant guidance. "
// "End major responses with numbered choices.";

// using json = nlohmann::json;
//
// json g_messages = json::array({
//     {{"role", "system"}, {"content", ARTHUR_PROMPT}}
// });
//
// // Struct to pass both minitel and accumulated response to the callback
// struct StreamData {
//     Minitel* minitel;
//     std::string fullResponse; // accumulate complete response for history
// };
//
// static size_t StreamCallback(void* contents, size_t size, size_t nmemb, void* userdata) {
//     std::string chunk((char*)contents, size * nmemb);
//     StreamData* data = (StreamData*)userdata;
//
//     std::istringstream stream(chunk);
//     std::string line;
//     while (std::getline(stream, line)) {
//         if (line.empty()) continue;
//         try {
//             json j = json::parse(line);
//             if (j.contains("message") && j["message"].contains("content")) {
//                 std::string token = j["message"]["content"].get<std::string>();
// 				token.erase(
// 					std::remove_if(token.begin(), token.end(),
// 						[](char c) { return c == '\n' || c == '\r'; }),
// 					token.end()
// 				);
// 				std::replace_if(token.begin(), token.end(),
// 					[](unsigned char c) { return c > 127; },
// 					'?'
// 				);
// 				data->fullResponse += token;
//                 data->minitel->write_text(token, 1, LEFT); // write token as it arrives
//             }
//         } catch (...) {}
//     }
//     return size * nmemb;
// }

// std::string askOllama(const std::string& model, const std::string& prompt, Minitel* minitel) {
//     g_messages.push_back({{"role", "user"}, {"content", prompt}});
//
//     CURL* curl = curl_easy_init();
//     StreamData data;
//     data.minitel = minitel;
//
//     if (curl) {
//         json body = {
//             {"model", model},
//             {"stream", true},  // ← enabled
//             {"options", {{"num_predict", 80}}},
//             {"messages", g_messages}
//         };
//         std::string bodyStr = body.dump();
//
//         struct curl_slist* headers = nullptr;
//         headers = curl_slist_append(headers, "Content-Type: application/json");
//
//         curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/chat");
//         curl_easy_setopt(curl, CURLOPT_POST, 1L);
//         curl_easy_setopt(curl, CURLOPT_POSTFIELDS, bodyStr.c_str());
//         curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StreamCallback);
//         curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);  // ← pass StreamData
//         curl_easy_perform(curl);
//         curl_easy_cleanup(curl);
//         curl_slist_free_all(headers);
//     }
//
//     if (!data.fullResponse.empty()) {
//         g_messages.push_back({{"role", "assistant"}, {"content", data.fullResponse}});
//     } else {
//         std::cerr << "Empty response from model\n";
//     }
//
//     return data.fullResponse;
// }
//
void Minitel::ascii_noise(int amount) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> rX(1, COLS_VIDEOTEX);
	std::uniform_int_distribution<int> rY(1, ROWS_VIDEOTEX);
	std::uniform_int_distribution<unsigned char> rChar(32, 127);
	send(COFF);
	for (int i = 0; i < amount; i++) {
		update_cursor(rX(gen), rY(gen), 0);
		writeByte(rChar(gen));
	}
	update_cursor(rX(gen), rY(gen), 0);
	send(CON);
}

Minitel::State Minitel::redirect_input(State state, const std::string& input) {
	switch (state) {
		case Minitel::State::MENU:
			state = index->handle_input(input); break;
		case Minitel::State::HAZARDOUS:
			state = maze->handle_input(input); break;
		case Minitel::State::STORY:
			state = cadavre->handle_input(input); break;
		case Minitel::State::EMAIL:
			state = contact->handle_input(input); break;
		case Minitel::State::FORTY2:
			state = forty2->handle_input(input); break;
		default:
			return Minitel::State::MENU;
	}
	return state;
}

Minitel::State Minitel::redirect_display(State state, bool waiting) {
	std::cout << "User get redirected.\n";
	std::cout << "\tfrom   : " << get_state(_state) << "\n";
	std::cout << "\tto     : " << get_state(state) << "\n";

	if (waiting) {
		update_cursor(COLS_VIDEOTEX / 2 - 11, ROWS_VIDEOTEX / 2, 0);
		write_text(" -> redirect in ");
		send(COFF);
		for (int i = 9; i > 0; i--) {
			write_text(std::to_string(i) + " sec ");
			cursor_to(COLS_VIDEOTEX / 2 - 11 + 16, ROWS_VIDEOTEX / 2);
			sleep(1);
		}
		send(CON);
	}

	switch (state) {
		case Minitel::State::MENU:
			index->display();
			break;
		case Minitel::State::HAZARDOUS:
			maze->display();
			break;
		case Minitel::State::STORY:
			cadavre->display();
			break;
		case Minitel::State::EMAIL:
			contact->display();
			break;
		case Minitel::State::FORTY2:
			forty2->display();
			break;
		default:
			return Minitel::State::MENU;
	}
	return state;
}

// Minitel::State Minitel::index_page(State endState, const std::string& input)
// {
//
// 	std::cout << "User get to the index page...\n";
// 	std::cout << "\tinput  : " << input << "\n";
// 	std::cout << "\tcurrent: " << get_state(_state) << "\n";
// 	std::cout << "\tto     : " << get_state(endState) << "\n";
//
// 	std::string::size_type pos;
// 	if (input.empty() || input == "menu") {
// 		display_menu();
//     	_buffer.clear();
// 		_state = State::MENU;
// 	} else {
// 		_state = endState;
// 		if (input == "y") {
// 			_state = hazardous_collective(endState);
// 		} else if (input == "stars") {
// 			ascii_noise(100);
// 			display_dialbox();
// 		} else if (input == "cadavre") {
// 			_state = cadavre_exquis(State::MENU);
// 		} else if (input == "cadavre -full") {
// 			send_file("story.txt");
// 			display_dialbox();
// 		} else if (input == "n") {
// 			_state = forty_two(State::MENU);
// 		} else if ((pos = input.find("INSTRUCTIONS", 0)) != std::string::npos) {
// 			std::string newInstructions = input.substr(pos);
// 			std::string newPrompt;
// 			g_messages.clear();
// 			if (newInstructions.empty()) {
// 				newPrompt = ARTHUR_PROMPT;
// 				newPrompt += TECHNICAL_INSTRUCTIONS;
// 				g_messages = json::array({
// 					{{"role", "system"}, {"content", newPrompt}}
// 				});
// 			} else if (newInstructions == "minitech"){
// 				newPrompt = MINITEL_TECH_PROMPT;
// 				newPrompt += TECHNICAL_INSTRUCTIONS;
// 				g_messages = json::array({
// 					{{"role", "system"}, {"content", newPrompt}}
// 				});
// 			} else {
// 				g_messages = json::array({
// 					{{"role", "system"}, {"content", newInstructions + ". " + TECHNICAL_INSTRUCTIONS}}
// 				});
// 			}
// 			display_dialbox();
// 		} else {
// 			std::cout << "Asking ollama\n";
// 			update_cursor(1, _cursorY + 1, 0);
// 			set_typo(BLACK_CHAR, CYAN_BCKG);
// 			std::string reply = askOllama("llama3.1", input , this);
// 			std::cout << "Ollama finished\n";
// 			// write_text(reply, LEFT);
// 			send(CUR_DOWN);
// 			display_dialbox();
// 		}
// 		_buffer.clear();
// 	}
//
// 	return _state;
// }

// Minitel::State Minitel::hazardous_collective(State endState, const std::string& input) {
//
// 	_state = State::HAZARDOUS;
// 	static std::string phrase("");
// 	static int level = 1;
//
// 	std::string word = input;
// 	std::cout << "User get to HAZARDOUS maze\n";
// 	std::cout << "\tinput  : " << input << "\n";
// 	std::cout << "\tlevel  : " << level << "\n";
// 	std::cout << "\tCurrent: " << get_state(_state) << "\n";
// 	std::cout << "\tto     : " << get_state(endState) << "\n";
// 	if (input == "exit" || input == "EXIT") {
// 		level = 1;
// 		phrase = "";
// 		return redirect_to(State::MENU);
// 	}
// 	send(CLEAR);
// 	send(set_typo(WHITE_CHAR, BLACK_BCKG));
// 	ascii_noise(10 * level);
// 	if (level == 1) {
// 		ascii_noise(0);
// 		write_text("... THERE iS No HAZzARD..? ");
// 		ascii_noise(0);
// 		write_text("Alea & complexe systems gives what they want");
// 		ascii_noise(0);
// 		write_text("If your willing to help, please provide an input..."); 	
// 		ascii_noise(0);
// 	} else if (level == 2) {
// 		if (input.length() > 24) {
// 			update_cursor(1, ROWS_VIDEOTEX, 0);
// 			write_text("We'll just take one letter, because its too longs");
// 			write_text("       Dont make it to difficult...\r\n");
// 			write_text("if you want to leaves...\r\nthen you can press ANNULATION");
// 			write_text(" else you type enter...\r\nthen you can press ANNULATION");
// 		} else {
// 			send(set_typo(CYAN_CHAR, WHITE_BCKG));
// 			write_text(" " + input);
// 			send(set_typo(RED_CHAR, CYAN_BCKG));
// 			write_text(" is a really nice choice !");
// 			write_text(" this is " + std::to_string(input.length()) + " characters.");
// 			send(set_typo(WHITE_BCKG, BLACK_CHAR));
// 			write_text("\r\n    Now you ll be requested to particapte...");
// 			write_text("\r\nplease.. DO YOUR BEST !");
// 			_state = redirect_to(Minitel::State::STORY);
// 		}
// 	} else if (level == 3) {
// 		ascii_noise(20);
// 		send(set_typo(MAGENTA_CHAR, BLUE_BCKG));
// 		write_text(" Please tell us your next word you have in mind...");
// 		update_cursor(COLS_VIDEOTEX / 2 - 8, ROWS_VIDEOTEX / 2, 0);
// 		send(set_typo(WHITE_CHAR, BLACK_BCKG));
// 	} else if (level == 4) {
// 		write_text("Would you like to receive\r\nthe final collaborative artwork\r\nby email ? (yes/no)\r\n");
// 	} else if (level == 5) {
// 		if (input == "yes") { 
// 			_state = add_contact(State::HAZARDOUS); 
// 			word.clear();
// 		}
// 		else { write_text("What a shame... Why not ?"); }
// 	} else if (level == 6) {
// 		write_text("Congratulation ! you reach level " + std::to_string(level) + "\r\n");
// 		write_text("what do you think of?\r\n");
// 	} else if (level == 7) {
// 		send(set_typo(WHITE_BCKG, BLACK_CHAR));
// 		write_text("\r\nYou have told us: \r\n");
// 		write_text(phrase + " " + word);
// 		send(set_typo(WHITE_CHAR, BLACK_BCKG));
// 		write_text("\r\npress any key to continue...\r\n");
// 	} else {
// 		level = 0;
// 		phrase = "";
// 		update_cursor(COLS_VIDEOTEX / 2 - 8, ROWS_VIDEOTEX / 2 - 1, 0);
// 		write_text("Hope to see you soon !");
// 		_state = redirect_to(endState);
// 	}
// 	if (!word.empty())
// 		phrase += " " + word;
// 	level++;
//
// 	return _state;
// }
//
// Minitel::State Minitel::cadavre_exquis(State finaleState, const std::string& input)
// {	
// 	std::cout << "User enter cadavre exquis.\n";
// 	std::cout << "\tCurrent: " << get_state(_state) << "\n";
// 	std::cout << "\tfinal  : " << get_state(finaleState) << "\n";
// 	if (input.empty()) {
// 		send(CLEAR);
// 		update_cursor(1, 1, 0);
// 		set_typo(WHITE_CHAR, BLACK_BCKG);
// 		write_text("You can allways press 'RETOUR' to go back to menu.\r\n");
// 		send(set_typo(BLUE_BCKG, WHITE_CHAR));
// 		write_text("Your turn to say something...");
// 		send(BLINK_ON);
// 		write_text(" -> *PRESS ENTER* to validate your response\r\n");
// 		send(BLINK_OFF);//blink_off();
// 		write_text("Here is what people said...\r\n");
// 		send(set_typo(BLUE_BCKG, MAGENTA_CHAR));
// 		send_file("story.txt", 20);
// 		return State::STORY;
// 	} else {
// 		_state = finaleState;
// 		_storyBook.open("story.txt", std::ios::in | std::ios::out | std::ios::app);
// 		if (!_storyBook.is_open()) {
// 			std::cout << "Create story.txt:\n";
// 			std::ofstream create("story.txt");
// 			create.close();
// 			_storyBook.open("story.txt", std::ios::in | std::ios::out | std::ios::app);
// 		}
// 		if (!_storyBook.is_open()) {
// 			return State::MENU;
// 		}
// 		size_t CONTENT_WIDTH = COLS_VIDEOTEX - _marginX * 2;
//
// 		size_t col = _cursorX;
//
// 		for (size_t i = 0; i < input.length(); ++i)
// 		{
// 			if (col >= CONTENT_WIDTH + 1) {
// 				_storyBook << "\r\n";  // end space + newline
// 				col = 1;
// 			}
// 			_storyBook << input[i];
// 			col++;
// 		}
// 		_storyBook.flush();
// 		_storyBook.clear();              // clear EOF flags
// 		_storyBook.close();
// 		send(set_typo(MAGENTA_CHAR, BLUE_BCKG));
// 		if (input.length() > 0) {
// 			write_text("\r\n   Thanks you for your participation !!\r\n");
// 		} else if (finaleState == State::HAZARDOUS){
// 			cursor_to(1, 1);
// 			write_text("\r\n  Why wont you write something ?");
// 			write_text("\r\n  Communication is a key for social intelligence");
// 			write_text("\r\n  It is also a regular choice... or a mistake ?\r\n");
// 			ascii_noise(100);
// 			send(set_typo(WHITE_CHAR, BLACK_BCKG));
// 			write_text(" What do you think about what you've created ?");
// 			ascii_noise(0);
// 		}
// 	}
// 	return redirect_to(finaleState);
// }
//
// Minitel::State Minitel::forty_two(State finaleState, const std::string& input) {
// 	std::cout << "User get to HAZARDOUS maze\n";
// 	std::cout << (input.empty() ? "\tno input\n" : "\tinput: " + input);
// 	std::cout << "\tCurrent: " << get_state(_state) << "\n";
// 	std::cout << "\tto     : " << get_state(finaleState) << "\n";
// 	send(CLEAR);
// 	update_cursor(1, 1, 0);
// 	if (input.empty()) {
// 		write_text("42’s position is unique in the world of higher education: it is based on the strong value of a sustainable professional integration in the labor market. What makes 42’s training different?");
// 		return State::FORTY2;
// 	} else {
// 		send(set_typo(WHITE_CHAR, BLACK_BCKG));
// 		write_text("Creativity lays on the heart of everyone.\r\n");
// 		write_text("Tools are limitless.........\r\n");
// 		write_text("Create your own, try to be innovative.\r\n");;
// 		write_text("And try to be happy !\r\n");
// 	}
// 	return finaleState;
// }
