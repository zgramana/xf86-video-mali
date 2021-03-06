/*
 * Copyright (C) 2010 ARM Limited. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/ioctl.h>
#include "xorg-server.h"
#include "xf86.h"
#include "xf86Crtc.h"

#include "mali_def.h"
#include "mali_fbdev.h"
#include "mali_lcd.h"
extern void MaliHWSave(ScrnInfoPtr pScrn);

static void fbdev_lcd_crtc_dpms(xf86CrtcPtr crtc, int mode)
{
	IGNORE(crtc);
	IGNORE(mode);
}

static Bool fbdev_lcd_crtc_lock(xf86CrtcPtr crtc)
{
	IGNORE( crtc );

	return TRUE;
}

static void fbdev_lcd_crtc_unlock(xf86CrtcPtr crtc)
{
	IGNORE( crtc );
}

static Bool fbdev_lcd_crtc_mode_fixup(xf86CrtcPtr crtc, DisplayModePtr mode, DisplayModePtr adjusted_mode)
{
	IGNORE( crtc );
	IGNORE( mode );
	IGNORE( adjusted_mode );

	return TRUE;
}

static void fbdev_lcd_crtc_prepare(xf86CrtcPtr crtc)
{
	IGNORE( crtc );
}

static void fbdev_lcd_crtc_mode_set(xf86CrtcPtr crtc, DisplayModePtr mode, DisplayModePtr adjusted_mode, int x, int y)
{
	IGNORE( crtc );
	IGNORE( mode );
	IGNORE( adjusted_mode );
	IGNORE( x );
	IGNORE( y );
}

static void fbdev_lcd_crtc_commit(xf86CrtcPtr crtc)
{
	IGNORE( crtc );
}

static void fbdev_lcd_crtc_gamma_set(xf86CrtcPtr crtc, CARD16 *red, CARD16 *green, CARD16 *blue, int size)
{
	IGNORE( crtc );
	IGNORE( red );
	IGNORE( green );
	IGNORE( blue );
	IGNORE( size );
}

static void fbdev_lcd_crtc_set_origin(xf86CrtcPtr crtc, int x, int y)
{
	IGNORE( crtc );
	IGNORE( x );
	IGNORE( y );
}

static const xf86CrtcFuncsRec fbdev_lcd_crtc_funcs = 
{
	.dpms = fbdev_lcd_crtc_dpms,
	.save = NULL, 
	.restore = NULL,
	.lock = fbdev_lcd_crtc_lock,
	.unlock = fbdev_lcd_crtc_unlock,
	.mode_fixup = fbdev_lcd_crtc_mode_fixup,
	.prepare = fbdev_lcd_crtc_prepare,
	.mode_set = fbdev_lcd_crtc_mode_set,
	.commit = fbdev_lcd_crtc_commit,
	.gamma_set = fbdev_lcd_crtc_gamma_set,
	.shadow_allocate = NULL, 
	.shadow_create = NULL,
	.shadow_destroy = NULL,
	.set_cursor_colors = NULL,
	.set_cursor_position = NULL,
	.show_cursor = NULL,
	.hide_cursor = NULL,
	.load_cursor_image = NULL,
	.load_cursor_argb = NULL,
	.destroy = NULL,
	.set_mode_major = NULL,
	.set_origin = fbdev_lcd_crtc_set_origin,
};

static void fbdev_lcd_output_dpms(xf86OutputPtr output, int mode)
{
	MaliPtr fPtr = MALIPTR(output->scrn);

	if ( mode == DPMSModeOn ) 
	{
		ioctl(fPtr->fb_lcd_fd, FBIOBLANK, FB_BLANK_UNBLANK);
	}
	else if( mode == DPMSModeOff )
	{
		ioctl(fPtr->fb_lcd_fd, FBIOBLANK, 1/* FB_BLANK_POWERDOWN */);
	}
}

static void fbdev_lcd_output_save(xf86OutputPtr output)
{
	IGNORE( output );
}

static void fbdev_lcd_output_restore(xf86OutputPtr output)
{
	IGNORE( output );
}

static int fbdev_lcd_output_mode_valid(xf86OutputPtr output, DisplayModePtr pMode)
{
	IGNORE( output );
	IGNORE( pMode );

	/* TODO: return MODE_ERROR in case of unsupported mode */
//	xf86DrvMsg(0, X_INFO, "Mode %i x %i valid\n", pMode->HDisplay, pMode->VDisplay );

	return MODE_OK;
}

static Bool fbdev_lcd_output_mode_fixup(xf86OutputPtr output, DisplayModePtr mode, DisplayModePtr adjusted_mode)
{
	IGNORE( output );
	IGNORE( mode );
	IGNORE( adjusted_mode );

	return TRUE;
}

static void fbdev_lcd_output_prepare(xf86OutputPtr output)
{
	IGNORE( output );
}

static void fbdev_lcd_output_commit(xf86OutputPtr output)
{
	output->funcs->dpms(output, DPMSModeOn);
}

