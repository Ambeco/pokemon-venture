#include "global.h"
#include "line_break.h"
#include "constants/characters.h"
#include "constants/vars.h"
#include "assertf.h"
#include "event_data.h"
#include "global.fieldmap.h"
#include "region_city.h"
#include "region_map.h"
#include "county_chain.h"
#include "plot_stage.h"
#include "subplot.h"
#include "region_county.h"
#include "region_tile.h"
#include "string_util.h"
#include "chapter.h"

static const u8 gText_Lilycove_Exhibit0FarCurrent[] = _("I heard the new exhibit hall at the MUSEUM in {HINT_CITY} is about to open.");
static const u8 gText_Lilycove_Exhibit0FarAfter0[] = _("The MUSEUM in {HINT_CITY} finally opened its new exhibit hall, I heard.");
static const u8 gText_Lilycove_Exhibit0FarAfter1[] = _("Did you hear? The exhibit hall at the MUSEUM in {HINT_CITY} is open.");
static const u8 gText_Lilycove_Exhibit0CloseCurrent[] = _("The curator is still busy preparing the\nexhibit hall.");
static const u8 gText_Lilycove_Exhibit0CloseAfter[] = _("The exhibit hall is open at last!\nThe curator must be so relieved.");
static const u8 gText_Lilycove_Exhibit1FarCurrent[] = _("The exhibit hall at the MUSEUM in {HINT_CITY} is open, and they want paintings for it!");
static const u8 gText_Lilycove_Exhibit1CloseBefore[] = _("There's talk of a new exhibit hall\n{HINT_ANCHOR}.\pIt's not ready yet, though.");
static const u8 gText_Lilycove_Exhibit1CloseCurrent[] = _("The curator is asking trainers for\npaintings to hang in the exhibit hall.");

static const u8 gText_Rustboro_Cut0FarCurrent[] = _("I heard a man in {HINT_CITY} is looking for a skilled TRAINER to give an HM to.");
static const u8 gText_Rustboro_Cut0FarAfter0[] = _("A man in {HINT_CITY} finally found a TRAINER worthy of his HIDDEN MACHINE.");
static const u8 gText_Rustboro_Cut0FarAfter1[] = _("Did you hear? Someone in {HINT_CITY} got an HM from a man who just stared at them.");
static const u8 gText_Rustboro_Cut0CloseCurrent[] = _("The man {HINT_ANCHOR} keeps staring at passing TRAINERS. What is he up to?");
static const u8 gText_Rustboro_Cut0CloseAfter[] = _("The man {HINT_ANCHOR} found his TRAINER. He was so pleased.");
static const u8 gText_Rustboro_Cut1FarCurrent[] = _("There's a TRAINER in {HINT_CITY} who was given an HM by a stranger, I heard.");
static const u8 gText_Rustboro_Cut1CloseBefore[] = _("People say the man {HINT_ANCHOR} has an HM to give away.");
static const u8 gText_Rustboro_Cut1CloseCurrent[] = _("That man {HINT_ANCHOR} gave away his HM. He says CUT can clear thin trees.");

static const u8 gText_Mossdeep_Rod0FarCurrent[] = _("I heard a fisherman in {HINT_CITY} is handing out SUPER RODS to anyone who asks.");
static const u8 gText_Mossdeep_Rod0FarAfter0[] = _("A fisherman in {HINT_CITY} gave someone a SUPER ROD, I heard.");
static const u8 gText_Mossdeep_Rod0FarAfter1[] = _("Did you hear? A fisherman in {HINT_CITY} gave away a SUPER ROD.");
static const u8 gText_Mossdeep_Rod0CloseCurrent[] = _("The fisherman {HINT_ANCHOR} won't stop talking about SUPER RODS.");
static const u8 gText_Mossdeep_Rod0CloseAfter[] = _("The fisherman {HINT_ANCHOR} finally gave his SUPER ROD away.");
static const u8 gText_Mossdeep_Rod1FarCurrent[] = _("Somebody in {HINT_CITY} got a SUPER ROD from a fisherman, I heard.");
static const u8 gText_Mossdeep_Rod1CloseBefore[] = _("People say the fisherman {HINT_ANCHOR} has a SUPER ROD to give away.");
static const u8 gText_Mossdeep_Rod1CloseCurrent[] = _("The fisherman {HINT_ANCHOR} says a SUPER ROD catches POKéMON off the seafloor.");

static const u8 gText_Dewford_Rod0FarCurrent[] = _("I heard a fisherman in {HINT_CITY} is handing out an OLD ROD to anyone with the itch to fish.");
static const u8 gText_Dewford_Rod0FarAfter0[] = _("A fisherman in {HINT_CITY} gave someone an OLD ROD, I heard.");
static const u8 gText_Dewford_Rod0FarAfter1[] = _("Did you hear? A fisherman in {HINT_CITY} gave away an OLD ROD.");
static const u8 gText_Dewford_Rod0CloseCurrent[] = _("The fisherman {HINT_ANCHOR} says this is a renowned fishing spot. Ask him about it!");
static const u8 gText_Dewford_Rod0CloseAfter[] = _("The fisherman {HINT_ANCHOR} gave his OLD ROD to a TRAINER.");
static const u8 gText_Dewford_Rod1FarCurrent[] = _("Somebody in {HINT_CITY} got an OLD ROD from a fisherman, I heard.");
static const u8 gText_Dewford_Rod1CloseBefore[] = _("People say the fisherman {HINT_ANCHOR} has an OLD ROD to give away.");
static const u8 gText_Dewford_Rod1CloseCurrent[] = _("The fisherman {HINT_ANCHOR} asks how your fishing is going.");

static const u8 gText_Route118_Rod0FarCurrent[] = _("I heard a fisherman by the river in {HINT_CITY} gives a GOOD ROD to anyone who agrees with him.");
static const u8 gText_Route118_Rod0FarAfter0[] = _("A fisherman in {HINT_CITY} gave someone a GOOD ROD, I heard.");
static const u8 gText_Route118_Rod0FarAfter1[] = _("Did you hear? A fisherman in {HINT_CITY} gave away a GOOD ROD.");
static const u8 gText_Route118_Rod0CloseCurrent[] = _("The fisherman {HINT_ANCHOR} is sure a GOOD ROD is really good. Ask him if you agree!");
static const u8 gText_Route118_Rod0CloseAfter[] = _("The fisherman {HINT_ANCHOR} gave his GOOD ROD to a TRAINER.");
static const u8 gText_Route118_Rod1FarCurrent[] = _("Somebody in {HINT_CITY} got a GOOD ROD from a fisherman, I heard.");
static const u8 gText_Route118_Rod1CloseBefore[] = _("People say the fisherman {HINT_ANCHOR} has a GOOD ROD to give away.");
static const u8 gText_Route118_Rod1CloseCurrent[] = _("The fisherman {HINT_ANCHOR} says to try catching all sorts of POKéMON with a GOOD ROD.");

static const u8 gText_Route110_Rival0FarCurrent[] = _("I heard your rival is looking for you around {HINT_CITY}.");
static const u8 gText_Route110_Rival0CloseCurrent[] = _("Your rival is looking for you {HINT_ANCHOR}. Show how strong your POKéMON have grown!");
static const u8 gText_Route110_Rival1FarCurrent[] = _("Somebody around {HINT_CITY} beat their rival, I heard.");
static const u8 gText_Route110_Rival1CloseCurrent[] = _("Your rival was here {HINT_ANCHOR}, and has already raced off.");

static const u8 gText_Route110_Finder0FarCurrent[] = _("I heard a rival around {HINT_CITY} rewards a good battle with an ITEMFINDER.");
static const u8 gText_Route110_Finder0CloseCurrent[] = _("A rival {HINT_ANCHOR} gives an ITEMFINDER to anyone who proves themselves in battle.");
static const u8 gText_Route110_Finder1FarCurrent[] = _("Somebody around {HINT_CITY} got an ITEMFINDER from a rival, I heard.");
static const u8 gText_Route110_Finder1CloseCurrent[] = _("A rival {HINT_ANCHOR} gave an ITEMFINDER to a TRAINER.");

static const u8 gText_Route119_Rival0FarCurrent[] = _("I heard your rival is racing around {HINT_CITY} on a BIKE.");
static const u8 gText_Route119_Rival0CloseCurrent[] = _("Your rival is waiting {HINT_ANCHOR} for another battle.");
static const u8 gText_Route119_Rival1FarCurrent[] = _("Somebody around {HINT_CITY} still has a present coming from their rival, I heard.");
static const u8 gText_Route119_Rival1CloseCurrent[] = _("Your rival {HINT_ANCHOR} wants to give you a present. Make room in your bag!");
static const u8 gText_Route119_Rival2FarCurrent[] = _("Somebody around {HINT_CITY} got a present from their rival, I heard.");
static const u8 gText_Route119_Rival2CloseCurrent[] = _("Your rival raced off {HINT_ANCHOR} after giving you a present.");

static const u8 gText_Lilycove_Rival0FarCurrent[] = _("I heard your rival is shopping around {HINT_CITY} and wants a battle.");
static const u8 gText_Lilycove_Rival0CloseCurrent[] = _("Your rival is out shopping {HINT_ANCHOR}. Show how strong your POKéMON have grown!");
static const u8 gText_Lilycove_Rival1FarCurrent[] = _("Somebody around {HINT_CITY} battled their rival while shopping, I heard.");
static const u8 gText_Lilycove_Rival1CloseCurrent[] = _("Your rival was here {HINT_ANCHOR}, and has already flown home.");

