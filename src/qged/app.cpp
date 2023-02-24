/*                      C A D A P P . C X X
 * BRL-CAD
 *
 * Copyright (c) 2014-2023 United States Government as represented by
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
/** @file cadapp.cxx
 *
 * Application level data and functionality implementations.
 *
 */

#include <set>
#include <unordered_set>
#include <QFileInfo>
#include <QFile>
#include <QPlainTextEdit>
#include <QTextStream>
#include "bu/malloc.h"
#include "bu/file.h"
#include "qtcad/QtAppExecDialog.h"
#include "qtcad/QgTreeSelectionModel.h"
#include "app.h"
#include "fbserv.h"

extern "C" int
app_close(void *p, int UNUSED(argc), const char **UNUSED(argv))
{
    CADApp *ap = (CADApp *)p;
    QtConsole *console = ap->w->console;
    ap->closedb();
    QtCADQuad *c4= ap->w->c4;
    for (int i = 1; i < 5; i++) {
	QtCADView *c = c4->get(i);
	//c->dm_set = NULL;
	c->set_view(NULL);
	c->set_dm_current(NULL);
	c->set_base2local(1);
	c->set_local2base(1);
    }
    if (console)
	console->printString("closed database\n");

    return 0;
}

extern "C" int
app_man(void *ip, int argc, const char **argv)
{
    CADApp *ap = (CADApp *)ip;
    QtConsole *console = ap->w->console;
    int bac = (argc > 1) ? 5 : 4;
    const char *bav[6];
    char brlman[MAXPATHLEN] = {0};
    bu_dir(brlman, MAXPATHLEN, BU_DIR_BIN, "brlman", BU_DIR_EXT, NULL);
    bav[0] = (const char *)brlman;
    bav[1] = "-g";
    bav[2] = "-S";
    bav[3] = "n";
    bav[4] = (argc > 1) ? argv[1] : NULL;
    bav[5] = NULL;
    struct bu_process *p = NULL;
    bu_process_exec(&p, bav[0], bac, (const char **)bav, 0, 0);
    if (bu_process_pid(p) == -1 && console) {
	console->printString("Failed to launch man page viewer\n") ;
	return -1;
    }
    return 0;
}

void
CADApp::initialize()
{
    // TODO - eventually, load these as plugins
    //app_cmd_map[QString("open")] = &app_open;
    app_cmd_map[QString("close")] = &app_close;
    app_cmd_map[QString("man")] = &app_man;
}

void
CADApp::do_quad_view_change(QtCADView *cv)
{
    QTCAD_SLOT("CADApp::do_quad_view_change", 1);
    mdl->gedp->ged_gvp = cv->view();
    emit view_update(QTCAD_VIEW_REFRESH);
}

void
CADApp::do_view_changed(unsigned long long flags)
{
    bv_log(1, "CADApp::do_view_changed");
    QTCAD_SLOT("CADApp::do_view_changed", 1);

    if (flags & QTCAD_VIEW_DRAWN) {
	// For all associated view states, execute any necessary changes to
	// view objects and lists
	std::unordered_map<BViewState *, std::unordered_set<struct bview *>> vmap;
	struct bu_ptbl *views = bv_set_views(&mdl->gedp->ged_views);
	if (mdl->gedp->dbi_state) {
	    for (size_t i = 0; i < BU_PTBL_LEN(views); i++) {
		struct bview *v = (struct bview *)BU_PTBL_GET(views, i);
		BViewState *bvs = mdl->gedp->dbi_state->get_view_state(v);
		if (!bvs)
		    continue;
		vmap[bvs].insert(v);
	    }
	    std::unordered_map<BViewState *, std::unordered_set<struct bview *>>::iterator bv_it;
	    for (bv_it = vmap.begin(); bv_it != vmap.end(); bv_it++) {
		bv_it->first->redraw(NULL, bv_it->second, 1);
	    }
	}
    }

    emit view_update(flags);
}

