//-----------------------------------------------------------------------------------------------------------------
this_ext_PHXS = {

     rasterize : function() {
        var idrasterizeLayer = stringIDToTypeID( "rasterizeLayer" );
        
        var desc = new ActionDescriptor();
        var idnull = charIDToTypeID( "null" );
        
        var ref = new ActionReference();
        
        var idLyr = charIDToTypeID( "Lyr " );
        var idOrdn = charIDToTypeID( "Ordn" );
        var idTrgt = charIDToTypeID( "Trgt" );
        
        ref.putEnumerated( idLyr, idOrdn, idTrgt );
    
        desc.putReference( idnull, ref );
        
        executeAction( idrasterizeLayer, desc, DialogModes.NO );
    },

    setSelectionToTransparency : function() {
        // step 1, select contents as if ctrl-clicking on layer thumbnail
        var idsetd = charIDToTypeID( "setd" );
        
        var desc8 = new ActionDescriptor();
        var idnull = charIDToTypeID( "null" );
        
        var ref1 = new ActionReference();
        var idChnl = charIDToTypeID( "Chnl" );
        var idfsel = charIDToTypeID( "fsel" );
        ref1.putProperty( idChnl, idfsel );
        desc8.putReference( idnull, ref1 );
        
        var idT = charIDToTypeID( "T   " );
        var ref2 = new ActionReference();
        var idChnl = charIDToTypeID( "Chnl" );
        var idChnl = charIDToTypeID( "Chnl" );
        var idTrsp = charIDToTypeID( "Trsp" );
        ref2.putEnumerated( idChnl, idChnl, idTrsp );
        desc8.putReference( idT, ref2 );
        executeAction( idsetd, desc8, DialogModes.NO );

        // step 2, enter quick mask mode
        var idsetd = charIDToTypeID( "setd" );
        
        var desc9 = new ActionDescriptor();
        var idnull = charIDToTypeID( "null" );
        
        var ref3 = new ActionReference();
        var idPrpr = charIDToTypeID( "Prpr" );
        var idQucM = charIDToTypeID( "QucM" );
        ref3.putProperty( idPrpr, idQucM );
        
        var idDcmn = charIDToTypeID( "Dcmn" );
        var idOrdn = charIDToTypeID( "Ordn" );
        var idTrgt = charIDToTypeID( "Trgt" );
        ref3.putEnumerated( idDcmn, idOrdn, idTrgt );
        desc9.putReference( idnull, ref3 );
        
        executeAction( idsetd, desc9, DialogModes.NO );

        // step 3, set the quick mask threshold
        // use a threshold of 16 (out of 256),
        // which excluses only the 6% most transparent pixels
        var idThrs = charIDToTypeID( "Thrs" );
        
        var desc12 = new ActionDescriptor();
        var idLvl = charIDToTypeID( "Lvl " );
        desc12.putInteger( idLvl, 16 );
        
        executeAction( idThrs, desc12, DialogModes.NO );

        // step 4, exit quick mask mode to finalize the selection
        var idCler = charIDToTypeID( "Cler" );
        
        var desc14 = new ActionDescriptor();
        var idnull = charIDToTypeID( "null" );
        
        var ref4 = new ActionReference();
        var idPrpr = charIDToTypeID( "Prpr" );
        var idQucM = charIDToTypeID( "QucM" );
        ref4.putProperty( idPrpr, idQucM );
        
        var idDcmn = charIDToTypeID( "Dcmn" );
        var idOrdn = charIDToTypeID( "Ordn" );
        var idTrgt = charIDToTypeID( "Trgt" );
        ref4.putEnumerated( idDcmn, idOrdn, idTrgt );
        desc14.putReference( idnull, ref4 );
        
        executeAction( idCler, desc14, DialogModes.NO );
    },

    //-----------------------------------------------------------------------------------------------------------------
    normalizeName : function (name) {
        // If the first character is not alphabetic a-zA-Z, then prepend an underscore
        if( !(isNaN(name[0])) )
        {
            name = '_' + name;
        }
        // Convert special characters to underscore
        name = name.replace(/[~`!@#$%^&*\(\)-+=:;'".,<>\/?\/\\|\[\]\{\}<> ]+/g,'_');
        // Remove trailing underscore if any
        if( (name.length>1) && (name[name.length-1]=="_") )
        {
            name = name.slice(0,-1);
        }
        return name;
     },


    //-----------------------------------------------------------------------------------------------------------------
    expandSelection : function ( size ) {
        //alert("expandSelection()");
        var idExpn = charIDToTypeID( "Expn" );
        var desc21 = new ActionDescriptor();
        var idBy = charIDToTypeID( "By  " );
        var idPxl = charIDToTypeID( "#Pxl" );
        desc21.putUnitDouble( idBy, idPxl, size );
        var idselectionModifyEffectAtCanvasBounds = stringIDToTypeID( "selectionModifyEffectAtCanvasBounds" );
        desc21.putBoolean( idselectionModifyEffectAtCanvasBounds, false );
        executeAction( idExpn, desc21, DialogModes.NO );
    },

    //-----------------------------------------------------------------------------------------------------------------
    makeWorkPath : function(tolerance) {
        var idMk = charIDToTypeID( "Mk  " );
        var desc = new ActionDescriptor();
        var idnull = charIDToTypeID( "null" );
            var ref1 = new ActionReference();
            var idPath = charIDToTypeID( "Path" );
            ref1.putClass( idPath );
        desc.putReference( idnull, ref1 );
        var idFrom = charIDToTypeID( "From" );
            var ref2 = new ActionReference();
            var idcsel = charIDToTypeID( "csel" );
            var idfsel = charIDToTypeID( "fsel" );
            ref2.putProperty( idcsel, idfsel );
        desc.putReference( idFrom, ref2 );
        var idTlrn = charIDToTypeID( "Tlrn" );
        var idPxl = charIDToTypeID( "#Pxl" );
        desc.putUnitDouble( idTlrn, idPxl, tolerance );
        executeAction( idMk, desc, DialogModes.NO );
    },

    //-----------------------------------------------------------------------------------------------------------------
    getWorkPath : function(doc) {
        var len = doc.pathItems.length;
        for ( var i = len-1; i >= 0; i--  ) {
            if( doc.pathItems[i].kind==PathKind.WORKPATH )
                return doc.pathItems[i];
        }
        return undefined;
    },

    //-----------------------------------------------------------------------------------------------------------------
    saveAsPng : function(name) {
        // get ids
        var idsave = charIDToTypeID( "save" );
        var idAs = charIDToTypeID( "As  " );
        var idIn = charIDToTypeID( "In  " );
        var idPGIT = charIDToTypeID( "PGIT" );
        var idPGIN = charIDToTypeID( "PGIN" );
        var idPNGf = charIDToTypeID( "PNGf" );
        var idPGAd = charIDToTypeID( "PGAd" );
        var idCmpr = charIDToTypeID( "Cmpr" );
        var idPNGF = charIDToTypeID( "PNGF" );
        var idsaveStage = stringIDToTypeID( "saveStage" );
        var idsaveStageType = stringIDToTypeID( "saveStageType" );
        var idsaveSucceeded = stringIDToTypeID( "saveSucceeded" );
        var idDocI = charIDToTypeID( "DocI" );
        var idCpy = charIDToTypeID( "Cpy " );
        
        // create desciptor
        var pngOptions = new ActionDescriptor();
        pngOptions.putEnumerated( idPGIT, idPGIT, idPGIN );
        pngOptions.putEnumerated( idPNGf, idPNGf, idPGAd );
        pngOptions.putInteger( idCmpr, 9 );
    
        var saveAs = new ActionDescriptor();
        saveAs.putObject( idAs, idPNGF, pngOptions );
        saveAs.putPath( idIn, new File( name + ".png" ) );
        saveAs.putInteger( idDocI, 195 );
        saveAs.putBoolean( idCpy, true );
        saveAs.putEnumerated( idsaveStage, idsaveStageType, idsaveSucceeded );
        
        // execute
        executeAction( idsave, saveAs, DialogModes.NO );
    },

    //-----------------------------------------------------------------------------------------------------------------
    saveAllLayersAsPng : function (doc, basePath) {
        // hide all
        for (var i = 0; i < doc.layers.length; i++)
            doc.layers[i].visible = false;
            
        for (var i = 0; i < doc.layers.length; i++) {
            var layer = doc.layers[i]
            layer.visible = true;
            // Normalize names for layer PNGs, files need their files names now,
            // workflow is to not create them later during import in other applications
            this.saveAsPng (basePath + '/' + this.normalizeName(layer.name) )
            layer.visible = false;
        }
        // show all
        for (var i = 0; i < doc.layers.length; i++)
            doc.layers[i].visible = true;
    },

    //-----------------------------------------------------------------------------------------------------------------
    save : function (doc) {
        var basePath = doc.fullName.path;
        var baseName = doc.fullName.name.split('.')[0];
        
        var folder = new Folder(basePath + '/' + baseName);
        if(!folder.exists)
            folder.create();

        this.saveAllLayersAsPng (doc, folder.fullName);
    },

    //-----------------------------------------------------------------------------------------------------------------
    ungroupLayers : function (doc, layers)
    {
        var layersArray = Array.prototype.slice.call( layers ); // make a copy
        
        // show some info in the console
        this.print("Ungrouping ... ");

        for ( var i = 0; i < layersArray.length; i++  )
        {
            var layer = layersArray[i];
            
            if( layer.typename == "LayerSet" )
            {
                this.print("Nested group...");
                // Recursive call for nested group
                var group = layer;
                this.ungroupLayers( doc, group.layers );
                group.remove();
            }
            else
            {
                this.print(layer.name);
                
                // Skip background
                if(layer.isBackgroundLayer)
                    return;
                
                // Don't normalize names for layers now, wait until imported in other applications
                //layer.name = normalizeName( layer.name );

                layer.move( doc, ElementPlacement.PLACEATEND );
                this.print('=============');
            }
        }
    },

    //-----------------------------------------------------------------------------------------------------------------
    exportLayer : function (doc, layer, expandValue) {
        // show some info in the console
        this.print("Exporting ... ");
        this.print(layer.name);
        this.print(layer.kind);
        
        // Skip background
        if(layer.isBackgroundLayer)
            return;
            
        // Make this layer active
        doc.activeLayer = layer;

        // SOLIDFILL layer need to be rasterized
        if (layer.kind == LayerKind.SOLIDFILL)
            this.rasterize ();

        // Make a work path around the non-transparent pixel in the layer
        // Give the name of the layer to the path.
        // Select non-transparent pixels, then try to expand the selection
        var haveSelection = false;
        try {
            this.setSelectionToTransparency ();
            haveSelection = true;
            // WORKAROUND: This can fail on background layers in CS6, so just ignore that error
            this.expandSelection (expandValue);
        }
        catch(err) {
                this.print (err);
        }
        // Make a work path from the selection
        if( haveSelection ) try {            
            this.makeWorkPath (2.0);
            var workPath = this.getWorkPath (doc);
            workPath.name = layer.name;
        }
        catch(err) {
                this.print (err);
                alert(err);
        }
    },

    //-----------------------------------------------------------------------------------------------------------------
    print : function (x) {
        $.writeln(x)
    },
    
    //-----------------------------------------------------------------------------------------------------------------
    main : function (expandValue, exportVisibleOnly) {
        
        //var filterFiles = 'PSD:*.psd'; // I am windows
        //var theFile = openDialog ("Choose the Psd file to load from" , filterFiles);

        // Get a reference to the active document
        var doc = app.activeDocument;
        
        var dskTop = Folder.desktop;
        var dskPth = String(dskTop);
        var newSpot = new File(dskPth + '/' + doc.name);
        var selectedFolder = newSpot.saveDlg('Select Destination Folder',"*.psd");

        var docDuplicate = doc.duplicate(selectedFolder.name);

        var illFilePath = selectedFolder.fullName;
        var psdOptions = new PhotoshopSaveOptions();
        psdOptions.layers = true;
        psdOptions.embedColorProfile = true;
        
        // Perfore deleting all paths, make sure to selecte a layer with no vector mask.
        // Create a temporary layer for this.  Otherwise select layer vector mask gets destroyed.
        var tempLayer=docDuplicate.artLayers.add();
        docDuplicate.activeLayer=tempLayer;

        docDuplicate.pathItems.removeAll();

        // Delete the temporary later
        tempLayer.remove();

        this.ungroupLayers( docDuplicate, docDuplicate.layers );

        // The layers property of a document is a collection holding all layers
        var layerIndex = 0;
        var layerCount = docDuplicate.layers.length;
        for (var i = 0; i < layerCount; i++)
        {
            // iterate through layers in forward order,
            // if visibleOnly is passed, delete non-visible layers
            // and don't increment index for those, but don't perform a reverse iteration
            // through layers, to ensure paths have same order in file as layer order
            var layer = docDuplicate.layers[ layerIndex ];
            
            if( (exportVisibleOnly) && !(layer.visible)  )
            {
                layer.remove();
            }
            else
            {
                this.exportLayer (docDuplicate, layer, expandValue);

                this.print('=============');
                
                layerIndex ++;
            }
        }

        docDuplicate.saveAs(new File(illFilePath), psdOptions)

        docDuplicate.close();
        
        alert("Export completed!");
    },
};

this_ext_PHXS.main( 20, false );