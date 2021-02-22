#include "driver.h"


/* "Pacman hardware" games */
extern struct GameDriver pacman_driver;
extern struct GameDriver pacmod_driver;
extern struct GameDriver namcopac_driver;
extern struct GameDriver pacmanjp_driver;
extern struct GameDriver hangly_driver;
extern struct GameDriver puckman_driver;
extern struct GameDriver piranha_driver;
extern struct GameDriver pacplus_driver;
extern struct GameDriver mspacman_driver;
extern struct GameDriver mspacatk_driver;
extern struct GameDriver crush_driver;
extern struct GameDriver eyes_driver;
extern struct GameDriver mrtnt_driver;
extern struct GameDriver ponpoko_driver;
extern struct GameDriver lizwiz_driver;
extern struct GameDriver theglob_driver;
extern struct GameDriver beastf_driver;
extern struct GameDriver jumpshot_driver;
extern struct GameDriver maketrax_driver;
extern struct GameDriver pengo_driver;
extern struct GameDriver pengo2_driver;
extern struct GameDriver pengo2u_driver;
extern struct GameDriver penta_driver;
extern struct GameDriver jrpacman_driver;

/* "Galaxian hardware" games */
extern struct GameDriver galaxian_driver;
extern struct GameDriver galmidw_driver;
extern struct GameDriver galnamco_driver;
extern struct GameDriver superg_driver;
extern struct GameDriver galapx_driver;
extern struct GameDriver galap1_driver;
extern struct GameDriver galap4_driver;
extern struct GameDriver galturbo_driver;
extern struct GameDriver pisces_driver;
extern struct GameDriver japirem_driver;
extern struct GameDriver uniwars_driver;
extern struct GameDriver warofbug_driver;
extern struct GameDriver redufo_driver;
extern struct GameDriver pacmanbl_driver;
extern struct GameDriver zigzag_driver;
extern struct GameDriver zigzag2_driver;
extern struct GameDriver mooncrgx_driver;
extern struct GameDriver mooncrst_driver;
extern struct GameDriver mooncrsg_driver;
extern struct GameDriver mooncrsb_driver;
extern struct GameDriver fantazia_driver;
extern struct GameDriver eagle_driver;
extern struct GameDriver moonqsr_driver;
extern struct GameDriver checkman_driver;
extern struct GameDriver moonal2_driver;
extern struct GameDriver moonal2b_driver;
extern struct GameDriver kingball_driver;

/* "Scramble hardware" (and variations) games */
extern struct GameDriver scramble_driver;
extern struct GameDriver atlantis_driver;
extern struct GameDriver theend_driver;
extern struct GameDriver ckongs_driver;
extern struct GameDriver froggers_driver;
extern struct GameDriver amidars_driver;
extern struct GameDriver triplep_driver;
extern struct GameDriver scobra_driver;
extern struct GameDriver scobrak_driver;
extern struct GameDriver scobrab_driver;
extern struct GameDriver stratgyx_driver;
extern struct GameDriver stratgyb_driver;
extern struct GameDriver armorcar_driver;
extern struct GameDriver armorca2_driver;
extern struct GameDriver moonwar2_driver;
extern struct GameDriver monwar2a_driver;
extern struct GameDriver darkplnt_driver;
extern struct GameDriver tazmania_driver;
extern struct GameDriver calipso_driver;
extern struct GameDriver anteater_driver;
extern struct GameDriver rescue_driver;
extern struct GameDriver minefld_driver;
extern struct GameDriver losttomb_driver;
extern struct GameDriver losttmbh_driver;
extern struct GameDriver superbon_driver;
extern struct GameDriver hustler_driver;
extern struct GameDriver pool_driver;
extern struct GameDriver billiard_driver;
extern struct GameDriver frogger_driver;
extern struct GameDriver frogsega_driver;
extern struct GameDriver frogger2_driver;
extern struct GameDriver amidar_driver;
extern struct GameDriver amidarjp_driver;
extern struct GameDriver amigo_driver;
extern struct GameDriver turtles_driver;
extern struct GameDriver turpin_driver;
extern struct GameDriver k600_driver;
extern struct GameDriver jumpbug_driver;
extern struct GameDriver jbugsega_driver;
extern struct GameDriver flyboy_driver;
extern struct GameDriver fastfred_driver;
extern struct GameDriver jumpcoas_driver;

/* "Crazy Climber hardware" games */
extern struct GameDriver cclimber_driver;
extern struct GameDriver ccjap_driver;
extern struct GameDriver ccboot_driver;
extern struct GameDriver ckong_driver;
extern struct GameDriver ckonga_driver;
extern struct GameDriver ckongjeu_driver;
extern struct GameDriver ckongalc_driver;
extern struct GameDriver monkeyd_driver;
extern struct GameDriver rpatrolb_driver;
extern struct GameDriver silvland_driver;
extern struct GameDriver swimmer_driver;
extern struct GameDriver swimmera_driver;
extern struct GameDriver guzzler_driver;
extern struct GameDriver friskyt_driver;
extern struct GameDriver seicross_driver;

/* "Phoenix hardware" (and variations) games */
extern struct GameDriver phoenix_driver;
extern struct GameDriver phoenixt_driver;
extern struct GameDriver phoenix3_driver;
extern struct GameDriver phoenixc_driver;
extern struct GameDriver pleiads_driver;
extern struct GameDriver pleiadce_driver;
extern struct GameDriver naughtyb_driver;
extern struct GameDriver popflame_driver;

/* Namco games */
extern struct GameDriver warpwarp_driver;
extern struct GameDriver warpwar2_driver;
extern struct GameDriver rallyx_driver;
extern struct GameDriver nrallyx_driver;
extern struct GameDriver jungler_driver;
extern struct GameDriver junglers_driver;
extern struct GameDriver locomotn_driver;
extern struct GameDriver commsega_driver;
extern struct GameDriver bosco_driver;
extern struct GameDriver bosconm_driver;
extern struct GameDriver galaga_driver;
extern struct GameDriver galagamw_driver;
extern struct GameDriver galagads_driver;
extern struct GameDriver gallag_driver;
extern struct GameDriver galagab2_driver;
extern struct GameDriver digdug_driver;
extern struct GameDriver digdugnm_driver;
extern struct GameDriver xevious_driver;
extern struct GameDriver xeviousa_driver;
extern struct GameDriver xevios_driver;
extern struct GameDriver sxevious_driver;
extern struct GameDriver superpac_driver;
extern struct GameDriver superpcn_driver;
extern struct GameDriver pacnpal_driver;
extern struct GameDriver mappy_driver;
extern struct GameDriver mappyjp_driver;
extern struct GameDriver digdug2_driver;
extern struct GameDriver todruaga_driver;
extern struct GameDriver motos_driver;
extern struct GameDriver pacland_driver;
extern struct GameDriver pacland2_driver;
extern struct GameDriver pacland3_driver;
extern struct GameDriver paclandm_driver;

/* Universal games */
extern struct GameDriver cosmica_driver;
extern struct GameDriver cheekyms_driver;
extern struct GameDriver panic_driver;
extern struct GameDriver panica_driver;
extern struct GameDriver panicger_driver;
extern struct GameDriver ladybug_driver;
extern struct GameDriver ladybugb_driver;
extern struct GameDriver snapjack_driver;
extern struct GameDriver cavenger_driver;
extern struct GameDriver mrdo_driver;
extern struct GameDriver mrdot_driver;
extern struct GameDriver mrdofix_driver;
extern struct GameDriver mrlo_driver;
extern struct GameDriver mrdu_driver;
extern struct GameDriver mrdoy_driver;
extern struct GameDriver yankeedo_driver;
extern struct GameDriver docastle_driver;
extern struct GameDriver docastl2_driver;
extern struct GameDriver dounicorn_driver;
extern struct GameDriver dowild_driver;
extern struct GameDriver jjack_driver;
extern struct GameDriver dorunrun_driver;
extern struct GameDriver spiero_driver;
extern struct GameDriver kickridr_driver;

/* Nintendo games */
extern struct GameDriver radarscp_driver;
extern struct GameDriver dkong_driver;
extern struct GameDriver dkongjp_driver;
extern struct GameDriver dkongjr_driver;
extern struct GameDriver dkngjrjp_driver;
extern struct GameDriver dkjrjp_driver;
extern struct GameDriver dkjrbl_driver;
extern struct GameDriver dkong3_driver;
extern struct GameDriver mario_driver;
extern struct GameDriver masao_driver;
extern struct GameDriver hunchy_driver;
extern struct GameDriver herocast_driver;
extern struct GameDriver popeye_driver;
extern struct GameDriver popeyebl_driver;
extern struct GameDriver punchout_driver;
extern struct GameDriver spnchout_driver;
extern struct GameDriver armwrest_driver;

/* Midway 8080 b/w games */
extern struct GameDriver seawolf_driver;
extern struct GameDriver gunfight_driver;
extern struct GameDriver tornbase_driver;
extern struct GameDriver zzzap_driver;
extern struct GameDriver maze_driver;
extern struct GameDriver boothill_driver;
extern struct GameDriver checkmat_driver;
extern struct GameDriver gmissile_driver;
extern struct GameDriver clowns_driver;
extern struct GameDriver spcenctr_driver;
extern struct GameDriver invaders_driver;
extern struct GameDriver invdelux_driver;
extern struct GameDriver invadpt2_driver;
extern struct GameDriver earthinv_driver;
extern struct GameDriver spaceatt_driver;
extern struct GameDriver sinvemag_driver;
extern struct GameDriver invrvnge_driver;
extern struct GameDriver invrvnga_driver;
extern struct GameDriver galxwars_driver;
extern struct GameDriver lrescue_driver;
extern struct GameDriver grescue_driver;
extern struct GameDriver desterth_driver;
extern struct GameDriver cosmicmo_driver;
extern struct GameDriver spaceph_driver;
extern struct GameDriver rollingc_driver;
extern struct GameDriver bandido_driver;
extern struct GameDriver ozmawars_driver;
extern struct GameDriver schaser_driver;
extern struct GameDriver lupin3_driver;
extern struct GameDriver helifire_driver;
extern struct GameDriver helifira_driver;
extern struct GameDriver spacefev_driver;
extern struct GameDriver sfeverbw_driver;
extern struct GameDriver astlaser_driver;
extern struct GameDriver intruder_driver;
extern struct GameDriver polaris_driver;
extern struct GameDriver polarisa_driver;
extern struct GameDriver lagunar_driver;
extern struct GameDriver m4_driver;
extern struct GameDriver phantom2_driver;
extern struct GameDriver dogpatch_driver;
extern struct GameDriver midwbowl_driver;
extern struct GameDriver blueshrk_driver;
extern struct GameDriver einnings_driver;
extern struct GameDriver dplay_driver;
extern struct GameDriver m79amb_driver;

/* "Midway" Z80 b/w games */
extern struct GameDriver astinvad_driver;
extern struct GameDriver kamikaze_driver;

/* Meadows S2650 games */
extern struct GameDriver lazercmd_driver;
extern struct GameDriver deadeye_driver;
extern struct GameDriver gypsyjug_driver;
extern struct GameDriver medlanes_driver;

/* Midway "Astrocade" games */
extern struct GameDriver wow_driver;
extern struct GameDriver robby_driver;
extern struct GameDriver gorf_driver;
extern struct GameDriver gorfpgm1_driver;
extern struct GameDriver seawolf2_driver;
extern struct GameDriver spacezap_driver;
extern struct GameDriver ebases_driver;

/* Bally Midway "MCR" games */
extern struct GameDriver solarfox_driver;
extern struct GameDriver kick_driver;
extern struct GameDriver kicka_driver;
extern struct GameDriver shollow_driver;
extern struct GameDriver tron_driver;
extern struct GameDriver tron2_driver;
extern struct GameDriver kroozr_driver;
extern struct GameDriver domino_driver;
extern struct GameDriver wacko_driver;
extern struct GameDriver twotiger_driver;
extern struct GameDriver journey_driver;
extern struct GameDriver tapper_driver;
extern struct GameDriver sutapper_driver;
extern struct GameDriver rbtapper_driver;
extern struct GameDriver dotron_driver;
extern struct GameDriver dotrone_driver;
extern struct GameDriver destderb_driver;
extern struct GameDriver timber_driver;
extern struct GameDriver spyhunt_driver;
extern struct GameDriver crater_driver;
extern struct GameDriver sarge_driver;
extern struct GameDriver rampage_driver;
extern struct GameDriver maxrpm_driver;
extern struct GameDriver xenophob_driver;

/* Irem games */
extern struct GameDriver mpatrol_driver;
extern struct GameDriver mpatrolw_driver;
extern struct GameDriver mranger_driver;
extern struct GameDriver yard_driver;
extern struct GameDriver vsyard_driver;
extern struct GameDriver kungfum_driver;
extern struct GameDriver kungfud_driver;
extern struct GameDriver kungfub_driver;
extern struct GameDriver kungfub2_driver;
extern struct GameDriver travrusa_driver;
extern struct GameDriver motorace_driver;
extern struct GameDriver ldrun_driver;
extern struct GameDriver ldruna_driver;
extern struct GameDriver ldrun2p_driver;
extern struct GameDriver kidniki_driver;
extern struct GameDriver spelunk2_driver;
extern struct GameDriver vigilant_driver;
extern struct GameDriver vigilntj_driver;

/* Gottlieb/Mylstar games (Gottlieb became Mylstar in 1983) */
extern struct GameDriver reactor_driver;
extern struct GameDriver mplanets_driver;
extern struct GameDriver qbert_driver;
extern struct GameDriver qbertjp_driver;
extern struct GameDriver sqbert_driver;
extern struct GameDriver krull_driver;
extern struct GameDriver mach3_driver;
extern struct GameDriver usvsthem_driver;
extern struct GameDriver stooges_driver;
extern struct GameDriver qbertqub_driver;
extern struct GameDriver curvebal_driver;

/* older Taito games */
extern struct GameDriver crbaloon_driver;
extern struct GameDriver crbalon2_driver;

/* Taito "Qix hardware" games */
extern struct GameDriver qix_driver;
extern struct GameDriver qix2_driver;
extern struct GameDriver sdungeon_driver;
extern struct GameDriver zookeep_driver;
extern struct GameDriver zookeepa_driver;
extern struct GameDriver elecyoyo_driver;
extern struct GameDriver elecyoy2_driver;
extern struct GameDriver kram_driver;
extern struct GameDriver kram2_driver;

/* Taito SJ System games */
extern struct GameDriver spaceskr_driver;
extern struct GameDriver junglek_driver;
extern struct GameDriver jhunt_driver;
extern struct GameDriver alpine_driver;
extern struct GameDriver alpinea_driver;
extern struct GameDriver timetunl_driver;
extern struct GameDriver wwestern_driver;
extern struct GameDriver frontlin_driver;
extern struct GameDriver elevator_driver;
extern struct GameDriver elevatob_driver;
extern struct GameDriver tinstar_driver;
extern struct GameDriver waterski_driver;
extern struct GameDriver bioatack_driver;
extern struct GameDriver sfposeid_driver;

/* other Taito games */
extern struct GameDriver bking2_driver;
extern struct GameDriver gsword_driver;
extern struct GameDriver gladiatr_driver;
extern struct GameDriver ogonsiro_driver;
extern struct GameDriver gcastle_driver;
extern struct GameDriver tokio_driver;
extern struct GameDriver tokiob_driver;
extern struct GameDriver bublbobl_driver;
extern struct GameDriver boblbobl_driver;
extern struct GameDriver sboblbob_driver;
extern struct GameDriver rastan_driver;
extern struct GameDriver rastsaga_driver;
extern struct GameDriver rainbow_driver;
extern struct GameDriver rainbowe_driver;
extern struct GameDriver jumping_driver;
extern struct GameDriver arkanoid_driver;
extern struct GameDriver arknoidu_driver;
extern struct GameDriver arkbl2_driver;
extern struct GameDriver arkatayt_driver;
extern struct GameDriver superqix_driver;
extern struct GameDriver sqixbl_driver;
extern struct GameDriver twincobr_driver;
extern struct GameDriver twincobu_driver;
extern struct GameDriver ktiger_driver;
extern struct GameDriver arkanoi2_driver;
extern struct GameDriver ark2us_driver;
extern struct GameDriver tnzs_driver;
extern struct GameDriver tnzs2_driver;
extern struct GameDriver tigerh_driver;
extern struct GameDriver tigerh2_driver;
extern struct GameDriver tigerhb1_driver;
extern struct GameDriver tigerhb2_driver;
extern struct GameDriver slapfigh_driver;
extern struct GameDriver slapbtjp_driver;
extern struct GameDriver slapbtuk_driver;
extern struct GameDriver getstar_driver;
extern struct GameDriver superman_driver;

/* Taito F2 games */
extern struct GameDriver ssi_driver;
extern struct GameDriver liquidk_driver;
extern struct GameDriver growl_driver;