static void fbdev_lcd_output_mode_set(xf86OutputPtr output, DisplayModePtr mode, DisplayModePtr adjusted_mode)
{
	MaliPtr fPtr = MALIPTR(output->scrn);

	IGNORE( output );
	IGNORE( mode );
	IGNORE( adjusted_mode );

	if ( ioctl( fPtr->fb_lcd_fd, FBIOGET_VSCREENINFO, &fPtr->fb_lcd_var ) < 0 )
	{
		xf86DrvMsg(0, X_INFO, "Unable to get VSCREENINFO\n");
	}

	fPtr->fb_lcd_var.xres = mode->HDisplay;
	fPtr->fb_lcd_var.yres = mode->VDisplay;
	fPtr->fb_lcd_var.xres_virtual = mode->HDisplay;
	fPtr->fb_lcd_var.yres_virtual = mode->VDisplay*2;
// add by liaods, from ivtv/ivtvhw.c
	fPtr->fb_lcd_var.xoffset = fPtr->fb_lcd_var.yoffset = 0;
	fPtr->fb_lcd_var.pixclock = mode->Clock ? 1000000000 / mode->Clock : 0;
	fPtr->fb_lcd_var.right_margin = mode->HSyncStart - mode->HDisplay;
	fPtr->fb_lcd_var.hsync_len = mode->HSyncEnd - mode->HSyncStart;
	fPtr->fb_lcd_var.left_margin = mode->HTotal - mode->HSyncEnd;
	fPtr->fb_lcd_var.lower_margin = mode->VSyncStart - mode->VDisplay;
	fPtr->fb_lcd_var.vsync_len = mode->VSyncEnd - mode->VSyncStart;
	fPtr->fb_lcd_var.upper_margin = mode->VTotal - mode->VSyncEnd;
	fPtr->fb_lcd_var.sync = 0;
	if (mode->Flags & V_PHSYNC)
		fPtr->fb_lcd_var.sync |= FB_SYNC_HOR_HIGH_ACT;
	if (mode->Flags & V_PVSYNC)
		fPtr->fb_lcd_var.sync |= FB_SYNC_VERT_HIGH_ACT;
	if (mode->Flags & V_PCSYNC)
		fPtr->fb_lcd_var.sync |= FB_SYNC_COMP_HIGH_ACT;
#if 1		/* Badly needed for PAL/NTSC on Amiga (amifb)!! [geert] */
	if (mode->Flags & V_BCAST)
		fPtr->fb_lcd_var.sync |= FB_SYNC_BROADCAST;
#endif
	if (mode->Flags & V_INTERLACE)
		fPtr->fb_lcd_var.vmode = FB_VMODE_INTERLACED;
	else if (mode->Flags & V_DBLSCAN)
		fPtr->fb_lcd_var.vmode = FB_VMODE_DOUBLE;
	else
		fPtr->fb_lcd_var.vmode = FB_VMODE_NONINTERLACED;
	
	xf86DrvMsg(0, X_INFO, "clk=%d, l=%d, r=%d, u=%d, l=%d, h=%d, v=%d, vmode=%d, sync=%d\n", 
		fPtr->fb_lcd_var.pixclock, fPtr->fb_lcd_var.left_margin, fPtr->fb_lcd_var.right_margin, 
		fPtr->fb_lcd_var.upper_margin, fPtr->fb_lcd_var.lower_margin,
		fPtr->fb_lcd_var.hsync_len, fPtr->fb_lcd_var.vsync_len,
		fPtr->fb_lcd_var.vmode, fPtr->fb_lcd_var.sync);
// add end

	xf86DrvMsg(0, X_INFO, "Changing mode to %i %i %i %i\n", fPtr->fb_lcd_var.xres, fPtr->fb_lcd_var.yres, fPtr->fb_lcd_var.xres_virtual, fPtr->fb_lcd_var.yres_virtual);

	if ( ioctl( fPtr->fb_lcd_fd, FBIOPUT_VSCREENINFO, &fPtr->fb_lcd_var ) < 0 )
	{
		xf86DrvMsg(0, X_INFO, "Unable to set mode!\n");
	}
	else
		MaliHWSave(output->scrn);
	system("/usr/bin/set_window.sh 2000000&");

}

static xf86OutputStatus fbdev_lcd_output_detect(xf86OutputPtr output)
{
	IGNORE( output );

	return XF86OutputStatusConnected;
}