static const u8 gText_Rival2_Hint0FarCurrent[] = _("I heard your rival is looking for you around {HINT_CITY} to compare POKéDEX entries.");
static const u8 gText_Rival2_Hint0CloseCurrent[] = _("Your rival is waiting {HINT_ANCHOR} for a battle. Show how much your POKéMON have grown!");
static const u8 gText_Rival2_Hint1FarCurrent[] = _("Somebody around {HINT_CITY} battled their rival and registered in the POKéNAV, I heard.");
static const u8 gText_Rival2_Hint1CloseCurrent[] = _("Your rival was here {HINT_ANCHOR}, and has already moved on.");

static const u8 gText_Mauville_Smash0FarCurrent[] = _("I heard a man in {HINT_CITY} hands HIDDEN MACHINES to anyone with strong POKéMON.");
static const u8 gText_Mauville_Smash0FarAfter0[] = _("A man in {HINT_CITY} gave someone an HM for smashing rocks, I heard.");
static const u8 gText_Mauville_Smash0FarAfter1[] = _("Did you hear? A man in {HINT_CITY} gave away an HM, and he's still shouting about it.");
static const u8 gText_Mauville_Smash0CloseCurrent[] = _("The man {HINT_ANCHOR} wants to meet TRAINERS with strong POKéMON. Woohoo!");
static const u8 gText_Mauville_Smash0CloseAfter[] = _("The man {HINT_ANCHOR} gave a TRAINER his HM. He's still celebrating.");
static const u8 gText_Mauville_Smash1FarCurrent[] = _("Somebody in {HINT_CITY} got an HM from a very loud man, I heard.");
static const u8 gText_Mauville_Smash1CloseBefore[] = _("People say the man {HINT_ANCHOR} has an HM to give to a strong TRAINER.");
static const u8 gText_Mauville_Smash1CloseCurrent[] = _("The man {HINT_ANCHOR} says a smashed rock can hide a POKéMON.");

static const u8 gText_Mauville_Case0FarCurrent[] = _("I heard a woman in {HINT_CITY} will trade a COIN CASE for HARBOR MAIL from a POKéMON MART.");
static const u8 gText_Mauville_Case0FarAfter[] = _("A woman in {HINT_CITY} traded a COIN CASE for a HARBOR MAIL, I heard.");
static const u8 gText_Mauville_Case0CloseCurrent[] = _("The woman {HINT_ANCHOR} wishes she could shop for HARBOR MAIL at a faraway MART.");
static const u8 gText_Mauville_Case0CloseAfter[] = _("The woman {HINT_ANCHOR} got her HARBOR MAIL and gave away a COIN CASE.");
static const u8 gText_Mauville_Case1FarCurrent[] = _("Somebody in {HINT_CITY} can play at the GAME CORNER now, thanks to a COIN CASE.");
static const u8 gText_Mauville_Case1CloseBefore[] = _("People say the woman {HINT_ANCHOR} swaps a COIN CASE for HARBOR MAIL.");
static const u8 gText_Mauville_Case1CloseCurrent[] = _("The woman {HINT_ANCHOR} already traded away her COIN CASE.");

static const u8 gText_Slateport_Mail0FarCurrent[] = _("I heard the MART in {HINT_CITY} sells HARBOR MAIL, and nowhere else does.");
static const u8 gText_Slateport_Mail0FarAfter[] = _("Somebody bought HARBOR MAIL at the MART in {HINT_CITY}, I heard.");
static const u8 gText_Slateport_Mail0CloseCurrent[] = _("The clerk {HINT_ANCHOR} says HARBOR MAIL is on the shelf. It's a local specialty.");
static const u8 gText_Slateport_Mail0CloseAfter[] = _("The clerk {HINT_ANCHOR} sold a TRAINER a HARBOR MAIL.");
static const u8 gText_Slateport_Mail1FarCurrent[] = _("Somebody in {HINT_CITY} bought HARBOR MAIL, I heard. What for?");
static const u8 gText_Slateport_Mail1CloseBefore[] = _("People say the MART {HINT_ANCHOR} stocks HARBOR MAIL.");
static const u8 gText_Slateport_Mail1CloseCurrent[] = _("The clerk {HINT_ANCHOR} is proud that HARBOR MAIL sold out once.");

static const u8 gText_Mauville_Bike0FarCurrent[] = _("I heard a bike shop owner in {HINT_CITY} gives a free BICYCLE to TRAINERS who came a long way.");
static const u8 gText_Mauville_Bike0FarAfter[] = _("The bike shop owner in {HINT_CITY} gave someone a BICYCLE, I heard.");
static const u8 gText_Mauville_Bike0CloseCurrent[] = _("The owner {HINT_ANCHOR} is looking for someone with dirty RUNNING SHOES.");
static const u8 gText_Mauville_Bike0CloseAfter[] = _("The owner {HINT_ANCHOR} gave a TRAINER a BICYCLE. He'll swap it any time.");
static const u8 gText_Mauville_Bike1FarCurrent[] = _("Somebody in {HINT_CITY} rides a BICYCLE from a shop there, I heard.");
static const u8 gText_Mauville_Bike1CloseBefore[] = _("People say the owner {HINT_ANCHOR} hands out a BICYCLE for free.");
static const u8 gText_Mauville_Bike1CloseCurrent[] = _("The owner {HINT_ANCHOR} will swap your BICYCLE for the other kind if you ask.");

static const u8 gText_Mauville_Gen0FarCurrent[] = _("I heard the GYM LEADER of {HINT_CITY} needs a favor once a strong TRAINER shows up.");
static const u8 gText_Mauville_Gen0CloseCurrent[] = _("WATTSON {HINT_ANCHOR} has been fretting about the old generator below the city.");
static const u8 gText_Mauville_Gen1FarCurrent[] = _("Somebody in {HINT_CITY} got a KEY to a place under the city, I heard.");
static const u8 gText_Mauville_Gen1CloseCurrent[] = _("WATTSON gave a TRAINER the key to the basement. The old place is {HINT_ANCHOR}.");
static const u8 gText_Mauville_Gen2FarCurrent[] = _("The lights under {HINT_CITY} went quiet, I heard. A TRAINER did it.");
static const u8 gText_Mauville_Gen2CloseCurrent[] = _("The generator is off at last. WATTSON {HINT_ANCHOR} is waiting for a report.");
static const u8 gText_Mauville_Gen3FarCurrent[] = _("WATTSON in {HINT_CITY} rewarded a TRAINER for shutting down the old generator.");
static const u8 gText_Mauville_Gen3CloseCurrent[] = _("WATTSON {HINT_ANCHOR} is happy again. The generator's quiet.");

static const u8 gText_Littleroot_Birch0FarCurrent[] = _("PROF. BIRCH of {HINT_CITY} spends all day in the tall grass studying wild POKéMON, I heard.");
static const u8 gText_Littleroot_Birch0FarAfter[] = _("PROF. BIRCH of {HINT_CITY} was jumped by a wild POKéMON, but a TRAINER saved him, I heard.");
static const u8 gText_Littleroot_Birch0CloseCurrent[] = _("PROF. BIRCH is out studying wild POKéMON again. I hope he's careful.");
static const u8 gText_Littleroot_Birch1FarCurrent[] = _("I heard PROF. BIRCH in {HINT_CITY} is in trouble, and nobody with a POKéMON is around to help.");
static const u8 gText_Littleroot_Birch1CloseCurrent[] = _("Something is going on {HINT_ANCHOR}. PROF. BIRCH is in trouble!");
static const u8 gText_Littleroot_Birch1CloseBefore[] = _("PROF. BIRCH went out {HINT_ANCHOR} to study wild POKéMON. It's dangerous without a POKéMON.");
static const u8 gText_Littleroot_Birch2FarCurrent[] = _("PROF. BIRCH of {HINT_CITY} gave a POKéMON to the TRAINER who saved him, I heard.");
static const u8 gText_Littleroot_Birch2CloseCurrent[] = _("PROF. BIRCH gave his rescuer a POKéMON. He wants them to visit his LAB.");

static const u8 gText_Littleroot_Move0FarCurrent[] = _("A moving truck pulled into {HINT_CITY} today, I heard. New neighbors!");
static const u8 gText_Littleroot_Move0CloseCurrent[] = _("A moving truck just pulled in {HINT_ANCHOR}. We have new neighbors!");
static const u8 gText_Littleroot_Move1FarCurrent[] = _("The new family in {HINT_CITY} is still unpacking, I heard. Their wall clock isn't even set.");
static const u8 gText_Littleroot_Move1CloseCurrent[] = _("The new family {HINT_ANCHOR} is still unpacking. Their clock isn't even set yet.");
static const u8 gText_Littleroot_Move2FarCurrent[] = _("The new family in {HINT_CITY} has settled in, I heard. Their kid should meet the neighbors.");
static const u8 gText_Littleroot_Move2CloseCurrent[] = _("The new family {HINT_ANCHOR} has settled in. Their kid should go and meet the neighbor.");
static const u8 gText_Littleroot_Move2FarAfter[] = _("A new family moved into {HINT_CITY} not long ago, I heard.");

static const u8 gText_Oldale_Dex0FarCurrent[] = _("A TRAINER from {HINT_CITY} is waiting to challenge anyone with a POKéMON, I heard.");
static const u8 gText_Oldale_Dex0CloseCurrent[] = _("The kid {HINT_ANCHOR} keeps asking passersby for a POKéMON battle.");
static const u8 gText_Oldale_Dex1FarCurrent[] = _("PROF. BIRCH of {HINT_CITY} wants to hand out a POKéDEX, I heard.");
static const u8 gText_Oldale_Dex1CloseCurrent[] = _("PROF. BIRCH has a POKéDEX for you. His LAB is {HINT_ANCHOR}.");
static const u8 gText_Oldale_Dex2FarCurrent[] = _("A TRAINER from {HINT_CITY} got a POKéDEX and set off to see the world, I heard.");
static const u8 gText_Oldale_Dex2CloseCurrent[] = _("A TRAINER with a POKéDEX and RUNNING SHOES came through {HINT_ANCHOR}.");