/* Williams games */
extern struct GameDriver robotron_driver;
extern struct GameDriver robotryo_driver;
extern struct GameDriver stargate_driver;
extern struct GameDriver joust_driver;
extern struct GameDriver joustr_driver;
extern struct GameDriver joustwr_driver;
extern struct GameDriver sinistar_driver;
extern struct GameDriver sinista1_driver;
extern struct GameDriver sinista2_driver;
extern struct GameDriver bubbles_driver;
extern struct GameDriver bubblesr_driver;
extern struct GameDriver defender_driver;
extern struct GameDriver splat_driver;
extern struct GameDriver blaster_driver;
extern struct GameDriver colony7_driver;
extern struct GameDriver colony7a_driver;
extern struct GameDriver lottofun_driver;
extern struct GameDriver defcmnd_driver;
extern struct GameDriver defence_driver;

/* Capcom games */
extern struct GameDriver vulgus_driver;
extern struct GameDriver vulgus2_driver;
extern struct GameDriver sonson_driver;
extern struct GameDriver higemaru_driver;
extern struct GameDriver c1942_driver;
extern struct GameDriver c1942a_driver;
extern struct GameDriver c1942b_driver;
extern struct GameDriver exedexes_driver;
extern struct GameDriver savgbees_driver;
extern struct GameDriver commando_driver;
extern struct GameDriver commandu_driver;
extern struct GameDriver commandj_driver;
extern struct GameDriver gng_driver;
extern struct GameDriver gngt_driver;
extern struct GameDriver gngcross_driver;
extern struct GameDriver gngjap_driver;
extern struct GameDriver diamond_driver;
extern struct GameDriver gunsmoke_driver;
extern struct GameDriver gunsmrom_driver;
extern struct GameDriver gunsmokj_driver;
extern struct GameDriver gunsmoka_driver;
extern struct GameDriver sectionz_driver;
extern struct GameDriver trojan_driver;
extern struct GameDriver trojanj_driver;
extern struct GameDriver srumbler_driver;
extern struct GameDriver srumblr2_driver;
extern struct GameDriver lwings_driver;
extern struct GameDriver lwings2_driver;
extern struct GameDriver lwingsjp_driver;
extern struct GameDriver sidearms_driver;
extern struct GameDriver sidearmr_driver;
extern struct GameDriver sidearjp_driver;
extern struct GameDriver avengers_driver;
extern struct GameDriver avenger2_driver;
extern struct GameDriver bionicc_driver;
extern struct GameDriver bionicc2_driver;
extern struct GameDriver c1943_driver;
extern struct GameDriver c1943jap_driver;
extern struct GameDriver blktiger_driver;
extern struct GameDriver bktigerb_driver;
extern struct GameDriver blkdrgon_driver;
extern struct GameDriver tigeroad_driver;
extern struct GameDriver f1dream_driver;
extern struct GameDriver f1dreamb_driver;
extern struct GameDriver c1943kai_driver;
extern struct GameDriver lastduel_driver;
extern struct GameDriver lstduelb_driver;
extern struct GameDriver ghouls_driver;
extern struct GameDriver ghoulsj_driver;
extern struct GameDriver madgear_driver;
extern struct GameDriver strider_driver;
extern struct GameDriver striderj_driver;
extern struct GameDriver dwj_driver;
extern struct GameDriver willow_driver;
extern struct GameDriver willowj_driver;
extern struct GameDriver unsquad_driver;
extern struct GameDriver area88_driver;
extern struct GameDriver ffight_driver;
extern struct GameDriver ffightu_driver; /*!*/
extern struct GameDriver ffightj_driver;
extern struct GameDriver c1941_driver;
extern struct GameDriver c1941j_driver;
extern struct GameDriver mercs_driver;
extern struct GameDriver mercsu_driver;
extern struct GameDriver mercsj_driver;
extern struct GameDriver mtwins_driver;
extern struct GameDriver chikij_driver;
extern struct GameDriver msword_driver;
extern struct GameDriver mswordu_driver;
extern struct GameDriver mswordj_driver;
extern struct GameDriver cawing_driver;
extern struct GameDriver cawingj_driver;
extern struct GameDriver nemo_driver;
extern struct GameDriver nemoj_driver;
extern struct GameDriver sf2_driver;
extern struct GameDriver sf2a_driver; /*!*/
extern struct GameDriver sf2b_driver; /*!*/
extern struct GameDriver sf2e_driver; /*!*/
extern struct GameDriver sf2j_driver;
extern struct GameDriver c3wonders_driver; /*!*/
extern struct GameDriver c3wonderj_driver; /*!*/
extern struct GameDriver kod_driver;
extern struct GameDriver kodj_driver; /*!*/
extern struct GameDriver kodb_driver;
extern struct GameDriver captcomm_driver;
extern struct GameDriver captcomu_driver; /*!*/
extern struct GameDriver captcomj_driver; /*!*/
extern struct GameDriver knights_driver;
extern struct GameDriver knightsj_driver; /*!*/
extern struct GameDriver sf2ce_driver; /*!*/
extern struct GameDriver sf2cej_driver; /*!*/
extern struct GameDriver varth_driver;
extern struct GameDriver varthj_driver; /*!*/
extern struct GameDriver sf2t_driver; /*!*/
extern struct GameDriver sf2tj_driver; /*!*/
extern struct GameDriver pnickj_driver;
extern struct GameDriver megaman_driver;
extern struct GameDriver rockmanj_driver;

/* "Capcom Bowling hardware" games */
extern struct GameDriver capbowl_driver;
extern struct GameDriver clbowl_driver;
extern struct GameDriver bowlrama_driver;

/* Mitchell Corp games */
extern struct GameDriver pang_driver;

/* Gremlin 8080 games */
extern struct GameDriver blockade_driver;
extern struct GameDriver comotion_driver;
extern struct GameDriver hustle_driver;
extern struct GameDriver blasto_driver;

/* Gremlin/Sega "VIC dual game board" games */
extern struct GameDriver depthch_driver;
extern struct GameDriver safari_driver;
extern struct GameDriver frogs_driver;
extern struct GameDriver sspaceat_driver;
extern struct GameDriver headon_driver;
extern struct GameDriver invho2_driver;
extern struct GameDriver samurai_driver;
extern struct GameDriver invinco_driver;
extern struct GameDriver invds_driver;
extern struct GameDriver tranqgun_driver;
extern struct GameDriver spacetrk_driver;
extern struct GameDriver sptrekct_driver;
extern struct GameDriver carnival_driver;
extern struct GameDriver pulsar_driver;
extern struct GameDriver heiankyo_driver;

/* Sega G-80 vector games */
extern struct GameDriver spacfury_driver;
extern struct GameDriver spacfura_driver;
extern struct GameDriver zektor_driver;
extern struct GameDriver tacscan_driver;
extern struct GameDriver elim2_driver;
extern struct GameDriver elim4_driver;
extern struct GameDriver startrek_driver;

/* Sega G-80 raster games */
extern struct GameDriver astrob_driver;
extern struct GameDriver astrob1_driver;
extern struct GameDriver s005_driver;
extern struct GameDriver monsterb_driver;
extern struct GameDriver spaceod_driver;
extern struct GameDriver pignewta_driver;
extern struct GameDriver pignewt_driver;

/* Sega "Zaxxon hardware" games */
extern struct GameDriver zaxxon_driver;
extern struct GameDriver szaxxon_driver;
extern struct GameDriver futspy_driver;
extern struct GameDriver congo_driver;
extern struct GameDriver tiptop_driver;

/* Sega System 8 games */
extern struct GameDriver starjack_driver;
extern struct GameDriver starjacs_driver;
extern struct GameDriver regulus_driver;
extern struct GameDriver regulusu_driver;
extern struct GameDriver upndown_driver;
extern struct GameDriver mrviking_driver;
extern struct GameDriver swat_driver;
extern struct GameDriver flicky_driver;
extern struct GameDriver flicky2_driver;
extern struct GameDriver bullfgtj_driver;
extern struct GameDriver pitfall2_driver;
extern struct GameDriver pitfallu_driver;
extern struct GameDriver seganinj_driver;
extern struct GameDriver seganinu_driver;
extern struct GameDriver nprinces_driver;
extern struct GameDriver nprinceb_driver;
extern struct GameDriver imsorry_driver;
extern struct GameDriver imsorryj_driver;
extern struct GameDriver teddybb_driver;
extern struct GameDriver hvymetal_driver;
extern struct GameDriver myhero_driver;
extern struct GameDriver myheroj_driver;
extern struct GameDriver chplft_driver;
extern struct GameDriver chplftb_driver;
extern struct GameDriver chplftbl_driver;
extern struct GameDriver fdwarrio_driver;
extern struct GameDriver brain_driver;
extern struct GameDriver wboy_driver;
extern struct GameDriver wboy2_driver;
extern struct GameDriver wboy3_driver;
extern struct GameDriver wboy4_driver;
extern struct GameDriver wboyu_driver;
extern struct GameDriver wboy4u_driver;
extern struct GameDriver wbdeluxe_driver;
extern struct GameDriver gardia_driver;
extern struct GameDriver blockgal_driver;
extern struct GameDriver tokisens_driver;
extern struct GameDriver dakkochn_driver;
extern struct GameDriver ufosensi_driver;
extern struct GameDriver wbml_driver;

/* Sega System 16 games */
extern struct GameDriver alexkidd_driver;
extern struct GameDriver aliensyn_driver;
extern struct GameDriver altbeast_driver;
extern struct GameDriver astormbl_driver;
extern struct GameDriver aurail_driver;
extern struct GameDriver dduxbl_driver;
extern struct GameDriver eswatbl_driver;
extern struct GameDriver fantzone_driver;
extern struct GameDriver fpointbl_driver;
extern struct GameDriver goldnaxe_driver;
extern struct GameDriver hwchamp_driver;
extern struct GameDriver mjleague_driver;
extern struct GameDriver passshtb_driver;
extern struct GameDriver quartet2_driver;
extern struct GameDriver sdi_driver;
extern struct GameDriver shinobi_driver;
extern struct GameDriver tetrisbl_driver;
extern struct GameDriver timscanr_driver;
extern struct GameDriver tturfbl_driver;
extern struct GameDriver wb3bl_driver;
extern struct GameDriver wrestwar_driver;

/* Data East "Burger Time hardware" games */
extern struct GameDriver lnc_driver;
extern struct GameDriver zoar_driver;
extern struct GameDriver btime_driver;
extern struct GameDriver btimea_driver;
extern struct GameDriver cookrace_driver;
extern struct GameDriver bnj_driver;
extern struct GameDriver brubber_driver;
extern struct GameDriver caractn_driver;
extern struct GameDriver eggs_driver;
extern struct GameDriver scregg_driver;
extern struct GameDriver tagteam_driver;

/* other Data East games */
extern struct GameDriver astrof_driver;
extern struct GameDriver astrof2_driver;
extern struct GameDriver astrof3_driver;
extern struct GameDriver tomahawk_driver;
extern struct GameDriver tomahaw5_driver;
extern struct GameDriver kchamp_driver;
extern struct GameDriver kchampvs_driver;
extern struct GameDriver karatedo_driver;
extern struct GameDriver firetrap_driver;
extern struct GameDriver firetpbl_driver;
extern struct GameDriver brkthru_driver;
extern struct GameDriver darwin_driver;
extern struct GameDriver shootout_driver;
extern struct GameDriver sidepckt_driver;
extern struct GameDriver exprraid_driver;
extern struct GameDriver wexpress_driver;
extern struct GameDriver wexpresb_driver;

/* Data East 8-bit games */
extern struct GameDriver ghostb_driver;
extern struct GameDriver mazeh_driver;
extern struct GameDriver cobracom_driver;
extern struct GameDriver oscar_driver;
extern struct GameDriver oscarj_driver;

/* Data East 16-bit games */
extern struct GameDriver karnov_driver;
extern struct GameDriver karnovj_driver;
extern struct GameDriver chelnov_driver;
extern struct GameDriver chelnovj_driver;
extern struct GameDriver hbarrel_driver;
extern struct GameDriver hbarrelj_driver;
extern struct GameDriver baddudes_driver;
extern struct GameDriver drgninja_driver;
extern struct GameDriver robocopp_driver;
extern struct GameDriver hippodrm_driver;
extern struct GameDriver ffantasy_driver;
extern struct GameDriver slyspy_driver;
extern struct GameDriver midres_driver;
extern struct GameDriver midresj_driver;
extern struct GameDriver darkseal_driver;
extern struct GameDriver gatedoom_driver;

/* Tehkan / Tecmo games (Tehkan became Tecmo in 1986) */
extern struct GameDriver bombjack_driver;
extern struct GameDriver starforc_driver;
extern struct GameDriver megaforc_driver;
extern struct GameDriver pbaction_driver;
extern struct GameDriver pbactio2_driver;
extern struct GameDriver tehkanwc_driver;
extern struct GameDriver gridiron_driver;
extern struct GameDriver teedoff_driver;
extern struct GameDriver solomon_driver;
extern struct GameDriver rygar_driver;
extern struct GameDriver rygar2_driver;
extern struct GameDriver rygarj_driver;
extern struct GameDriver gemini_driver;
extern struct GameDriver silkworm_driver;
extern struct GameDriver silkwrm2_driver;
extern struct GameDriver gaiden_driver;
extern struct GameDriver shadoww_driver;
extern struct GameDriver tknight_driver;
extern struct GameDriver wc90_driver;
extern struct GameDriver wc90b_driver;

/* Konami games */
extern struct GameDriver pooyan_driver;
extern struct GameDriver pooyans_driver;
extern struct GameDriver pootan_driver;
extern struct GameDriver timeplt_driver;
extern struct GameDriver timepltc_driver;
extern struct GameDriver spaceplt_driver;
extern struct GameDriver rocnrope_driver;
extern struct GameDriver ropeman_driver;
extern struct GameDriver gyruss_driver;
extern struct GameDriver gyrussce_driver;
extern struct GameDriver venus_driver;
extern struct GameDriver trackfld_driver;
extern struct GameDriver trackflc_driver;
extern struct GameDriver hyprolym_driver;
extern struct GameDriver hyprolyb_driver;
extern struct GameDriver circusc_driver;
extern struct GameDriver circusc2_driver;
extern struct GameDriver tp84_driver;
extern struct GameDriver tp84a_driver;
extern struct GameDriver hyperspt_driver;
extern struct GameDriver sbasketb_driver;
extern struct GameDriver mikie_driver;
extern struct GameDriver mikiej_driver;
extern struct GameDriver mikiehs_driver;
extern struct GameDriver roadf_driver;
extern struct GameDriver roadf2_driver;
extern struct GameDriver yiear_driver;
extern struct GameDriver kicker_driver;
extern struct GameDriver shaolins_driver;
extern struct GameDriver pingpong_driver;
extern struct GameDriver gberet_driver;
extern struct GameDriver rushatck_driver;
extern struct GameDriver jailbrek_driver;
extern struct GameDriver ironhors_driver;
extern struct GameDriver farwest_driver;
extern struct GameDriver jackal_driver;
extern struct GameDriver topgunr_driver;
extern struct GameDriver topgunbl_driver;
extern struct GameDriver contra_driver;
extern struct GameDriver contrab_driver;
extern struct GameDriver gryzorb_driver;
extern struct GameDriver mainevt_driver;
extern struct GameDriver devstors_driver;

/* Konami "Nemesis hardware" games */
extern struct GameDriver nemesis_driver;
extern struct GameDriver nemesuk_driver;
extern struct GameDriver konamigt_driver;
/* GX400 BIOS based games */
extern struct GameDriver rf2_driver;
extern struct GameDriver twinbee_driver;
extern struct GameDriver gradius_driver;
extern struct GameDriver gwarrior_driver;

/* Konami "TMNT hardware" games */
extern struct GameDriver tmnt_driver;
extern struct GameDriver tmntj_driver;
extern struct GameDriver tmht2p_driver;
extern struct GameDriver tmnt2pj_driver;
extern struct GameDriver punkshot_driver;

/* Exidy games */
extern struct GameDriver sidetrac_driver;
extern struct GameDriver targ_driver;
extern struct GameDriver spectar_driver;
extern struct GameDriver spectar1_driver;
extern struct GameDriver venture_driver;
extern struct GameDriver venture2_driver;
extern struct GameDriver venture4_driver;
extern struct GameDriver mtrap_driver;
extern struct GameDriver pepper2_driver;
extern struct GameDriver hardhat_driver;
extern struct GameDriver fax_driver;
extern struct GameDriver circus_driver;
extern struct GameDriver robotbwl_driver;
extern struct GameDriver crash_driver;
extern struct GameDriver ripcord_driver;
extern struct GameDriver starfire_driver;