void
CADApp::tree_update()
{
    QTCAD_SLOT("CADApp::tree_update", 1);
    if (!w)
	return;
    CADPalette *v = NULL;
    CADPalette *vc = w->vc;
    CADPalette *oc = w->oc;

    switch (mdl->interaction_mode) {
	case 0:
	    v = vc;
	    break;
	case 2:
	    v = oc;
	    break;
	default:
	    v = NULL;
	    break;
    }
    if (!v)
	return;
    vc->makeCurrent(v);
    oc->makeCurrent(v);
}

void
CADApp::open_file()
{
    QTCAD_SLOT("CADApp::open_file", 1);
    const char *file_filters = "BRL-CAD (*.g *.asc);;Rhino (*.3dm);;STEP (*.stp *.step);;All Files (*)";
    QString fileName = QFileDialog::getOpenFileName((QWidget *)this->w,
	    "Open Geometry File",
	    applicationDirPath(),
	    file_filters,
	    NULL,
	    QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty())
	return;

    int ac = 2;
    const char *av[3];
    av[0] = "open";
    av[1] = bu_strdup(fileName.toLocal8Bit().data());
    av[2] = NULL;
    int ret = mdl->run_cmd(mdl->gedp->ged_result_str, ac, (const char **)av);
    bu_free((void *)av[1], "filename cpy");
    if (!w)
	return;

    if (ret) {
	w->statusBar()->showMessage("open failed");
    } else {
	w->statusBar()->showMessage(fileName);
    }
    // Let the shell's completer know what the current gedp is (if any)
    w->cshellcomp->gedp = mdl->gedp;
}


void
CADApp::closedb()
{
    delete mdl;
    db_filename.clear();
}


int
qged_view_update(struct ged *gedp)
{
    int view_flags = 0;

    if (!gedp->dbi_state)
	return view_flags;

    unsigned long long updated = gedp->dbi_state->update();
    if (updated & GED_DBISTATE_VIEW_CHANGE)
	view_flags |= QTCAD_VIEW_DRAWN;

    return view_flags;
}

extern "C" void
raytrace_start(int val, void *ctx)
{
    CADApp *ap = (CADApp *)ctx;
    ap->w->vcw->raytrace_start(val);
}

extern "C" void
raytrace_done(int val, void *ctx)
{
    CADApp *ap = (CADApp *)ctx;
    ap->w->vcw->raytrace_done(val);
}