static const u8 *const sOldaleDex0Far[] = {gText_Oldale_Dex0FarCurrent};
static const u8 *const sOldaleDex0Close[] = {gText_Oldale_Dex0CloseCurrent};
static const u8 *const sOldaleDex1Far[] = {gText_Oldale_Dex1FarCurrent};
static const u8 *const sOldaleDex1Close[] = {gText_Oldale_Dex1CloseCurrent};
static const u8 *const sOldaleDex2Far[] = {gText_Oldale_Dex2FarCurrent};
static const u8 *const sOldaleDex2Close[] = {gText_Oldale_Dex2CloseCurrent};

static const u8 *const sLittlerootMove0Far[] ={gText_Littleroot_Move0FarCurrent};
static const u8 *const sLittlerootMove0Close[] = {gText_Littleroot_Move0CloseCurrent};
static const u8 *const sLittlerootMove1Far[] = {gText_Littleroot_Move1FarCurrent};
static const u8 *const sLittlerootMove1Close[] = {gText_Littleroot_Move1CloseCurrent};
static const u8 *const sLittlerootMove2Far[] = {gText_Littleroot_Move2FarCurrent, gText_Littleroot_Move2FarAfter};
static const u8 *const sLittlerootMove2Close[] = {gText_Littleroot_Move2CloseCurrent};

static const u8 *const sLittlerootStage01Far[] ={gText_Littleroot_Birch0FarCurrent};
static const u8 *const sLittlerootStage01Close[] = {gText_Littleroot_Birch0CloseCurrent, gText_Littleroot_Birch1CloseBefore};
static const u8 *const sLittlerootStage23Far[] = {gText_Littleroot_Birch1FarCurrent};
static const u8 *const sLittlerootStage23Close[] = {gText_Littleroot_Birch1CloseCurrent};
static const u8 *const sLittlerootStage45Far[] = {gText_Littleroot_Birch2FarCurrent, gText_Littleroot_Birch0FarAfter};
static const u8 *const sLittlerootStage45Close[] = {gText_Littleroot_Birch2CloseCurrent};

static const u8 gText_Lilycove_Case0FarCurrent[] = _("I heard the CONTEST HALL in {HINT_CITY} hands a POKéBLOCK CASE to newcomers.");
static const u8 gText_Lilycove_Case0FarAfter[] = _("A TRAINER got a POKéBLOCK CASE at the CONTEST HALL in {HINT_CITY}, I heard.");
static const u8 gText_Lilycove_Case0CloseCurrent[] = _("The receptionist {HINT_ANCHOR} has a POKéBLOCK CASE for you if you enter a CONTEST.");
static const u8 gText_Lilycove_Case0CloseAfter[] = _("The receptionist {HINT_ANCHOR} gave a TRAINER a POKéBLOCK CASE.");
static const u8 gText_Lilycove_Case1FarCurrent[] = _("Someone in {HINT_CITY} can now make POKéBLOCKS, thanks to the CONTEST HALL.");
static const u8 gText_Lilycove_Case1CloseBefore[] = _("People say the receptionist {HINT_ANCHOR} hands out a POKéBLOCK CASE.");
static const u8 gText_Lilycove_Case1CloseCurrent[] = _("The receptionist {HINT_ANCHOR} already gave out the POKéBLOCK CASE.");

static const u8 *const sLilycoveCase0Far[] = {gText_Lilycove_Case0FarCurrent};
static const u8 *const sLilycoveCase0Close[] = {gText_Lilycove_Case0CloseCurrent, gText_Lilycove_Case1CloseBefore};
static const u8 *const sLilycoveCase1Far[] = {gText_Lilycove_Case1FarCurrent, gText_Lilycove_Case0FarAfter};
static const u8 *const sLilycoveCase1Close[] = {gText_Lilycove_Case1CloseCurrent, gText_Lilycove_Case0CloseAfter};

static const u8 gText_Rustboro_Fossil0FarCurrent[] = _("I heard a researcher in {HINT_CITY} can bring fossils back to life.");
static const u8 gText_Rustboro_Fossil0FarAfter[] = _("A TRAINER in {HINT_CITY} had a fossil revived into a live POKéMON, I heard.");
static const u8 gText_Rustboro_Fossil0CloseCurrent[] = _("The DEVON researcher says his machine can revive fossils. Got any?");
static const u8 gText_Rustboro_Fossil1CloseBefore[] = _("Bring a fossil to the DEVON researcher, and his machine will revive it.");
static const u8 gText_Rustboro_Fossil1FarCurrent[] = _("A researcher in {HINT_CITY} is reviving a fossil right now, I heard.");
static const u8 gText_Rustboro_Fossil1CloseCurrent[] = _("The fossil machine is still running. The researcher says it takes a while.");
static const u8 gText_Rustboro_Fossil2FarCurrent[] = _("The fossil in {HINT_CITY} is revived and waiting for its TRAINER, I heard.");
static const u8 gText_Rustboro_Fossil2CloseCurrent[] = _("The fossil machine has finished. Talk to the researcher!");

static const u8 *const sRustboroFossil0Far[] = {gText_Rustboro_Fossil0FarCurrent, gText_Rustboro_Fossil0FarAfter};
static const u8 *const sRustboroFossil0Close[] = {gText_Rustboro_Fossil0CloseCurrent, gText_Rustboro_Fossil1CloseBefore};
static const u8 *const sRustboroFossil1Far[] = {gText_Rustboro_Fossil1FarCurrent};
static const u8 *const sRustboroFossil1Close[] = {gText_Rustboro_Fossil1CloseCurrent};
static const u8 *const sRustboroFossil2Far[] = {gText_Rustboro_Fossil2FarCurrent};
static const u8 *const sRustboroFossil2Close[] = {gText_Rustboro_Fossil2CloseCurrent};

static const u8 *const sLilycoveStage0Far[] ={gText_Lilycove_Exhibit0FarCurrent};
static const u8 *const sLilycoveStage0Close[] = {gText_Lilycove_Exhibit0CloseCurrent, gText_Lilycove_Exhibit1CloseBefore};
static const u8 *const sLilycoveStage1Far[] = {gText_Lilycove_Exhibit1FarCurrent, gText_Lilycove_Exhibit0FarAfter0, gText_Lilycove_Exhibit0FarAfter1};
static const u8 *const sLilycoveStage1Close[] = {gText_Lilycove_Exhibit1CloseCurrent, gText_Lilycove_Exhibit0CloseAfter};

static const u8 *const sRustboroStage0Far[] = {gText_Rustboro_Cut0FarCurrent};
static const u8 *const sRustboroStage0Close[] = {gText_Rustboro_Cut0CloseCurrent, gText_Rustboro_Cut1CloseBefore};
static const u8 *const sRustboroStage1Far[] = {gText_Rustboro_Cut1FarCurrent, gText_Rustboro_Cut0FarAfter0, gText_Rustboro_Cut0FarAfter1};
static const u8 *const sRustboroStage1Close[] = {gText_Rustboro_Cut1CloseCurrent, gText_Rustboro_Cut0CloseAfter};

static const u8 *const sMossdeepStage0Far[] = {gText_Mossdeep_Rod0FarCurrent};
static const u8 *const sMossdeepStage0Close[] = {gText_Mossdeep_Rod0CloseCurrent, gText_Mossdeep_Rod1CloseBefore};
static const u8 *const sMossdeepStage1Far[] = {gText_Mossdeep_Rod1FarCurrent, gText_Mossdeep_Rod0FarAfter0, gText_Mossdeep_Rod0FarAfter1};
static const u8 *const sMossdeepStage1Close[] = {gText_Mossdeep_Rod1CloseCurrent, gText_Mossdeep_Rod0CloseAfter};

static const u8 *const sDewfordStage0Far[] = {gText_Dewford_Rod0FarCurrent};
static const u8 *const sDewfordStage0Close[] = {gText_Dewford_Rod0CloseCurrent, gText_Dewford_Rod1CloseBefore};
static const u8 *const sDewfordStage1Far[] = {gText_Dewford_Rod1FarCurrent, gText_Dewford_Rod0FarAfter0, gText_Dewford_Rod0FarAfter1};
static const u8 *const sDewfordStage1Close[] = {gText_Dewford_Rod1CloseCurrent, gText_Dewford_Rod0CloseAfter};

static const u8 *const sRoute118Stage0Far[] = {gText_Route118_Rod0FarCurrent};
static const u8 *const sRoute118Stage0Close[] = {gText_Route118_Rod0CloseCurrent, gText_Route118_Rod1CloseBefore};
static const u8 *const sRoute118Stage1Far[] = {gText_Route118_Rod1FarCurrent, gText_Route118_Rod0FarAfter0, gText_Route118_Rod0FarAfter1};
static const u8 *const sRoute118Stage1Close[] = {gText_Route118_Rod1CloseCurrent, gText_Route118_Rod0CloseAfter};

