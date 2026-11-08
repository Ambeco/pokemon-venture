@ Subplot hinter NPCs (design.md, "City planning", step 6). The NPC's own script does
@     subplot_hinter slot, beat, selector  (macro in asm/macros/event.inc)
@ using the values of its HinterPlacement record (city_hinters.h), then falls back to its generic text when
@ VAR_RESULT is FALSE (no text for the current state). On TRUE the hint has been shown and closed.

Common_EventScript_TrySubplotHint::
	special ShowCurrentSubplotHint
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_NopReturn
	lock
	faceplayer
	special ShowFieldMessageStringVar4
	waitmessage
	waitbuttonpress
	closemessage
	release
	return

@ Pokémon Center news NPC (design.md, "City planning"): the NPC's own script is
@     goto Common_EventScript_PokemonCenterNews
@ ShowPokemonCenterNews always has a text (generic lines are its fallback), so the NPC needs no text of its own.
Common_EventScript_PokemonCenterNews::
	lock
	faceplayer
	special ShowPokemonCenterNews
	special ShowFieldMessageStringVar4
	waitmessage
	waitbuttonpress
	closemessage
	release
	end