/* Atari vector games */
extern struct GameDriver asteroid_driver;
extern struct GameDriver asteroi1_driver;
extern struct GameDriver astdelux_driver;
extern struct GameDriver bwidow_driver;
extern struct GameDriver bzone_driver;
extern struct GameDriver bzone2_driver;
extern struct GameDriver gravitar_driver;
extern struct GameDriver llander_driver;
extern struct GameDriver llander1_driver;
extern struct GameDriver redbaron_driver;
extern struct GameDriver spacduel_driver;
extern struct GameDriver tempest_driver;
extern struct GameDriver tempest1_driver;
extern struct GameDriver tempest2_driver;
extern struct GameDriver temptube_driver;
extern struct GameDriver starwars_driver;
extern struct GameDriver empire_driver;
extern struct GameDriver mhavoc_driver;
extern struct GameDriver mhavoc2_driver;
extern struct GameDriver mhavocrv_driver;
extern struct GameDriver quantum_driver;
extern struct GameDriver quantum1_driver;

/* Atari "Centipede hardware" games */
extern struct GameDriver warlord_driver;
extern struct GameDriver centiped_driver;
extern struct GameDriver centipd2_driver;
extern struct GameDriver milliped_driver;
extern struct GameDriver qwakprot_driver;

/* Atari "Kangaroo hardware" games */
extern struct GameDriver kangaroo_driver;
extern struct GameDriver kangarob_driver;
extern struct GameDriver arabian_driver;

/* Atari "Missile Command hardware" games */
extern struct GameDriver missile_driver;
extern struct GameDriver missile2_driver;
extern struct GameDriver suprmatk_driver;

/* Atari b/w games */
extern struct GameDriver sprint1_driver;
extern struct GameDriver sprint2_driver;
extern struct GameDriver sbrkout_driver;
extern struct GameDriver dominos_driver;
extern struct GameDriver nitedrvr_driver;
extern struct GameDriver bsktball_driver;
extern struct GameDriver copsnrob_driver;
extern struct GameDriver avalnche_driver;
extern struct GameDriver subs_driver;

/* Atari System 1 games */
extern struct GameDriver marble_driver;
extern struct GameDriver marble2_driver;
extern struct GameDriver marblea_driver;
extern struct GameDriver peterpak_driver;
extern struct GameDriver indytemp_driver;
extern struct GameDriver roadrunn_driver;
extern struct GameDriver roadblst_driver;

/* Atari System 2 games */
extern struct GameDriver paperboy_driver;
extern struct GameDriver ssprint_driver;
extern struct GameDriver csprint_driver;
extern struct GameDriver a720_driver;
extern struct GameDriver a720b_driver;
extern struct GameDriver apb_driver;
extern struct GameDriver apb2_driver;

/* later Atari games */
extern struct GameDriver gauntlet_driver;
extern struct GameDriver gauntir1_driver;
extern struct GameDriver gauntir2_driver;
extern struct GameDriver gaunt2p_driver;
extern struct GameDriver gaunt2_driver;
extern struct GameDriver atetris_driver;
extern struct GameDriver atetrisa_driver;
extern struct GameDriver atetrisb_driver;
extern struct GameDriver atetcktl_driver;
extern struct GameDriver atetckt2_driver;
extern struct GameDriver toobin_driver;
extern struct GameDriver vindictr_driver;
extern struct GameDriver klax_driver;
extern struct GameDriver klax2_driver;
extern struct GameDriver klax3_driver;
extern struct GameDriver blstroid_driver;
extern struct GameDriver eprom_driver;
extern struct GameDriver xybots_driver;

/* SNK / Rock-ola games */
extern struct GameDriver sasuke_driver;
extern struct GameDriver satansat_driver;
extern struct GameDriver zarzon_driver;
extern struct GameDriver vanguard_driver;
extern struct GameDriver vangrdce_driver;
extern struct GameDriver fantasy_driver;
extern struct GameDriver pballoon_driver;
extern struct GameDriver nibbler_driver;
extern struct GameDriver nibblera_driver;

/* Technos games */
extern struct GameDriver mystston_driver;
extern struct GameDriver matmania_driver;
extern struct GameDriver excthour_driver;
extern struct GameDriver maniach_driver;
extern struct GameDriver maniach2_driver;
extern struct GameDriver renegade_driver;
extern struct GameDriver kuniokub_driver;
extern struct GameDriver xsleena_driver;
extern struct GameDriver solarwar_driver;
extern struct GameDriver ddragon_driver;
extern struct GameDriver ddragonb_driver;
extern struct GameDriver ddragon2_driver;
extern struct GameDriver blockout_driver;

/* Stern "Berzerk hardware" games */
extern struct GameDriver berzerk_driver;
extern struct GameDriver berzerk1_driver;
extern struct GameDriver frenzy_driver;
extern struct GameDriver frenzy1_driver;

/* GamePlan games */
extern struct GameDriver megatack_driver;
extern struct GameDriver killcom_driver;
extern struct GameDriver challeng_driver;
extern struct GameDriver kaos_driver;

/* "stratovox hardware" games */
extern struct GameDriver route16_driver;
extern struct GameDriver stratvox_driver;
extern struct GameDriver speakres_driver;

/* Zaccaria games */
extern struct GameDriver monymony_driver;
extern struct GameDriver jackrabt_driver;
extern struct GameDriver jackrab2_driver;
extern struct GameDriver jackrabs_driver;

/* UPL games */
extern struct GameDriver nova2001_driver;
extern struct GameDriver pkunwar_driver;
extern struct GameDriver pkunwarj_driver;
extern struct GameDriver ninjakd2_driver;
extern struct GameDriver ninjak2a_driver;

/* Williams/Midway TMS34010 games */
extern struct GameDriver narc_driver;
extern struct GameDriver trog_driver;
extern struct GameDriver trog3_driver;
extern struct GameDriver trogp_driver;
extern struct GameDriver smashtv_driver;
extern struct GameDriver smashtv5_driver;
extern struct GameDriver hiimpact_driver;
extern struct GameDriver shimpact_driver;
extern struct GameDriver strkforc_driver;
extern struct GameDriver mk_driver;
extern struct GameDriver term2_driver;
extern struct GameDriver totcarn_driver;
extern struct GameDriver totcarnp_driver;
extern struct GameDriver mk2_driver;
extern struct GameDriver nbajam_driver;

/* Cinematronics raster games */
extern struct GameDriver jack_driver;
extern struct GameDriver jacka_driver;
extern struct GameDriver treahunt_driver;
extern struct GameDriver zzyzzyxx_driver;
extern struct GameDriver brix_driver;
extern struct GameDriver sucasino_driver;

/* "The Pit hardware" games */
extern struct GameDriver roundup_driver;
extern struct GameDriver thepit_driver;
extern struct GameDriver intrepid_driver;
extern struct GameDriver portman_driver;
extern struct GameDriver suprmous_driver;

/* Valadon Automation games */
extern struct GameDriver bagman_driver;
extern struct GameDriver bagmans_driver;
extern struct GameDriver bagmans2_driver;
extern struct GameDriver sbagman_driver;
extern struct GameDriver sbagmans_driver;
extern struct GameDriver pickin_driver;

/* Seibu Denshi / Seibu Kaihatsu games */
extern struct GameDriver stinger_driver;
extern struct GameDriver scion_driver;
extern struct GameDriver scionc_driver;
extern struct GameDriver wiz_driver;

/* Nichibutsu games */
extern struct GameDriver cop01_driver;
extern struct GameDriver cop01a_driver;
extern struct GameDriver terracre_driver;
extern struct GameDriver terracra_driver;
extern struct GameDriver galivan_driver;

/* Neo Geo games */
extern struct GameDriver joyjoy_driver;
extern struct GameDriver ridhero_driver;
extern struct GameDriver bstars_driver;
extern struct GameDriver lbowling_driver;
extern struct GameDriver superspy_driver;
extern struct GameDriver ttbb_driver;
extern struct GameDriver alpham2_driver;
extern struct GameDriver eightman_driver;
extern struct GameDriver ararmy_driver;
extern struct GameDriver fatfury1_driver;
extern struct GameDriver burningf_driver;
extern struct GameDriver kingofm_driver;
extern struct GameDriver gpilots_driver;
extern struct GameDriver lresort_driver;
extern struct GameDriver fbfrenzy_driver;
extern struct GameDriver socbrawl_driver;
extern struct GameDriver mutnat_driver;
extern struct GameDriver artfight_driver;
extern struct GameDriver countb_driver;
extern struct GameDriver ncombat_driver;
extern struct GameDriver crsword_driver;
extern struct GameDriver trally_driver;
extern struct GameDriver sengoku_driver;
extern struct GameDriver ncommand_driver;
extern struct GameDriver wheroes_driver;
extern struct GameDriver sengoku2_driver;
extern struct GameDriver androdun_driver;
extern struct GameDriver bjourney_driver;
extern struct GameDriver maglord_driver;
extern struct GameDriver janshin_driver;
extern struct GameDriver pulstar_driver;
extern struct GameDriver blazstar_driver;
extern struct GameDriver pbobble_driver;
extern struct GameDriver puzzledp_driver;
extern struct GameDriver pspikes2_driver;
extern struct GameDriver sonicwi2_driver;
extern struct GameDriver sonicwi3_driver;
extern struct GameDriver goalx3_driver;
extern struct GameDriver neodrift_driver;
extern struct GameDriver neomrdo_driver;
extern struct GameDriver spinmast_driver;
extern struct GameDriver karnov_r_driver;
extern struct GameDriver wjammers_driver;
extern struct GameDriver strhoops_driver;
extern struct GameDriver magdrop2_driver;
extern struct GameDriver magdrop3_driver;
extern struct GameDriver mslug_driver;
extern struct GameDriver turfmast_driver;
extern struct GameDriver kabukikl_driver;
extern struct GameDriver panicbom_driver;
extern struct GameDriver neobombe_driver;
extern struct GameDriver worlher2_driver;
extern struct GameDriver worlhe2j_driver;
extern struct GameDriver aodk_driver;
extern struct GameDriver whp_driver;
extern struct GameDriver ninjamas_driver;
extern struct GameDriver overtop_driver;
extern struct GameDriver twinspri_driver;
extern struct GameDriver stakwin_driver;
extern struct GameDriver ragnagrd_driver;
extern struct GameDriver shocktro_driver;
extern struct GameDriver tws96_driver;
extern struct GameDriver zedblade_driver;
extern struct GameDriver doubledr_driver;
extern struct GameDriver gowcaizr_driver;
extern struct GameDriver galaxyfg_driver;
extern struct GameDriver wakuwak7_driver;
extern struct GameDriver viewpoin_driver;
extern struct GameDriver gururin_driver;
extern struct GameDriver miexchng_driver;
extern struct GameDriver mahretsu_driver;
extern struct GameDriver nam_1975_driver;
extern struct GameDriver cyberlip_driver;
extern struct GameDriver tpgolf_driver;
extern struct GameDriver legendos_driver;
extern struct GameDriver fatfury2_driver;
extern struct GameDriver bstars2_driver;
extern struct GameDriver sidkicks_driver;
extern struct GameDriver kotm2_driver;
extern struct GameDriver samsho_driver;
extern struct GameDriver fatfursp_driver;
extern struct GameDriver fatfury3_driver;
extern struct GameDriver tophuntr_driver;
extern struct GameDriver savagere_driver;
extern struct GameDriver kof94_driver;
extern struct GameDriver aof2_driver;
extern struct GameDriver ssideki2_driver;
extern struct GameDriver sskick3_driver;
extern struct GameDriver samsho2_driver;
extern struct GameDriver samsho3_driver;
extern struct GameDriver kof95_driver;
extern struct GameDriver rbff1_driver;
extern struct GameDriver aof3_driver;
extern struct GameDriver kof96_driver;
extern struct GameDriver samsho4_driver;
extern struct GameDriver rbffspec_driver;
extern struct GameDriver kizuna_driver;
extern struct GameDriver kof97_driver;
extern struct GameDriver lastblad_driver;
extern struct GameDriver mslug2_driver;
extern struct GameDriver realbou2_driver;

extern struct GameDriver spacefb_driver;
extern struct GameDriver tutankhm_driver;
extern struct GameDriver tutankst_driver;
extern struct GameDriver junofrst_driver;
extern struct GameDriver ccastles_driver;
extern struct GameDriver ccastle2_driver;
extern struct GameDriver blueprnt_driver;
extern struct GameDriver omegrace_driver;
extern struct GameDriver bankp_driver;
extern struct GameDriver espial_driver;
extern struct GameDriver espiale_driver;
extern struct GameDriver cloak_driver;
extern struct GameDriver champbas_driver;
extern struct GameDriver champbb2_driver;
extern struct GameDriver exerion_driver;
extern struct GameDriver exeriont_driver;
extern struct GameDriver exerionb_driver;
extern struct GameDriver foodf_driver;
extern struct GameDriver vastar_driver;
extern struct GameDriver vastar2_driver;
extern struct GameDriver formatz_driver;
extern struct GameDriver aeroboto_driver;
extern struct GameDriver citycon_driver;
extern struct GameDriver psychic5_driver;
extern struct GameDriver jedi_driver;
extern struct GameDriver tankbatt_driver;
extern struct GameDriver liberatr_driver;
extern struct GameDriver dday_driver;
extern struct GameDriver toki_driver;
extern struct GameDriver snowbros_driver;
extern struct GameDriver snowbro2_driver;
extern struct GameDriver gundealr_driver;
extern struct GameDriver gundeala_driver;
extern struct GameDriver leprechn_driver;
extern struct GameDriver potogold_driver;
extern struct GameDriver hexa_driver;
extern struct GameDriver redalert_driver;
extern struct GameDriver irobot_driver;
extern struct GameDriver spiders_driver;
extern struct GameDriver stactics_driver;
extern struct GameDriver goldstar_driver;
extern struct GameDriver goldstbl_driver;
extern struct GameDriver exterm_driver;
extern struct GameDriver sharkatt_driver;
extern struct GameDriver turbo_driver;
extern struct GameDriver turboa_driver;
extern struct GameDriver turbob_driver;
extern struct GameDriver kingofb_driver;
extern struct GameDriver kingofbj_driver;
extern struct GameDriver ringking_driver;
extern struct GameDriver ringkin2_driver;
extern struct GameDriver zerozone_driver;
extern struct GameDriver exctsccr_driver;
extern struct GameDriver exctsccb_driver;
extern struct GameDriver speedbal_driver;
extern struct GameDriver sauro_driver;
extern struct GameDriver pow_driver;
extern struct GameDriver powj_driver;

/* MAME GP2X 1.4 Added Games (MAME 0.35b1-0.35b5) */
extern struct GameDriver rthunder_driver;
extern struct GameDriver pcktgal_driver;
extern struct GameDriver pcktgalb_driver;
extern struct GameDriver ikari_driver;
extern struct GameDriver ikarijp_driver;
extern struct GameDriver ikarijpb_driver;
extern struct GameDriver victroad_driver;
extern struct GameDriver dogosoke_driver;
extern struct GameDriver galpanic_driver;
extern struct GameDriver sf1_driver;
extern struct GameDriver sf1us_driver;
extern struct GameDriver sf1jp_driver;
extern struct GameDriver aerofgt_driver;
extern struct GameDriver turbofrc_driver;
extern struct GameDriver pangb_driver;
extern struct GameDriver bbros_driver;
extern struct GameDriver spang_driver;
extern struct GameDriver block_driver;
extern struct GameDriver pspikes_driver;
extern struct GameDriver zerowing_driver;
extern struct GameDriver hellfire_driver;
extern struct GameDriver vimana_driver;
extern struct GameDriver vimana2_driver;

/* MAME GP2X 1.5 Added Games */
extern struct GameDriver tumblep_driver;
extern struct GameDriver tumblep2_driver;
extern struct GameDriver ghostb3_driver;
extern struct GameDriver srdarwin_driver;
extern struct GameDriver gondo_driver;
extern struct GameDriver mekyosen_driver;
extern struct GameDriver lastmiss_driver;
extern struct GameDriver lastmss2_driver;
extern struct GameDriver shackled_driver;
extern struct GameDriver breywood_driver;
extern struct GameDriver csilver_driver;
extern struct GameDriver cworld2j_driver;
extern struct GameDriver qad_driver;
extern struct GameDriver qadj_driver;
extern struct GameDriver wof_driver;
extern struct GameDriver wofj_driver;
extern struct GameDriver dino_driver;
extern struct GameDriver punisher_driver;
extern struct GameDriver punishrj_driver;
extern struct GameDriver slammast_driver;
extern struct GameDriver mbomber_driver;
extern struct GameDriver mbomberj_driver;