static const u8 *const sRoute110Rival0Far[] = {gText_Route110_Rival0FarCurrent};
static const u8 *const sRoute110Rival0Close[] = {gText_Route110_Rival0CloseCurrent};
static const u8 *const sRoute110Rival1Far[] = {gText_Route110_Rival1FarCurrent};
static const u8 *const sRoute110Rival1Close[] = {gText_Route110_Rival1CloseCurrent};
static const u8 *const sRoute110Finder0Far[] = {gText_Route110_Finder0FarCurrent};
static const u8 *const sRoute110Finder0Close[] = {gText_Route110_Finder0CloseCurrent};
static const u8 *const sRoute110Finder1Far[] = {gText_Route110_Finder1FarCurrent};
static const u8 *const sRoute110Finder1Close[] = {gText_Route110_Finder1CloseCurrent};
static const u8 *const sRoute119Rival0Far[] = {gText_Route119_Rival0FarCurrent};
static const u8 *const sRoute119Rival0Close[] = {gText_Route119_Rival0CloseCurrent};
static const u8 *const sRoute119Rival1Far[] = {gText_Route119_Rival1FarCurrent};
static const u8 *const sRoute119Rival1Close[] = {gText_Route119_Rival1CloseCurrent};
static const u8 *const sRoute119Rival2Far[] = {gText_Route119_Rival2FarCurrent};
static const u8 *const sRoute119Rival2Close[] = {gText_Route119_Rival2CloseCurrent};

static const u8 *const sLilycoveRival0Far[] = {gText_Lilycove_Rival0FarCurrent};
static const u8 *const sLilycoveRival0Close[] = {gText_Lilycove_Rival0CloseCurrent};
static const u8 *const sLilycoveRival1Far[] = {gText_Lilycove_Rival1FarCurrent};
static const u8 *const sLilycoveRival1Close[] = {gText_Lilycove_Rival1CloseCurrent};
static const u8 *const sRival2Hint0Far[] = {gText_Rival2_Hint0FarCurrent};
static const u8 *const sRival2Hint0Close[] = {gText_Rival2_Hint0CloseCurrent};
static const u8 *const sRival2Hint1Far[] = {gText_Rival2_Hint1FarCurrent};
static const u8 *const sRival2Hint1Close[] = {gText_Rival2_Hint1CloseCurrent};

static const u8 *const sMauvilleStage0Far[] = {gText_Mauville_Smash0FarCurrent};
static const u8 *const sMauvilleStage0Close[] = {gText_Mauville_Smash0CloseCurrent, gText_Mauville_Smash1CloseBefore};
static const u8 *const sMauvilleStage1Far[] = {gText_Mauville_Smash1FarCurrent, gText_Mauville_Smash0FarAfter0, gText_Mauville_Smash0FarAfter1};
static const u8 *const sMauvilleStage1Close[] = {gText_Mauville_Smash1CloseCurrent, gText_Mauville_Smash0CloseAfter};

static const u8 *const sMauvilleCase0Far[] = {gText_Mauville_Case0FarCurrent};
static const u8 *const sMauvilleCase0Close[] = {gText_Mauville_Case0CloseCurrent, gText_Mauville_Case1CloseBefore};
static const u8 *const sMauvilleCase1Far[] = {gText_Mauville_Case1FarCurrent, gText_Mauville_Case0FarAfter};
static const u8 *const sMauvilleCase1Close[] = {gText_Mauville_Case1CloseCurrent, gText_Mauville_Case0CloseAfter};

static const u8 *const sSlateportMail0Far[] = {gText_Slateport_Mail0FarCurrent};
static const u8 *const sSlateportMail0Close[] = {gText_Slateport_Mail0CloseCurrent, gText_Slateport_Mail1CloseBefore};
static const u8 *const sSlateportMail1Far[] = {gText_Slateport_Mail1FarCurrent, gText_Slateport_Mail0FarAfter};
static const u8 *const sSlateportMail1Close[] = {gText_Slateport_Mail1CloseCurrent, gText_Slateport_Mail0CloseAfter};

static const u8 *const sMauvilleBike0Far[] = {gText_Mauville_Bike0FarCurrent};
static const u8 *const sMauvilleBike0Close[] = {gText_Mauville_Bike0CloseCurrent, gText_Mauville_Bike1CloseBefore};
static const u8 *const sMauvilleBike1Far[] = {gText_Mauville_Bike1FarCurrent, gText_Mauville_Bike0FarAfter};
static const u8 *const sMauvilleBike1Close[] = {gText_Mauville_Bike1CloseCurrent, gText_Mauville_Bike0CloseAfter};

static const u8 *const sMauvilleGen0Far[] = {gText_Mauville_Gen0FarCurrent};
static const u8 *const sMauvilleGen0Close[] = {gText_Mauville_Gen0CloseCurrent};
static const u8 *const sMauvilleGen1Far[] = {gText_Mauville_Gen1FarCurrent};
static const u8 *const sMauvilleGen1Close[] = {gText_Mauville_Gen1CloseCurrent};
static const u8 *const sMauvilleGen2Far[] = {gText_Mauville_Gen2FarCurrent};
static const u8 *const sMauvilleGen2Close[] = {gText_Mauville_Gen2CloseCurrent};
static const u8 *const sMauvilleGen3Far[] = {gText_Mauville_Gen3FarCurrent};
static const u8 *const sMauvilleGen3Close[] = {gText_Mauville_Gen3CloseCurrent};

static const u8 gText_Starter0Far[] = _("I heard a POKéMON researcher in {HINT_CITY} got chased into the tall grass by a wild POKéMON.");
static const u8 gText_Starter0Close[] = _("Someone is calling for help\n{HINT_ANCHOR}!");
static const u8 gText_Starter1Far[] = _("A kid from {HINT_CITY} set off with a first POKéMON from the PROFESSOR, I heard.");
static const u8 gText_Starter1Close[] = _("A kid {HINT_ANCHOR} came through with a brand-new POKéMON.");
static const u8 *const sStarter0Far[] = {gText_Starter0Far};
static const u8 *const sStarter0Close[] = {gText_Starter0Close};
static const u8 *const sStarter1Far[] = {gText_Starter1Far};
static const u8 *const sStarter1Close[] = {gText_Starter1Close};

static const u8 gText_Pokenav0Far[] = _("I heard DEVON CORPORATION in {HINT_CITY} wants to hand a new gadget to a TRAINER.");
static const u8 gText_Pokenav0Close[] = _("DEVON CORPORATION {HINT_ANCHOR} is looking for a TRAINER to try its new gadget.");
static const u8 gText_Pokenav1Far[] = _("A TRAINER in {HINT_CITY} got a POKéNAV from DEVON CORPORATION, I heard.");
static const u8 gText_Pokenav1Close[] = _("DEVON CORPORATION {HINT_ANCHOR} gave its POKéNAV to a TRAINER.");
static const u8 *const sPokenav0Far[] = {gText_Pokenav0Far};
static const u8 *const sPokenav0Close[] = {gText_Pokenav0Close};
static const u8 *const sPokenav1Far[] = {gText_Pokenav1Far};
static const u8 *const sPokenav1Close[] = {gText_Pokenav1Close};

static const u8 gText_Shipyard0Far[] = _("I heard the SEASIDE CARGO dock in {HINT_CITY} is waiting on a delivery.");
static const u8 gText_Shipyard0Close[] = _("The dock workers {HINT_ANCHOR} are waiting for a delivery.");
static const u8 gText_Shipyard1Far[] = _("A TRAINER stopped by the dock in {HINT_CITY} with a delivery, I heard.");
static const u8 gText_Shipyard1Close[] = _("The dock {HINT_ANCHOR} is busy again after that delivery.");
static const u8 gText_ShipyardRejectedFar[] = _("I heard the dock in {HINT_CITY} turned a delivery away.");
static const u8 gText_ShipyardRejectedClose[] = _("The dock workers {HINT_ANCHOR} turned down a delivery. They're still grumbling.");
static const u8 *const sShipyardRejectedFar[] = {gText_ShipyardRejectedFar};
static const u8 *const sShipyardRejectedClose[] = {gText_ShipyardRejectedClose};
static const u8 *const sShipyard0Far[] = {gText_Shipyard0Far};
static const u8 *const sShipyard0Close[] = {gText_Shipyard0Close};
static const u8 *const sShipyard1Far[] = {gText_Shipyard1Far};
static const u8 *const sShipyard1Close[] = {gText_Shipyard1Close};

static const u8 gText_EliteFour0Far[] = _("They say the POKéMON LEAGUE in {HINT_CITY} only lets in TRAINERS with eight BADGES.");
static const u8 gText_EliteFour0Close[] = _("The guard {HINT_ANCHOR} only lets in TRAINERS with eight BADGES.");
static const u8 gText_EliteFour1Far[] = _("A TRAINER from {HINT_CITY} is taking on the ELITE FOUR, I heard.");
static const u8 gText_EliteFour1Close[] = _("A TRAINER went through {HINT_ANCHOR} to face the ELITE FOUR.");
static const u8 gText_EliteFourMidFar[] = _("A TRAINER from {HINT_CITY} is fighting through the ELITE FOUR one by one, I heard.");
static const u8 gText_EliteFourMidClose[] = _("A TRAINER {HINT_ANCHOR} is partway through the ELITE FOUR.");
static const u8 gText_EliteFourLastFar[] = _("I heard the ELITE FOUR in {HINT_CITY} all fell, and only the CHAMPION is left.");
static const u8 gText_EliteFourLastClose[] = _("Only the CHAMPION is left\n{HINT_ANCHOR}.");
static const u8 *const sEliteFourMidFar[] = {gText_EliteFourMidFar};
static const u8 *const sEliteFourMidClose[] = {gText_EliteFourMidClose};
static const u8 *const sEliteFourLastFar[] = {gText_EliteFourLastFar};
static const u8 *const sEliteFourLastClose[] = {gText_EliteFourLastClose};
static const u8 *const sEliteFour0Far[] = {gText_EliteFour0Far};
static const u8 *const sEliteFour0Close[] = {gText_EliteFour0Close};
static const u8 *const sEliteFour1Far[] = {gText_EliteFour1Far};
static const u8 *const sEliteFour1Close[] = {gText_EliteFour1Close};

