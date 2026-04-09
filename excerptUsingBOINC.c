#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "cubiomes.h"
#define LOOT_LIBRARY
#include "src/loot_library.h"

#include "boinc/boinc_api.h"
#include "boinc/filesys.h"

struct checkpoint_vars {
    unsigned long long seedCount;
    unsigned long long outCount;
    double timeElapsed;
};
checkpoint_vars curr_checkpoint;

void loadCheckpoint() {
    FILE *checkpoint_data = boinc_fopen("rploot-checkpoint", "rb");
    if(!checkpoint_data){
        fprintf(stderr, "No checkpoint to load\n");
        curr_checkpoint.seedCount = 0;
        curr_checkpoint.outCount = 0;
        curr_checkpoint.timeElapsed = 0.0;
    }
    else {
        boinc_begin_critical_section();

        fread(&curr_checkpoint, sizeof(curr_checkpoint), 1, checkpoint_data);
        fprintf(stderr, "Checkpoint loaded, task time %.2f s, seed pos: %d\n", curr_checkpoint.timeElapsed, curr_checkpoint.seedCount);
        fclose(checkpoint_data);

        boinc_end_critical_section();
    }
}

void saveCheckpoint() {
    boinc_begin_critical_section(); // Boinc should not interrupt this

    FILE *checkpoint_data = boinc_fopen("rploot-checkpoint", "wb");
    fwrite(&curr_checkpoint, sizeof(curr_checkpoint), 1, checkpoint_data);
    fflush(checkpoint_data);
    fclose(checkpoint_data);
    
    boinc_end_critical_section();
    boinc_checkpoint_completed(); // Checkpointing completed
}