int
CADApp::run_cmd(struct bu_vls *msg, int argc, const char **argv)
{
    int ret = BRLCAD_OK;
    int view_flags = 0;

    if (!mdl || !argc || !argv)
	return BRLCAD_ERROR;

    struct ged *gedp = mdl->gedp;

    BSelectState *ss = (gedp->dbi_state) ? gedp->dbi_state->find_selected_state(NULL) : NULL;
    select_hash = (ss) ? ss->state_hash() : 0;

    /* Set the local unit conversions */
    if (gedp->dbip) {
	for (int i = 1; i < 5; i++) {
	    QtCADView *c = w->c4->get(i);
	    c->set_base2local(gedp->dbip->dbi_base2local);
	    c->set_local2base(gedp->dbip->dbi_local2base);
	}
    }

    if (!tmp_av.size()) {

	// If we're not in the middle of an incremental command,
	// stash the view state(s) for later comparison and make
	// sure our unit conversions are right
	w->c4->stash_hashes();
	//select_hash = ged_selection_hash_sets(gedp->ged_selection_sets);

	// If we need command-specific subprocess awareness for
	// a command, set it up
	if (BU_STR_EQUAL(argv[0], "ert")) {
	    gedp->ged_subprocess_init_callback = &raytrace_start;
	    gedp->ged_subprocess_end_callback = &raytrace_done;
	    gedp->ged_subprocess_clbk_context = (void *)this;
	}

	// Ask the model to execute the command
	ret = mdl->run_cmd(msg, argc, argv);

	gedp->ged_subprocess_init_callback = NULL;
	gedp->ged_subprocess_end_callback = NULL;
	gedp->ged_subprocess_clbk_context = NULL;

    } else {
	for (int i = 0; i < argc; i++) {
	    char *tstr = bu_strdup(argv[i]);
	    tmp_av.push_back(tstr);
	}
	char **av = (char **)bu_calloc(tmp_av.size() + 1, sizeof(char *), "argv array");
	// Assemble the full command we have thus var
	for (size_t i = 0; i < tmp_av.size(); i++) {
	    av[i] = tmp_av[i];
	}
	int ac = (int)tmp_av.size();
	ret = mdl->run_cmd(msg, ac, (const char **)av);
    }

    if (!(ret & GED_MORE)) {

	// Handle any necessary redrawing.
	view_flags = qged_view_update(gedp);

	/* Check if the ged_exec call changed either the display manager or
	 * the view settings - in either case we'll need to redraw */
	// TODO - there would be some utility in checking only the camera or only
	// the who list, since we can set different update flags for each case...
	if (w->c4->diff_hashes())
	    view_flags |= QTCAD_VIEW_DRAWN;

	unsigned long long cs_hash = (ss) ? ss->state_hash() : 0;
	if (cs_hash != select_hash) {
	    view_flags |= QTCAD_VIEW_SELECT;
	    // This is what notifies currently drawn solids to update
	    // in response to a command line selection change
	    if (ss->draw_sync())
		view_flags |= QTCAD_VIEW_DRAWN;
	}
    }

    if (ret & GED_MORE) {
	// If this is the first time through stash in tmp_av, since we
	// didn't know to do it above
	if (!tmp_av.size()) {
	    for (int i = 0; i < argc; i++) {
		char *tstr = bu_strdup(argv[i]);
		tmp_av.push_back(tstr);
	    }
	    QtConsole *console = w->console;
	    if (console)
		history_mark_start = console->historyCount() - 2;
	}
    } else {
	// If we were in an incremental command, we're done now -
	if (tmp_av.size()) {
	    // clear tmp_av
	    for (size_t i = 0; i < tmp_av.size(); i++) {
		delete tmp_av[i];
	    }
	    tmp_av.clear();
	    // let the console know that we're done with MORE
	    QtConsole *console = w->console;
	    if (console)
		history_mark_end = console->historyCount() - 1;
	}
    }

    // TODO - should be able to emit this regardless - if execution methods
    // are well behaved, they'll just ignore a zero flags value...
    if (view_flags)
	emit view_update(view_flags);

    return ret;
}

void
CADApp::run_qcmd(const QString &command)
{
    QTCAD_SLOT("CADApp::run_qcmd", 1);
    if (!w)
	return;

    QtConsole *console = w->console;
    const char *cmd = bu_strdup(command.toLocal8Bit().data());

    if (BU_STR_EQUAL(cmd, "q"))
	bu_exit(0, "exit");

    if (BU_STR_EQUAL(cmd, "clear")) {
	if (console) {
	    console->clear();
	    console->prompt("$ ");
	}
	bu_free((void *)cmd, "cmd");
	return;
    }

    if (!mdl) {
	bu_free((void *)cmd, "cmd");
	return;
    }

    // make an argv array
    struct bu_vls ged_prefixed = BU_VLS_INIT_ZERO;
    bu_vls_sprintf(&ged_prefixed, "%s", cmd);
    char *input = bu_strdup(bu_vls_addr(&ged_prefixed));
    bu_vls_free(&ged_prefixed);
    char **av = (char **)bu_calloc(strlen(input) + 1, sizeof(char *), "argv array");
    int ac = bu_argv_from_string(av, strlen(input), input);
    struct bu_vls msg = BU_VLS_INIT_ZERO;

    // First, see if we have an application level command.
    int cmd_run = 0;
    if (app_cmd_map.contains(QString(av[0]))) {
	app_cmd_ptr acmd = app_cmd_map.value(QString(av[0]));
	(*acmd)((void *)this, ac, (const char **)av);
	cmd_run = 1;
    }

    // If it wasn't an app level command, try it as a GED command.
    int ret = BRLCAD_OK;
    if (!cmd_run) {
	ret = run_cmd(&msg, ac, (const char **)av);
	if (bu_vls_strlen(&msg) > 0 && console) {
	    console->printString(bu_vls_cstr(&msg));
	}
    }

    if (console) {
	if (ret & GED_MORE) {
	    console->prompt(bu_vls_cstr(mdl->gedp->ged_result_str));
	} else {
	    console->prompt("$ ");
	    if (history_mark_start >= 0 && history_mark_end >= 0) {
		console->consolidateHistory(history_mark_start, history_mark_end);
		history_mark_start = -1;
		history_mark_end = -1;
	    }
	}
    }

    if (mdl->gedp) {
	bu_vls_trunc(mdl->gedp->ged_result_str, 0);
    }

    bu_free((void *)cmd, "cmd");
    bu_vls_free(&msg);
    bu_free(input, "input copy");
    bu_free(av, "input argv");
}