static const u8 gText_Champion0Far[] = _("They say the CHAMPION in {HINT_CITY} waits for whoever beats the ELITE FOUR.");
static const u8 gText_Champion0Close[] = _("The CHAMPION waits {HINT_ANCHOR} for a challenger.");
static const u8 gText_Champion1Far[] = _("Someone from {HINT_CITY} became the new CHAMPION, I heard.");
static const u8 gText_Champion1Close[] = _("The HALL OF FAME {HINT_ANCHOR} has a brand-new entry.");
static const u8 *const sChampion0Far[] = {gText_Champion0Far};
static const u8 *const sChampion0Close[] = {gText_Champion0Close};
static const u8 *const sChampion1Far[] = {gText_Champion1Far};
static const u8 *const sChampion1Close[] = {gText_Champion1Close};

static const u8 gText_Lobby0Far[] = _("I heard the road to the POKéMON LEAGUE in {HINT_CITY} ends at a lobby for TRAINERS.");
static const u8 gText_Lobby0Close[] = _("The lobby {HINT_ANCHOR} is quiet. Few TRAINERS get this far.");
static const u8 gText_Lobby1Far[] = _("A TRAINER from {HINT_CITY} walked through the LEAGUE lobby, I heard.");
static const u8 gText_Lobby1Close[] = _("A TRAINER walked through the lobby {HINT_ANCHOR} to the ELITE FOUR.");
static const u8 *const sLobby0Far[] = {gText_Lobby0Far};
static const u8 *const sLobby0Close[] = {gText_Lobby0Close};
static const u8 *const sLobby1Far[] = {gText_Lobby1Far};
static const u8 *const sLobby1Close[] = {gText_Lobby1Close};

#include "data/region/emerald_chapters.h"
#include "data/region/emerald_subplots.h"

const struct SubplotHost *GetSubplotHosts(u32 *numSubplots)
{
    *numSubplots = ARRAY_COUNT(sEmeraldSubplotHosts);
    return sEmeraldSubplotHosts;
}

const struct ChapterHost *GetChapterHosts(u32 *numHosts)
{
    *numHosts = ARRAY_COUNT(sEmeraldChapterHosts);
    return sEmeraldChapterHosts;
}

const struct ChapterDesc *GetCountyChapter(u32 countyId, u32 slot)
{
    u32 i;

    assertf(slot < CHAPTERS_PER_COUNTY, "chapter slot %d >= %d; var 0 of a county slice is reserved", slot, CHAPTERS_PER_COUNTY)
    {
        return NULL;
    }
    for (i = 0; i < ARRAY_COUNT(sEmeraldChapterHosts); i++)
    {
        if (sEmeraldChapterHosts[i].county == countyId && sEmeraldChapterHosts[i].slot == slot)
            return sEmeraldChapterHosts[i].desc;
    }
    return NULL;
}

const struct ChapterDesc *GetCountyStatelessChapter(u32 countyId, u32 index)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sEmeraldChapterHosts); i++)
    {
        if (sEmeraldChapterHosts[i].county == countyId && sEmeraldChapterHosts[i].slot == CHAPTER_SLOT_STATELESS && index-- == 0)
            return sEmeraldChapterHosts[i].desc;
    }
    return NULL;
}

u16 GetCountySlotVar(u32 countyId, u32 slot)
{
    u32 slice = GetCountyVarSlice(countyId);

    assertf(slice < COUNTY_VAR_SLICES_FREE, "county %d uses var slice %d, which overlaps legacy vars (only slices 0-%d are free); retire legacy vars first", countyId, slice, COUNTY_VAR_SLICES_FREE - 1)
    {
        return 0;
    }
    assertf(slot < CHAPTERS_PER_COUNTY, "chapter slot %d >= %d", slot, CHAPTERS_PER_COUNTY)
    {
        return 0;
    }
    return COUNTY_VARS_START + slice * VARS_PER_COUNTY + 1 + slot;
}

u32 GetChapterStage(u32 countyId, u32 slot, const struct ChapterDesc *desc)
{
    u32 stage;

    assertf(desc != NULL, "county %d hosts no chapter in slot %d", countyId, slot)
    {
        return 0;
    }
    if (desc->backend == CHAPTER_PROXY)
    {
        u32 numHosts;
        const struct ChapterHost *hosts = GetChapterHosts(&numHosts);

        return IsMcGuffinProducedBy(hosts, numHosts, ResolveChapterKey(&desc->requiredKeys[0], countyId)) ? desc->numStages - 1 : 0;
    }
    if (desc->backend == CHAPTER_STORED)
    {
        stage = VarGet(GetCountySlotVar(countyId, slot));
        assertf(stage < desc->numStages, "county %d slot %d holds stage %d but the chapter has %d", countyId, slot, stage, desc->numStages)
        {
            return 0;
        }
        return stage;
    }
    for (stage = desc->numStages - 1; stage > 0; stage--)
    {
        if (IsStoryPredicateTrue(&desc->stages[stage].done))
            return stage;
    }
    return 0;
}

void SetChapterStage(u32 countyId, u32 slot, const struct ChapterDesc *desc, u32 stage)
{
    assertf(desc != NULL, "county %d hosts no chapter in slot %d", countyId, slot)
    {
        return;
    }
    assertf(desc->backend == CHAPTER_STORED, "county %d slot %d is an adapter or proxy chapter, which is read-only; did you mean to port it to the stored backend, or to set the subplot it stands for?", countyId, slot)
    {
        return;
    }
    assertf(stage < desc->numStages, "stage %d >= %d stages", stage, desc->numStages)
    {
        return;
    }
    VarSet(GetCountySlotVar(countyId, slot), stage);
}

u32 GetCountySlotStage(u32 countyId, u32 slot)
{
    return GetChapterStage(countyId, slot, GetCountyChapter(countyId, slot));
}

void SetCountySlotStage(u32 countyId, u32 slot, u32 stage)
{
    SetChapterStage(countyId, slot, GetCountyChapter(countyId, slot), stage);
}

// Position on the line BEFORE, 0, 1, ..., AFTER, so ranges can end at a pseudo-stage.
static u32 StageRank(u32 stage)
{
    if (stage == CHAPTER_STAGE_BEFORE)
        return 0;
    return stage == CHAPTER_STAGE_AFTER ? 0x200 : stage + 1;
}

bool32 DoesChapterHintCover(const struct ChapterHint *hint, u32 chapterStage)
{
    u32 rank = StageRank(chapterStage);

    return StageRank(hint->firstStage) <= rank && rank <= StageRank(hint->lastStage);
}

const struct ChapterHint *FindChapterHint(const struct ChapterDesc *desc, u32 chapterStage)
{
    u32 i;

    for (i = 0; i < desc->numHints; i++)
    {
        if (DoesChapterHintCover(&desc->hints[i], chapterStage))
            return &desc->hints[i];
    }
    return NULL;
}

const u8 *GetChapterHint(const struct ChapterDesc *desc, u32 chapterStage, u32 distance, u32 selector)
{
    const struct ChapterHint *hint;
    const struct ChapterTextList *list;

    assertf(desc != NULL, "no chapter to hint about")
    {
        return NULL;
    }
    hint = FindChapterHint(desc, chapterStage);
    if (hint == NULL)
        return NULL;
    list = distance == CHAPTER_FAR ? &hint->far : &hint->close;
    if (list->count == 0)
        return NULL;
    return list->texts[selector % list->count];
}

const u8 *GetCountySlotHint(u32 countyId, u32 slot, u32 distance, u32 selector)
{
    const struct ChapterDesc *desc = GetCountyChapter(countyId, slot);

    return GetChapterHint(desc, GetChapterStage(countyId, slot, desc), distance, selector);
}

const u8 *GetCountyStatelessHint(u32 countyId, u32 index, u32 distance, u32 selector)
{
    const struct ChapterDesc *desc = GetCountyStatelessChapter(countyId, index);

    assertf(desc != NULL, "county %d hosts no stateless chapter %d; did you mean a stored chapter's slot (GetCountySlotHint)?", countyId, index)
    {
        return NULL;
    }
    return GetChapterHint(desc, GetChapterStage(countyId, CHAPTER_SLOT_STATELESS, desc), distance, selector);
}

bool32 DoesChapterProduce(const struct ChapterDesc *desc, u32 kind, u32 ordinal)
{
    return desc->producesMainGet && desc->mainGet.kind == kind && desc->mainGet.ordinal == ordinal;
}

bool32 GetChapterSuppliedSlot(const struct ChapterDesc *desc, struct PlotSlot *out)
{
    // A proxy supplies the get of its subplot's chapter for planning only; the news still points at that chapter (DoesChapterProduce).
    if (desc->producesMainGet || desc->backend == CHAPTER_PROXY)
        *out = GetVanillaPlotSlot(desc->mainGet.kind, desc->mainGet.ordinal);
    else if (desc->mainHm != PLOT_HM_NONE)
        *out = GetVanillaHmSlot(desc->mainHm);
    else
        return FALSE;
    return TRUE;
}

u32 GetChapterActors(const struct ChapterDesc *desc, u32 stage, const struct ChapterActor **out, u32 maxOut)
{
    u32 i, total = 0;

    assertf(desc != NULL, "no chapter to list actors for")
    {
        return 0;
    }
    for (i = 0; i < desc->numActors; i++)
    {
        if (StageRank(stage) < StageRank(desc->actors[i].firstStage) || StageRank(stage) > StageRank(desc->actors[i].lastStage))
            continue;
        if (total < maxOut)
            out[total] = &desc->actors[i];
        total++;
    }
    return total;
}