DisplayModePtr fbdev_make_mode( int xres, int yres, DisplayModePtr prev )
{
	DisplayModePtr mode_ptr;
	unsigned int hactive_s = xres;
	unsigned int vactive_s = yres;
	return NULL; // by liaods, do not allow to make mode

	mode_ptr = xnfcalloc(1, sizeof(DisplayModeRec));

	mode_ptr->HDisplay = hactive_s;
	mode_ptr->HSyncStart = hactive_s + 20;
	mode_ptr->HSyncEnd = hactive_s + 40;
	mode_ptr->HTotal = hactive_s + 80;

	mode_ptr->VDisplay = vactive_s;
	mode_ptr->VSyncStart = vactive_s + 20;
	mode_ptr->VSyncEnd = vactive_s + 40;
	mode_ptr->VTotal = vactive_s + 80;

	mode_ptr->VRefresh = 60.0;

	mode_ptr->Clock = (int) (mode_ptr->VRefresh * mode_ptr->VTotal * mode_ptr->HTotal / 1000.0);

	mode_ptr->type = M_T_DRIVER;

	xf86SetModeDefaultName(mode_ptr);
		
	mode_ptr->next = NULL;
	mode_ptr->prev = prev;

	return mode_ptr;
}

static DisplayModePtr fbdev_lcd_output_get_modes(xf86OutputPtr output)
{
	MaliPtr fPtr = MALIPTR(output->scrn);
	DisplayModePtr mode_ptr;
	ScrnInfoPtr pScrn = output->scrn;

	unsigned int hactive_s = fPtr->fb_lcd_var.xres;
	unsigned int vactive_s = fPtr->fb_lcd_var.yres;

	if ( pScrn->modes != NULL )
	{
		/* Use the modes supplied by the implementation if available */
		DisplayModePtr mode, first = mode = pScrn->modes;
		DisplayModePtr modeptr = NULL, modeptr_prev = NULL, modeptr_first = NULL;
		do {
			int xres = mode->HDisplay;
			int yres = mode->VDisplay;

			//xf86DrvMsg(0, X_INFO, "Adding mode: %i x %i\n", xres, yres);

			if ( modeptr_first == NULL ) 
			{
				modeptr_first = fbdev_make_mode( xres, yres, NULL );
				modeptr = modeptr_first;
			}
			else
			{
				modeptr->next = fbdev_make_mode( xres, yres, modeptr_prev);
				modeptr = modeptr->next;
			}
			modeptr_prev = modeptr;

			mode = mode->next;
		} while (mode != NULL && mode != first);

		return modeptr_first;
	}

	mode_ptr = xnfcalloc(1, sizeof(DisplayModeRec));

	mode_ptr->HDisplay = hactive_s;
	mode_ptr->HSyncStart = hactive_s + 20;
	mode_ptr->HSyncEnd = hactive_s + 40;
	mode_ptr->HTotal = hactive_s + 80;

	mode_ptr->VDisplay = vactive_s;
	mode_ptr->VSyncStart = vactive_s + 20;
	mode_ptr->VSyncEnd = vactive_s + 40;
	mode_ptr->VTotal = vactive_s + 80;

	mode_ptr->VRefresh = 60.0;

	mode_ptr->Clock = (int) (mode_ptr->VRefresh * mode_ptr->VTotal * mode_ptr->HTotal / 1000.0);

	mode_ptr->type = M_T_DRIVER;

	xf86SetModeDefaultName(mode_ptr);
		
	mode_ptr->next = NULL;
	mode_ptr->prev = NULL;

	return mode_ptr;
}

#ifdef RANDR_GET_CRTC_INTERFACE
static xf86CrtcPtr fbdev_lcd_output_get_crtc(xf86OutputPtr output)
{
	return output->crtc;
}
#endif

static void fbdev_lcd_output_destroy(xf86OutputPtr output)
{
	IGNORE( output );
}

static const xf86OutputFuncsRec fbdev_lcd_output_funcs = 
{
	.create_resources = NULL,
	.dpms = fbdev_lcd_output_dpms,
	.save = fbdev_lcd_output_save,
	.restore = fbdev_lcd_output_restore,
	.mode_valid = fbdev_lcd_output_mode_valid,
	.mode_fixup = fbdev_lcd_output_mode_fixup,
	.prepare = fbdev_lcd_output_prepare,
	.commit = fbdev_lcd_output_commit,
	.mode_set = fbdev_lcd_output_mode_set,
	.detect = fbdev_lcd_output_detect,
	.get_modes = fbdev_lcd_output_get_modes,
#ifdef RANDR_12_INTERFACE
	.set_property = NULL,
#endif
#ifdef RANDR_13_INTERFACE
	.get_property = NULL,
#endif
#ifdef RANDR_GET_CRTC_INTERFACE
	.get_crtc = fbdev_lcd_output_get_crtc,
#endif
	.destroy = fbdev_lcd_output_destroy,
};


Bool FBDEV_lcd_init(ScrnInfoPtr pScrn)
{
	xf86CrtcPtr crtc;
	xf86OutputPtr output;

	crtc = xf86CrtcCreate(pScrn, &fbdev_lcd_crtc_funcs);

	if(crtc == NULL) return FALSE;

	output = xf86OutputCreate(pScrn, &fbdev_lcd_output_funcs, "LCD");

	if(output == NULL) return FALSE;

	output->possible_crtcs = (1 << 0);

	return TRUE;
}
