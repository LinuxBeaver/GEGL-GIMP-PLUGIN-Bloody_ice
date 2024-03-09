/* This file is an image processing operation for GEGL
 *
 * GEGL is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * GEGL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GEGL; if not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright 2006 Øyvind Kolås <pippin@gimp.org>
 * 2023 Beaver, Bloody Ice


This Gimp plugin can be tested without installing by pasting this syntax into Gimp's GEGL Graph filter. This is also something a GEGL dev should study if they want to make plugins of their own..
--

gaussian-blur std-dev-x=0.5 std-dev-y=0.5 color-overlay value=#840917 id=0 gimp:layer-mode layer-mode=hardlight aux=[ ref=0 emboss depth=1  ] id=1 gimp:layer-mode layer-mode=hsl-color opacity=0.91 aux=[ ref=1 color-overlay value=#ff0000  ] noise-reduction iterations=3 

id=2  gimp:layer-mode layer-mode=overlay opacity=0.6 blend-space=rgb-perceptual aux=[ ref=2 wind direction=bottom style=blast threshold=1 seed=23 strength=1 oilify mask-radius=3  ]

 noise-reduction iterations=4 

id=4 gimp:layer-mode layer-mode=overlay opacity=0.2 blend-space=rgb-linear aux=[ ref=4 wind direction=bottom style=blast threshold=4 seed=13 strength=21 oilify mask-radius=2  softglow brightness=6]

id=6 gimp:layer-mode layer-mode=overlay opacity=0.2 blend-space=rgb-linear aux=[ ref=6 wind direction=bottom style=blast threshold=4 seed=13 strength=30 oilify mask-radius=2  softglow brightness=10]

cubism tile-size=1 gaussian-blur std-dev-x=1std-dev-y=1 id=bevelmanual gimp:layer-mode layer-mode=overlay opacity=0.70 aux=[ ref=bevelmanual emboss depth=19 elevation=39 azimuth=30 ] opacity=2 median-blur radius=0 opacity value=1.5 softglow glow-radius=3 hue-chroma chroma=10 id=5 gimp:layer-mode layer-mode=overlay aux=[ ref=5 edge ] saturation scale=0.7 mean-curvature-blur iterations=6 gimp:desaturate mode=luma id=1 gimp:layer-mode layer-mode=grain-merge composite-mode=auto aux=[ ref=1 color value=#ff000a ]  median-blur radius=0 id=extrabevel gimp:layer-mode layer-mode=grain-extract composite-mode=auto opacity=0.04 aux=[ ref=1  gaussian-blur std-dev-x=7 std-dev-y=7 emboss  azimuth=40  elevation=45 depth=40 gimp:threshold-alpha value=0.100 ]

--end of syntax
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#ifdef GEGL_PROPERTIES

#define FIRST \
" gaussian-blur std-dev-x=0.5 std-dev-y=0.5 color-overlay value=#840917 id=0 gimp:layer-mode layer-mode=hardlight aux=[ ref=0 emboss depth=1  ] id=1 gimp:layer-mode layer-mode=hsl-color opacity=0.91 aux=[ ref=1 color-overlay value=#ff0000  ] noise-reduction iterations=3   "\


#define SECOND \
" cubism tile-size=1 gaussian-blur std-dev-x=1std-dev-y=1 id=bevelmanual gimp:layer-mode layer-mode=overlay opacity=0.70 aux=[ ref=bevelmanual emboss depth=19 elevation=39 azimuth=30 ] opacity=2 median-blur radius=0 opacity value=1.5 softglow glow-radius=3 hue-chroma chroma=10 id=5 gimp:layer-mode layer-mode=overlay aux=[ ref=5 edge ] saturation scale=0.7 mean-curvature-blur iterations=6 gimp:desaturate mode=luma id=1 gimp:layer-mode layer-mode=grain-merge composite-mode=auto aux=[ ref=1 color value=#ff000a ]  median-blur radius=0 id=extrabevel gimp:layer-mode layer-mode=grain-extract composite-mode=auto opacity=0.04 aux=[ ref=1 gaussian-blur std-dev-x=7 std-dev-y=7 emboss  azimuth=40  elevation=45 depth=40 gimp:threshold-alpha value=0.100 ] median-blur radius=0 "\

property_int  (radius, _("Size Control"), 3)
  value_range (0, 10)
  ui_range    (0, 10)
  ui_meta     ("unit", "pixel-distance")
  description (_("Median Radius for size control of the blood ice text"))

property_int (icicle1, _("Icicle control 1"), 1)
 description (_("Length of second layer of icicles"))
 value_range (1, 100)

property_int (icicle2, _("Icicle control 2"), 21)
 description (_("Length of second layer of icicles"))
 value_range (1, 100)


property_int (icicle3, _("Icicle control 3"), 30)
 description (_("Length of third layer of icicles"))
 value_range (1, 100)

property_seed (seed, _("Random seed for icicles"), rand)


property_double (hue, _("Hue Rotation"),  0.0)
   description  (_("Color rotation. Either '-180/180' with lightness tweaks is Aqua Blue which resembles Ice text styles. Default '0' is Blood Red"))
   value_range  (-180.0, 180.0)


property_double (light, _("Light Tweak"), 0)
   description  (_("Lightness adjustment for the entire text style"))
   value_range  (0.0, 15.0)


#else

#define GEGL_OP_META
#define GEGL_OP_NAME     bloody_ice
#define GEGL_OP_C_SOURCE bloody-ice.c

#include "gegl-op.h"

static void attach (GeglOperation *operation)
{
  GeglNode *gegl = operation->node;
  GeglNode *input, *output, *sizecontrol, *syntax1, *idref1, *overlay1, *nr, *idref2,  *overlay2, *idref3, *overlay3, *syntax2, *huerotation, *oilify1, *oilify2, *oilify3, *softglow1, *softglow2, *wind1, *wind2, *wind3;

  input    = gegl_node_get_input_proxy (gegl, "input");
  output   = gegl_node_get_output_proxy (gegl, "output");

  syntax1    = gegl_node_new_child (gegl,
                                  "operation", "gegl:gegl", "string", FIRST,
                                  NULL);

 syntax2   = gegl_node_new_child (gegl,
                                  "operation", "gegl:gegl", "string", SECOND,  NULL);

  sizecontrol    = gegl_node_new_child (gegl,
                                  "operation", "gegl:median-blur", "alpha-percentile", 100.0,
                                  NULL);

  idref1    = gegl_node_new_child (gegl,
                                  "operation", "gegl:nop",
                                  NULL);

  idref2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:nop",
                                  NULL);


  idref3    = gegl_node_new_child (gegl,
                                  "operation", "gegl:nop",
                                  NULL);

  wind1    = gegl_node_new_child (gegl,
                                  "operation", "gegl:wind", "direction", 3,  "style", 1, "threshold", 4,
                                  NULL);


  wind2   = gegl_node_new_child (gegl,
                                  "operation", "gegl:wind", "direction", 3,  "style", 1, "threshold", 4,
                                  NULL);


  wind3    = gegl_node_new_child (gegl,
                                  "operation", "gegl:wind", "direction", 3, "style", 1, "threshold", 4,
                                  NULL);

overlay1 = gegl_node_new_child (gegl,
                                  "operation", "gimp:layer-mode", "layer-mode", 23, "opacity", 0.2, "blend-space", 1, NULL);

overlay2 = gegl_node_new_child (gegl,
                                  "operation", "gimp:layer-mode", "layer-mode", 23, "opacity", 0.2, "blend-space", 1, NULL);

overlay3 = gegl_node_new_child (gegl,
                                  "operation", "gimp:layer-mode", "layer-mode", 23, "opacity", 0.2, "blend-space", 1, NULL);


  nr    = gegl_node_new_child (gegl,
                                  "operation", "gegl:noise-reduction", "iterations", 4,
                                  NULL);

  huerotation    = gegl_node_new_child (gegl,
                                  "operation", "gegl:hue-chroma", 
                                  NULL);


  oilify1    = gegl_node_new_child (gegl,
                                  "operation", "gegl:oilify", "mask-radius", 2,
                                  NULL);


  oilify2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:oilify", "mask-radius", 2,
                                  NULL);

  oilify3    = gegl_node_new_child (gegl,
                                  "operation", "gegl:oilify", "mask-radius", 2,
                                  NULL);

  softglow1    = gegl_node_new_child (gegl,
                                  "operation", "gegl:softglow", "brightness", 6.0,
                                  NULL);

  softglow2    = gegl_node_new_child (gegl,
                                  "operation", "gegl:softglow", "brightness", 6.0,
                                  NULL);

  gegl_node_link_many (input, sizecontrol, syntax1, idref1, overlay1, nr, idref2, overlay2, idref3, overlay3, syntax2, huerotation, output, NULL);
  gegl_node_connect (overlay1, "aux", oilify1, "output");
  gegl_node_link_many (idref1, wind1, oilify1, NULL);
  gegl_node_connect (overlay2, "aux", softglow1, "output");
  gegl_node_link_many (idref2, wind2, oilify2, softglow1, NULL);
  gegl_node_connect (overlay3, "aux", softglow2, "output");
  gegl_node_link_many (idref3, wind3, oilify3, softglow2, NULL);

  gegl_operation_meta_redirect (operation, "icicle1",  wind1, "strength");
  gegl_operation_meta_redirect (operation, "icicle2",  wind2, "strength");
  gegl_operation_meta_redirect (operation, "icicle3",  wind3, "strength");

  gegl_operation_meta_redirect (operation, "seed",  wind1, "seed");
  gegl_operation_meta_redirect (operation, "seed",  wind2, "seed");
  gegl_operation_meta_redirect (operation, "seed",  wind3, "seed");

  gegl_operation_meta_redirect (operation, "radius",  sizecontrol, "radius");
  gegl_operation_meta_redirect (operation, "hue",  huerotation, "hue");
  gegl_operation_meta_redirect (operation, "light",  huerotation, "lightness");

}

static void
gegl_op_class_init (GeglOpClass *klass)
{
  GeglOperationClass *operation_class;

  operation_class = GEGL_OPERATION_CLASS (klass);

  operation_class->attach = attach;

  gegl_operation_class_set_keys (operation_class,
    "name",        "lb:bloody-ice",
    "title",       _("Bloody Ice"),
    "reference-hash", "nblodvricasa48j3gj21g4ac",
    "description", _("Transform plain text into a 'Bloody to Icicle text style' The reason this filter's name is 'bloody ice' is because when red it looks like blood themed horror-esque text but when blue/aqua it looks like ice themed text."),
    "gimp:menu-path", "<Image>/Filters/Text Styling",
    "gimp:menu-label", _("Bloody Ice..."),
    NULL);
}

#endif