u32 GetCountyFromCityMapSec(u32 mapSec)
{
    u32 county;

    for (county = 0; county < NUM_COUNTIES; county++)
    {
        if (DoesCountyHaveCity(county) && GetCountyCityMapSec(county) == mapSec)
            return county;
    }
    assertf(FALSE, "MAPSEC %d is no county's city; only a city can host chapters for now", mapSec)
    {
        return 0;
    }
    return 0;
}

u32 ResolveChapterKey(const struct ChapterKeyRef *ref, u32 hostCounty)
{
    switch (ref->type)
    {
    case CHAPTER_KEY_NONE:
        return MCGUFFIN_ID_NONE;
    case CHAPTER_KEY_HOST_POOL:
        return GetCountyMcGuffinId(hostCounty, ref->a);
    case CHAPTER_KEY_ITEM:
        return GetPlotItemMcGuffinId(ref->a, ref->b);
    case CHAPTER_KEY_SHOP_ITEM:
        return GetShopItemMcGuffinId(ref->a | ref->b << 8);
    }
    assertf(FALSE, "unknown ChapterKeyType %d; did you mean to add a case to ResolveChapterKey?", ref->type)
    {
        return MCGUFFIN_ID_NONE;
    }
    return MCGUFFIN_ID_NONE;
}

u32 ResolveHintStage(const struct ChapterDesc *desc, u32 stage)
{
    if (stage == CHAPTER_STAGE_BEFORE)
        return 0;
    if (stage == CHAPTER_STAGE_AFTER)
        return desc->numStages - 1;
    return stage;
}

u32 GetChapterHintCount(const struct ChapterDesc *desc, u32 beat, u32 badgesHeld)
{
    beat = ResolveHintStage(desc, beat);
    assertf(beat < desc->numStages, "no chapter stage %d", beat)
    {
        return 0;
    }
    switch (desc->stages[beat].hintCount)
    {
    case CHAPTER_COUNT_NONE:
        return 0;
    case CHAPTER_COUNT_BADGES_SHORT:
        return desc->minBadges > badgesHeld ? desc->minBadges - badgesHeld : 0;
    }
    assertf(FALSE, "unknown ChapterHintCount %d; did you mean to add a case to GetChapterHintCount?", desc->stages[beat].hintCount)
    {
        return 0;
    }
    return 0;
}

void BufferChapterHintCount(const struct ChapterDesc *desc, u32 beat)
{
    beat = ResolveHintStage(desc, beat);
    assertf(beat < desc->numStages, "no chapter stage %d", beat)
    {
        return;
    }
    if (desc->stages[beat].hintCount != CHAPTER_COUNT_NONE)
        ConvertIntToDecimalStringN(gStringVar1, GetChapterHintCount(desc, beat, GetBadgeCount()), STR_CONV_MODE_LEFT_ALIGN, 3);
}

u32 GetChapterNeededRoles(const struct ChapterDesc *desc)
{
    u32 stage, roles = desc->requiredRoles;

    for (stage = 0; stage < desc->numStages; stage++)
        roles |= desc->stages[stage].anchor.role | desc->stages[stage].extraRoles;
    return roles;
}

bool32 IsMcGuffinProducedBy(const struct ChapterHost *hosts, u32 numHosts, u32 mcguffinId)
{
    u32 i;

    for (i = 0; i < numHosts; i++)
    {
        const struct ChapterDesc *desc = hosts[i].desc;

        if (desc->producedKey.type == CHAPTER_KEY_NONE || ResolveChapterKey(&desc->producedKey, hosts[i].county) != mcguffinId)
            continue;
        if (GetChapterStage(hosts[i].county, hosts[i].slot, desc) >= desc->producedKeyStage)
            return TRUE;
    }
    return FALSE;
}

bool32 IsChapterAvailable(const struct ChapterDesc *desc, u32 hostCounty, const struct ChapterHost *hosts, u32 numHosts)
{
    u32 i;

    if (GetBadgeCount() < desc->minBadges)
        return FALSE;
    for (i = 0; i < CHAPTER_MAX_REQUIRED_KEYS; i++)
    {
        if (desc->requiredKeys[i].type != CHAPTER_KEY_NONE
         && !IsMcGuffinProducedBy(hosts, numHosts, ResolveChapterKey(&desc->requiredKeys[i], hostCounty)))
            return FALSE;
    }
    return TRUE;
}

static const struct ChapterHost *FindHostBySlot(const struct ChapterHost *hosts, u32 numHosts, u32 county, u32 slot)
{
    u32 i;

    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].county == county && hosts[i].slot == slot)
            return &hosts[i];
    }
    return NULL;
}

static const struct ChapterHost *FindHostByDesc(const struct ChapterHost *hosts, u32 numHosts, u32 county, const struct ChapterDesc *desc)
{
    u32 i;

    for (i = 0; i < numHosts; i++)
    {
        if (hosts[i].county == county && hosts[i].desc == desc)
            return &hosts[i];
    }
    return NULL;
}

bool32 IsChainPredecessorComplete(const struct ChapterHost *hosts, u32 numHosts, u32 county, u32 slot, u32 mainStoryIndex)
{
    u32 numSubplots;
    const struct SubplotHost *subplots = GetSubplotHosts(&numSubplots);
    const struct ChapterHost *self = FindHostBySlot(hosts, numHosts, county, slot);
    const struct ChapterHost *prev;
    const struct ChapterDesc *prevDesc;

    // Earlier counties are done once the main story has reached this county's slots (4x + s).
    if (mainStoryIndex / COUNTY_MAIN_SLOTS < GetCountyHeader(county)->storyIdx)
        return FALSE;
    if (self == NULL)
        return TRUE;
    prevDesc = GetSubplotPredecessor(subplots, numSubplots, county, self->desc);
    if (prevDesc == NULL)
        return TRUE;
    prev = FindHostByDesc(hosts, numHosts, county, prevDesc);
    return prev == NULL || GetChapterStage(prev->county, prev->slot, prev->desc) >= (u32)prev->desc->numStages - 1;
}

bool32 IsCountySlotAvailable(u32 countyId, u32 slot)
{
    u32 numHosts;
    const struct ChapterHost *hosts = GetChapterHosts(&numHosts);
    const struct ChapterDesc *desc = GetCountyChapter(countyId, slot);

    assertf(desc != NULL, "county %d hosts no chapter in slot %d", countyId, slot)
    {
        return FALSE;
    }
    return IsChainPredecessorComplete(hosts, numHosts, countyId, slot, GetMainStoryIndex()) && IsChapterAvailable(desc, countyId, hosts, numHosts);
}

// Story order (design_chapters.md, "Parallel subplots"): county story index, then main-story position. An off-chain
// chapter is open from its county's entry, so it sorts before every main-story slot of that county.
static u32 GetStoryPosition(u32 county, u32 mainSlot)
{
    if (mainSlot == CHAPTER_MAIN_SLOT_OFF_CHAIN)
        return GetCountyHeader(county)->storyIdx << 8;
    if (mainSlot >= CHAPTER_MAIN_SLOT_CALLBACK_BASE)
        return (mainSlot - CHAPTER_MAIN_SLOT_CALLBACK_BASE) << 8 | 2; // callbacks are step 1
    return GetCountyHeader(county)->storyIdx << 8 | (mainSlot + 1);
}

// Positions tie within one county and slot, and a tie is not "earlier": key order inside a slot would admit cycles.
static bool32 HasProducerBefore(u32 mcguffinId, u32 position, const struct ChapterHost *hosts, u32 numHosts)
{
    u32 i;

    for (i = 0; i < numHosts; i++)
    {
        const struct ChapterDesc *desc = hosts[i].desc;

        if (desc->producedKey.type == CHAPTER_KEY_NONE || ResolveChapterKey(&desc->producedKey, hosts[i].county) != mcguffinId)
            continue;
        if (GetStoryPosition(hosts[i].county, hosts[i].mainSlot) < position)
            return TRUE;
    }
    return FALSE;
}

// A chapter flagged waitsForLaterKey is not part of the main story's order, so it may wait at runtime for a producer that
// comes later (New Mauville waits for Norman's callback). Deadlock needs a producer that itself consumes, and a chapter is never both.
static bool32 HasProducerAtOtherPosition(u32 mcguffinId, u32 position, const struct ChapterHost *hosts, u32 numHosts)
{
    u32 i;

    for (i = 0; i < numHosts; i++)
    {
        const struct ChapterDesc *desc = hosts[i].desc;

        if (desc->producedKey.type == CHAPTER_KEY_NONE || ResolveChapterKey(&desc->producedKey, hosts[i].county) != mcguffinId)
            continue;
        if (GetStoryPosition(hosts[i].county, hosts[i].mainSlot) != position)
            return TRUE;
    }
    return FALSE;
}

u32 CheckChapterPlacement(const struct ChapterDesc *desc, u32 county, u32 mainSlot, const struct ChapterHost *hosts, u32 numHosts)
{
    u32 needed = GetChapterNeededRoles(desc), bit, i;

    // A county without a city places the buildings its chapters need on its routes, so it has no roles to miss.
    for (bit = 0; bit < NUM_BUILDING_ROLES && DoesCountyHaveCity(county); bit++)
    {
        if ((needed & (1u << bit)) && !DoesCountyCityHaveBuilding(county, 1u << bit))
            return CHAPTER_PLACEMENT_MISSING_ROLE;
    }
    if (desc->producesMainGet && desc->minBadges > 0)
    {
        struct PlotSlot get = GetVanillaPlotSlot(desc->mainGet.kind, desc->mainGet.ordinal);
        struct GymSlot gym = GetGymSlot(desc->minBadges);

        if (4 * get.block + get.step <= 4 * gym.block + gym.step)
            return CHAPTER_PLACEMENT_BEFORE_BADGES;
    }
    for (i = 0; i < CHAPTER_MAX_REQUIRED_KEYS; i++)
    {
        u32 key, position;

        if (desc->requiredKeys[i].type == CHAPTER_KEY_NONE)
            continue;
        key = ResolveChapterKey(&desc->requiredKeys[i], county);
        position = GetStoryPosition(county, mainSlot);
        if ((mainSlot == CHAPTER_MAIN_SLOT_OFF_CHAIN && desc->waitsForLaterKey)
            ? !HasProducerAtOtherPosition(key, position, hosts, numHosts)
            : !HasProducerBefore(key, position, hosts, numHosts))
            return CHAPTER_PLACEMENT_KEY_NOT_EARLIER;
    }
    return CHAPTER_PLACEMENT_OK;
}

