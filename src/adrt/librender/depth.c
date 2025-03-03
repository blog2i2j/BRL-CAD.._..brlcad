/*                         D E P T H . C
 * BRL-CAD / ADRT
 *
 * Copyright (c) 2007-2025 United States Government as represented by
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
/** @file librender/depth.c
 *
 */

#include "adrt_struct.h"
#include <stdio.h>


void
render_depth_free(render_t *UNUSED(render))
{
    return;
}


void
render_depth_work(render_t *UNUSED(render), struct tie_s *tie, struct tie_ray_s *ray, vect_t *pixel)
{
    struct tie_id_s id;

    /* Visualize ray depth, must put ray->depth++ hack into bsp for this to be of any use */
    if (TIE_WORK(tie, ray, &id, render_hit, NULL) != NULL)
	*pixel[0] = 0.0075 * ray->kdtree_depth;
}


int
render_depth_init(render_t *render, const char *UNUSED(usr))
{
    render->work = render_depth_work;
    render->free = render_depth_free;
    return 0;
}


/*
 * Local Variables:
 * mode: C
 * tab-width: 8
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */
