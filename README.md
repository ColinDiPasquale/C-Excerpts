This repo is dedicated to highlighting different code I've written in C. 
Most of the C code I write is used for Minecraft seedfinding, to reverse engineer world generation and PRNG behavior. I've contributed to both generation emulation and have also written custom filters.
Using both distributed computing and compute clusters, my code has filtered the entire seedspace of 2^64 seeds to find seeds that meet specific requirements.

Some of the files are filters for finding actual seeds, while others are contributions I've made to community libraries to help other seedfinders in their efforts.
The most popular library in C is Cubiomes, as well as a fork of Cubiomes maintained by Xpple. I've contributed to both. 
Because Minecraft is written in Java, there are still ongoing efforts (to which I have and am currently contributing) to emulate the world generation in C to allow for faster and more flexible filtering.
So far, I've helped with end island, gateway, city, and city loot generation emulation. I've also used BOINC, a distributed computing platform, to speed up filtering via the community.
This has allowed me to run filters that I've written myself across the entire seedspace of 2^64 seeds.
I've also written filters for desert temple and ruined portal loot, as well as structures, biomes, features, and decorators.
My current project is recreating mineshaft and stronghold generation and loot generation, which has proven to be more difficult than expected, as it's also required me to implement carver generation (things like caves and ravines), and their waterlogged counterparts.
Additionally, loot generation depends on multiple other generation steps, which are still in development between myself, Xpple, and other seedfinders.
Most of the resources, filters, libraries, and seedfinders (including myself) are part of the MinecraftAtHome community.