// The one hosted chapter of the proxy's county that produces the proxy's key; NULL when none or several do.
static const struct ChapterHost *FindProxySubplotHost(const struct ChapterHost *proxy, const struct ChapterHost *hosts, u32 numHosts)
{
    const struct ChapterHost *found = NULL;
    u32 key = ResolveChapterKey(&proxy->desc->requiredKeys[0], proxy->county), i;

    for (i = 0; i < numHosts; i++)
    {
        const struct ChapterDesc *desc = hosts[i].desc;

        if (hosts[i].county != proxy->county || desc == proxy->desc || desc->producedKey.type == CHAPTER_KEY_NONE
         || ResolveChapterKey(&desc->producedKey, hosts[i].county) != key)
            continue;
        if (found != NULL)
            return NULL;
        found = &hosts[i];
    }
    return found;
}

static const struct SubplotDesc *FindSubplotOfChapter(const struct SubplotHost *subplots, u32 numSubplots, u32 county, const struct ChapterDesc *desc)
{
    u32 h, i;

    for (h = 0; h < numSubplots; h++)
    {
        for (i = 0; subplots[h].county == county && i < subplots[h].subplot->numEntries; i++)
        {
            if (subplots[h].subplot->entries[i].chapter == desc)
                return subplots[h].subplot;
        }
    }
    return NULL;
}

u32 CheckProxySoftLock(const struct ChapterHost *proxy, const struct ChapterHost *hosts, u32 numHosts, const struct SubplotHost *subplots, u32 numSubplots)
{
    const struct ChapterHost *sub = FindProxySubplotHost(proxy, hosts, numHosts);
    const struct SubplotDesc *subplot;
    u32 i, proxyPosition = GetStoryPosition(proxy->county, proxy->mainSlot);

    if (sub == NULL)
        return PROXY_NO_SUBPLOT;
    subplot = FindSubplotOfChapter(subplots, numSubplots, proxy->county, sub->desc);
    if (subplot == NULL || sub->desc->producedKeyStage != sub->desc->numStages - 1)
        return PROXY_NO_SUBPLOT;
    for (i = 0; i < subplot->numEntries; i++)
    {
        if (subplot->entries[i].chapter->minBadges > proxy->desc->minBadges)
            return PROXY_BADGES_BELOW_SUBPLOT;
    }
    for (i = 0; i < subplot->numEntries; i++)
    {
        if (!subplot->entries[i].optional)
            return PROXY_SUBPLOT_ON_CHAIN;
    }
    for (i = 0; i < subplot->numEntries; i++)
    {
        const struct ChapterDesc *desc = subplot->entries[i].chapter;
        u32 k;

        for (k = 0; k < CHAPTER_MAX_REQUIRED_KEYS; k++)
        {
            if (desc->requiredKeys[k].type != CHAPTER_KEY_NONE
             && !HasProducerBefore(ResolveChapterKey(&desc->requiredKeys[k], proxy->county), proxyPosition, hosts, numHosts))
                return PROXY_SUBPLOT_NEEDS_LATER_KEY;
        }
    }
    return CheckChapterPlacement(sub->desc, sub->county, sub->mainSlot, hosts, numHosts) == CHAPTER_PLACEMENT_OK ? PROXY_OK : PROXY_SUBPLOT_MISPLACED;
}

static const u8 *const sBuildingRoleNames[] =
{
    COMPOUND_STRING("POKéMON CENTER"), COMPOUND_STRING("MART"), COMPOUND_STRING("GYM"), COMPOUND_STRING("POKéMON LEAGUE"), COMPOUND_STRING("home"), COMPOUND_STRING("bedroom"), COMPOUND_STRING("MARKET"),
    COMPOUND_STRING("MOTEL"), COMPOUND_STRING("HERB SHOP"), COMPOUND_STRING("HOT SPRINGS"), COMPOUND_STRING("HALL"), COMPOUND_STRING("FAN CLUB"), COMPOUND_STRING("SCHOOL"), COMPOUND_STRING("SERVICE house"),
    COMPOUND_STRING("house"), COMPOUND_STRING("CONTEST HALL"),
};

static const u8 *const sLocationKindNames[] =
{
    [ROUTE_LOCATION_CAVE] = COMPOUND_STRING("cave"),
    [ROUTE_LOCATION_TUNNEL] = COMPOUND_STRING("tunnel"),
    [ROUTE_LOCATION_VOLCANO] = COMPOUND_STRING("volcano"),
    [ROUTE_LOCATION_MOUNTAIN] = COMPOUND_STRING("mountain"),
    [ROUTE_LOCATION_FOREST] = COMPOUND_STRING("forest"),
    [ROUTE_LOCATION_RUINS] = COMPOUND_STRING("ruins"),
    [ROUTE_LOCATION_TOWER] = COMPOUND_STRING("tower"),
    [ROUTE_LOCATION_HIDEOUT] = COMPOUND_STRING("hideout"),
    [ROUTE_LOCATION_ISLAND] = COMPOUND_STRING("island"),
    [ROUTE_LOCATION_UNDERWATER] = COMPOUND_STRING("undersea spot"),
    [ROUTE_LOCATION_WRECK] = COMPOUND_STRING("wreck"),
    [ROUTE_LOCATION_PARK] = COMPOUND_STRING("park"),
    [ROUTE_LOCATION_FACILITY] = COMPOUND_STRING("facility"),
    [ROUTE_LOCATION_ROADSIDE_BUILDING] = COMPOUND_STRING("roadside building"),
    [ROUTE_LOCATION_ROUTE_FEATURE] = COMPOUND_STRING("landmark"),
    [TOWN_LOCATION_MUSEUM] = COMPOUND_STRING("MUSEUM"),
    [TOWN_LOCATION_ENTERTAINMENT] = COMPOUND_STRING("entertainment hall"),
    [TOWN_LOCATION_STORE] = COMPOUND_STRING("DEPT. STORE"),
    [TOWN_LOCATION_HARBOR] = COMPOUND_STRING("harbor"),
    [TOWN_LOCATION_INSTITUTION] = COMPOUND_STRING("institute"),
    [TOWN_LOCATION_SACRED_CAVE] = COMPOUND_STRING("sacred cave"),
};

static const u8 sNearThe[] = _("near the ");
static const u8 sNearTheRoadTo[] = _("near the road to ");
static const u8 sNearTheRoad[] = _("near the road");
static const u8 sInCity[] = _("in ");
static const u8 sOutInTheArea[] = _("out in these parts");

u32 GetBeatAnchorCounty(u32 hostCounty, const struct BeatAnchor *anchor)
{
    return hostCounty;
}

u32 GetBeatDistance(u32 hostCounty, const struct BeatAnchor *anchor, u32 hinterCounty)
{
    return hinterCounty == GetBeatAnchorCounty(hostCounty, anchor) ? CHAPTER_CLOSE : CHAPTER_FAR;
}

static const u8 sFallbackPlace[] = _("spot");

// The anchor's building role name, else its location kind name, else NULL (a route or outdoors anchor).
static const u8 *GetAnchorPlaceName(const struct BeatAnchor *anchor)
{
    u32 bit;

    if (anchor->role != 0)
    {
        assertf((anchor->role & (anchor->role - 1)) == 0, "anchor role 0x%x has several bits; did you mean one BuildingRole?", anchor->role)
        {
            return NULL;
        }
        for (bit = 0; bit < ARRAY_COUNT(sBuildingRoleNames); bit++)
        {
            if (anchor->role == 1u << bit)
                return sBuildingRoleNames[bit];
        }
        assertf(FALSE, "anchor role 0x%x has no name; did you mean to add it to sBuildingRoleNames?", anchor->role)
        {
            return NULL;
        }
    }
    if (anchor->locationKind != BEAT_LOCATION_NONE)
    {
        assertf(anchor->locationKind < ARRAY_COUNT(sLocationKindNames), "anchor location kind %d >= %d", anchor->locationKind, ARRAY_COUNT(sLocationKindNames))
        {
            return NULL;
        }
        return sLocationKindNames[anchor->locationKind];
    }
    return NULL;
}

u8 *WriteBeatAnchorPlace(u8 *dst, const struct BeatAnchor *anchor)
{
    const u8 *name = GetAnchorPlaceName(anchor);

    return StringCopy(dst, name != NULL ? name : sFallbackPlace);
}

u8 *WriteBeatAnchorReference(u8 *dst, u32 hostCounty, const struct BeatAnchor *anchor)
{
    const u8 *name = GetAnchorPlaceName(anchor);

    if (name != NULL)
    {
        StringAppend(StringCopy(dst, sNearThe), name);
        return dst + StringLength(dst);
    }
    if (anchor->routeId < NUM_COUNTIES && DoesCountyHaveCity(anchor->routeId))
    {
        StringAppend(StringCopy(dst, sNearTheRoadTo), GetCountyCityName(anchor->routeId));
        return dst + StringLength(dst);
    }
    if (anchor->routeId != ROUTE_ID_NONE)
    {
        StringCopy(dst, sNearTheRoad);
        return dst + StringLength(dst);
    }
    if (!DoesCountyHaveCity(hostCounty))
    {
        StringCopy(dst, sOutInTheArea);
        return dst + StringLength(dst);
    }
    StringAppend(StringCopy(dst, sInCity), GetCountyCityName(hostCounty));
    return dst + StringLength(dst);
}

