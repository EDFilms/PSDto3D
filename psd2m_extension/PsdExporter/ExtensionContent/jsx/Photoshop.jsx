//-----------------------------------------------------------------------------------------------------------------
$._ext_PHXS={

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
        var idsetd = charIDToTypeID( "setd" );
        
        var desc2 = new ActionDescriptor();
        var idnull = charIDToTypeID( "null" );
        
            var ref1 = new ActionReference();
            var idChnl = charIDToTypeID( "Chnl" );
            var idfsel = charIDToTypeID( "fsel" );
            ref1.putProperty( idChnl, idfsel );
        desc2.putReference( idnull, ref1 );

        var idT = charIDToTypeID( "T   " );
            var ref2 = new ActionReference();
            var idChnl = charIDToTypeID( "Chnl" );
            var idChnl = charIDToTypeID( "Chnl" );
            var idTrsp = charIDToTypeID( "Trsp" );
            ref2.putEnumerated( idChnl, idChnl, idTrsp );
        desc2.putReference( idT, ref2 );
        executeAction( idsetd, desc2, DialogModes.NO );
    },

    //-----------------------------------------------------------------------------------------------------------------
    expendSelection : function ( size ) {
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
    makePath : function() {
        var idMk = charIDToTypeID( "Mk  " );
        var desc22 = new ActionDescriptor();
        var idnull = charIDToTypeID( "null" );
            var ref5 = new ActionReference();
            var idPath = charIDToTypeID( "Path" );
            ref5.putClass( idPath );
        desc22.putReference( idnull, ref5 );
        var idFrom = charIDToTypeID( "From" );
            var ref6 = new ActionReference();
            var idcsel = charIDToTypeID( "csel" );
            var idfsel = charIDToTypeID( "fsel" );
            ref6.putProperty( idcsel, idfsel );
        desc22.putReference( idFrom, ref6 );
        var idTlrn = charIDToTypeID( "Tlrn" );
        var idPxl = charIDToTypeID( "#Pxl" );
        desc22.putUnitDouble( idTlrn, idPxl, 2.000000 );
        executeAction( idMk, desc22, DialogModes.NO );
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
            this.saveAsPng (basePath + '/' + layer.name)
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
    print : function (x) {
        $.writeln(x)
    },
    
    //-----------------------------------------------------------------------------------------------------------------
    main : function (expendValue, exportPng) {
        
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
        
        docDuplicate.pathItems.removeAll();

        // The layers property of a document is a collection holding all layers
        for (var i = 0; i < docDuplicate.layers.length; i++)
        {
            var layer = docDuplicate.layers[i];
            
            // show some info in the console
            this.print(layer.name);
            this.print(layer.kind);
            
            // Make this layer active
            docDuplicate.activeLayer = layer;
            
            // Skip background
            if(layer.isBackgroundLayer)
                continue;
            
            // SOLIDFILL layer need to be rasterized
            if (layer.kind == LayerKind.SOLIDFILL)
                this.rasterize ();

            // Make a work path around the non-transparent pixel in the layer
            // Give the name of the layer to the path.
            try {
                this.setSelectionToTransparency ();
                this.expendSelection (expendValue);
                
                docDuplicate.selection.makeWorkPath (2.0);
                
                // Get the last pathitem create -> .add() because get the work path by name is not persistent with the localisation of Photoshop.
                var workPath =  docDuplicate.pathItems[docDuplicate.pathItems.length-1];
                workPath.name = layer.name;
            }
            catch(err) {
                    print (err);
            }

            this.print('=============')
        }
              
        docDuplicate.saveAs(new File(illFilePath), psdOptions)

        if(exportPng)
        {
            this.save (docDuplicate);
            docDuplicate.save();
        }
        
        docDuplicate.close();
        
        alert("Exportation completed!");
    },
};