{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 8,
			"minor" : 1,
			"revision" : 8,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"classnamespace" : "box",
		"rect" : [ 373.0, 230.0, 548.0, 322.0 ],
		"bglocked" : 0,
		"openinpresentation" : 1,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Montserrat SemiBold",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 2,
		"toptoolbarpinned" : 2,
		"righttoolbarpinned" : 2,
		"bottomtoolbarpinned" : 2,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"style" : "",
		"subpatcher_template" : "",
		"assistshowspatchername" : 0,
		"boxes" : [ 			{
				"box" : 				{
					"fontsize" : 12.0,
					"id" : "obj-14",
					"linecount" : 5,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 341.5, 14.0, 177.0, 80.0 ],
					"presentation" : 1,
					"presentation_linecount" : 5,
					"presentation_rect" : [ 341.5, 14.0, 177.0, 80.0 ],
					"text" : "A set of objects that allow any Max patch or Max for Live device to work as a client with the MTS-ESP tuning system.",
					"textcolor" : [ 0.87843137254902, 0.87843137254902, 0.87843137254902, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"bgcolor" : [ 0.588235, 0.682353, 0.780392, 0.0 ],
					"button" : 1,
					"fontsize" : 14.0,
					"htabcolor" : [ 0.952941, 0.564706, 0.098039, 1.0 ],
					"id" : "obj-39",
					"margin" : 5,
					"maxclass" : "tab",
					"numinlets" : 1,
					"numoutlets" : 3,
					"outlettype" : [ "int", "", "" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 341.5, 149.0, 145.0, 135.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 341.5, 149.0, 145.0, 135.0 ],
					"rounded" : 9.0,
					"spacing_x" : 12.0,
					"spacing_y" : 14.0,
					"tabcolor" : [ 0.219414, 0.219414, 0.219414, 1.0 ],
					"tabs" : [ "MTS-ESP.mtof", "MTS-ESP.mtof~", "MTS-ESP.ftom" ],
					"textcolor" : [ 0.298039215686275, 0.627450980392157, 0.874509803921569, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-12",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 404.5, 327.0, 47.0, 19.0 ],
					"text" : "pcontrol"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"fontsize" : 9.0,
					"id" : "obj-13",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 404.5, 301.0, 66.0, 19.0 ],
					"text" : "prepend help"
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 12.0,
					"id" : "obj-10",
					"linecount" : 3,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 124.0, 240.0, 186.0, 50.0 ],
					"presentation" : 1,
					"presentation_linecount" : 3,
					"presentation_rect" : [ 124.0, 240.0, 186.0, 50.0 ],
					"text" : "Obtain the MIDI note number nearest to a given frequency.",
					"textcolor" : [ 0.87843137254902, 0.87843137254902, 0.87843137254902, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 12.0,
					"id" : "obj-9",
					"linecount" : 4,
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 124.0, 149.0, 186.0, 65.0 ],
					"presentation" : 1,
					"presentation_linecount" : 4,
					"presentation_rect" : [ 124.0, 149.0, 186.0, 65.0 ],
					"text" : "Query retuning for a given MIDI note either as an absolute frequency or relative to 12-TET tuning.",
					"textcolor" : [ 0.87843137254902, 0.87843137254902, 0.87843137254902, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 14.0,
					"id" : "obj-8",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 341.5, 113.0, 134.0, 24.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 341.5, 113.0, 134.0, 24.0 ],
					"text" : "HELP EXAMPLES:",
					"textcolor" : [ 0.87843137254902, 0.87843137254902, 0.87843137254902, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 14.0,
					"id" : "obj-7",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 15.0, 113.0, 96.0, 24.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 15.0, 113.0, 96.0, 24.0 ],
					"text" : "OBJECTS:",
					"textcolor" : [ 0.87843137254902, 0.87843137254902, 0.87843137254902, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 24.0,
					"id" : "obj-6",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 15.0, 52.0, 314.0, 36.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 15.0, 52.0, 314.0, 36.0 ],
					"text" : "MTS-ESP MAX PACKAGE",
					"textcolor" : [ 0.87843137254902, 0.87843137254902, 0.87843137254902, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 24.0,
					"id" : "obj-5",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 76.0, 14.0, 103.0, 36.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 76.0, 14.0, 103.0, 36.0 ],
					"text" : "SOUND",
					"textcolor" : [ 0.298039215686275, 0.627450980392157, 0.874509803921569, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontsize" : 24.0,
					"id" : "obj-4",
					"maxclass" : "comment",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 15.0, 14.0, 164.0, 36.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 15.0, 14.0, 164.0, 36.0 ],
					"text" : "ODD",
					"textcolor" : [ 0.745098039215686, 0.56078431372549, 0.341176470588235, 1.0 ]
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"id" : "obj-3",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "" ],
					"patching_rect" : [ 15.0, 254.0, 96.0, 22.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 15.0, 254.0, 96.0, 22.0 ],
					"text" : "MTS-ESP.ftom"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"id" : "obj-2",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 3,
					"outlettype" : [ "signal", "signal", "signal" ],
					"patching_rect" : [ 15.0, 189.0, 96.0, 22.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 15.0, 189.0, 96.0, 22.0 ],
					"text" : "MTS-ESP.mtof~"
				}

			}
, 			{
				"box" : 				{
					"fontname" : "Arial",
					"id" : "obj-1",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 4,
					"outlettype" : [ "", "", "", "" ],
					"patching_rect" : [ 15.0, 157.0, 96.0, 22.0 ],
					"presentation" : 1,
					"presentation_rect" : [ 15.0, 157.0, 96.0, 22.0 ],
					"text" : "MTS-ESP.mtof"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-12", 0 ],
					"source" : [ "obj-13", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"source" : [ "obj-39", 1 ]
				}

			}
 ],
		"dependency_cache" : [ 			{
				"name" : "MTS-ESP.mtof.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "MTS-ESP.mtof~.mxo",
				"type" : "iLaX"
			}
, 			{
				"name" : "MTS-ESP.ftom.mxo",
				"type" : "iLaX"
			}
 ],
		"autosave" : 0,
		"clearcolor" : [ 0.298039215686275, 0.027450980392157, 0.027450980392157, 0.0 ],
		"bgcolor" : [ 0.035294117647059, 0.043137254901961, 0.050980392156863, 1.0 ],
		"editing_bgcolor" : [ 0.035294117647059, 0.043137254901961, 0.050980392156863, 1.0 ]
	}

}
