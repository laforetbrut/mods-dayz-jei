# JEI - Just Enough Items (DayZ)

Author: vyrriox

## English

A recipe browser for DayZ inspired by the Minecraft mod JEI. Press a key, search any item,
see how it is crafted and what it is used in, then craft it straight from your inventory
and the storage around you.

### Features

- **Configurable key** (default `J`), listed under *JEI* in the game's controls menu.
- **Full item index** built from `CfgVehicles`, `CfgWeapons` and `CfgMagazines` (scope 2):
  every item of every loaded mod shows up, with a 3D preview.
- **Automatic recipe detection**: JEI hooks `PluginRecipesManager.RegisterRecipe`, so vanilla
  recipes and recipes registered by any other mod are found without a compatibility patch.
- **"How to craft" and "Used in"** views, base class ingredients included (a recipe asking for
  `Knife` lists every knife).
- **Item grid with 3D icons**: left click shows how to craft an item, right click what it is used
  in, the mouse wheel turns pages, hovering shows a tooltip.
- **Sort and filter**: sort by name, mod or class, show one mod at a time, or press **Craftable**
  to list only what can be crafted right now from the inventory and nearby storage.
- **Search syntax**: plain text (name or class), `@mod` (items of one mod, e.g. `@dayz`),
  `=Class` (every item inheriting from a class).
- **Quick craft from nearby storage**: ingredients are taken from the player inventory, loose
  items, open containers, tents, barrels and vehicle cargo in a configurable radius. Green means
  available, red means missing.
- **Server authoritative**: the server rescans the inventory itself, runs the recipe's own checks
  (`CheckRecipe`), enforces a cooldown and a batch limit. A client cannot craft from storage it
  is not standing next to.
- **Admin tools** (players listed in `Admins`):
  - spawn the selected item (x1 / x10) in the inventory, or on the ground if full;
  - create custom recipes (one or two ingredients, consume or keep each ingredient, several results);
  - edit a game recipe: the edit is saved as a custom replacement and the original is disabled;
  - disable / re-enable any game recipe; a disabled recipe is also blocked for normal crafting;
  - delete a custom recipe (a deleted replacement re-enables the original).
  Every admin action is validated on the server and written to the server log.

New item *classes* cannot be created at runtime in DayZ (they need a config inside a PBO);
the admin tools create item *instances* and recipes.

### Installation

1. Pack the `JEI` folder into `JEI.pbo` (Addon Builder, prefix `JEI`), put it in `@JEI/addons`
   with `mod.cpp` at the root of `@JEI`, and sign it with your server key.
2. Load `@JEI` on both the server and the client (`-mod=@JEI`).
3. Start the server once: `$profile:JEI/settings.json` and `custom_recipes.json` are created.

### Server settings (`$profile:JEI/settings.json`)

| Key | Default | Meaning |
| --- | --- | --- |
| `AllowCrafting` | `true` | Allow crafting from the JEI window |
| `AllowNearbyStorage` | `true` | Use items around the player, not only the inventory |
| `AllowVehicleCargo` | `true` | Include vehicle cargo |
| `NearbyRadius` | `3.0` | Search radius in meters (0.5 to 15) |
| `MaxCraftBatch` | `10` | Crafts done by the "Craft xN" button (1 to 50) |
| `CraftCooldown` | `0.5` | Seconds between two craft requests of one player |
| `BlacklistedRecipes` | `[]` | Disabled game recipe class names |
| `HiddenItems` | `[]` | Item classes hidden from the list |
| `Admins` | `[]` | Steam64 IDs or BattlEye GUIDs of JEI admins (never sent to clients) |

Custom recipes live in `custom_recipes.json` and can be edited by hand while the server is stopped.
In offline mode the local player is admin.

### Notes

- Crafting from JEI is instant (no animation).
- Developed without a test server; please report issues on the repository.

## Français

Un navigateur de recettes pour DayZ inspiré du mod Minecraft JEI. Une touche, une recherche,
et on voit comment fabriquer un objet, à quoi il sert, puis on le fabrique directement depuis
son inventaire et les stockages autour.

### Fonctionnalités

- **Touche configurable** (par défaut `J`), dans la catégorie *JEI* du menu des contrôles.
- **Index complet des objets** construit depuis `CfgVehicles`, `CfgWeapons` et `CfgMagazines`
  (scope 2) : tous les objets de tous les mods chargés, avec aperçu 3D.
- **Détection automatique des recettes** : JEI intercepte `PluginRecipesManager.RegisterRecipe`,
  donc les recettes vanilla et celles des autres mods sont trouvées sans patch de compatibilité.
- **Vues "Comment fabriquer" et "Utilisé dans"**, y compris les ingrédients par classe parente.
- **Grille d'icônes 3D** : clic gauche pour la fabrication, clic droit pour les utilisations,
  molette pour les pages, info-bulle au survol.
- **Tri et filtres** : tri par nom, mod ou classe, un mod à la fois, ou bouton **Craftable** pour
  n'afficher que ce qui est fabricable tout de suite avec l'inventaire et les stockages proches.
- **Recherche** : texte (nom ou classe), `@mod` (objets d'un mod, ex. `@dayz`), `=Classe`
  (tous les objets qui héritent d'une classe).
- **Craft rapide depuis les stockages proches** : inventaire du joueur, objets au sol, conteneurs
  ouverts, tentes, barils et coffres de véhicules dans un rayon réglable. Vert = disponible,
  rouge = manquant.
- **Autorité serveur** : le serveur rescanne lui-même, applique les vérifications de la recette,
  un délai entre deux crafts et une limite par lot.
- **Outils admin** (joueurs listés dans `Admins`) :
  - faire apparaître l'objet sélectionné (x1 / x10) ;
  - créer des recettes personnalisées (un ou deux ingrédients, consommés ou non, plusieurs résultats) ;
  - modifier une recette du jeu : la modification est enregistrée comme remplacement et l'originale
    est désactivée ;
  - désactiver / réactiver une recette du jeu, aussi bloquée pour le craft normal ;
  - supprimer une recette personnalisée (supprimer un remplacement réactive l'originale).
  Chaque action admin est vérifiée côté serveur et inscrite dans le log serveur.

DayZ ne permet pas de créer de nouvelles *classes* d'objets en jeu (il faut un config dans un PBO) :
les outils admin créent des *instances* d'objets et des recettes.

### Installation

1. Packer le dossier `JEI` en `JEI.pbo` (Addon Builder, préfixe `JEI`), le placer dans
   `@JEI/addons` avec `mod.cpp` à la racine de `@JEI`, et le signer avec la clé du serveur.
2. Charger `@JEI` sur le serveur et le client (`-mod=@JEI`).
3. Lancer le serveur une fois : `$profile:JEI/settings.json` et `custom_recipes.json` sont créés.

### Réglages serveur

Voir le tableau de la section anglaise. Les recettes personnalisées sont dans
`custom_recipes.json`. Hors ligne, le joueur local est admin.

### Notes

- Le craft depuis JEI est instantané (sans animation).
- Développé sans serveur de test ; merci de signaler les problèmes sur le dépôt.
