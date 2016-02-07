/* -*- C -*-
 *
 * Lua binding of Cairo
 *
 */
#ifndef LUACAIRO
# error private include file
#endif




// --------------------------------------------------------------------------
// SolidPattern userdata for oo usage
// --------------------------------------------------------------------------



#if LUACAIRO_HAS_OO_INTERFACE


static int new_SolidPattern (lua_State *L)
{
    double red;
    double green;
    double blue;
    double alpha;

    cairo_pattern_t* cp = NULL;

    lua_remove(L, 1); // remove cairo.SolidPattern

    //{"create_rgb",                       l_cairo_pattern_create_rgb},
    //{"create_rgba",                      l_cairo_pattern_create_rgba},

    red = luaL_checknumber(L, 1);
    green = luaL_checknumber(L, 2);
    blue = luaL_checknumber(L, 3);
    alpha = luaL_optnumber(L, 4, -1);

    if (alpha < 0)
        cp = cairo_pattern_create_rgb (red, green, blue);
    else
        cp = cairo_pattern_create_rgba (red, green, blue, alpha);

    return new_Pattern(L, LUACAIRO ".SolidPattern.mt", cp, CAIRO_PATTERN_TYPE_SOLID, 1);
}

static const struct luaL_Reg SolidPattern_lib_f [] = {
    {"new", new_SolidPattern},
    {NULL, NULL}
};

static const struct luaL_Reg SolidPattern_lib_m [] = {
    {"__gc",                                     gc_Pattern},
    {"__tostring",                               tostring_Pattern},

  //{"create_rgb",                               l_cairo_pattern_create_rgb},
  //{"create_rgba",                              l_cairo_pattern_create_rgba},
  //{"create_for_surface",                       l_cairo_pattern_create_for_surface},
  //{"create_linear",                            l_cairo_pattern_create_linear},
  //{"create_radial",                            l_cairo_pattern_create_radial},
    {"reference",                                l_cairo_pattern_reference},
    {"destroy",                                  gc_Pattern},
    {"get_reference_count",                      l_cairo_pattern_get_reference_count},
    {"status",                                   l_cairo_pattern_status},
    {"get_user_data",                            l_cairo_pattern_get_user_data},
    {"set_user_data",                            l_cairo_pattern_set_user_data},
    {"get_type",                                 l_cairo_pattern_get_type},
  //{"add_color_stop_rgb",                       l_cairo_pattern_add_color_stop_rgb},
  //{"add_color_stop_rgba",                      l_cairo_pattern_add_color_stop_rgba},
    {"set_matrix",                               l_cairo_pattern_set_matrix},
    {"get_matrix",                               l_cairo_pattern_get_matrix},
  //{"set_extend",                               l_cairo_pattern_set_extend},
  //{"get_extend",                               l_cairo_pattern_get_extend},
  //{"set_filter",                               l_cairo_pattern_set_filter},
  //{"get_filter",                               l_cairo_pattern_get_filter},
    {"get_rgba",                                 l_cairo_pattern_get_rgba},
  //{"get_surface",                              l_cairo_pattern_get_surface},
  //{"get_color_stop_rgba",                      l_cairo_pattern_get_color_stop_rgba},
  //{"get_color_stop_count",                     l_cairo_pattern_get_color_stop_count},
  //{"get_linear_points",                        l_cairo_pattern_get_linear_points},
  //{"get_radial_circles",                       l_cairo_pattern_get_radial_circles},

    {NULL, NULL}
};


static int luaopen_lSolidPattern (lua_State *L)
{
    luaL_newmetatable(L, LUACAIRO ".SolidPattern.mt");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
	luaL_newlib(L, SolidPattern_lib_m);
    //luaL_register(L, NULL, SolidPattern_lib_m);
	luaL_newlib(L, SolidPattern_lib_f);
	lua_setglobal(L, "SolidPattern");
    //luaL_register(L, LUACAIRO ".SolidPattern", SolidPattern_lib_f);
    lua_newtable(L);
    lua_pushstring(L, "__call");
    lua_pushcfunction(L, new_SolidPattern);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
    //lua_pop(L, 1);
    return 0;
}


#endif  /* LUACAIRO_HAS_OO_INTERFACE */



static int luaopen_lcairo_ud_solid_pattern (lua_State* L)
{
#if LUACAIRO_HAS_OO_INTERFACE

    luaopen_lSolidPattern (L);

#endif  /* LUACAIRO_HAS_OO_INTERFACE */
    return 0;
}