/* MAME GP2X 1.8 Added Games */
extern struct GameDriver prehisle_driver;
extern struct GameDriver prehislu_driver;
extern struct GameDriver prehislj_driver;
extern struct GameDriver mainevt2_driver;
extern struct GameDriver tnk3_driver;
extern struct GameDriver aso_driver;
extern struct GameDriver athena_driver;
extern struct GameDriver fitegolf_driver;
extern struct GameDriver gwar_driver;
extern struct GameDriver bermudat_driver;
extern struct GameDriver psychos_driver;
extern struct GameDriver psychosa_driver;
extern struct GameDriver chopper_driver;
extern struct GameDriver legofair_driver;
extern struct GameDriver tdfever_driver;
extern struct GameDriver tdfeverj_driver;
extern struct GameDriver lomakaj_driver;
extern struct GameDriver p47_driver;
extern struct GameDriver p47j_driver;
extern struct GameDriver street64_driver;
extern struct GameDriver edf_driver;
extern struct GameDriver rodlandj_driver;
extern struct GameDriver avspirit_driver;
extern struct GameDriver sichuan2_driver;
extern struct GameDriver shisen_driver;
extern struct GameDriver truxton_driver;
extern struct GameDriver outzone_driver;
extern struct GameDriver cabal_driver;
extern struct GameDriver cabal2_driver;
extern struct GameDriver cabalbl_driver;
extern struct GameDriver goindol_driver;
extern struct GameDriver homo_driver;

/* MAME GP2X 2.0 Added Games */
extern struct GameDriver gaplus_driver; /*0.35b9*/
extern struct GameDriver gaplusa_driver;
extern struct GameDriver galaga3_driver;
extern struct GameDriver galaga3a_driver;
extern struct GameDriver salamand_driver; /*0.35b12*/
extern struct GameDriver lifefrce_driver;
extern struct GameDriver lifefrcj_driver;
extern struct GameDriver megazone_driver;
extern struct GameDriver pang3_driver; /*0.35rc1*/

/* MAME GP2X 2.3 Added Games */
extern struct GameDriver slyspy2_driver; /* 0.35b5 */
extern struct GameDriver bouldash_driver;
extern struct GameDriver gatedoma_driver;

/* MAME GP2X 3.3 Added Games */
extern struct GameDriver canyon_driver;
extern struct GameDriver canbprot_driver;
extern struct GameDriver marineb_driver;
extern struct GameDriver wanted_driver;
extern struct GameDriver troangel_driver;

/* MAME GP2X 3.7 Added Games */
extern struct GameDriver dangar_driver; /*0.35b11*/
extern struct GameDriver dangar2_driver; /*0.35b11*/
extern struct GameDriver dangarb_driver; /*0.35b11*/
extern struct GameDriver galivan2_driver; /*0.35b11*/
extern struct GameDriver ambush_driver; /*0.35b3*/

/* MAME GP2X 3.9 Added Games */
extern struct GameDriver turtship_driver; /*0.35b1*/

/* MAME GP2X 4.1 Added Games */
extern struct GameDriver cosmica_driver; /*0.35b13*/
extern struct GameDriver magspot2_driver; /*0.35b3*/
extern struct GameDriver devzone_driver; /*0.35b3*/
extern struct GameDriver nomnlnd_driver; /*0.35b13*/
extern struct GameDriver nomnlndg_driver; /*0.35b13*/
extern struct GameDriver cninja_driver; /*0.35b8*/
extern struct GameDriver cninja0_driver; /*0.35b8*/
extern struct GameDriver cninjau_driver; /*0.35b8*/
extern struct GameDriver joemac_driver; /*0.35b8*/
extern struct GameDriver stoneage_driver; /*0.35b8*/
extern struct GameDriver combasc_driver; /*0.35b12*/
extern struct GameDriver combascb_driver; /*0.35b12*/
extern struct GameDriver ddrible_driver; /*0.35b11*/
extern struct GameDriver armedf_driver; /*0.35b11*/
extern struct GameDriver terraf_driver; /*0.35b11*/
extern struct GameDriver terrafu_driver; /*0.35b11*/
extern struct GameDriver pandoras_driver; /*0.37b12*/
extern struct GameDriver skylancr_driver; /*0.35*/
extern struct GameDriver funkybee_driver; /*0.35*/
extern struct GameDriver wardner_driver; /*0.35*/
extern struct GameDriver wardnerj_driver; /*0.35*/
extern struct GameDriver pyros_driver; /*0.35*/

/* MAME GP2X 4.6 Added Games (0.36b1) */
extern struct GameDriver altbeas2_driver;	/* (c) 1988 */
extern struct GameDriver atomicp_driver;	/* (c) 1990 Philko */
extern struct GameDriver bodyslam_driver;	/* (c) 1986 */
extern struct GameDriver goldnaxa_driver;	/* (c) 1989 */
extern struct GameDriver hangon_driver;		/* (c) 1985 */
extern struct GameDriver moonwlkb_driver;	/* bootleg */
extern struct GameDriver quartet_driver;	/* (c) 1986 */
extern struct GameDriver riotcity_driver;	/* (c) 1991 Sega / Westone */
extern struct GameDriver shdancer_driver;	/* (c) 1989 */
extern struct GameDriver shdancrj_driver;	/* (c) 1989 */
extern struct GameDriver shinobl_driver;	/* (c) 1987 (but bootleg) */
extern struct GameDriver tturf_driver;		/* (c) 1989 Sega / Sunsoft */
extern struct GameDriver wb3_driver;		/* (c) 1988 Sega / Westone */

/* MAME GP2X 4.7 Added Games (0.36b4) */
extern struct GameDriver alexkida_driver;	/* (c) 1986 */
extern struct GameDriver aliensya_driver;	/* (c) 1987 (protected) */
extern struct GameDriver aliensyb_driver;	/* (c) 1987 (protected) */
extern struct GameDriver astorm_driver;		/* (c) 1990 (protected) */
extern struct GameDriver auraila_driver;	/* (c) 1990 Sega / Westone (protected) */
extern struct GameDriver bayrouta_driver;	/* (c) 1989 (protected) */
extern struct GameDriver bayroute_driver;	/* (c) 1989 */
extern struct GameDriver bayrtbl1_driver;	/* (c) 1989 (protected) (bootleg) */
extern struct GameDriver bayrtbl2_driver;	/* (c) 1989 (protected) (bootleg) */
extern struct GameDriver endurob2_driver;	/* (c) 1985 (Beta bootleg) */
extern struct GameDriver endurobl_driver;	/* (c) 1985 (Herb bootleg) */
extern struct GameDriver enduror_driver;	/* (c) 1985 (protected) */
extern struct GameDriver eswat_driver;		/* (c) 1989 (protected) */
extern struct GameDriver fpoint_driver;		/* (c) 1989 (protected) */
extern struct GameDriver goldnabl_driver;	/* (c) 1989 (bootleg) */
extern struct GameDriver goldnaxb_driver;	/* (c) 1989 (protected) */
extern struct GameDriver goldnaxc_driver;	/* (c) 1989 (protected) */
extern struct GameDriver moonwalk_driver;	/* (c) 1990 (protected) */
extern struct GameDriver outrun_driver;		/* (c) 1986 (bootleg)*/
extern struct GameDriver outruna_driver;	/* (c) 1986 (bootleg) */
extern struct GameDriver outrunb_driver;	/* (c) 1986 (protected beta bootleg) */
extern struct GameDriver passsht_driver;	/* (protected) */
extern struct GameDriver shangon_driver;	/* (c) 1992 (protected) */
extern struct GameDriver shangonb_driver;	/* (c) 1992 (but bootleg) */
extern struct GameDriver sharrier_driver;	/* (c) 1985 */
extern struct GameDriver shdancbl_driver;	/* (c) 1989 (but bootleg) */
extern struct GameDriver shinobia_driver;	/* (c) 1987 (protected) */
extern struct GameDriver tetris_driver;		/* (c) 1988 (protected) */
extern struct GameDriver wb3a_driver;		/* (c) 1988 Sega / Westone (protected) */

/* MAME GP2X 4.8 Added Games (???) */
extern struct GameDriver dkongex_driver;

/* MAME GP2x 5.1 Added Games */
extern struct GameDriver kncljoe_driver;        /* (c) 1987 Seibu Kaihatsu */