static const u8 sTheArea[] = _("the area ");
static const u8 sHere[] = _("these parts");
static const u8 sOfHere[] = _(" of here");
static const u8 sOf[] = _(" of ");
static const u8 sNear[] = _("near ");
static const u8 sFarTo[] = _("the far ");
static const u8 sCommaNear[] = _(", near ");

const u8 *GetCountyDirectionName(u32 a, u32 b)
{
    static const u8 *const sNames[3][3] =
    {
        {COMPOUND_STRING("northwest"), COMPOUND_STRING("north"), COMPOUND_STRING("northeast")},
        {COMPOUND_STRING("west"), COMPOUND_STRING("here"), COMPOUND_STRING("east")},
        {COMPOUND_STRING("southwest"), COMPOUND_STRING("south"), COMPOUND_STRING("southeast")},
    };
    s32 dx = (s32)COUNTY_X(b) - (s32)COUNTY_X(a);
    s32 dy = (s32)COUNTY_Y(b) - (s32)COUNTY_Y(a);
    s32 ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
    s32 major = ax > ay ? ax : ay, minor = ax > ay ? ay : ax;
    // Diagonal only when the minor axis is over tan(22.5 degrees) (about 5/12) of the major one; the boundary itself is straight.
    bool32 diagonal = minor * 12 > major * 5;
    s32 sx = dx == 0 ? 0 : (dx < 0 ? -1 : 1), sy = dy == 0 ? 0 : (dy < 0 ? -1 : 1);

    if (!diagonal)
    {
        if (ay >= ax)
            sx = 0;
        else
            sy = 0;
    }
    return sNames[sy + 1][sx + 1];
}

// The mapsec of the county's first landmark, else its first detour or pitstop; FALSE when it has none.
static bool32 FindCountyLandmarkMapSec(u32 county, u32 *mapSec)
{
    u32 count = GetCountySpecialLocationCount(county), i, fallback = count;

    for (i = 0; i < count; i++)
    {
        u32 class = GetCountySpecialLocationClass(county, i);

        if (class == LOCATION_CLASS_LANDMARK)
            break;
        if (fallback == count && (class == LOCATION_CLASS_DETOUR || class == LOCATION_CLASS_PITSTOP))
            fallback = i;
    }
    if (i == count)
        i = fallback;
    if (i == count)
        return FALSE;
    *mapSec = GetCountySpecialLocation(county, i).mapSec;
    return TRUE;
}

static u8 *WriteLandmarkName(u8 *dst, u32 county)
{
    u32 mapSec = 0;
    bool32 found = FindCountyLandmarkMapSec(county, &mapSec);

    assertf(found, "county %d has no landmark, detour or pitstop to refer to; did the generator give every county a place?", county)
    {
        return dst;
    }
    return StringCopy(dst, gRegionMapEntries[mapSec].name);
}

static u8 *WriteDirectionOf(u8 *dst, u32 from, u32 to)
{
    return StringCopy(StringCopy(dst, GetCountyDirectionName(from, to)), sOf);
}

// The adjacent county with a smaller story index than `target` and the highest such index that satisfies the
// predicate (a city when needCity, else a landmark); NUM_COUNTIES when none.
static u32 FindEarlierNeighbor(u32 target, bool32 needCity)
{
    u32 targetIdx = GetCountyHeader(target)->storyIdx, best = NUM_COUNTIES, bestIdx = 0, m, unused;

    for (m = 0; m < NUM_COUNTIES; m++)
    {
        u32 idx = GetCountyHeader(m)->storyIdx;

        if (!AreCountiesAdjacent(m, target) || idx >= targetIdx || (best != NUM_COUNTIES && idx <= bestIdx))
            continue;
        if (needCity ? DoesCountyHaveCity(m) : FindCountyLandmarkMapSec(m, &unused))
        {
            best = m;
            bestIdx = idx;
        }
    }
    return best;
}

u8 *WriteCountyReferenceAt(u8 *dst, u32 referrer, u32 target, bool32 callback, u32 mainBlock)
{
    u32 neighbor;

    if (DoesCountyHaveCity(target))
        return StringCopy(dst, GetCountyCityName(target));
    if (referrer == target)
        return StringCopy(dst, sHere);
    if (AreCountiesAdjacent(referrer, target))
        return StringCopy(StringCopy(StringCopy(dst, sTheArea), GetCountyDirectionName(referrer, target)), sOfHere);
    if (callback || mainBlock > GetCountyHeader(target)->storyIdx)
        return WriteLandmarkName(StringCopy(StringCopy(dst, sTheArea), sNear), target);
    neighbor = FindEarlierNeighbor(target, TRUE);
    if (neighbor != NUM_COUNTIES)
        return StringCopy(WriteDirectionOf(StringCopy(dst, sTheArea), neighbor, target), GetCountyCityName(neighbor));
    neighbor = FindEarlierNeighbor(target, FALSE);
    if (neighbor != NUM_COUNTIES)
        return WriteLandmarkName(WriteDirectionOf(StringCopy(dst, sTheArea), neighbor, target), neighbor);
    dst = StringCopy(StringCopy(dst, sFarTo), GetCountyDirectionName(referrer, target));
    return WriteLandmarkName(StringCopy(dst, sCommaNear), target);
}

u8 *WriteCountyReference(u8 *dst, u32 referrer, u32 target, bool32 callback)
{
    return WriteCountyReferenceAt(dst, referrer, target, callback, GetMainStoryIndex() >> 2);
}

u8 *ExpandHintTemplate(u8 *dst, const u8 *src, u32 hostCounty, const struct BeatAnchor *anchor)
{
    return ExpandHintTemplateFrom(dst, src, hostCounty, hostCounty, anchor, FALSE);
}

u8 *ExpandHintTemplateFrom(u8 *dst, const u8 *src, u32 referrer, u32 hostCounty, const struct BeatAnchor *anchor, bool32 callback)
{
    while (*src != EOS)
    {
        if (src[0] == PLACEHOLDER_BEGIN && src[1] == PLACEHOLDER_ID_HINT_ANCHOR)
        {
            dst = WriteBeatAnchorReference(dst, hostCounty, anchor);
            src += 2;
        }
        else if (src[0] == PLACEHOLDER_BEGIN && src[1] == PLACEHOLDER_ID_HINT_CITY)
        {
            dst = WriteCountyReference(dst, referrer, hostCounty, callback);
            src += 2;
        }
        else if (src[0] == PLACEHOLDER_BEGIN && src[1] == PLACEHOLDER_ID_HINT_PLACE)
        {
            dst = WriteBeatAnchorPlace(dst, anchor);
            src += 2;
        }
        else if (src[0] == PLACEHOLDER_BEGIN)
        {
            *dst++ = *src++;
            *dst++ = *src++;
        }
        else
        {
            *dst++ = *src++;
        }
    }
    *dst = EOS;
    return dst;
}

static u32 GetCurrentMapCounty(void)
{
    return GetCountyFromCityMapSec(gMapHeader.regionMapSectionId);
}

// Script special: VAR_0x8004 = slot; VAR_RESULT = stage of the current map's county chapter.
void GetCurrentChapterStage(void)
{
    gSpecialVar_Result = GetCountySlotStage(GetCurrentMapCounty(), gSpecialVar_0x8004);
}

// Script special: VAR_0x8004 = slot, VAR_0x8005 = stage.
void SetCurrentChapterStage(void)
{
    SetCountySlotStage(GetCurrentMapCounty(), gSpecialVar_0x8004, gSpecialVar_0x8005);
}

// Script special: VAR_0x8004 = slot, VAR_0x8005 = beat (the stage whose anchor and count the text names),
// VAR_0x8006 = the NPC's selector. Puts the hinter's text for the chapter's current stage in gStringVar4 and
// sets VAR_RESULT = TRUE; FALSE (buffer untouched) means "say generic text". The hinter stands in the host
// city, so it is always close.
void ShowCurrentChapterHint(void)
{
    u32 county = GetCurrentMapCounty();
    u32 slot = gSpecialVar_0x8004;
    u32 beat = gSpecialVar_0x8005;
    const struct ChapterDesc *desc = GetCountyChapter(county, slot);
    const u8 *text;

    gSpecialVar_Result = FALSE;
    assertf(desc != NULL && ResolveHintStage(desc, beat) < desc->numStages, "county %d slot %d has no stage %d to hint about; did the script set VAR_0x8004 (slot) and VAR_0x8005 (beat)?", county, slot, beat)
    {
        return;
    }
    text = GetCountySlotHint(county, slot, CHAPTER_CLOSE, gSpecialVar_0x8006);
    if (text == NULL)
        return;
    BufferChapterHintCount(desc, beat);
    ExpandHintTemplate(gStringVar4, text, county, &desc->stages[ResolveHintStage(desc, beat)].anchor);
    BreakStringForFieldMessage(gStringVar4);
    gSpecialVar_Result = TRUE;
}

// Script special: VAR_0x8004 = slot; VAR_RESULT = TRUE iff the current map's county chapter has its badges and keys.
void IsCurrentChapterAvailable(void)
{
    gSpecialVar_Result = IsCountySlotAvailable(GetCurrentMapCounty(), gSpecialVar_0x8004);
}

// Script special: VAR_0x8004 = main-story-index s; VAR_RESULT = TRUE once the index is past s.
void IsMainStoryIndexPast(void)
{
    gSpecialVar_Result = GetMainStoryIndex() > gSpecialVar_0x8004;
}
