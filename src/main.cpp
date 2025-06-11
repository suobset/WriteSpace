#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>
#include <thread>
#include <future>
#include <atomic>
#include <array>
#include <cstdio>
#include <map>
#include <regex>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/component/event.hpp"
#include "ftxui/component/captured_mouse.hpp"
#include "ftxui/component/container.hpp"
#include "ftxui/component/input.hpp"
#include "ftxui/component/button.hpp"
#include "ftxui/component/modal.hpp"
#include "ftxui/component/toggle.hpp"
#include "ftxui/dom/node.hpp"


// --- Constants and Structs ---
const std::filesystem::path DRAFTS_DIR_CPP = "01_drafts";
const std::filesystem::path TEMPLATES_DIR_CPP = "02_templates";
const std::string ROOT_MAKEFILE_NAME = "Makefile";

struct FileSystemEntry { std::wstring display_name_w; std::filesystem::path path; bool is_directory; int depth; };
enum class GitFileStatus { UNKNOWN, CLEAN, MODIFIED, ADDED, DELETED, RENAMED, COPIED, UNTRACKED, CONFLICTED };
struct GitStatusInfo { GitFileStatus status = GitFileStatus::CLEAN; std::filesystem::path old_path; };
struct OutlineEntry { std::wstring text; int level; };

// --- Global State ---
std::vector<FileSystemEntry> g_project_files_entries;
std::vector<std::string> g_project_menu_display_names_std;
int g_project_menu_selected = 0;

std::vector<std::string> g_editor_lines;
std::filesystem::path g_current_editor_file_path;
std::wstring g_current_file_display_name_w = L"N/A";
int g_editor_active_line_idx = 0;
std::string g_editor_current_line_buffer;
bool g_show_line_editor_modal = false; // For line editing modal

std::vector<std::string> g_log_messages;
const size_t MAX_LOG_MESSAGES = 500;
ftxui::ScreenInteractive* g_screen_ptr = nullptr;
std::mutex g_shared_data_mutex;

std::filesystem::path g_root_makefile_path;
const std::vector<std::pair<std::string, std::string>> PALETTE_COMMANDS = {
    {"Build: All", "build_all"}, {"Build: HTML", "build_html"}, {"Build: HTML (Plain)", "build_html-plain"},
    {"Build: PDF (Paper)", "build_pdf-paper"}, {"Build: PDF (IEEE)", "build_pdf-ieee"},
    {"Build: PDF (MLA)", "build_pdf-mla"}, {"Build: TeX Source", "build_tex"}, {"Build: Clean", "build_clean"},
    {"Git: Refresh Status", "git_refresh_status"},
    {"Git: Show Status Output", "git_show_status_output"}
};
bool g_show_command_palette = false;
std::vector<std::string> g_command_palette_entries;
int g_command_palette_selected = 0;
std::atomic<bool> g_is_background_task_running(false);

std::map<std::filesystem::path, GitStatusInfo> g_git_file_statuses;
std::atomic<bool> g_is_git_status_loading(false);
bool g_git_repo_found = false;
const std::map<GitFileStatus, std::wstring> GIT_STATUS_SYMBOLS_W = { /* ... */
    {GitFileStatus::MODIFIED, L" [M]"}, {GitFileStatus::ADDED, L" [A]"}, {GitFileStatus::DELETED, L" [D]"},
    {GitFileStatus::RENAMED, L" [R]"}, {GitFileStatus::COPIED, L" [C]"}, {GitFileStatus::UNTRACKED, L" [?]"},
    {GitFileStatus::CONFLICTED, L" [!]"}
};

std::vector<OutlineEntry> g_outline_entries;
std::vector<std::string> g_outline_menu_display_names_std;
int g_outline_menu_selected = 0;
int g_info_panel_selected_tab = 0;
std::vector<std::string> INFO_PANEL_TAB_NAMES = {"Outline", "Stats", "Citations"};

bool g_show_new_project_dialog = false;
std::string g_new_project_name_input_str;
std::string g_new_project_template_input_str;
bool g_show_delete_dialog = false;
std::filesystem::path g_path_to_delete;
std::string g_item_name_to_delete_str;