const struct GameDriver *drivers[] =
{
#ifndef NO_DRIVER_GENERIC_8BIT
&s005_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&vsyard_driver,		/* (c) 1983/1984 */                                                                                                  
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&yard_driver,		/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_CPS1
&c1941j_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&c1941_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CAPCOM
&c1942_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&c1942a_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&c1942b_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&c1943jap_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&c1943_driver,		/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&c1943kai_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&ttbb_driver,       /* 1991, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&zzzap_driver,		/* 610 [1976] */                                                                                                     
#endif
#ifndef NO_DRIVER_NEOGEO
&countb_driver,     /* 1993, SNK */                                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&midwbowl_driver,	/* 730 [1978] Midway */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&fdwarrio_driver,	/* 834-5918 (c) 1985 Coreland / Sega */                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&k600_driver,		/* GX353 (c) 1981 Konami */                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&street64_driver,	/* (c) 1991 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&a720_driver,		/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&a720b_driver,		/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&apb_driver,		/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&apb2_driver,		/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&aerofgt_driver,	/* (c) 1992 Video System Co. */
#endif
#ifndef NO_DRIVER_NEOGEO
&sonicwi2_driver,   /* 1994, Visco */                                                                                                        
#endif
#ifndef NO_DRIVER_NEOGEO
&sonicwi3_driver,   /* 1995, Visco */                                                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&aeroboto_driver,	/* (c) 1984 Williams */                                                                                              
#endif
#ifndef NO_DRIVER_NEOGEO
&aodk_driver,       /* 1994, ADK */                                                                                                          
#endif
#ifndef NO_DRIVER_SYSTEM16
&alexkidd_driver,	/* (c) 1986 (but bootleg) */                                                                                         
#endif
#ifndef NO_DRIVER_SYSTEM16
&alexkida_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&astorm_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&astormbl_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_SYSTEM16
&aliensyn_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&aliensya_driver,	/* (c) 1987 (protected) */
#endif
#ifndef NO_DRIVER_SYSTEM16
&aliensyb_driver,	/* (c) 1987 (protected) */
#endif
#ifndef NO_DRIVER_NEOGEO
&alpham2_driver,    /* 1991, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&alpine_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&alpinea_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&altbeast_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&altbeas2_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&maze_driver,		/* 611 [1976] */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ambush_driver,		/* (c) 1983 Nippon Amuse Co-Ltd */                                                                                                      
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&amidarjp_driver,	/* GX337 (c) 1981 Konami */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&amidars_driver,	/* (c) 1982 Konami */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&amidar_driver,		/* GX337 (c) 1982 Konami + Stern license */                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&amigo_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_NEOGEO
&androdun_driver,   /* 1992, Visco */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&anteater_driver,	/* (c) 1982 Tago */                                                                                                  
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&arabian_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_CPS1
&area88_driver,		/* (c) 1989 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_TAITO2
&arkbl2_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_TAITO2
&arkatayt_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_TAITO2
&arknoidu_driver,	/* (c) 1986 Taito America + Romstar license */                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&arkanoid_driver,	/* (c) 1986 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_TAITO2
&ark2us_driver,		/* (c) 1987 + Romstar license */                                                                                     
#endif
#ifndef NO_DRIVER_TAITO2
&arkanoi2_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&armwrest_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_NICHIBUT
&armedf_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&armorcar_driver,	/* (c) 1981 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&armorca2_driver,	/* (c) 1981 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&artfight_driver,   /* 1992, SNK */                                                                                                          
&aof2_driver,       /* 1994, SNK */                                                                                                          
&aof3_driver,       /* 1996, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&aso_driver,		/* (c) 1985 */
#endif
#ifndef NO_DRIVER_ATARIVG
&asteroi1_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_ATARIVG
&asteroid_driver,	/* (c) 1979 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&astdelux_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&astrob1_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&astrob_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&astrof2_driver,	/* (c) [1980?] */                                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&astrof3_driver,	/* (c) [1980?] */                                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&astrof_driver,		/* (c) [1980?] */                                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&astinvad_driver,	/* (c) 1980 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&astlaser_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_OTHER
&athena_driver	,	/* (c) 1986 */
#endif
#ifndef NO_DRIVER_SYSTEM16
&atomicp_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&aurail_driver,                                                                                                                              
#endif
#ifndef NO_DRIVER_SYSTEM16
&auraila_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&avalnche_driver,	/* no copyright notice [1978] */                                                                                     
#endif
#ifndef NO_DRIVER_CAPCOM
&avengers_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&avenger2_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&avspirit_driver,	/* (c) 1991 */
#endif
#ifndef NO_DRIVER_DEC0
&baddudes_driver,	/* (c) 1988 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bagmans_driver,	/* (c) 1982 + Stern license */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bagmans2_driver,	/* (c) 1982 + Stern license */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bagman_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bandido_driver,	/* (c) Exidy */                                                                                                      
#endif
#ifndef NO_DRIVER_OTHER
&bankp_driver,		/* (c) 1984 Sega */                                                                                                  
#endif
#ifndef NO_DRIVER_NEOGEO
&bstars2_driver,    /* 1992, SNK */                                                                                                          
&bstars_driver,     /* 1990, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bsktball_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_ATARIVG
&bzone2_driver,		/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&bzone_driver,		/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&atlantis_driver,	/* (c) 1981 Comsoft */                                                                                               
#endif
#ifndef NO_DRIVER_SYSTEM16
&bayroute_driver,	/* (c) 1989 */
#endif
#ifndef NO_DRIVER_SYSTEM16
&bayrouta_driver,	/* (c) 1989 (protected) */
#endif
#ifndef NO_DRIVER_SYSTEM16
&bayrtbl1_driver,	/* (c) 1989 (protected) (bootleg) */
#endif
#ifndef NO_DRIVER_SYSTEM16
&bayrtbl2_driver,	/* (c) 1989 (protected) (bootleg) */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&beastf_driver,		/* (c) 1984 Epos Corporation */                                                                                      
#endif
#ifndef NO_DRIVER_OTHER
&bermudat_driver,	/* (c) 1987 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&berzerk1_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&berzerk_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bioatack_driver,	/* (c) 1983 + Fox Video Games license */                                                                             
#endif
#ifndef NO_DRIVER_CAPCOM
&bionicc_driver,	/* (c) 1987 */	/* aka Top Secret */                                                                                 
#endif
#ifndef NO_DRIVER_CAPCOM
&bionicc2_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&bking2_driver,		/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&blkdrgon_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&bktigerb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_CAPCOM
&blktiger_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&bwidow_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&blaster_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&blstroid_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&blasto_driver,		/* [1978 Gremlin] */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&blazstar_driver,   /* 1998, Yumekobo */                                                                                                     
#endif
#ifndef NO_DRIVER_OTHER
&block_driver,		/* (c) 1991 Capcom (World) only one listed on Capcom site */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&blockgal_driver,	/* 834-6303 */                                                                                                       
#endif
#ifndef NO_DRIVER_TECHNOS
&blockout_driver,	/* TA-0029 (c) 1989 + California Dreams */                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&blockade_driver,	/* 1-4 [1977 Gremlin] */                                                                                             
#endif
#ifndef NO_DRIVER_OTHER
&blueprnt_driver,	/* (c) 1982 Bally Midway */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&blueshrk_driver,	/* 742 [1978] */                                                                                                     
#endif
#ifndef NO_DRIVER_NEOGEO
&bjourney_driver,   /* 1990, Alpha Denshi Co */                                                                                              
#endif
#ifndef NO_DRIVER_TAITO2
&boblbobl_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_SYSTEM16
&bodyslam_driver,
#endif
#ifndef NO_DRIVER_TEHKAN
&bombjack_driver,	/* (c) 1984 Tehkan */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&boothill_driver,	/* 612 [1977] */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bosco_driver,		/* (c) 1981 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bosconm_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_DEC0
&bouldash_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bowlrama_driver,	/* (c) 1991 P & P Marketing */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&brain_driver,		/* (c) 1986 Coreland / Sega */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&brkthru_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&breywood_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&brix_driver,		/* (c) 1982 Cinematronics */                                                                                         
#endif
#ifndef NO_DRIVER_TAITO2
&bublbobl_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bubblesr_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bubbles_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bullfgtj_driver,	/* 834-5478 (c) 1984 Sega / Coreland */                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&bnj_driver,		/* (c) 1982 + Midway */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&btimea_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&btime_driver,		/* (c) 1982 + Midway */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&brubber_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&burningf_driver,   /* 1991, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&bbros_driver,		/* (c) 1989 Capcom (US) */
#endif
#ifndef NO_DRIVER_OTHER
&cabal_driver,		/* (c) 1988 Tad + Fabtek license */
#endif
#ifndef NO_DRIVER_OTHER
&cabal2_driver,		/* (c) 1988 Tad + Fabtek license */
#endif
#ifndef NO_DRIVER_OTHER
&cabalbl_driver,	/* bootleg */
#endif
#ifndef NO_DRIVER_CPS1
&dino_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&calipso_driver,	/* (c) 1982 Tago */                                                                                                  
#endif
#ifndef NO_DRIVER_OTHER
&canyon_driver,
#endif
#ifndef NO_DRIVER_OTHER
&canbprot_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&capbowl_driver,	/* (c) 1988 Incredible Technologies */                                                                               
#endif
#ifndef NO_DRIVER_CPS1
&cworld2j_driver,
#endif
#ifndef NO_DRIVER_CPS1
&captcomm_driver,	/* (c) 1991 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&captcomj_driver,
#endif
#ifndef NO_DRIVER_CPS1
&captcomu_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&csilver_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&caractn_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&carnival_driver,	/* 651-666 (c) 1980 Sega */                                                                                          
#endif
#ifndef NO_DRIVER_CPS1
&cawingj_driver,	/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&cawing_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_DEC0
&cninja0_driver,	/* (c) 1991 */
#endif
#ifndef NO_DRIVER_DEC0
&cninja_driver,	/* (c) 1991 */
#endif
#ifndef NO_DRIVER_DEC0
&cninjau_driver,	/* (c) 1991 */
#endif
#ifndef NO_DRIVER_CENTIPEDE
&centipd2_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_CENTIPEDE
&centiped_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&challeng_driver,	/* (c) 1981 Centuri */                                                                                               
#endif
#ifndef NO_DRIVER_OTHER
&champbb2_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_OTHER
&champbas_driver,	/* (c) 1983 Sega */                                                                                                  
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&csprint_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&checkman_driver,	/* (c) 1982 Zilec-Zenitone */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&checkmat_driver,	/* 615 [1977] */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&cheekyms_driver,	/* (c) [1980?] */                                                                                                    
#endif
#ifndef NO_DRIVER_DEC0
&chelnovj_driver,	/* (c) 1988 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_DEC0
&chelnov_driver,	/* (c) 1988 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&chplftb_driver,	/* 834-5795 (c) 1985, (c) 1982 Dan Gorlin */                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&chplftbl_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&chplft_driver,		/* 834-5795 (c) 1985, (c) 1982 Dan Gorlin */                                                                         
#endif
#ifndef NO_DRIVER_OTHER
&chopper_driver,	/* (c) 1988 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&circusc2_driver,	/* GX380 (c) 1984 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&circusc_driver,	/* GX380 (c) 1984 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&circus_driver,		/* no copyright notice [1977?] */                                                                                    
#endif
#ifndef NO_DRIVER_OTHER
&citycon_driver,	/* (c) 1985 Jaleco */                                                                                                
#endif
#ifndef NO_DRIVER_OTHER
&cloak_driver,		/* (c) 1983 Atari */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&clowns_driver,		/* 630 [1978] */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&cobracom_driver,	/* (c) 1988 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&colony7_driver,	/* (c) 1981 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&colony7a_driver,	/* (c) 1981 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&combasc_driver,	/* GX611 (c) 1988 */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&combascb_driver,	/* bootleg */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&commsega_driver,	/* (c) 1983 Sega */                                                                                                  
#endif
#ifndef NO_DRIVER_CAPCOM
&commandu_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&commando_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&comotion_driver,	/* 5-7 [1977 Gremlin] */                                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&congo_driver,		/* 605-5167 (c) 1983 */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gryzorb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&contrab_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&contra_driver,		/* GX633 (c) 1987 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&cookrace_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&clbowl_driver,		/* (c) 1989 Incredible Technologies */                                                                               
#endif
#ifndef NO_DRIVER_NICHIBUT
&cop01_driver,		/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_NICHIBUT
&cop01a_driver,		/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&copsnrob_driver,	/* [1976] */                                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&cosmica_driver,	/* (c) [1979] */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&cavenger_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&cosmicmo_driver,	/* Universal */                                                                                                      
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&crash_driver,		/* Exidy [1979?] */                                                                                                  
#endif
#ifndef NO_DRIVER_MCR
&crater_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&crbaloon_driver,	/* (c) 1980 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&crbalon2_driver,	/* (c) 1980 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ccjap_driver,		/* (c) 1980 Nichibutsu */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&cclimber_driver,	/* (c) 1980 Nichibutsu */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ccboot_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ckongalc_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ckongjeu_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ckongs_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ckong_driver,		/* (c) 1981 Falcon */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ckonga_driver,		/* (c) 1981 Falcon */                                                                                                
#endif
#ifndef NO_DRIVER_NEOGEO
&crsword_driver,    /* 1991, Alpha Denshi Co */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&crush_driver,		/* bootleg Make Trax */                                                                                              
#endif
#ifndef NO_DRIVER_OTHER
&ccastles_driver,	/* (c) 1983 Atari */                                                                                                 
#endif
#ifndef NO_DRIVER_OTHER
&ccastle2_driver,	/* (c) 1983 Atari */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&curvebal_driver,	/* GV-134 (c) 1984 Mylstar */                                                                                        
#endif
#ifndef NO_DRIVER_NEOGEO
&cyberlip_driver,   /* 1990, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&dday_driver,		/* (c) 1982 Centuri */                                                                                               
#endif
#ifndef NO_DRIVER_CPS1
&ghoulsj_driver,	/* (c) 1988 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dakkochn_driver,	/* 836-6483? */                                                                                                      
#endif
#ifndef NO_DRIVER_NICHIBUT
&dangar_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_NICHIBUT
&dangar2_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_NICHIBUT
&dangarb_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&darkplnt_driver,	/* (c) 1982 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_DEC0
&darkseal_driver,	/* (c) 1990 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&darwin_driver,                                                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&deadeye_driver,	/* [1978?] */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&defence_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&redufo_driver,		/* ? */                                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&defender_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&defcmnd_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_MCR
&destderb_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&depthch_driver,	/* 50-55 [1977 Gremlin?] */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&desterth_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&devstors_driver,	/* GX890 (c) 1988 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&devzone_driver,	/* (c) [1979] */                                                                                                   
#endif
#ifndef NO_DRIVER_CAPCOM
&diamond_driver,	/* (c) 1989 KH Video (NOT A CAPCOM GAME but runs on GnG hardware) */                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&digdug_driver,		/* (c) 1982 Atari */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&digdugnm_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&digdug2_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_MCR
&dotrone_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_MCR
&dotron_driver,		/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dogpatch_driver,	/* 644 [1977] */                                                                                                     
#endif
#ifndef NO_DRIVER_OTHER
&dogosoke_driver,	/* (c) 1986 */
#endif
#ifndef NO_DRIVER_MCR
&domino_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dominos_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dkongjp_driver,	/* (c) 1981 Nintendo */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dkong_driver,		/* (c) 1981 Nintendo of America */                                                                                   
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dkong3_driver,		/* (c) 1983 Nintendo of America */                                                                                   
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dkongex_driver,		/* (c) 1981 Nintendo of America */                                                                                   
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dkngjrjp_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dkjrjp_driver,		/* (c) 1982 Nintendo */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dkongjr_driver,	/* (c) 1982 Nintendo of America */                                                                                   
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dkjrbl_driver,		/* (c) 1982 Nintendo of America */                                                                                   
#endif
#ifndef NO_DRIVER_NEOGEO
&doubledr_driver,   /* 1995, Technos */                                                                                                      
#endif
#ifndef NO_DRIVER_TECHNOS
&ddragonb_driver,	/* TA-0021 bootleg */                                                                                                
#endif
#ifndef NO_DRIVER_TECHNOS
&ddragon2_driver,	/* TA-0026 (c) 1988 */                                                                                               
#endif
#ifndef NO_DRIVER_TECHNOS
&ddragon_driver,                                                                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ddrible_driver,	/* GX690 (c) 1986 */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dplay_driver,		/* 619 [1977] */                                                                                                     
#endif
#ifndef NO_DRIVER_DEC0
&drgninja_driver,	/* (c) 1988 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_SYSTEM16
&dduxbl_driver,                                                                                                                              
#endif
#ifndef NO_DRIVER_SYSTEM16
&eswat_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&eswatbl_driver,	/* (c) 1989 (but bootleg) */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&eagle_driver,		/* (c) Centuri */                                                                                                    
#endif
#ifndef NO_DRIVER_OTHER
&edf_driver,		/* (c) 1991 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&eggs_driver,		/* (c) 1983 Universal USA */                                                                                         
#endif
#ifndef NO_DRIVER_NEOGEO
&eightman_driver,   /* 1991, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&elecyoyo_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&elecyoy2_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&elevatob_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&elevator_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&elim2_driver,		/* (c) 1981 Gremlin */                                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&elim4_driver,		/* (c) 1981 Gremlin */                                                                                               
#endif
#ifndef NO_DRIVER_SYSTEM16
&enduror_driver,	/* (c) 1985 (protected) */
#endif
#ifndef NO_DRIVER_SYSTEM16
&endurobl_driver,	/* (c) 1985 (Herb bootleg) */
#endif
#ifndef NO_DRIVER_SYSTEM16
&endurob2_driver,	/* (c) 1985 (Beta bootleg) */
#endif
#ifndef NO_DRIVER_ATARI
&eprom_driver,		/* (c) 1989 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&espiale_driver,	/* (c) 1983 Thunderbolt */                                                                                           
#endif
#ifndef NO_DRIVER_OTHER
&espial_driver,		/* (c) 1983 Thunderbolt */                                                                                           
#endif
#ifndef NO_DRIVER_TECHNOS
&excthour_driver,	/* TA-0015 (c) 1985 + Taito license */                                                                               
#endif
#ifndef NO_DRIVER_OTHER
&exctsccb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&exctsccr_driver,	/* (c) 1983 Alpha Denshi Co. */                                                                                      
#endif
#ifndef NO_DRIVER_CAPCOM
&exedexes_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&exeriont_driver,	/* (c) 1983 Jaleco + Taito America license */                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&exerionb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&exerion_driver,	/* (c) 1983 Jaleco */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&exprraid_driver,	/* (c) 1986 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_TMS
&exterm_driver,		/* (c) 1989 Premier Technology - a Gottlieb game */                                                                  
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ebases_driver,                                                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&einnings_driver,	/* 642 [1978] Midway */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&eyes_driver,		/* (c) 1982 Digitrex Techstar + "Rockola presents" */                                                                
#endif
#ifndef NO_DRIVER_CAPCOM
&f1dreamb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_CAPCOM
&f1dream_driver,	/* (c) 1988 + Romstar */                                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sqbert_driver,		/* (c) 1983 Mylstar - never released */                                                                              
#endif
#ifndef NO_DRIVER_SYSTEM16
&fantzone_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&fantasy_driver,	/* (c) 1981 Rock-ola */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&fantazia_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&farwest_driver,                                                                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&fastfred_driver,	/* (c) 1982 Atari */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&fatfury1_driver,   /* 1991, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&fatfury2_driver,   /* 1992, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&fatfury3_driver,   /* 1993, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&fatfursp_driver,   /* 1993, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&fax_driver,		/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_DEC0
&ffantasy_driver,	/* (c) 1989 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_OTHER
&fitegolf_driver,	/* (c) 1988 */
#endif
#ifndef NO_DRIVER_CPS1
&ffightj_driver,	/* (c) [1989] (CPS1) */                                                                                              
#endif
#ifndef NO_DRIVER_CPS1
&ffightu_driver,
#endif
#ifndef NO_DRIVER_CPS1
&ffight_driver,		/* (c) [1989] (CPS1) */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&firetpbl_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&firetrap_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&fpoint_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&fpointbl_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&flicky_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&flicky2_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&flyboy_driver,		/* (c) 1982 Kaneko (bootleg?) */                                                                                     
#endif
#ifndef NO_DRIVER_OTHER
&foodf_driver,		/* (c) 1982 Atari */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&fbfrenzy_driver,   /* 1992, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&formatz_driver,	/* (c) 1984 Jaleco */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&frenzy1_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&frenzy_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&friskyt_driver,	/* (c) 1981 Nichibutsu */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&froggers_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&frogger2_driver,	/* 800-3110 (c) 1981 Sega */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&frogger_driver,	/* 834-0068 (c) 1981 Sega */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&frogsega_driver,	/* (c) 1981 Sega */                                                                                                  
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&frogs_driver,		/* 112-119 [1978 Gremlin?] */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&frontlin_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&funkybee_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&futspy_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEMESIS
&gwarrior_driver,	/* GX578 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galagamw_driver,	/* (c) 1981 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galaga_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galagab2_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galagads_driver,	/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galmidw_driver,	/* (c) Midway */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galaga3_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galaga3a_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galaxian_driver,	/* (c) Namco */                                                                                                      
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galnamco_driver,	/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galap4_driver,		/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galapx_driver,		/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galturbo_driver,	/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_NEOGEO
&galaxyfg_driver,   /* 1995, Sunsoft */                                                                                                      
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&grescue_driver,	/* bootleg? */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galxwars_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_NICHIBUT
&galivan_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_NICHIBUT
&galivan2_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gallag_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&galpanic_driver,	/* (c) 1990 Kaneko */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gaplus_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gaplusa_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gardia_driver,		/* 834-6119 */                                                                                                       
#endif
#ifndef NO_DRIVER_DEC0
&gatedoom_driver,	/* (c) 1990 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_DEC0
&gatedoma_driver,	/* (c) 1990 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_ATARI
&gaunt2p_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&gauntir1_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&gauntir2_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&gaunt2_driver,		/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&gauntlet_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_TEHKAN
&gemini_driver,		/* (c) 1987 Tecmo */                                                                                                 
#endif
#ifndef NO_DRIVER_OTHER
&prehislj_driver,	/* (c) 1989 */
#endif
#ifndef NO_DRIVER_TAITO2
&getstar_driver,	/* (c) 1986 Taito, but bootleg */                                                                                    
#endif
#ifndef NO_DRIVER_NEOGEO
&gpilots_driver,    /* 1991, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_CAPCOM
&gngcross_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&gngt_driver,		/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&gng_driver,		/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_CPS1
&ghouls_driver,		/* (c) 1988 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&japirem_driver,	/* (c) Irem */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&gladiatr_driver,	/* (c) 1986 Taito America */                                                                                         
#endif
#ifndef NO_DRIVER_NEOGEO
&goalx3_driver,     /* 1995, Visco */                                                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&goindol_driver,	/* (c) 1987 Sun a Electronics */
#endif
#ifndef NO_DRIVER_SYSTEM16
&goldnaxe_driver,	/* (c) 1989 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&goldnaxa_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&goldnaxb_driver,	/* (c) 1989 (protected) */
#endif
#ifndef NO_DRIVER_SYSTEM16
&goldnaxc_driver,	/* (c) 1989 (protected) */
#endif
#ifndef NO_DRIVER_SYSTEM16
&goldnabl_driver,	/* (c) 1989 (bootleg) */
#endif
#ifndef NO_DRIVER_TAITO2
&gcastle_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&goldstbl_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_OTHER
&goldstar_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gondo_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gorfpgm1_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gorf_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEMESIS
&gradius_driver,	/* GX456 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_ATARIVG
&gravitar_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&gsword_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gberet_driver,		/* GX577 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_TEHKAN
&gridiron_driver,	/* (c) 1985 Tehkan */                                                                                                
#endif
#ifndef NO_DRIVER_TAITO2
&growl_driver,		/* (c) 1990 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&gwar_driver,		/* (c) 1987 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gmissile_driver,	/* 623 [1977] */                                                                                                     
#endif
#ifndef NO_DRIVER_OTHER
&gundealr_driver,	/* (c) 1990 Dooyong */                                                                                               
#endif
#ifndef NO_DRIVER_OTHER
&gundeala_driver,	/* (c) Dooyong */                                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gunfight_driver,	/* 597 [1975] */                                                                                                     
#endif
#ifndef NO_DRIVER_CAPCOM
&gunsmokj_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&gunsmrom_driver,	/* (c) 1985 + Romstar */                                                                                             
#endif
#ifndef NO_DRIVER_CAPCOM
&gunsmoka_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&gunsmoke_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&gururin_driver,    /* 1994, Face */                                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&guzzler_driver,	/* (c) 1983 Tehkan */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gypsyjug_driver,	/* [1978?] */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gyrussce_driver,	/* GX347 (c) 1983 + Centuri license */                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&gyruss_driver,		/* GX347 (c) 1983 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&hangly_driver,		/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_SYSTEM16
&hangon_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&hardhat_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&headon_driver,		/* 163-167/192-193 (c) Gremlin */                                                                                    
#endif
#ifndef NO_DRIVER_DEC0
&hbarrelj_driver,	/* (c) 1987 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_DEC0
&hbarrel_driver,	/* (c) 1987 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&hvymetal_driver,	/* 834-5745 (c) 1985 */                                                                                              
#endif
#ifndef NO_DRIVER_SYSTEM16
&hwchamp_driver,                                                                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&heiankyo_driver,	/* (c) [1979?] Denki Onkyo */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&helifira_driver,	/* (c) Nintendo */                                                                                                   
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&helifire_driver,	/* (c) Nintendo */                                                                                                   
#endif
#ifndef NO_DRIVER_OTHER
&hellfire_driver,	/* (c) 1989 Toaplan + Taito license */
#endif
#ifndef NO_DRIVER_OTHER
&hexa_driver,		/* D. R. Korea */                                                                                                    
#endif
#ifndef NO_DRIVER_CAPCOM
&higemaru_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_TMS
&hiimpact_driver,	/* (c) 1990 Williams */                                                                                              
#endif
#ifndef NO_DRIVER_DEC0
&hippodrm_driver,	/* (c) 1989 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_OTHER
&homo_driver,		/* bootleg */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&hustle_driver,		/* 16-21 [1977 Gremlin] */                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&hyprolyb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&hyprolym_driver,	/* GX361 (c) 1983 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&hyperspt_driver,	/* GX330 (c) 1984 + Centuri */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&imsorryj_driver,	/* 834-5707 (c) 1985 Coreland / Sega */                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&imsorry_driver,	/* 834-5707 (c) 1985 Coreland / Sega */                                                                              
#endif
#ifndef NO_DRIVER_OTHER
&irobot_driver,                                                                                                                              
#endif
#ifndef NO_DRIVER_OTHER
&ikarijp_driver,	/* (c) 1986 */
#endif
#ifndef NO_DRIVER_OTHER
&ikarijpb_driver,	/* (c) 1986 */
#endif
#ifndef NO_DRIVER_OTHER
&ikari_driver,		/* (c) 1986 */
#endif
#ifndef NO_DRIVER_ATARISY1
&indytemp_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&intrepid_driver,	/* (c) 1983 Nova Games Ltd. */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&intruder_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&invrvnga_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&invrvnge_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&invds_driver,		/* 367-382 (c) 1979 Sega */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&invho2_driver,		/* 271-286 (c) 1979 Sega */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&invinco_driver,	/* 310-318 (c) 1979 Sega */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ironhors_driver,	/* GX560 (c) 1986 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jackrabt_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jackrab2_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jackrabs_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jack_driver,		/* (c) 1982 Cinematronics */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jacka_driver,		/* (c) 1982 Cinematronics */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jackal_driver,		/* GX631 (c) 1986 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jailbrek_driver,	/* GX507 (c) 1986 */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&janshin_driver,    /* 1994, Aicom */                                                                                                        
#endif
#ifndef NO_DRIVER_DEC0
&joemac_driver,		/* (c) 1991 */
#endif
#ifndef NO_DRIVER_MCR
&journey_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&joustr_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&joust_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&joustwr_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jrpacman_driver,	/* (c) 1983 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jbugsega_driver,	/* (c) 1981 Sega */                                                                                                  
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jumpbug_driver,	/* (c) 1981 Rock-ola */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jumpcoas_driver,	/* (c) 1983 Kaneko */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jumpshot_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jjack_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&jumping_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jhunt_driver,		/* (c) 1982 Taito America */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&junglek_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&jungler_driver,	/* GX327 (c) 1981 Konami */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&junglers_driver,	/* GX327 (c) 1981 Stern */                                                                                           
#endif
#ifndef NO_DRIVER_OTHER
&junofrst_driver,	/* GX310 (c) 1983 Konami */                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&kabukikl_driver,   /* 1995, Hudson */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kamikaze_driver,	/* Leijac Corporation */                                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kangarob_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kangaroo_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kaos_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kchampvs_driver,	/* (c) 1984 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kchamp_driver,		/* (c) 1984 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_DEC0
&karnovj_driver,	/* (c) 1987 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_DEC0
&karnov_driver,		/* (c) 1987 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_NEOGEO
&karnov_r_driver,   /* 1994, Data East Corporation */                                                                                        
#endif
#ifndef NO_DRIVER_MCR
&kick_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_MCR
&kicka_driver,		/* bootleg? */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kickridr_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kicker_driver,		/* GX477 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kidniki_driver,	/* (c) 1986 + Data East USA license */                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&killcom_driver,	/* (c) 1980 Centuri */                                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kingball_driver,	/* no copyright notice (the char set contains Namco) */                                                              
#endif
#ifndef NO_DRIVER_CPS1
&kod_driver,		/* (c) 1991 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&kodb_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_CPS1
&kodj_driver,
#endif
#ifndef NO_DRIVER_OTHER
&kingofb_driver,	/* (c) 1985 Woodplace Inc. */                                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&kingofbj_driver,	/* (c) 1985 Woodplace Inc. */                                                                                        
#endif
#ifndef NO_DRIVER_NEOGEO
&kotm2_driver,      /* 1992, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&kingofm_driver,    /* 1991, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&kizuna_driver,     /* 1996, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_ATARI
&klax_driver,		/* (c) 1989 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&klax2_driver,		/* (c) 1989 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&klax3_driver,		/* (c) 1989 */                                                                                                       
#endif
#ifndef NO_DRIVER_CPS1
&knights_driver,	/* (c) 1991 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&knightsj_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kncljoe_driver,
#endif
#ifndef NO_DRIVER_NEMESIS
&konamigt_driver,	/* GX561 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_MCR
&kroozr_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kram_driver,		/* (c) 1982 Taito America */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kram2_driver,		/* (c) 1982 Taito America */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&krull_driver,		/* GV-105 (c) 1983 Gottlieb */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kungfud_driver,	/* (c) 1984 + Data East license */                                                                                   
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kungfub_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kungfub2_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&kungfum_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&ktiger_driver,		/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ladybugb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ladybug_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&lagunar_driver,	/* 622 [1977] */                                                                                                     
#endif
#ifndef NO_DRIVER_NEOGEO
&lastblad_driver,   /* 1997, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_CAPCOM
&lstduelb_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&lastduel_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&lastmss2_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&lastmiss_driver,
#endif
#ifndef NO_DRIVER_NEOGEO
&lresort_driver,    /* 1992, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&lazercmd_driver,	/* [1976?] */                                                                                                        
#endif
#ifndef NO_DRIVER_NEOGEO
&lbowling_driver,   /* 1990, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&lomakaj_driver,	/* (c) 1988 */
#endif
#ifndef NO_DRIVER_NEOGEO
&legendos_driver,   /* 1991, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_CAPCOM
&lwingsjp_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&lwings_driver,		/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&lwings2_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&leprechn_driver,	/* (c) 1982 Tong Electronic */                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&liberatr_driver,	/* (c) 1982 Atari */                                                                                                 
#endif
#ifndef NO_DRIVER_NEMESIS
&lifefrcj_driver,	/* GX587 (c) 1986 */
#endif
#ifndef NO_DRIVER_NEMESIS
&lifefrce_driver,	/* GX587 (c) 1986 */
#endif
#ifndef NO_DRIVER_TAITO2
&liquidk_driver,	/* (c) 1990 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&lizwiz_driver,		/* (c) 1985 Techstar + "Sunn presents" */                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&lnc_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&locomotn_driver,	/* GX359 (c) 1982 Konami + Centuri license */                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ldrun_driver,		/* (c) 1984 licensed from Broderbund */                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ldruna_driver,		/* (c) 1984 licensed from Broderbund */                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ldrun2p_driver,	/* (c) 1986 licensed from Broderbund */                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&losttomb_driver,	/* (c) 1982 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&losttmbh_driver,	/* (c) 1982 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&lottofun_driver,	/* (c) 1987 H.A.R. Management */                                                                                     
#endif
#ifndef NO_DRIVER_ATARIVG
&llander1_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_ATARIVG
&llander_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&lrescue_driver,	/* (c) 1979 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&lupin3_driver,		/* (c) 1980 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&m4_driver,			/* 626 [1977] */                                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mach3_driver,		/* GV-109 (c) 1983 Mylstar */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&m79amb_driver,                                                                                                                              
#endif
#ifndef NO_DRIVER_CAPCOM
&madgear_driver,	/* (c) 1989 */	/* aka Led Storm */                                                                                  
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mplanets_driver,	/* GV-102 (c) 1983 Gottlieb */                                                                                       
#endif
#ifndef NO_DRIVER_CPS1
&mswordj_driver,	/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&mswordu_driver,	/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&msword_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_NEOGEO
&magdrop2_driver,   /* 1996, Data East Corporation */                                                                                        
#endif
#ifndef NO_DRIVER_NEOGEO
&magdrop3_driver,   /* 1997, Data East Corporation */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&magspot2_driver,	/* (c) [1979] */                                                                                                     
#endif
#ifndef NO_DRIVER_NEOGEO
&maglord_driver,    /* 1990, Alpha Denshi Co */                                                                                              
#endif
#ifndef NO_DRIVER_NEOGEO
&mahretsu_driver,   /* 1990, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_ATARIVG
&mhavocrv_driver,	/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_ATARIVG
&mhavoc2_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&mhavoc_driver,		/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&mjleague_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&gngjap_driver,		/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&maketrax_driver,	/* (c) 1981 Williams */                                                                                              
#endif
#ifndef NO_DRIVER_TECHNOS
&maniach_driver,	/* TA-???? (c) 1986 + Taito America license */                                                                       
#endif
#ifndef NO_DRIVER_TECHNOS
&maniach2_driver,	/* TA-???? (c) 1986 + Taito America license */                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mappyjp_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mappy_driver,		/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARISY1
&marble_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARISY1
&marble2_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARISY1
&marblea_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&marineb_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mario_driver,		/* (c) 1983 Nintendo of America */                                                                                   
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&masao_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_TECHNOS
&matmania_driver,	/* TA-0015 (c) 1985 + Taito America license */                                                                       
#endif
#ifndef NO_DRIVER_MCR
&maxrpm_driver,		/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mazeh_driver,		/* (c) 1987 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&medlanes_driver,	/* [1977?] */                                                                                                        
#endif
#ifndef NO_DRIVER_TEHKAN
&megaforc_driver,	/* (c) 1985 Tehkan + Video Ware license */                                                                           
#endif
#ifndef NO_DRIVER_CPS1
&megaman_driver,	/* (c) 1995 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&chikij_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&mtwins_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_OTHER
&megazone_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&megatack_driver,	/* (c) 1980 Centuri */                                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mekyosen_driver,
#endif
#ifndef NO_DRIVER_CPS1
&mercsu_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&mercs_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_NEOGEO
&mslug2_driver,     /* 1998, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&mslug_driver,      /* 1996, Nazca */                                                                                                        
#endif
#ifndef NO_DRIVER_DEC0
&midresj_driver,	/* (c) 1989 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_DEC0
&midres_driver,		/* (c) 1989 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mikiehs_driver,	/* GX469 (c) 1984 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mikie_driver,		/* GX469 (c) 1984 */                                                                                                 
#endif
#ifndef NO_DRIVER_CENTIPEDE
&milliped_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&minefld_driver,	/* (c) 1983 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mspacatk_driver,	/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&missile_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&missile2_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mrviking_driver,	/* 834-5383 (c) 1984 */                                                                                              
#endif
#ifndef NO_DRIVER_TMS
&mk2_driver,		/* (c) 1993 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&monymony_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&miexchng_driver,   /* 1997, Face */                                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&monkeyd_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&monsterb_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pignewta_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pignewt_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&moonal2b_driver,	/* Nichibutsu */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&moonal2_driver,	/* Nichibutsu */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mooncrsg_driver,	/* (c) 1980 Gremlin */                                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mooncrst_driver,	/* (c) 1980 Nichibutsu */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mooncrgx_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mooncrsb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mpatrolw_driver,	/* (c) 1982 + Williams license */                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mpatrol_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&moonqsr_driver,	/* (c) 1980 Nichibutsu */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mranger_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_SYSTEM16
&moonwalk_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&moonwlkb_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&moonwar2_driver,	/* (c) 1981 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&monwar2a_driver,	/* (c) 1981 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_TMS
&mk_driver,			/* (c) 1992 Midway */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&motorace_driver,	/* (c) 1983 Williams license */                                                                                      
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&motos_driver,		/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mtrap_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dounicorn_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mrdot_driver,		/* (c) 1982 + Taito license */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mrdo_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mrdoy_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mrdofix_driver,	/* (c) 1982 + Taito license */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dorunrun_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&docastle_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&docastl2_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&dowild_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mrdu_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mrlo_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mrtnt_driver,		/* (c) 1983 Telko */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mspacman_driver,	/* (c) 1981 Midway (but it's a bootleg) */                                                                           
#endif
#ifndef NO_DRIVER_CPS1
&mbomberj_driver,
#endif
#ifndef NO_DRIVER_CPS1
&mbomber_driver,
#endif
#ifndef NO_DRIVER_NEOGEO
&mutnat_driver,     /* 1992, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&myhero_driver,		/* 834-5755 (c) 1985 */                                                                                              
#endif
#ifndef NO_DRIVER_TECHNOS
&mystston_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&nam_1975_driver,   /* 1990, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_TMS
&nbajam_driver,		/* (c) 1993 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_TMS
&narc_driver,		/* (c) 1988 Williams */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&naughtyb_driver,	/* (c) 1982 Jaleco + Cinematronics */                                                                                
#endif
#ifndef NO_DRIVER_TECHNOS
&kuniokub_driver,	/* TA-0018 bootleg */                                                                                                
#endif
#ifndef NO_DRIVER_NEMESIS
&nemesuk_driver,	/* GX456 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_NEMESIS
&nemesis_driver,	/* GX456 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_CPS1
&nemoj_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&nemo_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_NEOGEO
&neobombe_driver,   /* 1997, Hudson */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&neodrift_driver,   /* 1996, Visco */                                                                                                        
#endif
#ifndef NO_DRIVER_NEOGEO
&neomrdo_driver,    /* 1996, Visco */                                                                                                        
#endif
#ifndef NO_DRIVER_NEOGEO
&turfmast_driver,   /* 1996, Nazca */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&nrallyx_driver,	/* (c) 1981 Namco */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&nibbler_driver,	/* (c) 1982 Rock-ola */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&nibblera_driver,	/* (c) 1982 Rock-ola */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&nitedrvr_driver,	/* no copyright notice [1976] */                                                                                     
#endif
#ifndef NO_DRIVER_NEOGEO
&ncombat_driver,    /* 1990, Alpha Denshi Co */                                                                                              
#endif
#ifndef NO_DRIVER_NEOGEO
&ncommand_driver,   /* 1992, Alpha Denshi Co */                                                                                              
#endif
#ifndef NO_DRIVER_TEHKAN
&gaiden_driver,		/* 6215 - (c) 1988 Tecmo */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ninjak2a_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ninjakd2_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&ninjamas_driver,   /* 1996, ADK/SNK */                                                                                                      
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&nprinceb_driver,	/* 834-5677 (c) 1985 */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&nprinces_driver,	/* 834-5677 (c) 1985 */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&nomnlnd_driver,	/* (c) [1979] */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&nomnlndg_driver,	/* (c) [1979] */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&nova2001_driver,	/* (c) [1984?] + Universal license */                                                                                
#endif
#ifndef NO_DRIVER_TAITO2
&ogonsiro_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&omegrace_driver,	/* (c) 1981 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_NEOGEO
&ragnagrd_driver,   /* 1996, Saurus */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&outrun_driver,		/* (c) 1986 (bootleg)*/
#endif
#ifndef NO_DRIVER_SYSTEM16
&outruna_driver,	/* (c) 1986 (bootleg) */
#endif
#ifndef NO_DRIVER_SYSTEM16
&outrunb_driver,	/* (c) 1986 (protected beta bootleg) */
#endif
#ifndef NO_DRIVER_OTHER
&outzone_driver,
#endif
#ifndef NO_DRIVER_NEOGEO
&overtop_driver,    /* 1996, ADK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ozmawars_driver,	/* Shin Nihon Kikaku (SNK) */                                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&powj_driver,		/* (c) 1988 SNK */                                                                                                   
#endif
#ifndef NO_DRIVER_OTHER
&pow_driver,		/* (c) 1988 SNK */                                                                                                   
#endif
#ifndef NO_DRIVER_OTHER
&p47j_driver,		/* (c) 1988 */
#endif
#ifndef NO_DRIVER_OTHER
&p47_driver,		/* (c) 1988 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pacnpal_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pacman_driver,		/* (c) 1980 Midway */                                                                                                
#endif
//#ifndef NO_DRIVER_NEOGEO
#ifndef NO_DRIVER_GENERIC_8BIT
&namcopac_driver,	/* (c) 1980 Namco */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pacmanjp_driver,	/* (c) 1980 Namco */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pacmanbl_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pacmod_driver,		/* (c) 1981 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pacplus_driver,                                                                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&paclandm_driver,	/* (c) 1984 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pacland_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pacland2_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pacland3_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&pandoras_driver,
#endif
#ifndef NO_DRIVER_OTHER
&pangb_driver,		/* bootleg */
#endif
#ifndef NO_DRIVER_OTHER
&pang_driver,		/* (c) 1990 Mitchell */                                                                                              
#endif
#ifndef NO_DRIVER_CPS1
&pang3_driver,
#endif
#ifndef NO_DRIVER_NEOGEO
&panicbom_driver,   /* 1995, Hudson */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&paperboy_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&passsht_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&passshtb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pengo_driver,		/* 834-0386 (c) 1982 Sega */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pengo2u_driver,	/* 834-0386 (c) 1982 Sega */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pengo2_driver,		/* 834-0386 (c) 1982 Sega */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pkunwarj_driver,	/* [1985?] */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pkunwar_driver,	/* [1985?] */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&penta_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pepper2_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARISY1
&peterpak_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&phantom2_driver,	/* 652 [1979] */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&phoenix_driver,	/* (c) 1980 Amstar */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&phoenixc_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&phoenix3_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&phoenixt_driver,	/* (c) 1980 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pickin_driver,		/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_TEHKAN
&pbaction_driver,	/* (c) 1985 Tehkan */                                                                                                
#endif
#ifndef NO_DRIVER_TEHKAN
&pbactio2_driver,	/* (c) 1985 Tehkan */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pingpong_driver,	/* GX555 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pballoon_driver,	/* (c) 1982 SNK */                                                                                                   
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&piranha_driver,	/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pisces_driver,		/* ? */                                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pitfallu_driver,	/* 834-5627 [1985?] reprogrammed, (c) 1984 Activision */                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pitfall2_driver,	/* 834-5627 [1985?] reprogrammed, (c) 1984 Activision */                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pleiadce_driver,	/* (c) 1981 Centuri + Tehkan */                                                                                      
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pleiads_driver,	/* (c) 1981 Tehkan */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&pnickj_driver,		/* (c) 1994 + Compile license (CPS2?) not listed on Capcom's site? */                                                
#endif
#ifndef NO_DRIVER_OTHER
&pcktgal_driver,
#endif
#ifndef NO_DRIVER_OTHER
&pcktgalb_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&polaris_driver,	/* (c) 1980 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&polarisa_driver,	/* (c) 1980 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ponpoko_driver,	/* (c) 1982 Sigma Ent. Inc. */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pool_driver,                                                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pootan_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pooyan_driver,		/* GX320 (c) 1982 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pooyans_driver,	/* GX320 (c) 1982 Stern */                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&popflame_driver,	/* (c) 1982 Jaleco */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&popeyebl_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&popeye_driver,                                                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&portman_driver,	/* (c) 1982 Nova Games Ltd. */                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&potogold_driver,	/* (c) 1982 Tong Electronic */                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&pspikes_driver,	/* (c) 1991 */
#endif
#ifndef NO_DRIVER_NEOGEO
&pspikes2_driver,   /* 1994, Visco */                                                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&prehislu_driver,	/* (c) 1989 */
#endif
#ifndef NO_DRIVER_OTHER
&prehisle_driver,	/* (c) 1989 */
#endif
#ifndef NO_DRIVER_OTHER
&psychic5_driver,	/* (c) 1987 Jaleco */                                                                                                
#endif
#ifndef NO_DRIVER_OTHER
&psychos_driver,	/* (c) 1987 */
#endif
#ifndef NO_DRIVER_OTHER
&psychosa_driver,	/* (c) 1987 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&oscarj_driver,		/* (c) 1988 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&oscar_driver,		/* (c) 1988 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&puckman_driver,	/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&pulsar_driver,		/* 790-805 (c) 1981 Sega */                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&pulstar_driver,    /* 1995, Aicom */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&punchout_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_CPS1
&punishrj_driver,
#endif
#ifndef NO_DRIVER_CPS1
&punisher_driver,
#endif
#ifndef NO_DRIVER_TMNT
&punkshot_driver,	/* GX907 (c) 1990 */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&pbobble_driver,    /* 1994, Taito */                                                                                                        
#endif
#ifndef NO_DRIVER_NEOGEO
&puzzledp_driver,   /* 1995, Taito (Visco license) */                                                                                        
#endif
#ifndef NO_DRIVER_NEOGEO
&joyjoy_driver,     /* 1990, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&pyros_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&qbertjp_driver,	/* GV-103 (c) 1982 Gottlieb + Konami license */                                                                      
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&qbert_driver,		/* GV-103 (c) 1982 Gottlieb */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&qbertqub_driver,	/* GV-119 (c) 1983 Mylstar */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&qix2_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&qix_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&quantum1_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&quantum_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&quartet_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&quartet2_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_CPS1
&qadj_driver,
#endif
#ifndef NO_DRIVER_CPS1
&qad_driver,
#endif
#ifndef NO_DRIVER_CENTIPEDE
&qwakprot_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&radarscp_driver,	/* (c) 1980 Nintendo */                                                                                              
#endif
#ifndef NO_DRIVER_TAITO2
&rainbowe_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&rainbow_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&rallyx_driver,		/* (c) 1980 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_MCR
&rampage_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&rastsaga_driver,	/* (c) 1987 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_TAITO2
&rastan_driver,		/* (c) 1987 Taito Japan */                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&reactor_driver,	/* GV-100 (c) 1982 Gottlieb */                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&realbou2_driver,   /* 1998, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&rbffspec_driver,   /* 1996, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&rbff1_driver,      /* 1995, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&redalert_driver,	/* (c) 1981 Irem (GDI game) */                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&redbaron_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&regulusu_driver,	/* 834-5328 (c) 1983 */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&regulus_driver,	/* 834-5328 (c) 1983 */                                                                                              
#endif
#ifndef NO_DRIVER_TECHNOS
&renegade_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&rescue_driver,		/* (c) 1982 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_OTHER
&jedi_driver,		/* (c) 1984 Atari */                                                                                                 
#endif
#ifndef NO_DRIVER_NEMESIS
&rf2_driver,		/* GX561 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&ridhero_driver,    /* 1990, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&ringking_driver,	/* (c) 1985 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_OTHER
&ringkin2_driver,	/* (c) 1985 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_SYSTEM16
&riotcity_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ripcord_driver,	/* Exidy [1977?] */                                                                                                  
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&rpatrolb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_ATARISY1
&roadblst_driver,	/* (c) 1986, 1987 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&roadf_driver,		/* GX461 (c) 1984 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&roadf2_driver,		/* GX461 (c) 1984 */                                                                                                 
#endif
#ifndef NO_DRIVER_ATARISY1
&roadrunn_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&robby_driver,		/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&ararmy_driver,     /* 1991, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_DEC0
&robocopp_driver,	/* (c) 1988 Data East Corp. */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&robotbwl_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&robotron_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&robotryo_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&rocnrope_driver,	/* GX364 (c) 1983 + Kosuka */                                                                                        
#endif
#ifndef NO_DRIVER_CPS1
&rockmanj_driver,	/* (c) 1995 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_OTHER
&rodlandj_driver,	/* (c) 1990 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&rollingc_driver,	/* Nichibutsu */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&rthunder_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ropeman_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&roundup_driver,	/* (c) 1981 Amenip/Centuri */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&route16_driver,	/* (c) 1981 Leisure and Allied (bootleg) */                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&rushatck_driver,	/* GX577 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_TEHKAN
&rygarj_driver,		/* 6002 - (c) 1986 Tecmo */                                                                                          
#endif
#ifndef NO_DRIVER_TEHKAN
&rygar_driver,		/* 6002 - (c) 1986 Tecmo */                                                                                          
#endif
#ifndef NO_DRIVER_TEHKAN
&rygar2_driver,		/* 6002 - (c) 1986 Tecmo */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&safari_driver,		/* 57-66 [1977 Gremlin?] */                                                                                          
#endif
#ifndef NO_DRIVER_NEMESIS
&salamand_driver,	/* GX587 (c) 1986 */
#endif
#ifndef NO_DRIVER_NEOGEO
&samsho_driver,     /* 1992, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
#ifndef NO_DRIVER_NEOGEO
&samsho2_driver,    /* 1994, SNK */                                                                                                          
#endif
#endif
#ifndef NO_DRIVER_NEOGEO
#ifndef NO_DRIVER_NEOGEO
&samsho3_driver,    /* 1995, SNK */                                                                                                          
#endif
#endif
#ifndef NO_DRIVER_NEOGEO
#ifndef NO_DRIVER_NEOGEO
&samsho4_driver,    /* 1996, SNK */                                                                                                          
#endif
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&samurai_driver,	/* 289-302 + upgrades (c) 1980 Sega */                                                                               
#endif
#ifndef NO_DRIVER_MCR
&sarge_driver,		/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sasuke_driver,		/* [1980] Shin Nihon Kikaku (SNK) */                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&satansat_driver,	/* (c) 1981 SNK */                                                                                                   
#endif
#ifndef NO_DRIVER_MCR
&shollow_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&sauro_driver,		/* (c) 1987 Tecfri */                                                                                                
#endif
#ifndef NO_DRIVER_CAPCOM
&savgbees_driver,	/* (c) 1985 + Memetron license */                                                                                    
#endif
#ifndef NO_DRIVER_NEOGEO
&savagere_driver,   /* 1995, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_SEIBU
&scionc_driver,		/* (c) 1984 Seibu Denshi + Cinematronics license */                                                                  
#endif
#ifndef NO_DRIVER_SEIBU
&scion_driver,		/* (c) 1984 Seibu Denshi */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&scramble_driver,	/* GX387 (c) 1981 Stern */                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&scregg_driver,		/* TA-0001 (c) 1983 Technos Japan */                                                                                 
#endif
#ifndef NO_DRIVER_SYSTEM16
&sdi_driver,		/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sfposeid_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&seawolf2_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&seawolf_driver,	/* 596 [1976] */                                                                                                     
#endif
#ifndef NO_DRIVER_CAPCOM
&sectionz_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&seganinu_driver,	/* 834-5677 (c) 1985 */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&seganinj_driver,	/* 834-5677 (c) 1985 */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sspaceat_driver,	/* 139-146 (c) */                                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&seicross_driver,	/* (c) 1984 Nichibutsu + Alice */                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&myheroj_driver,	/* 834-5755 (c) 1985 Coreland / Sega */                                                                              
#endif
#ifndef NO_DRIVER_NEOGEO
#ifndef NO_DRIVER_NEOGEO
&sengoku2_driver,   /* 1993, Alpha Denshi Co */                                                                                              
#endif
#endif
#ifndef NO_DRIVER_NEOGEO
&sengoku_driver,    /* 1991, Alpha Denshi Co */                                                                                              
#endif
#ifndef NO_DRIVER_CPS1
&mercsj_driver,		/* (c) 1990 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CAPCOM
&commandj_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&shackled_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&shdancer_driver,	/* (c) 1989 */
#endif
#ifndef NO_DRIVER_SYSTEM16
&shdancbl_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&shdancrj_driver,	/* (c) 1989 */
#endif
#ifndef NO_DRIVER_TEHKAN
&shadoww_driver,	/* 6215 - (c) 1988 Tecmo */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&shaolins_driver,	/* GX477 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_OTHER
&sharkatt_driver,	/* (c) Pacific Novelty */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mikiej_driver,		/* GX469 (c) 1984 */                                                                                                 
#endif
#ifndef NO_DRIVER_SYSTEM16
&shinobi_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&shinobia_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&shinobl_driver,
#endif
#ifndef NO_DRIVER_OTHER
&shisen_driver,		/* (c) 1989 Tamtex */
#endif
#ifndef NO_DRIVER_NEOGEO
&shocktro_driver,   /* 1997, Saurus */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&shootout_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&sichuan2_driver,	/* (c) 1989 Tamtex */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sidepckt_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sidetrac_driver,	/* (c) 1979 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&sidearjp_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&sidearmr_driver,	/* (c) 1986 + Romstar license */                                                                                     
#endif
#ifndef NO_DRIVER_CAPCOM
&sidearms_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_TEHKAN
&silkworm_driver,	/* 6217 - (c) 1988 Tecmo */                                                                                          
#endif
#ifndef NO_DRIVER_TEHKAN
&silkwrm2_driver,	/* 6217 - (c) 1988 Tecmo */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&silvland_driver,	/* Falcon */                                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sinista1_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sinista2_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sinistar_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&skylancr_driver,
#endif
#ifndef NO_DRIVER_CPS1
&slammast_driver,
#endif
#ifndef NO_DRIVER_TAITO2
&slapbtuk_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_TAITO2
&slapbtjp_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_TAITO2
&slapfigh_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_DEC0
&slyspy_driver,		/* (c) 1989 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_DEC0
&slyspy2_driver,		/* (c) 1989 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_TMS
&smashtv5_driver,	/* (c) 1990 Williams */                                                                                              
#endif
#ifndef NO_DRIVER_TMS
&smashtv_driver,	/* (c) 1990 Williams */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&snapjack_driver,	/* (c) */                                                                                                            
#endif
#ifndef NO_DRIVER_OTHER
&snowbros_driver,	/* (c) 1990 Toaplan + Romstar license */                                                                             
#endif
#ifndef NO_DRIVER_OTHER
&snowbro2_driver,	/* (c) 1990 Toaplan + Romstar license */                                                                             
#endif
#ifndef NO_DRIVER_NEOGEO
&socbrawl_driver,   /* 1992, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_MCR
&solarfox_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_TECHNOS
&solarwar_driver,	/* TA-0019 (c) 1986 + Memetron license */                                                                            
#endif
#ifndef NO_DRIVER_TEHKAN
&solomon_driver,	/* (c) 1986 Tecmo */                                                                                                 
#endif
#ifndef NO_DRIVER_CAPCOM
&sonson_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spaceatt_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&schaser_driver,	/* Taito */                                                                                                          
#endif
#ifndef NO_DRIVER_ATARIVG
&spacduel_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sdungeon_driver,	/* (c) 1981 Taito America */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spcenctr_driver,	/* 645 (c) 1980 Midway */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sfeverbw_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spacefev_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_OTHER
&spacefb_driver,	/* 834-0031 (c) [1980?] Nintendo */                                                                                  
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spacfura_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spacfury_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&sharrier_driver,	/* (c) 1985 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&galap1_driver,		/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&invdelux_driver,	/* 852 [1980] Midway */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&invadpt2_driver,	/* 851 [1980] Taito */                                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&invaders_driver,	/* 739 [1979] */                                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spaceod_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&panicger_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&panic_driver,		/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&panica_driver,		/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spaceph_driver,	/* Zilec Games */                                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spaceplt_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spaceskr_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&stactics_driver,	/* [1981 Sega] */                                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sptrekct_driver,	/* (c) 1980 Sega */                                                                                                  
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spacetrk_driver,	/* 630-645 (c) 1980 Sega */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spacezap_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&speakres_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spectar1_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spectar_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&speedbal_driver,	/* (c) 1987 Tecfri */                                                                                                
#endif
#ifndef NO_DRIVER_CAPCOM
&srumbler_driver,	/* (c) 1986 */	/* aka Rush'n Crash */                                                                               
#endif
#ifndef NO_DRIVER_CAPCOM
&srumblr2_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spelunk2_driver,	/* (c) 1986 licensed from Broderbund */                                                                              
#endif
#ifndef NO_DRIVER_OTHER
&spiders_driver,	/* (c) 1981 Sigma */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&spinmast_driver,   /* 1993, Data East Corporation */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&splat_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sprint1_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sprint2_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_MCR
&spyhunt_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&stakwin_driver,    /* 1995, Saurus */                                                                                                       
#endif
#ifndef NO_DRIVER_TEHKAN
&starforc_driver,	/* (c) 1984 Tehkan */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&starjack_driver,	/* 834-5191 (c) 1983 */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&starjacs_driver,	/* (c) 1983 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&startrek_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&starwars_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&starfire_driver,	/* Exidy */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&stargate_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_SEIBU
&stinger_driver,	/* (c) 1983 Seibu Denshi */                                                                                          
#endif
#ifndef NO_DRIVER_DEC0
&stoneage_driver,	/* bootleg */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&stratgyx_driver,	/* (c) 1981 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&stratvox_driver,	/* Taito */                                                                                                          
#endif
#ifndef NO_DRIVER_CAPCOM
&sf1jp_driver,		/* (c) 1987 (Japan) */
#endif
#ifndef NO_DRIVER_CAPCOM
&sf1us_driver,		/* (c) 1987 (US) */
#endif
#ifndef NO_DRIVER_CAPCOM
&sf1_driver,		/* (c) 1987 (World) */
#endif
#ifndef NO_DRIVER_CPS1
&sf2_driver,		/* (c) 1991 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&sf2j_driver,		/* (c) 1991 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&sf2a_driver,
#endif
#ifndef NO_DRIVER_CPS1
&sf2b_driver,
#endif
#ifndef NO_DRIVER_CPS1
&sf2e_driver,
#endif
#ifndef NO_DRIVER_CPS1
&sf2ce_driver,
#endif
#ifndef NO_DRIVER_CPS1
&sf2cej_driver,
#endif
#ifndef NO_DRIVER_CPS1
&sf2t_driver,
#endif
#ifndef NO_DRIVER_CPS1
&sf2tj_driver,
#endif
#ifndef NO_DRIVER_NEOGEO
&strhoops_driver,   /* 1994, Data East Corporation */                                                                                        
#endif
#ifndef NO_DRIVER_CPS1
&striderj_driver,	/* (c) 1989 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&strider_driver,	/* (c) 1989 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_TMS
&strkforc_driver,	/* (c) 1991 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&stratgyb_driver,	/* bootleg (of the Konami version?) */                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&subs_driver,		/* no copyright notice [1976] */                                                                                     
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sbagmans_driver,	/* (c) 1984 + Stern license */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sbagman_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sbasketb_driver,	/* GX405 (c) 1984 */                                                                                                 
#endif
#ifndef NO_DRIVER_TAITO2
&sboblbob_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&superbon_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sbrkout_driver,	/* no copyright notice */                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sucasino_driver,	/* (c) 1982 Data Amusement */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&scobrak_driver,	/* GX316 (c) 1981 Konami */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&scobra_driver,		/* GX316 (c) 1981 Stern */                                                                                           
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&scobrab_driver,	/* GX316 (c) 1981 Karateco (bootleg?) */                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&earthinv_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&superg_driver,		/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_SYSTEM16
&shangon_driver,	/* (c) 1992 (protected) */
#endif
#ifndef NO_DRIVER_SYSTEM16
&shangonb_driver,	/* (c) 1992 (but bootleg) */
#endif
#ifndef NO_DRIVER_TMS
&shimpact_driver,	/* (c) 1991 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sinvemag_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&suprmatk_driver,	/* (c) 1980 + (c) 1981 Gencomp */                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&suprmous_driver,	/* (c) 1982 Taito */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&superpac_driver,	/* (c) 1982 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&superpcn_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&spang_driver,		/* (c) 1990 Mitchell (World) */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spiero_driver,		/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&spnchout_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&sqixbl_driver,		/* bootleg? but (c) 1987 */                                                                                          
#endif
#ifndef NO_DRIVER_TAITO2
&superqix_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&srdarwin_driver,
#endif
#ifndef NO_DRIVER_NEOGEO
&ssideki2_driver,   /* 1994, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&sskick3_driver,    /* 1995, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&sidkicks_driver,   /* 1992, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_TAITO2
&ssi_driver,		/* (c) 1990 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ssprint_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&superspy_driver,   /* 1990, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&sxevious_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&szaxxon_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&superman_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&swat_driver,		/* 834-5388 (c) 1984 Coreland / Sega */                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&swimmer_driver,	/* (c) 1982 Tehkan */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&swimmera_driver,	/* (c) 1982 Tehkan */                                                                                                
#endif
#ifndef NO_DRIVER_TMNT
&tmht2p_driver,		/* GX963 (c) 1989 */                                                                                                 
#endif
#ifndef NO_DRIVER_TMNT
&tmnt2pj_driver,	/* GX963 (c) 1990 */                                                                                                 
#endif
#ifndef NO_DRIVER_TMNT
&tmntj_driver,		/* GX963 (c) 1989 */                                                                                                 
#endif
#ifndef NO_DRIVER_TMNT
&tmnt_driver,		/* GX963 (c) 1989 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tacscan_driver,	/* (c) */                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tagteam_driver,	/* TA-0007 (c) 1983 + Technos Japan license */                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&karatedo_driver,	/* (c) 1984 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_OTHER
&tankbatt_driver,	/* (c) 1980 Namco */                                                                                                 
#endif
#ifndef NO_DRIVER_MCR
&tapper_driver,		/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_MCR
&rbtapper_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_MCR
&sutapper_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&targ_driver,		/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tazmania_driver,	/* (c) 1982 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_TEHKAN
&tknight_driver,	/* (c) 1989 Tecmo */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&tws96_driver,      /* 1996, Tecmo */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&teddybb_driver,	/* 834-5712 (c) 1985 */                                                                                              
#endif
#ifndef NO_DRIVER_TEHKAN
&teedoff_driver,	/* 6102 - (c) 1986 Tecmo */                                                                                          
#endif
#ifndef NO_DRIVER_TEHKAN
&tehkanwc_driver,	/* (c) 1985 Tehkan */                                                                                                
#endif
#ifndef NO_DRIVER_ATARIVG
&tempest1_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&tempest2_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&tempest_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARIVG
&temptube_driver,	/* hack */                                                                                                           
#endif
#ifndef NO_DRIVER_CPS1
&dwj_driver,		/* (c) 1989 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&wofj_driver,
#endif
#ifndef NO_DRIVER_TMS
&term2_driver,		/* (c) 1992 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_NICHIBUT
&terracra_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_NICHIBUT
&terracre_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_NICHIBUT
&terraf_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_NICHIBUT
&terrafu_driver,	/* (c) 1987 Nichibutsu USA */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&atetcktl_driver,	/* (c) 1989 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&atetckt2_driver,	/* (c) 1989 */                                                                                                       
#endif
#ifndef NO_DRIVER_SYSTEM16
&tetris_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&tetrisbl_driver,	/* (c) 1988 (but bootleg) */                                                                                         
#endif
#ifndef NO_DRIVER_ATARI
&atetrisb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_ATARI
&atetris_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&atetrisa_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&billiard_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_ATARIVG
&empire_driver,		/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&theend_driver,		/* (c) 1980 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&theglob_driver,	/* (c) 1983 Epos Corporation */                                                                                      
#endif
#ifndef NO_DRIVER_NEOGEO
&kof94_driver,      /* 1994, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&kof95_driver,      /* 1995, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&kof96_driver,      /* 1996, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&kof97_driver,      /* 1997, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&legofair_driver,	/* (c) 1988 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mainevt2_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&mainevt_driver,	/* GX799 (c) 1988 */                                                                                                 
#endif
#ifndef NO_DRIVER_TAITO2
&tnzs_driver,		/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&tnzs2_driver,		/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&thepit_driver,		/* (c) 1982 Centuri */                                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ghostb_driver,		/* (c) 1987 Data East USA */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ghostb3_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tinstar_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&trally_driver,     /* 1991, Alpha Denshi Co */                                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&stooges_driver,	/* GV-113 (c) 1984 Mylstar */                                                                                        
#endif
#ifndef NO_DRIVER_CPS1
&c3wonders_driver,
#endif
#ifndef NO_DRIVER_TAITO2
&tigerhb1_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_TAITO2
&tigerhb2_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_TAITO2
&tigerh_driver,		/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&tigerh2_driver,	/* (c) 1985 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&tigeroad_driver,	/* (c) 1987 + Romstar */                                                                                             
#endif
#ifndef NO_DRIVER_MCR
&timber_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&timepltc_driver,	/* GX393 (c) 1982 + Centuri license*/                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tp84_driver,		/* GX388 (c) 1984 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tp84a_driver,		/* GX388 (c) 1984 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&timeplt_driver,	/* GX393 (c) 1982 */                                                                                                 
#endif
#ifndef NO_DRIVER_SYSTEM16
&timscanr_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&timetunl_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tiptop_driver,		/* 605-5167 (c) 1983 */                                                                                              
#endif
#ifndef NO_DRIVER_OTHER
&tnk3_driver,		/* (c) 1985 */
#endif
#ifndef NO_DRIVER_OTHER
&toki_driver,		/* (c) 1990 Datsu (bootleg) */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tokisens_driver,	/* (c) 1987 (from a bootleg board) */                                                                                
#endif
#ifndef NO_DRIVER_TAITO2
&tokiob_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_TAITO2
&tokio_driver,		/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tomahawk_driver,	/* (c) [1980?] */                                                                                                    
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tomahaw5_driver,	/* (c) [1980?] */                                                                                                    
#endif
#ifndef NO_DRIVER_ATARI
&toobin_driver,		/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&topgunbl_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&topgunr_driver,	/* GX631 (c) 1986 */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&tophuntr_driver,   /* 1994, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&tpgolf_driver,     /* 1990, SNK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tornbase_driver,	/* 605 [1976] */                                                                                                     
#endif
#ifndef NO_DRIVER_TMS
&totcarnp_driver,	/* (c) 1992 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_TMS
&totcarn_driver,	/* (c) 1992 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_OTHER
&tdfever_driver,	/* (c) 1987 */
#endif
#ifndef NO_DRIVER_OTHER
&tdfeverj_driver,	/* (c) 1987 */
#endif
#ifndef NO_DRIVER_SYSTEM16
&tturf_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&tturfbl_driver,                                                                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&todruaga_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&trackflc_driver,	/* GX361 (c) 1983 + Centuri license */                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&trackfld_driver,	/* GX361 (c) 1983 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&tranqgun_driver,	/* 413-428 (c) 1980 Sega */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&travrusa_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&treahunt_driver,                                                                                                                            
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&triplep_driver,	/* (c) 1982 KKI */                                                                                                   
#endif
#ifndef NO_DRIVER_TMS
&trogp_driver,		/* (c) 1990 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_TMS
&trog3_driver,		/* (c) 1990 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_TMS
&trog_driver,		/* (c) 1990 Midway */                                                                                                
#endif
#ifndef NO_DRIVER_CAPCOM
&trojanj_driver,	/* (c) 1986 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&trojan_driver,		/* (c) 1986 + Romstar */                                                                                             
#endif
#ifndef NO_DRIVER_MCR
&tron_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_MCR
&tron2_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&troangel_driver,
#endif
#ifndef NO_DRIVER_OTHER
&truxton_driver,
#endif
#ifndef NO_DRIVER_DEC0
&tumblep_driver,
#endif
#ifndef NO_DRIVER_DEC0
&tumblep2_driver,
#endif
#ifndef NO_DRIVER_OTHER
&turboa_driver,		/* (c) 1981 Sega */                                                                                                  
#endif
#ifndef NO_DRIVER_OTHER
&turbob_driver,		/* (c) 1981 Sega */                                                                                                  
#endif
#ifndef NO_DRIVER_OTHER
&turbo_driver,		/* (c) 1981 Sega */                                                                                                  
#endif
#ifndef NO_DRIVER_OTHER
&turbofrc_driver,	/* (c) 1991 Video System Co. */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&turpin_driver,		/* (c) 1981 Sega */                                                                                                  
#endif
#ifndef NO_DRIVER_CAPCOM
&turtship_driver,	/* (c) 1986 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&turtles_driver,	/* (c) 1981 Stern */                                                                                                 
#endif
#ifndef NO_DRIVER_OTHER
&tutankhm_driver,	/* GX350 (c) 1982 Konami */                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&tutankst_driver,	/* GX350 (c) 1982 Stern */                                                                                           
#endif
#ifndef NO_DRIVER_TAITO2
&twincobu_driver,	/* (c) 1987 Taito America + Romstar license */                                                                       
#endif
#ifndef NO_DRIVER_TAITO2
&twincobr_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEMESIS
&twinbee_driver,	/* GX412 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&twinspri_driver,   /* 1997, ADK */                                                                                                          
#endif
#ifndef NO_DRIVER_MCR
&twotiger_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_CPS1
&unsquad_driver,	/* (c) 1989 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&ufosensi_driver,	/* 834-6659 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&uniwars_driver,	/* (c) Karateco (bootleg?) */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&upndown_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&usvsthem_driver,	/* GV-??? (c) 198? Mylstar */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&vangrdce_driver,	/* (c) 1981 SNK + Centuri */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&vanguard_driver,	/* (c) 1981 SNK */                                                                                                   
#endif
#ifndef NO_DRIVER_CPS1
&varth_driver,		/* (c) 1992 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&varthj_driver,
#endif
#ifndef NO_DRIVER_OTHER
&vastar_driver,		/* (c) 1983 Sesame Japan */                                                                                          
#endif
#ifndef NO_DRIVER_OTHER
&vastar2_driver,	/* (c) 1983 Sesame Japan */                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&venture_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&venture2_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&venture4_driver,	/* (c) 1981 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&venus_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&victroad_driver,	/* (c) 1986 */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&hustler_driver,	/* GX343 (c) 1981 Konami */                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&viewpoin_driver,   /* 1992, Sammy */                                                                                                        
#endif
#ifndef NO_DRIVER_OTHER
&vigilntj_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&vigilant_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&vimana_driver,		/* (c) 1991 Toaplan (+ Tecmo license when set to Japan) */
#endif
#ifndef NO_DRIVER_OTHER
&vimana2_driver,	/* (c) 1991 Toaplan (+ Tecmo license when set to Japan)  */
#endif
#ifndef NO_DRIVER_ATARI
&vindictr_driver,	/* (c) 1988 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&gowcaizr_driver,   /* 1995, Technos */                                                                                                      
#endif
#ifndef NO_DRIVER_CAPCOM
&vulgus_driver,		/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_CAPCOM
&vulgus2_driver,	/* (c) 1984 */                                                                                                       
#endif
#ifndef NO_DRIVER_MCR
&wacko_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&wakuwak7_driver,   /* 1996, Sunsoft */                                                                                                      
#endif
#ifndef NO_DRIVER_OTHER
&wanted_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&warofbug_driver,	/* (c) 1981 Armenia */                                                                                               
#endif
#ifndef NO_DRIVER_OTHER
&wardnerj_driver,
#endif
#ifndef NO_DRIVER_OTHER
&wardner_driver,
#endif
#ifndef NO_DRIVER_CENTIPEDE
&warlord_driver,	/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&warpwarp_driver,	/* (c) 1981 Rock-ola - different hardware */                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&warpwar2_driver,	/* (c) 1981 Rock-ola - different hardware */                                                                         
#endif
#ifndef NO_DRIVER_CPS1
&wof_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&waterski_driver,	/* (c) 1983 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wexpresb_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wexpress_driver,	/* (c) 1986 Data East Corporation */                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wwestern_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_CPS1
&willow_driver,		/* (c) 1989 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_CPS1
&willowj_driver,	/* (c) 1989 (CPS1) */                                                                                                
#endif
#ifndef NO_DRIVER_NEOGEO
&wjammers_driver,   /* 1994, Data East Corporation */                                                                                        
#endif
#ifndef NO_DRIVER_SEIBU
&wiz_driver,		/* (c) 1985 Seibu Kaihatsu */                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wow_driver,		/* (c) 1980 */                                                                                                       
#endif
#ifndef NO_DRIVER_CPS1
&c3wonderj_driver,
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wboyu_driver,		/* 834-5753 (? maybe a conversion) (c) 1986 + Escape license */                                                      
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wboy_driver,		/* 834-5984 (c) 1986 + Escape license */                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wboy2_driver,		/* 834-5984 (c) 1986 + Escape license */                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wboy3_driver,                                                                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wboy4u_driver,		/* 834-5984 (c) 1986 + Escape license */                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wboy4_driver,		/* 834-5984 (c) 1986 + Escape license */                                                                             
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wbdeluxe_driver,	/* (c) 1986 + Escape license */                                                                                      
#endif
#ifndef NO_DRIVER_SYSTEM16
&wb3_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&wb3a_driver,
#endif
#ifndef NO_DRIVER_SYSTEM16
&wb3bl_driver,                                                                                                                               
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&wbml_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_TEHKAN
&wc90b_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_TEHKAN
&wc90_driver,		/* (c) 1989 Tecmo */                                                                                                 
#endif
#ifndef NO_DRIVER_NEOGEO
&worlhe2j_driver,   /* 1994, ADK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&worlher2_driver,   /* 1993, ADK */                                                                                                          
#endif
#ifndef NO_DRIVER_NEOGEO
&whp_driver,        /* 1995, ADK/SNK */                                                                                                      
#endif
#ifndef NO_DRIVER_NEOGEO
&wheroes_driver,    /* 1992, Alpha Denshi Co */                                                                                              
#endif
#ifndef NO_DRIVER_SYSTEM16
&wrestwar_driver,	/* (c) 1989 */                                                                                                       
#endif
#ifndef NO_DRIVER_TECHNOS
&xsleena_driver,	/* TA-0019 (c) 1986 */                                                                                               
#endif
#ifndef NO_DRIVER_MCR
&xenophob_driver,	/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&xevios_driver,		/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&xeviousa_driver,	/* (c) 1982 + Atari license */                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&xevious_driver,	/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_ATARI
&xybots_driver,		/* (c) 1987 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&yankeedo_driver,	/* bootleg */                                                                                                        
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&yiear_driver,		/* GX407 (c) 1985 */                                                                                                 
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&zarzon_driver,		/* (c) 1981 Taito, gameplay says SNK */                                                                              
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&zaxxon_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_NEOGEO
&zedblade_driver,   /* 1994, NMK */                                                                                                          
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&zektor_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_OTHER
&zerozone_driver,	/* (c) 1993 Comad */                                                                                                 
#endif
#ifndef NO_DRIVER_OTHER
&zerowing_driver,	/* (c) 1989 Toaplan */
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&zigzag_driver,		/* (c) 1982 LAX */                                                                                                   
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&zigzag2_driver,	/* (c) 1982 LAX */                                                                                                   
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&zoar_driver,		/* (c) 1982 */                                                                                                       
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&zookeep_driver,	/* (c) 1982 Taito America */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&zookeepa_driver,	/* (c) 1982 Taito America */                                                                                         
#endif
#ifndef NO_DRIVER_GENERIC_8BIT
&zzyzzyxx_driver,	/* (c) 1982 Cinematronics */                                                                                         
#endif
	0	/* end of array */
};
