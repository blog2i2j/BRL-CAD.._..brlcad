/*                         V I E W 2 G R I D _ L U . C
 * BRL-CAD
 *
 * Copyright (c) 2008-2025 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file libged/view2grid_lu.c
 *
 * The view2grid_lu command.
 *
 */

#include "common.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../ged_private.h"


int
ged_view2grid_lu_core(struct ged *gedp, int argc, const char *argv[])
{
    /* intentionally double for scan */
    double view_pt[3];

    fastf_t f;
    point_t model_pt = VINIT_ZERO;
    point_t mo_view_pt;           /* model origin in view space */
    point_t diff;
    static const char *usage = "x y z";
    double b2lval = (gedp->dbip) ? gedp->dbip->dbi_base2local : 1.0;

    GED_CHECK_DATABASE_OPEN(gedp, BRLCAD_ERROR);
    GED_CHECK_VIEW(gedp, BRLCAD_ERROR);
    GED_CHECK_ARGC_GT_0(gedp, argc, BRLCAD_ERROR);

    /* initialize result */
    bu_vls_trunc(gedp->ged_result_str, 0);

    if (argc != 4)
	goto bad;

    if (sscanf(argv[1], "%lf", &view_pt[X]) != 1 ||
	sscanf(argv[2], "%lf", &view_pt[Y]) != 1 ||
	sscanf(argv[3], "%lf", &view_pt[Z]) != 1)
	goto bad;

    MAT4X3PNT(mo_view_pt, gedp->ged_gvp->gv_model2view, model_pt);
    f = gedp->ged_gvp->gv_scale * b2lval;
    VSCALE(mo_view_pt, mo_view_pt, f);
    VSUB2(diff, view_pt, mo_view_pt);

    bu_vls_printf(gedp->ged_result_str, "%.15e %.15e", diff[X], diff[Y]);

    return BRLCAD_OK;

bad:
    bu_vls_printf(gedp->ged_result_str, "Usage: %s %s", argv[0], usage);
    return BRLCAD_ERROR;
}


#ifdef GED_PLUGIN
#include "../include/plugin.h"
struct ged_cmd_impl view2grid_lu_cmd_impl = {
    "view2grid_lu",
    ged_view2grid_lu_core,
    GED_CMD_DEFAULT
};

const struct ged_cmd view2grid_lu_cmd = { &view2grid_lu_cmd_impl };
const struct ged_cmd *view2grid_lu_cmds[] = { &view2grid_lu_cmd, NULL };

static const struct ged_plugin pinfo = { GED_API,  view2grid_lu_cmds, 1 };

COMPILER_DLLEXPORT const struct ged_plugin *ged_plugin_info(void)
{
    return &pinfo;
}
#endif /* GED_PLUGIN */

/*
 * Local Variables:
 * mode: C
 * tab-width: 8
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
