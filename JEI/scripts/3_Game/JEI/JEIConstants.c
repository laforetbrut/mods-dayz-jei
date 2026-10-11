// JEI - Just Enough Items
// Author: vyrriox

const int JEI_RPC_CRAFT                = 0x4A454901;
const int JEI_RPC_SETTINGS_REQUEST     = 0x4A454902;
const int JEI_RPC_SETTINGS             = 0x4A454903;
const int JEI_RPC_CRAFT_RESULT         = 0x4A454904;
const int JEI_RPC_ADMIN_GIVE           = 0x4A454910;
const int JEI_RPC_ADMIN_SAVE_RECIPE    = 0x4A454911;
const int JEI_RPC_ADMIN_DELETE_RECIPE  = 0x4A454912;
const int JEI_RPC_ADMIN_TOGGLE_VANILLA = 0x4A454913;
const int JEI_RPC_ADMIN_RESULT         = 0x4A454914;

const string JEI_PROFILE_DIR   = "$profile:JEI";
const string JEI_SETTINGS_FILE = "$profile:JEI/settings.json";
const string JEI_RECIPES_FILE  = "$profile:JEI/custom_recipes.json";

const int JEI_MAX_LIST_ROWS = 800;
const int JEI_MAX_GIVE = 100;