int main()
{
    BOINC_OPTIONS options;
    boinc_options_defaults(options);
    options.normal_thread_priority = true;
    boinc_init_options(&options);
    loadCheckpoint();

    FILE *inputFile = fopen("inputSeeds.txt", "r");
    if (!inputFile) {
        fprintf(stderr, "Error opening inputSeeds.txt");
        boinc_finish(1);
        return 1;
    }

    FILE *outputFile = fopen("outputSeeds.txt", "w");
    if (!outputFile) {
        fprintf(stderr, "Error opening outputSeeds.txt");
        boinc_finish(1);
        return 1;
    }

    Generator generator;
    setupGenerator(&generator, MC_1_16_1, 0);

    uint64_t structureSeed;
    int seedIndex = 0;

    while (fscanf(inputFile, "%" SCNu64, &structureSeed) == 1) {

        if (seedIndex < curr_checkpoint.seedCount) {
            seedIndex++;
            continue;
        }

        for (int j = 0; j < 65536; j++) {
            uint64_t currentSeed = ((uint64_t)j << 48) | structureSeed;
            applySeed(&generator, DIM_OVERWORLD, currentSeed);
            Pos spawn = getSpawn(&generator);
            int spawnDistance = (abs(spawn.x) > abs(spawn.z)) ? abs(spawn.x) : abs(spawn.z);
            Pos closestTemple = {0, 0};
            Pos templePos = {0, 0};
            Pos closestRP = {-1, -1};
            Pos portalPos = {0, 0};

            if (spawnDistance <= 272) {
                for (int regionX = -1; regionX <= 0; regionX++) {
                    for (int regionZ = -1; regionZ <= 0; regionZ++) {
                        getStructurePos(Desert_Pyramid, MC_1_16_1, currentSeed, regionX, regionZ, &templePos);
                        if (isViableStructurePos(Desert_Pyramid, &generator, templePos.x, templePos.z, 0)) {
                            int dx = templePos.x - spawn.x;
                            int dz = templePos.z - spawn.z;
                            int templeDistance = (abs(dx) > abs(dz)) ? abs(dx) : abs(dz);
                            if (templeDistance <= 96) {
                                closestTemple = templePos;
                                for (int rx = -2; rx <= 1; rx++) {
                                    for (int rz = -2; rz <= 1; rz++) {
                                        getStructurePos(Ruined_Portal, MC_1_16_1, currentSeed, rx, rz, &portalPos);
                                        if (isViableStructurePos(Ruined_Portal, &generator, portalPos.x, portalPos.z, 0)) {
                                            int pdx = portalPos.x - closestTemple.x;
                                            int pdz = portalPos.z - closestTemple.z;
                                            int portalDistance = (abs(pdx) > abs(pdz)) ? abs(pdx) : abs(pdz);
                                            if (portalDistance <= 96) {
                                                closestRP = portalPos;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                for (int regionX = -2; regionX <= 1; regionX++) {
                    for (int regionZ = -2; regionZ <= 1; regionZ++) {
                        getStructurePos(Desert_Pyramid, MC_1_16_1, currentSeed, regionX, regionZ, &templePos);
                        if (isViableStructurePos(Desert_Pyramid, &generator, templePos.x, templePos.z, 0)) {
                            int dx = templePos.x - spawn.x;
                            int dz = templePos.z - spawn.z;
                            int templeDistance = (abs(dx) > abs(dz)) ? abs(dx) : abs(dz);
                            if (templeDistance <= 96) {
                                closestTemple = templePos;
                                for (int rx = -2; rx <= 1; rx++) {
                                    for (int rz = -2; rz <= 1; rz++) {
                                        getStructurePos(Ruined_Portal, MC_1_16_1, currentSeed, rx, rz, &portalPos);
                                        if (isViableStructurePos(Ruined_Portal, &generator, portalPos.x, portalPos.z, 0)) {
                                            int pdx = portalPos.x - closestTemple.x;
                                            int pdz = portalPos.z - closestTemple.z;
                                            int portalDistance = (abs(pdx) > abs(pdz)) ? abs(pdx) : abs(pdz);
                                            if (portalDistance <= 96) {
                                                closestRP = portalPos;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (closestRP.x == -1 && closestRP.z == -1) continue;

            fprintf(outputFile, "A %" PRIu64 " %d %d\n", currentSeed, closestRP.x, closestRP.z); // Portal coords

            int biome = getBiomeAt(&generator, 4, closestRP.x >> 2, 64, closestRP.z >> 2);
            StructureVariant variant;
            getVariant(&variant, Ruined_Portal, MC_1_16_1, currentSeed, closestRP.x, closestRP.z, biome);

            LootTable table = init_ruined_portal_loot_table();
            uint64_t populationSeed = getPopulationSeed(MC_1_16_1, currentSeed, closestRP.x, closestRP.z);
            uint64_t tempSeed;
            setSeed(&tempSeed, populationSeed + 5 + 10000 * 4);
            uint64_t lootSeed = nextLong(&tempSeed);

            LootItem items[64] = {0};
            size_t num_items;
            ruined_portal_loot(&table, lootSeed, items, &num_items);

            bool completable = false;
            bool looting = false;
            bool garrots = false;

            for (size_t i = 0; i < num_items; i++) {
                if ((variant.start == 6 || variant.start == 7) && (items[i].item == OBSIDIAN && items[i].quantity >= 1)) completable = true;
                if ((variant.start == 1 || variant.start == 9) && (items[i].item == OBSIDIAN && items[i].quantity >= 2)) completable = true;
                if ((variant.start == 4 || variant.start == 8) && (items[i].item == OBSIDIAN && items[i].quantity >= 3)) completable = true;
                if ((variant.start == 2 || variant.start == 3) && (items[i].item == OBSIDIAN && items[i].quantity >= 4)) completable = true;
                if ((variant.giant || variant.start == 5) && (items[i].item == OBSIDIAN && items[i].quantity >= 5)) completable = true;
                if ((variant.start == 10) && (items[i].item == OBSIDIAN && items[i].quantity >= 7)) completable = true;

                if (items[i].item == GOLDEN_SWORD && items[i].enchant == LOOTING && (items[i].enchant_level == 2 || items[i].enchant_level == 3)) {
                    fprintf(outputFile, "B %" PRIu64 "\n", currentSeed); // Looting 2+
                    looting = true;
                }
                if (items[i].item == GOLDEN_CARROT && items[i].quantity >= 1) {
                    garrots = true;
                }
            }

            if (completable) fprintf(outputFile, "C %" PRIu64 "\n", currentSeed); // Completable
            if (completable && looting) {
                fprintf(outputFile, "D %" PRIu64 "\n", currentSeed); // Completable & looting 2+
                if (garrots) {
                    fprintf(outputFile, "E %" PRIu64 "\n", currentSeed); // Super seeds
                }
            }
        }

        curr_checkpoint.outCount++;
        seedIndex++;
        curr_checkpoint.seedCount++;

        boinc_fraction_done((double)curr_checkpoint.seedCount / totalLines);
        saveCheckpoint();
    }

    #ifdef BOINC
        boinc_begin_critical_section();
    #endif

    fclose(inputFile);
    fclose(rpCoords);
    fclose(loot2PlusSeeds);
    fclose(completableSeeds);
    fclose(completableLoot2PlusSeeds);
    fclose(superSeeds);
    
    fprintf(stderr, "Done.\n");
    fflush(stderr);

    boinc_delete_file("checkpoint");

    boinc_end_critical_section();
    boinc_finish(0);
    return 0;
}