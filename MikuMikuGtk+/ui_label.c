#include "ui_label.h"

UI_LABEL LoadDefaultUiLabel(void)
{
	const UI_LABEL default_label =
	{
		{
			"File",
			"Add Model/Accessory",
			"Edit",
			"Undo",
			"Redo"
		},
		{
			"Environment",
			"Camera",
			"Light",
			"Position",
			"Rotation",
			"Direction",
			"Model",
			"Bone",
			"Morph",
			"Load Models",
			"Load Pose",
			"Control Model",
			"No Select",
			"Reset",
			"Apply Center Position",
			"Scale",
			"Color",
			"Opacity",
			"Rate",
			"Edge Size",
			"Distance",
			"Field of View",
			"Connect to",
			"Enable Physics",
			"Display Grid",
			"Render Edge Only",
			{
				"Select",
				"Move",
				"Rotate"
			},
			{
				"Eye",
				"Lip",
				"Eye Blow",
				"Others"
			}
		}
	};

	return default_label;
}