void
CADApp::element_selected(QToolPaletteElement *el)
{
    QTCAD_SLOT("CADApp::element_selected", 1);
    if (!el->controls->isVisible()) {
	// Apparently this can happen when we have docked widgets
	// closed and we click on the border between the view and
	// the dock - need to avoid messing with the event filters
	// if that happens or we break the user interactions.
	return;
    }

    if (curr_view->curr_event_filter) {
	curr_view->clear_event_filter(curr_view->curr_event_filter);
	curr_view->curr_event_filter = NULL;
    }

    if (el->use_event_filter) {
	curr_view->add_event_filter(el->controls);
	curr_view->curr_event_filter = el->controls;
    }
    if (curr_view->view()) {
	curr_view->view()->gv_width = curr_view->width();
	curr_view->view()->gv_height = curr_view->height();
    }
}

int
CADApp::exec_console_app_in_window(QString command, QStringList options, QString lfile)
{
    if (command.length() > 0) {

	QtAppExecDialog *out_win = new QtAppExecDialog(0, command, options, lfile);
	QString win_title("Running ");
	win_title.append(command);
	out_win->setWindowTitle(win_title);
	out_win->proc = new QProcess(out_win);
	out_win->console->setMinimumHeight(800);
	out_win->console->setMinimumWidth(800);
	out_win->console->printString(command);
	out_win->console->printString(QString(" "));
	out_win->console->printString(options.join(" "));
	out_win->console->printString(QString("\n"));
	out_win->proc->setProgram(command);
	out_win->proc->setArguments(options);
	connect(out_win->proc, &QProcess::readyReadStandardOutput, out_win, &QtAppExecDialog::read_stdout);
	connect(out_win->proc, &QProcess::readyReadStandardError, out_win, &QtAppExecDialog::read_stderr);
	connect(out_win->proc, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), out_win, &QtAppExecDialog::process_done);
	out_win->proc->start();
	out_win->exec();
    }
    return 0;
}

void
CADApp::readSettings()
{
    QTCAD_SLOT("CADApp::readSettings", 1);
    QSettings settings("BRL-CAD", "QGED");

    settings.beginGroup("BRLCAD_MainWindow");
    if (w)
	w->resize(settings.value("size", QSize(1100, 800)).toSize());
    settings.endGroup();
}

void
CADApp::write_settings()
{
    QTCAD_SLOT("CADApp::write_settings", 1);
    QSettings settings("BRL-CAD", "QGED");

    if (w) {
	settings.beginGroup("BRLCAD_MainWindow");
	settings.setValue("size", w->size());
	settings.endGroup();
    }
}

void
CADApp::switch_to_single_view()
{
    QTCAD_SLOT("CADApp::switch_to_single_view", 1);
    w->c4->changeToSingleFrame();
}

void
CADApp::switch_to_quad_view()
{
    QTCAD_SLOT("CADApp::switch_to_quad_view", 1);
    w->c4->changeToQuadFrame();
}

/*
 * Local Variables:
 * mode: C++
 * tab-width: 8
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * c-file-style: "stroustrup"
 * End:
 * ex: shiftwidth=4 tabstop=8
 */