// --- Helper Functions (WStringToString, StringToWString, AppLog, scan_directory_recursive, etc. are assumed to be here and correct) ---
std::string WStringToString(const std::wstring& ws){try{std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>,wchar_t>c;return c.to_bytes(ws);}catch(const std::exception&){std::string r;for(wchar_t wc:ws){if(wc>=0&&wc<128)r+=static_cast<char>(wc);else r+='?';}return r;}}
std::wstring StringToWString(const std::string& s){try{std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>,wchar_t>c;return c.from_bytes(s);}catch(const std::exception&){std::wstring r;for(char sc:s){if(static_cast<unsigned char>(sc)<128)r+=static_cast<wchar_t>(sc);else r+=L'?';}return r;}}
void AppLog(const std::string& message){std::lock_guard<std::mutex>guard(g_shared_data_mutex);if(g_log_messages.size()>=MAX_LOG_MESSAGES)g_log_messages.erase(g_log_messages.begin());g_log_messages.push_back(message);if(g_screen_ptr)g_screen_ptr->PostEvent(ftxui::Event::Custom);}
void AppLog(const std::wstring& message_w){AppLog(WStringToString(message_w));}
void scan_directory_recursive(std::vector<FileSystemEntry>&ev,const std::filesystem::path&cp,int cd){if(!std::filesystem::exists(cp)||!std::filesystem::is_directory(cp))return;std::vector<std::filesystem::path>itms;for(const auto&ei:std::filesystem::directory_iterator(cp)){std::filesystem::path ip=ei.path();if(ip.filename().empty())continue;std::wstring fnw=ip.filename().native();if(fnw.empty()||fnw.front()==L'.')continue;itms.push_back(ip);}std::sort(itms.begin(),itms.end());for(const auto&ip:itms){ev.push_back({std::wstring(cd*2,L' ')+ip.filename().native(),ip.lexically_normal(),std::filesystem::is_directory(ip),cd});if(std::filesystem::is_directory(ip))scan_directory_recursive(ev,ip,cd+1);}}
void regenerate_project_browser_menu_entries();
void refresh_project_browser(){std::lock_guard<std::mutex>g(g_shared_data_mutex);g_project_files_entries.clear();if(std::filesystem::exists(DRAFTS_DIR_CPP)){g_project_files_entries.push_back({L"01_drafts",DRAFTS_DIR_CPP.lexically_normal(),true,0});scan_directory_recursive(g_project_files_entries,DRAFTS_DIR_CPP,1);}else{g_project_files_entries.push_back({L"01_drafts(NF)",DRAFTS_DIR_CPP.lexically_normal(),true,0});}g_project_files_entries.push_back({L"",{},false,-1});if(std::filesystem::exists(TEMPLATES_DIR_CPP)){g_project_files_entries.push_back({L"02_templates",TEMPLATES_DIR_CPP.lexically_normal(),true,0});scan_directory_recursive(g_project_files_entries,TEMPLATES_DIR_CPP,1);}else{g_project_files_entries.push_back({L"02_templates(NF)",TEMPLATES_DIR_CPP.lexically_normal(),true,0});}regenerate_project_browser_menu_entries();if(g_project_menu_selected>=static_cast<int>(g_project_menu_display_names_std.size())){g_project_menu_selected=std::max(0,static_cast<int>(g_project_menu_display_names_std.size())-1);}AppLog("Proj browser refreshed.");if(g_screen_ptr)g_screen_ptr->PostEvent(ftxui::Event::Custom);}
void regenerate_project_browser_menu_entries(){std::lock_guard<std::mutex>g(g_shared_data_mutex);g_project_menu_display_names_std.clear();for(const auto&e:g_project_files_entries){if(e.depth==-1){g_project_menu_display_names_std.push_back("──────────");continue;}std::wstring fd_w=e.display_name_w;if(g_git_repo_found&&!e.is_directory){auto it=g_git_file_statuses.find(e.path);if(it!=g_git_file_statuses.end()){auto sit=GIT_STATUS_SYMBOLS_W.find(it->second.status);if(sit!=GIT_STATUS_SYMBOLS_W.end())fd_w+=sit->second;}}g_project_menu_display_names_std.push_back(WStringToString(fd_w));}if(g_project_menu_display_names_std.empty())g_project_menu_display_names_std.push_back("(No files)");}
void refresh_git_statuses_threaded(){if(g_is_git_status_loading.load())return;g_is_git_status_loading=true;AppLog("Git status refresh...");std::map<std::filesystem::path,GitStatusInfo>ns;std::filesystem::path rp_cmd=std::filesystem::current_path();std::string cmd="git -C \""+rp_cmd.string()+"\" status --porcelain -uall";FILE*pipe=popen(cmd.c_str(),"r");if(!pipe){AppLog("ERR popen git");g_is_git_status_loading=false;g_git_repo_found=false;if(g_screen_ptr)g_screen_ptr->PostEvent(ftxui::Event::Custom);return;}g_git_repo_found=true;std::array<char,512>buf;while(fgets(buf.data(),buf.size(),pipe)!=nullptr){std::string l=buf.data();if(l.length()<3)continue;if(!l.empty()&&l.back()=='\n')l.pop_back();std::string xy=l.substr(0,2);std::string ps=l.substr(3);GitStatusInfo i;char idx=xy[0],wt=xy[1];if(idx=='R'||idx=='C'){size_t arr=ps.find("->");if(arr!=std::string::npos){i.old_path=rp_cmd/ps.substr(0,arr);ps=ps.substr(arr+4);i.status=(idx=='R')?GitFileStatus::RENAMED:GitFileStatus::COPIED;}}else if(idx=='A'||wt=='A')i.status=GitFileStatus::ADDED;else if(idx=='M'||wt=='M')i.status=GitFileStatus::MODIFIED;else if(idx=='D'||wt=='D')i.status=GitFileStatus::DELETED;else if(xy=="??")i.status=GitFileStatus::UNTRACKED;else if(xy=="UU"||xy=="AA"||xy=="DD"||xy=="AU"||xy=="UA")i.status=GitFileStatus::CONFLICTED;if(i.status!=GitFileStatus::CLEAN)ns[(rp_cmd/ps).lexically_normal()]=i;}pclose(pipe);{std::lock_guard<std::mutex>g(g_shared_data_mutex);g_git_file_statuses=ns;}g_is_git_status_loading=false;AppLog("Git status refreshed.");regenerate_project_browser_menu_entries();if(g_screen_ptr)g_screen_ptr->PostEvent(ftxui::Event::Custom);}
void execute_build_in_thread(const std::string&pds,const std::string&mfps,const std::string&t){g_is_background_task_running=true;AppLog("Build: "+t+" in "+pds);std::string cmd="make -C \""+pds+"\" -f \""+mfps+"\" "+t+" 2>&1";AppLog("Cmd: "+cmd);std::array<char,256>buf;FILE*pipe=popen(cmd.c_str(),"r");if(!pipe){AppLog("ERR popen build");g_is_background_task_running=false;return;}while(fgets(buf.data(),buf.size(),pipe)!=nullptr){std::string l=buf.data();if(!l.empty()&&l.back()=='\n')l.pop_back();AppLog(l);}int s=pclose(pipe);AppLog((s==0)?("Build OK: "+t):("Build FAIL: "+t+" (exit "+std::to_string(WEXITSTATUS(s))+")"));g_is_background_task_running=false;if(g_screen_ptr)g_screen_ptr->PostEvent(ftxui::Event::Custom);}
void execute_git_status_output_threaded(){if(g_is_background_task_running.load()){AppLog("Task running.");return;}g_is_background_task_running=true;AppLog("Git status output...");std::filesystem::path rp=std::filesystem::current_path();std::string cmd="git -C \""+rp.string()+"\" status";FILE*pipe=popen(cmd.c_str(),"r");if(!pipe){AppLog("ERR popen git status");g_is_background_task_running=false;return;}AppLog("--- Git Status ---");std::array<char,256>buf;while(fgets(buf.data(),buf.size(),pipe)!=nullptr){std::string l=buf.data();if(!l.empty()&&l.back()=='\n')l.pop_back();AppLog(l);}pclose(pipe);AppLog("--- End Git Status ---");g_is_background_task_running=false;if(g_screen_ptr)g_screen_ptr->PostEvent(ftxui::Event::Custom);}
void regenerate_outline_view(){std::lock_guard<std::mutex>g(g_shared_data_mutex);g_outline_entries.clear();g_outline_menu_display_names_std.clear();if(g_current_editor_file_path.empty()){g_outline_menu_display_names_std.push_back("(No file)");return;}std::string ext=g_current_editor_file_path.extension().string();std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);if(ext!=".md"&&ext!=".markdown"){g_outline_menu_display_names_std.push_back("(Not MD)");return;}if(g_editor_lines.empty()){g_outline_menu_display_names_std.push_back("(Empty)");return;}std::wregex hr(L"^(#+) +(.*)");for(const auto&ls:g_editor_lines){std::wstring wl=StringToWString(ls);std::wsmatch m;if(std::regex_match(wl,m,hr)){if(m.size()==3){std::wstring h=m[1].str(),t=m[2].str();int lvl=static_cast<int>(h.length());t.erase(0,t.find_first_not_of(L" \t"));t.erase(t.find_last_not_of(L" \t")+1);g_outline_entries.push_back({std::wstring((lvl-1)*2,L' ')+t,lvl});}}}for(const auto&e:g_outline_entries)g_outline_menu_display_names_std.push_back(WStringToString(e.text));if(g_outline_entries.empty()&&!g_editor_lines.empty())g_outline_menu_display_names_std.push_back("(No headers)");if(g_outline_menu_display_names_std.empty()&&g_editor_lines.empty())g_outline_menu_display_names_std.push_back("(Empty)");}

