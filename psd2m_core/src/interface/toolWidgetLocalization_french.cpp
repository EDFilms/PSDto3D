//----------------------------------------------------------------------------------------------
// ===============================================
//  Copyright (C) 2022, EDFilms.
//  All Rights Reserved.
// ===============================================
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential
//
//  @file toolWidgetLocalization_french.cpp
//  @author Michaelson Britt
//  @date 15-Mar-2022
//
//  @section DESCRIPTION
//
//----------------------------------------------------------------------------------------------


#include "toolWidgetLocalization.h"

const util::StringTable::StringTableItem stringTableItems_french[] =
{
	//----------------------------------------------------------------------------------------------
	// Format strings and popup messages

	{ IDS_LICENSING_ERROR, "Erreur" },
	{ IDS_LICENSING_ERROR_MSG, "Échec d'activation de la licence. Vérifiez vos informations d'accès\net retentez de vous connecter en suivant les étapes détaillés dans votre courriel de confirmation d'achat." },

	{ IDS_TEXTURE_ATLAS_GROUP_DEFAULT, "Sans nom" },
	{ IDS_TEXTURE_ATLAS_INFO_CURRENT_SIZE_FORMAT, "Résolution de l'Atlas %i x %i" },
	{ IDS_TEXTURE_ATLAS_INFO_OPTIMAL_SIZE_FORMAT, "Résolution optimale de l'Atlas %i x %i" },
	{ IDS_TEXTURE_ATLAS_INFO_OPTIMAL_SIZE_CALCULATING, "calcul..." },
	{ IDS_TEXTURE_ATLAS_INFO_SCALING_FORMAT, "<font color='#f48c42'>Les calques seront réduits de %i%% pour s'ajuster</font>" },
	{ IDS_TEXTURE_ATLAS_INFO_PADDING_FORMAT, "<font color='#f48c42'>Un bordure de %i pixels sera appliqué</font>" },
	{ IDS_LAYER_LIST_SELECTED_STATUS_FORMAT, "%i de %i sélectionnés" }, // %i are numbers of selected and total layers, for example "1 of 20 selected"
	{ IDS_LAYER_LIST_CALCULATING, "calcul..." },
	{ IDS_LAYER_LIST_MODE_SELECTED, "Mode sélectionné : " },
	{ IDS_LAYER_LIST_MODE_SUPPORTED, "Mode supporté : " },
	{ IDS_LAYER_LIST_LINEAR, "Linéaire" },
	{ IDS_LAYER_LIST_DELAUNAY, "Delaunay" },
	{ IDS_LAYER_LIST_VECTOR, "Vectoriel" },
	{ IDS_LAYER_LIST_BILLBOARD, "Plan" },
	{ IDS_LAYER_LIST_SIZE, "Dimensions : " },
	{ IDS_LAYER_LIST_GROUP, "Groupe : " },
	{ IDS_LAYER_LIST_GROUP_NONE, "Aucun" },
	{ IDS_LAYER_LIST_ATLAS, "Atlas : " },
	{ IDS_LAYER_LIST_ATLAS_OVERSIZE, "Surdimensionné - retirer des calques" },
	{ IDS_LAYER_LIST_ATLAS_SCALE, "échelle" },
	{ IDS_LAYER_LIST_ATLAS_PADDING, "bordure" },
	{ IDS_LAYER_LIST_INFLUENCE, "Influence : " },
	{ IDS_LAYER_LIST_INFLUENCE_NOT_AVAILABLE, "Non disponible" },
	{ IDS_LAYER_LIST_INFLUENCE_ACTIVE, "Actif" },
	{ IDS_LAYER_LIST_INFLUENCE_INACTIVE, "Inactif" },

	{ IDS_IMPORTING_PSD, "Chargement du fichier PSD..." },

	{ IDS_EXPORTING_CALCULATING_MESHES, "Calcul des meshes..." }, // TODO: Review this
	{ IDS_EXPORTING_PNG_TEXTURES, "Exportation des textures..." },
	{ IDS_EXPORTING_PNG_ATLASES, "Exportations des atlas..." },
	{ IDS_EXPORTING_JOB_FBX, "Exportation FBX..." }, /// TODO: Review this
	{ IDS_EXPORTING_JOB_MESH, "Exportation de la grille..." }, // TODO: Review this
	{ IDS_EXPORTING_MESH_MESHES, "Exportation des grilles..." },
	{ IDS_EXPORTING_MESH_SUCCCESS_1, "Exportation réussie pour " },
	{ IDS_EXPORTING_MESH_SUCCCESS_2, " grille(s), échec pour " },
	{ IDS_EXPORTING_MESH_SUCCCESS_3, " grille(s)." },
	{ IDS_EXPORTING_PNG_ERROR_DIALOG, "Message PSD to 3D" },
	{ IDS_EXPORTING_FINALIZING, "Finalisation..." }, // TODO: Review this
	{ IDS_EXPORTING_OK, "OK" },
	{ IDS_EXPORTING_CANCEL, "Annuler" },
	{ IDS_EXPORTING_DONE, "Réussi!" },

	{ IDS_PSD_LOAD_FILE_DIALOG, "Charger Psd" },
	{ IDS_PSD_LOAD_FILE_DIALOG_PATTERN, "fichier psd (*.psd)" },

	{ IDS_FBX_SAVE_FILE_DIALOG, "Sauver Fbx" },
	{ IDS_FBX_SAVE_FILE_DIALOG_PATTERN_ASCII, "FBX ascii (*.fbx)" },
	{ IDS_FBX_SAVE_FILE_DIALOG_PATTERN_BINARY, "FBX binaire (*.fbx)" },

	{ IDS_FBX_SAVE_PATH_DIALOG, "Exporter" },

	{ IDS_MAYA_TEXTURE_NODE_PREFIX, "TextureAtlas_" },
	{ IDS_MAYA_GROUP_NODE_POSTFIX, "_Groupe_PSDto3D" },
	{ IDS_MAYA_SPLINE_NODE_POSTFIX, "_Courbe vectorielle" },

	{ IDS_HELP_URL, "https://edfilms.notion.site/PSD-to-3D-user-manual-6221458afd534a5aa14a46b77048a396" },

	//----------------------------------------------------------------------------------------------
	// Menu

	{ IDS_FILE_MENU, "Fichier" },
	{ IDS_EXPORT_MENU, "Exporter" },
	{ IDS_GENERATE_MENU, "Générer" },
	{ IDS_HELP_MENU, "Aide" },
	{ IDS_FILE_MENU_IMPORT_PSD, "Importer PSD..." },
	{ IDS_FILE_MENU_RELOAD, "Recharger" },
	{ IDS_FILE_MENU_EXIT, "Quitter" },
	{ IDS_EXPORT_MENU_EXPORT_SELECTED, "Exporter selectionnés..." },
	{ IDS_EXPORT_MENU_EXPORT_ALL, "Exporter tous..." },
	{ IDS_GENERATE_MENU_GENERATE_MESH, "Générer object 3D..." },
	{ IDS_GENERATE_MENU_GENERATE_PNG, "Générer PNG..." },
	{ IDS_GENERATE_MENU_GENERATE_BOTH, "Générer object 3D + PNG..." },
	{ IDS_HELP_MENU_DOCS, "Documentation et tutoriels en ligne" },

	//----------------------------------------------------------------------------------------------
	// UI controls

	{ IDS_MAIN_WIDGET_FBX, "PSD to FBX" },
	{ IDS_MAIN_WIDGET_MAYA, "PSD to Maya" },
	{ IDS_PSD_IMPORT_LABEL, "Importer un fichier PSD :" },
	{ IDS_PSD_IMPORT_BTN, "Importer..." },
	{ IDS_PSD_RELOAD_BTN, "Recharger..." },
	{ IDS_PSD_EXPORT_PATH_LABEL, "Chemin d'exportation et nom :" },
	{ IDS_PSD_EXPORT_PATH_BTN, "Rechercher un dossier..." },
	{ IDS_SELECTED_PSD_SIZE_LABEL, "Dimensions :" },
	{ IDS_GENERATION_ALGO_LABEL, "Mode :" },
	  { IDS_GENERATION_ALGO_DROPDOWN_LINEAR, "Linéaire" },
	  { IDS_GENERATION_ALGO_DROPDOWN_BILLBOARD, "Plan" },
	  { IDS_GENERATION_ALGO_DROPDOWN_DELAUNAY, "Delaunay" },
	  { IDS_GENERATION_ALGO_DROPDOWN_VECTOR, "Vectoriel" },
	{ IDS_BILLBOARD_ALGO_LABEL, "Algorithme du mode Plan" },
	{ IDS_BILLBOARD_ALPHA_THRESH_LABEL, "Seuil Alpha :" },
	{ IDS_LINEAR_ALGO_LABEL, "Algorithme Linéraire" },
	{ IDS_LINEAR_PRECISION_LABEL, "Nombre de polygones :" },
	{ IDS_DELAUNAY_ALGO_LABEL, "Algorithme Delaunay" },
	{ IDS_DELAUNAY_OUTER_DETAIL_LABEL, "Détail extérieur :" },
	{ IDS_DELAUNAY_INNER_DETAIL_LABEL, "Détail intérieur :" },
	{ IDS_DELAUNAY_INNER_FALLOFF_LABEL, "Progression :" },
	{ IDS_VECTOR_ALGO_LABEL, "Algorithme Vectoriel" },
	{ IDS_MERGE_VERTICES_CHK, "Fusionner les sommets" },
	{ IDS_MERGE_VERTEX_DISTANCE_LABEL, "Fusionner la distance des sommets :" },
	{ IDS_MULTIPLE_ALGO_LABEL_1, "Attention : Les calques sélectionnés n'ont pas les mêmes aglorhitmes sélectionnés. Vous ne serez pas en mesure de modifier les paramètres jusqu'à ce que ce soit résolu." },
	{ IDS_MULTIPLE_ALGO_LABEL_2, "Durant le processus de génération, les algorithmes spécifiques à chaques calques seront utilisés pour ce calque." },
	{ IDS_INFLUENCE_LAYER_CHK, "Influencer le calque" },
	{ IDS_INFLUENCE_MINIMUM_POLYGON_SIZE, "Taille minimum du polygone :" },
	{ IDS_INFLUENCE_MAXIMUM_POLYGON_SIZE, "Taille maximum du polygone :" },
	{ IDS_MULTIPLE_SETTINGS_LABEL_1, "Attention : Les calques sélectionnés n'ont pas les mêmes paramètres. Modifier les paramètres vont affecter tous les calques." },
	{ IDS_MULTIPLE_SETTINGS_LABEL_2, "Durant le processus de génération, les algorithmes spécifiques à chques calques seront utilisés pour ce calque." },
	{ IDS_TEXTURE_ATLAS_GROUP_LABEL, "Groupe de textures atlas :" },
	  { IDS_TEXTURE_ATLAS_GROUP_DROPDOWN_NONE, "Aucun, séparer les fichiers" },
	  { IDS_TEXTURE_ATLAS_GROUP_DROPDOWN_NEW, "Nouveau groupe..." },
	{ IDS_TEXTURE_ATLAS_SELECTOR_BTN, "Sélectionner les éléments de l'atlas" },
	{ IDS_TEXTURE_ATLAS_CLEANUP_BTN, "Supprimer les groupes vides" },
	{ IDS_TEXTURE_ATLAS_ALGO_LABEL, "Algorithme d'empaquetage :" },
	  { IDS_TEXTURE_ATLAS_ALGO_DROPDOWN_BEST_SHORT_SIDE_FIT, "Ajustement sur la largeur" },
	  { IDS_TEXTURE_ATLAS_ALGO_DROPDOWN_BEST_LONG_SIDE_FIT, "Ajustement sur la longueur" },
	  { IDS_TEXTURE_ATLAS_ALGO_DROPDOWN_BEST_AREA_FIT, "Optimisation de la surface" },
	  { IDS_TEXTURE_ATLAS_ALGO_DROPDOWN_BOTTOM_LEFT_RULE, "Ajustement au coin inférieur gauche" },
	  { IDS_TEXTURE_ATLAS_ALGO_DROPDOWN_CONTACT_POINT_RULE, "Ajustement au point de contact" },
	{ IDS_TEXTURE_ATLAS_OVERRIDE_LABEL, "Taille du groupe atlas :" },
	{ IDS_TEXTURE_ATLAS_OVERRIDE_CHK, "Remplacer la résolution par défaut" },
	{ IDS_TEXTURE_ATLAS_PADDING_LABEL, "Bordure" },
	{ IDS_TEXTURE_OUTPUT_SIZE_LABEL, "Recadrage des textures :" },
	  { IDS_TEXTURE_OUTPUT_SIZE_DROPDOWN_CROPPED, "Ajusté au contenu du calque" },
	  { IDS_TEXTURE_OUTPUT_SIZE_DROPDOWN_FULL, "Pleine dimension de l'espace de travail" },
	{ IDS_EMPTY_SETTINGS_LABEL_1, "Sélectionner un ou plusieurs calques pour modifier les paramètres et générer des grilles pour les calques sélectionnés." },
	{ IDS_LAYERS_SELECT_ALL_BTN, "Tous" },
	{ IDS_LAYERS_SELECT_NONE_BTN, "Aucun" },
	{ IDS_LAYERS_SELECT_COUNT_FORMAT, "%i de %i sélectionnés" },
	{ IDS_TEXTURE_PROXY_LABEL, "Taille du proxy de texture :" },
	  { IDS_TEXTURE_PROXY_DROPDOWN_FULL, "Taille réelle" },
	  { IDS_TEXTURE_PROXY_DROPDOWN_HALF, "Réduire à 1/2" },
	  { IDS_TEXTURE_PROXY_DROPDOWN_QUARTER, "Réduire à 1/4" },
	  { IDS_TEXTURE_PROXY_DROPDOWN_EIGHTH, "Réduire à 1/8" },
	{ IDS_GLOBAL_SETTINGS_LABEL, "Paramètres généraux" },
	{ IDS_MESH_GROUP_NAME_LABEL, "Nommer le groupe :" },
	{ IDS_DEPTH_MODIFIER_LABEL, "Profondeur entre les calques :" },
	{ IDS_MESH_SCALE_LABEL, "Échelle de la grille :" },
	{ IDS_GROUP_STRUCTURE_LABEL, "Structure du groupe PSD" },
	{ IDS_GROUP_STRUCTURE_FLAT, "Hiérarchie à plat" },
	{ IDS_GROUP_STRUCTURE_KEEP, "Conserver la hiérarchie" },
	{ IDS_WRITE_MODE_LABEL, "Format d'exportation :" },
	  { IDS_WRITE_MODE_BINARY, "Binaire" },
	  { IDS_WRITE_MODE_ASCII, "ASCII" },
	{ IDS_WRITE_LAYOUT_LABEL, "Aménagement d'exportation :" },
	  { IDS_WRITE_LAYOUT_SINGLE, "Fichier FBX combiné" },
	  { IDS_WRITE_LAYOUT_MULTI_TEXTURE, "Un FBX par calque" },
	  { IDS_WRITE_LAYOUT_MULTI_LAYER, "Un FBX par texture" },
	{ IDS_GENERATE_ACTION_LABEL, "Exportation :" },
	{ IDS_GENERATE_MESH_BTN, "Objet 3D" },
	{ IDS_GENERATE_PNG_BTN, "PNG" },
	{ IDS_GENERATE_BOTH_BTN, "Objet 3D + PNG" },
	{ IDS_EXPORT_SELECTED_BTN, "Exporter sélectionnés" },
	{ IDS_EXPORT_ALL_BTN, "Exporter TOUS" },

	{ IDS_LICENSING_WIDGET_FBX, "Licence PSD to FBX" },
	{ IDS_LICENSING_WIDGET_MAYA, "Licence PSD to Maya" },
	{ IDS_LICENSING_INSTRUCTIONS_1, "Activation de la licence" },
	{ IDS_LICENSING_INSTRUCTIONS_2, "<html><head/><body><p><br/>Pour activer votre licence PSD to 3D, entrez les informations d'enregristrement reçue dans votre courriel de confirmation d'achat.</p><p>Si vous avez fait l'achat de ce logiciel mais que vous n'avez pas reçu un courriel de confirmation d'achat contenant une clé de licence, s'il-vous-plait écrivez à info@edfilms.net</p></body></html>" },
	{ IDS_LICENSING_USER_INFO_FIRST_NAME_LABEL, "Prénom :" },
	{ IDS_LICENSING_USER_INFO_LAST_NAME_LABEL, "Nom de famille :" },
	{ IDS_LICENSING_USER_INFO_EMAIL_LABEL, "Courriel :" },
	{ IDS_LICENSING_USER_INFO_LICENSE_KEY_LABEL, "Clé de licence :" },
	{ IDS_LICENSING_ACTIVATE_BTN, "Activer la licence" },
	{ IDS_LICENSING_CANCEL_BTN, "Annuler" },

	//----------------------------------------------------------------------------------------------
	// Tooltips

	{ IDS_GENERATE_MESH_TOOLTIP_FBX, "Génère un fichier FBX contenant les objets 3D des calques sélectionnées, avec les références des matériaux. Requiert l'emplacement du dossier." },
	{ IDS_GENERATE_PNG_TOOLTIP_FBX, "Exporte toutes les textures et atlas des calques sélectionnées en tant que fichier(s) PNG. Requiert l'emplacement du dossier." },
	{ IDS_GENERATE_BOTH_TOOLTIP_FBX, "Génère à la fois un fichier FBX, contenant les objets 3D et les références des matériaux, et le(s) fichier(s) PNG correspondant(s) pour les calques et atlas sélectionnés. Requiert l'emplacement du dossier." },

	{ IDS_GENERATE_MESH_TOOLTIP_MAYA, "Génère les objets 3D des calques sélectionnés en créant des noeuds de scène et en appliquant les matériaux appropriés." },
	{ IDS_GENERATE_PNG_TOOLTIP_MAYA, "Exporter toutes les textures et atlas des calques sélectionnés en tant que fichiers PNG dans un dossier situé au même endroit que la source PSD." },
	{ IDS_GENERATE_BOTH_TOOLTIP_MAYA, "Génère tous les objets 3D en tant que noeuds de scènes et les textures en tant que PNG pour les calques sélectionnés, en plaçant les textures dans un dossier situé au même endroit que la source PSD." },

	{ IDS_PSD_SELECTOR_TOOLTIP, "Localisation du fichier PSD sur le disque" },
	{ IDS_PSD_IMPORT_TOOLTIP, "Importer un fichier PSD à partir du disque" },
	{ IDS_PSD_RELOAD_TOOLTIP, "Recharger l'importation du fichier PSD à partir du disque" },
	{ IDS_PSD_EXPORT_PATH_TOOLTIP, "Dossier de destination des fichiers exportés" },
	{ IDS_PSD_EXPORT_NAME_TOOLTIP, "Préfixe du nom des fichiers exportés" },
	{ IDS_PSD_EXPORT_PATH_BTN_TOOLTIP, "Sélectionner l'emplacement du dossier pour les fichiers FBX exportés" },
	{ IDS_LAYER_LIST_TOOLTIP, "<html><head/><body><p><span style=\" font-weight:600;\">Mode sélectionné</span> : Le mode de génération de l'object 3D appliqué à ce calque.</p><p><span style=\" font-weight:600;\">Dimensions</span> : Taille X et Y en pixels du contenu du calque.</p><p><span style=\" font-weight:600;\">Groupe</span> : Le groupe atlas de texture associé à ce calque.</p><p><span style=\" font-weight:600;\">Mode supporté</span> : Les modes de génération pris en charge disponibles pour ce calque. Cela dépend de la disponibilité dans le fichier PSD de chemins associés ou de masques vectoriels.</p></body></html>" },
	{ IDS_SELECTED_PSD_SIZE_TOOLTIP, "Hauteur et largeur du fichier PSD" },
	{ IDS_GENERATION_ALGO_TOOLTIP_FBX, "<html><head/><body><p><span style=\" font-weight:600;\">Mode Delaunay</span> : Génère un objet 3D à densité ajustable en utilisant l’algorithme de triangulation Delaunay. Requiert un tracé fermé pour définir le périmètre de l’objet 3D enregistré dans la liste des tracés Photoshop avec un nom correspondant à celui du calque. </p><p><span style=\" font-weight:600;\">Mode Vectoriel</span> : Génère un objet 3D à partir de courbes personnalisées dessinées par l’utilisateur. Requiert un masque vectoriel Photoshop composé d’au moins un tracé à périmètre fermé et de tracés supplémentaires dessinés à travers le périmètre. Chaque croisement de tracé crée un sommet. </p><p><span style=\" font-weight:600;\">Mode Plan</span> : Génère un plan de travail rectangulaire recadré à la couche alpha. Ne requiert pas de configuration Photoshop supplémentaire. </p></body></html>" },
	{ IDS_GENERATION_ALGO_TOOLTIP_MAYA, "<html><head/><body><p><span style=\" font-size:8pt; font-weight:600;\">Mode Linéaire</span><span style=\" font-size:8pt;\"> : Génère un objet 3D à densité ajustable en utilisant le modificateur de tessellation planaire NURBS dans Maya. Requiert un tracé fermé pour définir le périmètre de l’objet 3D enregistré dans la liste des tracés Photoshop avec un nom correspondant à celui du calque. </span></p><p><span style=\" font-size:8pt; font-weight:600;\">Mode Delaunay</span><span style=\" font-size:8pt;\"> : Génère un objet 3D à densité ajustable en utilisant l’algorithme de triangulation Delaunay. Requiert un tracé fermé pour définir le périmètre de l’objet 3D enregistré dans la liste des tracés Photoshop avec un nom correspondant à celui du calque. </span></p><p><span style=\" font-size:8pt; font-weight:600;\">Mode Vectoriel</span><span style=\" font-size:8pt;\"> : Génère un objet 3D à partir de courbes personnalisées dessinées par l’utilisateur. Requiert un masque vectoriel Photoshop composé d’au moins un tracé à périmètre fermé et de tracés supplémentaires dessinés à travers le périmètre. Chaque croisement de tracé crée un sommet. </span></p><p><span style=\" font-size:8pt; font-weight:600;\">Mode Plan</span><span style=\" font-size:8pt;\"> : Génère un plan de travail rectangulaire recadré à la couche alpha. Ne requiert pas de configuration Photoshop supplémentaire. </span></p></body></html>" },
	{ IDS_BILLBOARD_ALPHA_THRESH_TOOLTIP, "<html><head/><body><p>L'algorithme du mode plan englobe toutes les pixels du calque avec la valeur alpha spécifiée ou une valeur inférieure. Des valeurs plus grandes produisent un plan plus petit et pourraient rogner les pixels des bords. </p><p><span style=\" font-weight:600;\">Plage [0 - 255]</span><br/><span style=\" font-weight:600;\">Par défault [4]</span></p></body></html>" },
	{ IDS_LINEAR_PRECISION_TOOLTIP, "Nombre maximal de polygones dans l'objet 3D généré" },
	{ IDS_DELAUNAY_OUTER_DETAIL_TOOLTIP, "<html><head/><body><p align=\"justify\">Affecte le nombre de sommets le long du périmètre de l&quot;objet 3D.</p><p>Des valeurs élevées produisent un maillage plus dense. </p><p><span style=\" font-weight:600;\">Plage [1.0 - 100.0]</span></p><p><span style=\" font-weight:600;\">Par défault [50.0]</span></p></body></html>" },
	{ IDS_DELAUNAY_INNER_DETAIL_TOOLTIP, "<html><head/><body><p>Affecte le nombre de sommets à l'intérieur de l'objet 3D.</p><p>Des valeurs plus élevées produisent un maillage plus dense. </p><p><span style=\" font-weight:600;\">Plage [1.0 - 100.0]</span></p><p><span style=\" font-weight:600;\">Par défault [50.0]</span></p></body></html>" },
	{ IDS_DELAUNAY_INNER_FALLOFF_TOOLTIP, "<html><head/><body><p>Réduit le nombre de sommets vers le centre de l'objet 3D ; le périmètre n'est pas affecté.</p><p>Des valeurs plus élevées réduisent la densité globale sans sacrifier les détails des contours. Entrer la valeur 0.0 pour la désactiver.</p><p><span style=\" font-weight:600;\">Plage [0.0 - 100.0]</span></p><p><span style=\" font-weight:600;\">Par défault [40.0]</span></p></body></html>" },
	{ IDS_MERGE_VERTICES_TOOLTIP, "Si cette option est cochée, elle fusionne (soude) tous les sommets excédants le long du contour, réduisant ainsi la densité globale du maillage." },
	{ IDS_MERGE_VERTEX_DISTANCE_TOOLTIP, "<html><head/><body><p>La distance minimale entre les sommets dans l'espace UV le long du périmètre avant qu'ils ne soient fusionnés (soudés).</p><p><span style=\" font-weight:600;\">Plage [0.001 - 0.2]</span></p><p><span style=\" font-weight:600;\">Par défault [0.001]</span></p></p></body></html>" },
	{ IDS_INFLUENCE_LAYER_TOOLTIP, "<html><head/><body><p>Si un calque d'influence est détecté, la variable permet d'utiliser le calque pour influencer la sous division sur chaque poly. </p><p>Pour créer un calque d'influence, copiez le calque original, activez le verrou de la couche Alpha et ajoutez &quot;<span style=\" font-weight:600;\">_INFLUENCE&quot;</span> à la fin du nom. Remplissez en tons de gris par dessus. </p><p>Le niveu de subdivision sera plus bas là où le calque d'influence est noir et plus élevé là ou le calcque est blanc.</p></body></html>" },
	{ IDS_INFLUENCE_MINIMUM_POLYGON_SIZE_TOOLTIP, "<html><head/><body><p>Paramètre permettant de spécifier le nombre minimum de divisions en fonction de la valeur d'influence du blanc.<br/><span style=\" font-weight:600;\">Plage [0.01-0.25]</span></p></body></html>" },
	{ IDS_INFLUENCE_MAXIMUM_POLYGON_SIZE_TOOLTIP, "<html><head/><body><p>Paramètre permettant de spécifier le nombre maximal de divisions en fonction de la valeur de l'influence du noir.<br/><span style=\" font-weight:600;\">Plage [0.1-1.0]</span></p></body></html>" },
	{ IDS_TEXTURE_ATLAS_GROUP_TOOLTIP, "<html><head/><body><p>Définit des groupes de calques pour partager un atlas de matériaux et de textures (tuile UV unique).</p><p>Les calques PSD inclus dans le groupe nommé seront organisés, à l'aide de l'algorithme d'emballage sélectionné, en un seul atlas de texture. Les atlas de texture ont une résolution minimale de 512x512 et maximale de 8192x8192. Les groupes constitués de petits calques peuvent être sous-emballés, laissant un espace vide, tandis que les groupes constitués de grands calques peuvent être suremballés et seront réduits pour s'adapter.</p><p>Les groupes de textures atlas réduisent le nombre de matériaux uniques générés pour une scène, permettant aux ajustements de matériaux d'affecter plusieurs objets à la fois. Des atlas de textures serrés peuvent améliorer les performances dans les jeux et réduire les temps de rendu.</p></body></html>" },
	{ IDS_TEXTURE_ATLAS_SELECTOR_TOOLTIP, "Sélectionne les calques du groupe spécifié dans la liste de sélection des calques" },
	{ IDS_TEXTURE_ATLAS_CLEANUP_TOOLTIP, "Supprime les noms de groupe de la liste déroulante si aucun calque n'y est attribué (groupes vides)." },
	{ IDS_TEXTURE_ATLAS_ALGO_TOOLTIP, "<html><head/><body><p><span style=\" font-weight:600;\">Ajustement sur la largeur</span> : Positionne chaque îlot contre l'arête court du rectangle libre dans lequel il s'insère le mieux.</p><p><span style=\" font-weight:600;\">Ajustement sur la longueur</span> : Positionne chaque îlot contre l'arête long du rectangle libre dans lequel il s'insère le mieux.</p><p><span style=\" font-weight:600;\">Optimisation de la surface</span> : Positionne chaque îlot dans le plus petit rectangle libre dans lequel il s'insère le mieux.</p><p><span style=\" font-weight:600;\">Ajustement au coin inférieur gauche</span> : Positionnement de type Tétris.</p><p><span style=\" font-weight:600;\">Ajustement au point de contact</span> : Choisit le positionnement où un îlot touche autant que possible les autres îlots.</p></body></html>" },
	{ IDS_TEXTURE_ATLAS_OVERRIDE_TOOLTIP, "<html><head/><body><p><span style=\" font-size:8pt;\">Lorsque activé, permet de régler le groupe de textures Atlas à une résolution personnalisée, sans modifier la disposition de l'atlas.</span></p><p><span style=\" font-size:8pt;\">La </span><span style=\" font-size:8pt; font-style:italic;\">Résolution optimale de l’Atlas </span><span style=\" font-size:8pt;\">ci-dessous indique la taille de l’Atlas en pixels lorsqu’il est empaqueté de manière optimale avec un minimum d’espaces vides. </span></p></body></html>" },
	{ IDS_TEXTURE_ATLAS_SIZE_TOOLTIP, "Définisser votre résolution de texture personnalisée. Les moteurs de jeu ne prendront en charge que les tailles de texture carrée standard de 512, 1024, 2048, 4096 et 8192." },
	{ IDS_TEXTURE_ATLAS_PADDING_TOOLTIP, "Le nombre de pixels vides entre chaque îlot UV. Si des artefacts indésirables apparaissent sur les bords du maillage, augmenter cette valeur." },
	{ IDS_TEXTURE_OUTPUT_SIZE_TOOLTIP, "<html><head/><body><p>Définit la taille des fichiers de texture de sortie lorsque l'atlas de texture n'est pas utilisé. Non applicable si un groupe d'atlas de textures est sélectionné.</p><p><span style=\" font-weight:600;\">Ajusté au contenu du calque</span> : Le fichier de texture de sortie est découpé en fonction de la boîte de délimitation du contenu du calque, et les UV sont définis en conséquence. Cela permet de produire des fichiers plus petits. La texture n'est pas redimensionnée, les pixels restent donc identiques à ceux de l'original.</p><p><span style=\" font-weight:600;\">Pleine dimension de l'espace de travail</span> : Le fichier de texture de sortie a les mêmes dimensions que le fichier PSD, et les UV sont définis en conséquence. Cela permet de produire des fichiers plus volumineux, tout en préservant le positionnement et la disposition des calques d'origine. La texture n'est pas redimensionnée, les pixels restent donc identiques à ceux de l'original.</p></body></html>" },
	// no tooltip for layer count, all and none buttons
	{ IDS_TEXTURE_PROXY_TOOLTIP, "Réduit l'échelle de toutes les textures générées par la quantité choisie, améliore la performance des jeux et réduit le temps de rendu. Ne modifie pas la disposition de l'atlas ni les UV des objets 3D." },
	{ IDS_MESH_GROUP_NAME_TOOLTIP, "<html><head/><body><p><span style=\" font-size:8pt;\">Nommer le groupe d'objets 3D générés pour les calques sélectionnés.</span></p><p><span style=\" font-size:8pt;\">S'il n'est pas spécifié, le nom du fichier PSD importé sera utilisé par défaut.</span></p></body></html>" },
	{ IDS_DEPTH_MODIFIER_TOOLTIP, "Distance entre les calques dans la scène à exporter." },
	{ IDS_MESH_SCALE_TOOLTIP, "<html><head/><body><p>Affecte la taille de la scène à exporter. Des valeurs plus grandes augmentent la largeur et la hauteur de tous les calques des objets 3D.<br/><span style=\" font-weight:600;\">Plage [0.1 - 1000.0]</span></p><p><span style=\" font-weight:600;\">Par défault [1.0]</span></p></body></html>" },
	{ IDS_GROUP_STRUCTURE_TOOLTIP, "<html><head/><body><p>Génère un parent de transformation pour le calques correspondant au <span style=\" font-weight:600;\">Fichier du Groupe</span> du calque PSD si les paramètres sont sur &quot;<span style=\" font-weight:600;\">Conserver la hiérarchie</span>&quot;.</p></body></html>" },
	{ IDS_WRITE_MODE_TOOLTIP, "<html><head/><body><p><span style=\" font-weight:600;\">Binaire</span> : Le fichier FBX exporté est stocké en format binaire. La taille du fichier est plus petite mais le contenu n'est pas lisible.</p><p><span style=\" font-weight:600;\">ASCII</span> : Le fichier FBX exporté est stocké en format texte. Lisible par l'être humain mais la taille du fichier est plus importante.</p></body></html>" },
	{ IDS_WRITE_LAYOUT_TOOLTIP, "<html><head/><body><p><span style=\" font-weight:600;\">Fichier FBX combiné</span> : Exportation d'un FBX avec tous les calques ensemble. Les fichiers PNG sont tous liés au même fichier FBX.</p><p><span style=\" font-weight:600;\">FBX séparés par texture</span> : Créer plusieurs fichiers FBX, un pour chaque groupe de calques d'un atlas de texture, plus un pour chaque calque sans affectation de groupe. Les fichiers PNG sont liés un à un avec les fichiers FBX.</p><p><span style=\" font-weight:600;\">FBX séparé par calque</span> : Créez plusieurs fichiers FBX, un pour chaque calque. Les fichiers PNG peuvent être liés à plusieurs fichiers FBX, s'ils représentent un atlas de texture, ou à un seul fichier FBX dans le cas contraire.</p></body></html>" },

	{ -1, "" } // bookend
};

const util::StringTable::StringTableItem* GetStringTableItems_french() { return stringTableItems_french; }