// --- Main Application ---
ftxui::Component line_editor_input_field; // Forward declare

int main(int argc, char* argv[]) {
    using namespace ftxui;
    auto screen = ScreenInteractive::Fullscreen();
    g_screen_ptr = &screen;
    AppLog("WriteSpace TUI (C++) Initializing...");

    std::filesystem::path CWD = std::filesystem::current_path(); g_root_makefile_path = CWD / ROOT_MAKEFILE_NAME;
    if(!std::filesystem::exists(g_root_makefile_path)) g_root_makefile_path = CWD.parent_path()/ROOT_MAKEFILE_NAME;
    if(std::filesystem::exists(g_root_makefile_path)) AppLog("Makefile: "+g_root_makefile_path.string()); else {AppLog("WARN: Makefile not found."); g_root_makefile_path.clear();}
    refresh_project_browser();
    for(const auto&cmd:PALETTE_COMMANDS)g_command_palette_entries.push_back(cmd.first);
    AppLog("Static data populated.");
    std::thread(refresh_git_statuses_threaded).detach();

    // Component Definitions
    auto project_browser_menu = Menu(&g_project_menu_display_names_std, &g_project_menu_selected);
    auto header = Renderer([&]{std::wstring ht=L"WS TUI | File: "+g_current_file_display_name_w; return text(ht)|center|bold;});
    auto footer = Renderer([]{return text(L"Ctrl+Q:Quit | Ctrl+P:Cmd | n:New | d:Del | e:Edit Line | Arrows:Nav")|center;}); // Added 'e'
    auto templates_panel = Renderer([]{return vbox({text(L"Templates")})|border|size(HEIGHT,EQUAL,5);});
    auto docs_panel = Renderer([]{return vbox({text(L"Docs")})|border|size(HEIGHT,EQUAL,4);});

    auto outline_menu = Menu(&g_outline_menu_display_names_std, &g_outline_menu_selected);
    auto info_panel_tab_content = Container::Tab({outline_menu, Renderer([]{return text(L"Stats...")|center;}), Renderer([]{return text(L"Citations...")|center;})}, &g_info_panel_selected_tab );
    auto info_panel_tabs_selection = Toggle(&INFO_PANEL_TAB_NAMES, &g_info_panel_selected_tab);
    auto info_panel_component = Container::Vertical({ info_panel_tabs_selection, info_panel_tab_content | yflex_grow });
    regenerate_outline_view();

    // Editor Pane: Highlights active line
    auto editor_pane = Renderer([&] {
        Elements line_elements;
        if (g_editor_lines.empty()) {
            line_elements.push_back(text(g_current_editor_file_path.empty()?L"Select file":L"(File empty)")|dim);
        } else {
            for (int i = 0; i < static_cast<int>(g_editor_lines.size()); ++i) {
                std::wstring line_w = StringToWString(g_editor_lines[i]);
                Element line_el = text(line_w);
                if (i == g_editor_active_line_idx) { // Highlight active line
                    line_el = line_el | inverted;
                }
                line_elements.push_back(line_el);
            }
        }
        return vbox(std::move(line_elements)) | yframe | border;
    });

    // Line Editor Input component (global as it's used in modal)
    line_editor_input_field = Input(&g_editor_current_line_buffer, ""); // Placeholder empty for actual line content

    auto status_log = Renderer([&]{ Elements logs; if(g_log_messages.empty()){logs.push_back(text(L"Log empty")|dim);}else{for(const auto&m:g_log_messages)logs.push_back(text(StringToWString(m)));} return vbox(std::move(logs))|yframe|border;});

    // Dialogs
    auto new_project_name_input_comp = Input(&g_new_project_name_input_str, "Project Name");
    auto new_project_template_input_comp = Input(&g_new_project_template_input_str, "Template (e.g., blog.md)");
    auto new_project_create_button = Button("Create", [&] { /* ... */
        if(g_new_project_name_input_str.empty()){AppLog("ERR: Project name empty."); return;}
        std::filesystem::path np_p=DRAFTS_DIR_CPP/g_new_project_name_input_str,tp_p=TEMPLATES_DIR_CPP/g_new_project_template_input_str;
        try{if(std::filesystem::exists(np_p)){AppLog("ERR: Proj exists: "+np_p.string());g_show_new_project_dialog=false;return;}std::filesystem::create_directory(np_p);AppLog("Created: "+np_p.string());if(!g_new_project_template_input_str.empty()){if(std::filesystem::exists(tp_p)&&std::filesystem::is_regular_file(tp_p)){std::filesystem::copy_file(tp_p,np_p/(g_new_project_name_input_str+tp_p.extension().string()));AppLog("Template copied.");}else AppLog("WARN: Tmpl not found: "+tp_p.string());}refresh_project_browser();std::thread(refresh_git_statuses_threaded).detach();}catch(const std::exception&e){AppLog("ERR creating proj: "+std::string(e.what()));}g_show_new_project_dialog=false;});
    auto new_project_cancel_button = Button("Cancel", [&]{g_show_new_project_dialog=false;});
    auto new_project_dialog_content = Container::Vertical({ Renderer([]{return text(L"New Project")|bold|center;}), new_project_name_input_comp, new_project_template_input_comp, Container::Horizontal({new_project_create_button, new_project_cancel_button})|center }) | border;

    auto delete_confirm_button = Button("DELETE", [&] { /* ... */
        if(g_path_to_delete.empty()){AppLog("ERR: No path to del.");g_show_delete_dialog=false;return;}try{if(std::filesystem::exists(g_path_to_delete)){if(std::filesystem::is_directory(g_path_to_delete))std::filesystem::remove_all(g_path_to_delete);else std::filesystem::remove(g_path_to_delete);AppLog("Deleted: "+g_path_to_delete.string());refresh_project_browser();std::thread(refresh_git_statuses_threaded).detach();}else AppLog("ERR: Item not found for del: "+g_path_to_delete.string());}catch(const std::exception&e){AppLog("ERR deleting: "+std::string(e.what()));}g_show_delete_dialog=false;});
    auto delete_cancel_button = Button("Cancel", [&]{g_show_delete_dialog=false;});
    auto delete_dialog_content = Container::Vertical({ Renderer([&]{return text(L"Confirm Deletion: "+StringToWString(g_item_name_to_delete_str))|bold|center;}), Renderer([]{return text(L"This cannot be undone.")|color(Color::Red);}), Container::Horizontal({delete_confirm_button,delete_cancel_button})|center})|border;

    auto command_palette_menu_comp = Menu(&g_command_palette_entries, &g_command_palette_selected);
    auto command_palette_modal_content = Container::Vertical({ Renderer([]{return text(L"Cmd Palette")|bold|center;}), command_palette_menu_comp|size(HEIGHT,LESS_THAN,15)|size(WIDTH,EQUAL,60) });

    // Line Editor Modal
    auto line_editor_modal_content = Container::Vertical({
        Renderer([&]{ return text(L"Edit Line " + StringToWString(std::to_string(g_editor_active_line_idx + 1))) | bold; }),
        line_editor_input_field, // This is the Input component
        // Buttons could be added here, e.g., Save, Cancel
        // For now, Enter in Input field will save, Escape will cancel (handled globally)
    }) | border | size(WIDTH, GREATER_THAN, 60); // Ensure it's wide enough

    // Assemble Layout with Modals
    auto base_ui_renderer = Renderer([&] { /* ... (same as before, uses info_panel_component) ... */
        Element h=header->Render()|size(HEIGHT,EQUAL,1); Element f=footer->Render()|size(HEIGHT,EQUAL,1);
        Element pbm=project_browser_menu->Render()|yflex_grow|border; Element tp=templates_panel->Render(); Element dp=docs_panel->Render();
        Element ip=info_panel_component->Render()|size(HEIGHT,EQUAL,7);
        Element lc=vbox({pbm,tp,dp,ip});
        Element ep=editor_pane->Render()|yflex_grow; Element sl=status_log->Render();
        Element rc=vbox({ep,sl});
        Element mca=hbox({lc|flex_basis(0)|flex_grow(1), separatorHeavy(), rc|flex_basis(0)|flex_grow(2)});
        return vbox({h, separatorHeavy(), mca|flex_grow, separatorHeavy(), f});
    });
    auto ui_cp_modal = Modal(base_ui_renderer, command_palette_modal_content, &g_show_command_palette);
    auto ui_new_proj_modal = Modal(ui_cp_modal, new_project_dialog_content, &g_show_new_project_dialog);
    auto ui_del_modal = Modal(ui_new_proj_modal, delete_dialog_content, &g_show_delete_dialog);
    auto ui_with_all_modals = Modal(ui_del_modal, line_editor_modal_content, &g_show_line_editor_modal); // Add line editor modal

    AppLog("UI Components assembled.");

    // Event Handling
    auto global_event_handler = CatchEvent(ui_with_all_modals, [&](Event event) {
        if (event == Event::Custom) { regenerate_outline_view(); return true; } // Generic refresh
        if (event == Event::Character('q') && event.control()) { /* ... */
             if (g_is_background_task_running.load()) AppLog("Task running. Quit again or wait."); else screen.ExitLoop(); return true;
        }

        // Modal specific Esc handling
        if (event == Event::Escape) {
            if (g_show_line_editor_modal) { g_show_line_editor_modal = false; return true; }
            if (g_show_delete_dialog) { g_show_delete_dialog = false; return true; }
            if (g_show_new_project_dialog) { g_show_new_project_dialog = false; return true; }
            if (g_show_command_palette) { g_show_command_palette = false; return true; }
        }

        // Modal specific Enter handling (and other events if consumed by modal)
        if (g_show_line_editor_modal) {
            if (event == Event::Return) { // Enter from Input field saves the line
                if (g_editor_active_line_idx >=0 && static_cast<size_t>(g_editor_active_line_idx) < g_editor_lines.size()) {
                     g_editor_lines[g_editor_active_line_idx] = g_editor_current_line_buffer;
                     AppLog("Line " + std::to_string(g_editor_active_line_idx + 1) + " updated.");
                     // TODO: mark file as dirty
                }
                g_show_line_editor_modal = false;
                return true;
            }
            return line_editor_modal_content->OnEvent(event); // Pass other events (like text input) to modal
        }
        if (g_show_delete_dialog) return delete_dialog_content->OnEvent(event);
        if (g_show_new_project_dialog) return new_project_dialog_content->OnEvent(event);
        if (g_show_command_palette) { /* ... (Palette logic, including Enter) ... */
             if (event == Event::Return) { if(!g_is_background_task_running.load()){ const auto&cmd_pair=PALETTE_COMMANDS[g_command_palette_selected]; AppLog("Palette: "+cmd_pair.first); g_show_command_palette=false; if(cmd_pair.second.rfind("build_",0)==0){std::string target=cmd_pair.second.substr(6); if(g_current_editor_file_path.empty())AppLog("ERR: No file for build context"); else if(g_root_makefile_path.empty())AppLog("ERR: Makefile not found"); else {std::filesystem::path pd=g_current_editor_file_path.parent(),bpd=pd.has_parent()?pd.parent().lexically_normal():std::filesystem::path(); if(bpd==DRAFTS_DIR_CPP.lexically_normal())std::thread(execute_build_in_thread,pd.string(),g_root_makefile_path.string(),target).detach(); else AppLog("ERR: File not in drafts ("+bpd.string()+" vs "+DRAFTS_DIR_CPP.string()+")");}} else if(cmd_pair.second=="git_refresh_status"){if(!g_is_git_status_loading.load())std::thread(refresh_git_statuses_threaded).detach();else AppLog("Git status refresh ongoing.");} else if(cmd_pair.second=="git_show_status_output")std::thread(execute_git_status_output_threaded).detach();} else AppLog("Task running."); return true;}
             return command_palette_modal_content->OnEvent(event);
        }

        // No dialogs active, handle main UI keys
        if (event == Event::Character('p') && event.control()) { /* ... */
             g_show_command_palette = !g_show_command_palette.load(); if (g_show_command_palette.load()) g_command_palette_selected = 0; return true;
        }

        // Editor Pane Navigation and Editing Trigger
        // Assuming editor pane is implicitly "active" if no modals are shown and project browser is not focused.
        // This part needs careful focus management, which is not fully implemented.
        // For now, global up/down arrows control active line if no modal.
        if (event == Event::ArrowUp) {
            if (!g_editor_lines.empty() && g_editor_active_line_idx > 0) {
                g_editor_active_line_idx--; return true;
            }
        } else if (event == Event::ArrowDown) {
            if (!g_editor_lines.empty() && g_editor_active_line_idx < static_cast<int>(g_editor_lines.size()) - 1) {
                g_editor_active_line_idx++; return true;
            }
        } else if (event == Event::Character('e')) { // 'e' to edit line
            if (!g_editor_lines.empty() && g_editor_active_line_idx >= 0 && static_cast<size_t>(g_editor_active_line_idx) < g_editor_lines.size()) {
                g_editor_current_line_buffer = g_editor_lines[g_editor_active_line_idx];
                g_show_line_editor_modal = true;
                // TODO: Need to ensure line_editor_input_field gets focus.
                // screen.SetFocus(line_editor_input_field) or similar if Modal doesn't handle it.
                // For now, assume Modal gives focus to first element.
                return true;
            }
        }

        if (project_browser_menu->Focused()) { /* ... (Project Browser n, d, Enter logic) ... */
            if(event==Event::Character('n')){g_new_project_name_input_str.clear();g_new_project_template_input_str.clear();g_show_new_project_dialog=true;return true;}
            if(event==Event::Character('d')){if(g_project_menu_selected>=0&&static_cast<size_t>(g_project_menu_selected)<g_project_files_entries.size()){const auto&entry=g_project_files_entries[g_project_menu_selected];if(entry.depth!=-1&&!entry.path.empty()&&std::filesystem::exists(entry.path)){g_path_to_delete=entry.path;g_item_name_to_delete_str=entry.path.filename().string();g_show_delete_dialog=true;return true;}else AppLog("Cannot del item.");}}
            if(event==Event::Return){if(g_project_menu_selected>=0&&static_cast<size_t>(g_project_menu_selected)<g_project_files_entries.size()){const auto&entry=g_project_files_entries[g_project_menu_selected];AppLog("Browser: "+entry.path.string()+(entry.is_directory?"[D]":"[F]"));if(entry.depth!=-1&&!entry.path.empty()&&!entry.is_directory){AppLog("Opening: "+entry.path.string());g_current_editor_file_path=entry.path;g_editor_lines.clear();std::ifstream fs(g_current_editor_file_path);if(fs.is_open()){std::string l;while(std::getline(fs,l)){size_t t=0;while((t=l.find('\t',t))!=std::string::npos)l.replace(t,1,"    ");g_editor_lines.push_back(l);}fs.close();g_current_file_display_name_w=g_current_editor_file_path.filename().native();AppLog("Loaded: "+entry.path.string());}else{AppLog("ERR open: "+entry.path.string());g_editor_lines.push_back("Error opening.");g_current_file_display_name_w=L"Error";}regenerate_outline_view();}return true;}}
            return project_browser_menu->OnEvent(event);
        }
        if (info_panel_component->Focused()) return info_panel_component->OnEvent(event);

        return false;
    });

    AppLog("WriteSpace TUI Ready.");
    screen.Loop(global_event_handler);
    g_screen_ptr = nullptr;
    return 0;
}
