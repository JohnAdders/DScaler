/////////////////////////////////////////////////////////////////////////////
// AspectRect.hpp
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2000 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//	This file is subject to the terms of the GNU General Public License as
//	published by the Free Software Foundation.  A copy of this license is
//	included with this software distribution in the file COPYING.  If you
//	do not have a copy, you may obtain a copy by writing to the Free
//	Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//	This software is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////
//
// This software was based on Multidec 5.6 Those portions are
// Copyright (C) 1999/2000 Espresso (echter_espresso@hotmail.com)
//
/////////////////////////////////////////////////////////////////////////////
// Change Log
//
// Date          Developer             Changes
//
// 14 Mar 2001   Michael Samblanet     File created
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __aspectrect_hpp__
#define __aspectrect_hpp__

// Aspect aware smart-rectangle
// All function in-line for efficency when compiled...
class AspectRect : public RECT {
	protected:
		// Adjustment made to aspect ratio on output usage
		// For example, using a 4:3 resolution with a anamorphic (16:9) lens would have an output
		// adjustment of (16/9)/(4/3) = 1.3333
		// An anamorphic source would be (4/3)/(16/9) = .75
		double m_outputAdjustment;

	public:
		AspectRect() { left = right = top = bottom = 0; m_outputAdjustment = 1; }
		AspectRect(RECT const &src) { left = src.left; right = src.right; top = src.top; bottom = src.bottom; m_outputAdjustment = 1; }
		AspectRect(AspectRect const &src) { operator=(src); }
		AspectRect& operator=(AspectRect const &src) {
			if (&src != this) {
				left = src.left; right = src.right; top = src.top; bottom = src.bottom;
				m_outputAdjustment = src.m_outputAdjustment;
			}
			return *this;
		}
		// Only considers exact matches equal...
		BOOL operator==(const AspectRect &src) {
			return left==src.left && right==src.right && top==src.top && bottom==src.bottom
				&& fabs(m_outputAdjustment-src.m_outputAdjustment) < .00001;
		}
		// Considers 2 rectangles equal if they are within n pixels of each other on each edge
		BOOL tolerantEquals(const AspectRect &src, int tolerance = 4) {
			if (fabs(m_outputAdjustment-src.m_outputAdjustment) > .00001) return false;
			if (abs(left - src.left) > tolerance) return false;
			if (abs(right - src.right) > tolerance) return false;
			if (abs(top - src.top) > tolerance) return false;
			if (abs(bottom - src.bottom) > tolerance) return false;
			return true;
		}

		int width() { return right - left; }
		int height() { return bottom - top; }
		double sourceAspect() { return (double)width()/(double)height(); }
		double targetAspect() { return sourceAspect() * m_outputAdjustment; }
		void setTargetAspect(double target) { m_outputAdjustment = target / sourceAspect(); }
		void setAspectAdjust(double source, double target) { m_outputAdjustment = target/source; }

		void normalizeRect() {
			// Ensure left and top are less than bottom and right.
			if (left > right) {
				int t = left;
				left = right;
				right = t;
			}
			if (top > bottom) {
				int t = top;
				top = bottom;
				bottom = t;
			}
		}
		// Aligns the rectangle to be on a Nth pixel boundary
		// by Shrinking the rectangle.
		void align(int n = 4) { align(n,n,n,n); }
		void align(int x, int y) { align(x,x,y,y); }
		void align(int l, int r, int t, int b) {
			normalizeRect(); // Need a normalized rectangle
			int i = left % l; if (i > 0) left += l-i;
			right -= right % r;
			top += top % t;
			i = top % t; if (i > 0) top += t-i;
			bottom -= bottom % b;
		}

		// Shrinks the rectangle by x pixels
		// by Shrinking the rectangle.
		void shrink(int n = 4) { shrink(n,n,n,n); }
		void shrink(int x, int y) { shrink(x,x,y,y); }
		void shrink(int l, int r, int t, int b) {
			normalizeRect(); // Need a normalized rectangle
			left += l; right -= r;
			top += t; bottom -= b;
		}

		// Ensure the rectangle is at least n pixels in size...
		void enforceMinSize(int n = 4) { enforceMinSize(n,n); }
		void enforceMinSize(int x, int y) {
			normalizeRect(); // Need a normalized rectangle
			if (width() < x) right = left + x;
			if (height() < y) bottom = top + y;
		}

		// Shift the rectangle n pixels
		void shift(int dx, int dy) {
			left += dx; right += dx;
			top += dy; bottom += dy;
		}

		// Crops this rectangle to a specified rectangle
		// Optionally Proprotionally crops a second rectangle at the same time...
		void crop(RECT cropToRect, AspectRect *r2) {
			if (width() > 0) {		
				if (left < cropToRect.left) {
					if (r2) r2->left -= MulDiv(left-cropToRect.left, r2->width(),width());
					left = cropToRect.left;
				}
				if (right > cropToRect.right) {
					if (r2) r2->right -= MulDiv(right - cropToRect.right,r2->width(),width());
					right = cropToRect.right;
				}
			}
			if (height() > 0) {
				if (top < cropToRect.top) {
					if (r2) r2->top -= MulDiv(top-cropToRect.top, r2->height(),height());
					top = cropToRect.top;
				}
				if (bottom > cropToRect.bottom) {
					if (r2) r2->bottom -= MulDiv(bottom - cropToRect.bottom,r2->height(),height());
					bottom = cropToRect.bottom;
				}
			}
		}
		
		// Ensure the rectangle is located in another rectangle
		// Crop will just cut off excess
		// Shift will attempt to shift the rectangle to fit.  Will crop Right/bottom if needed.
		void cropToFitRect(const RECT &r) {
			if (left < r.left) left = r.left;
			if (right > r.right) right = r.right;
			if (top < r.top) top = r.top;
			if (bottom > r.bottom) bottom = r.bottom;
		}
		void shiftToFitRect(const RECT &r) {
			int dx, dy;
			if (left < r.left) dx = r.left-left;
			else if (right > r.right) dx = r.right-right;
			else dx = 0;

			if (top < r.top) dy = r.top-top;
			else if (bottom > r.bottom) dy = r.bottom-bottom;
			else dy = 0;

			if (dx || dy) {
				shift(dx,dy); // Perform the shift...
				cropToFitRect(r); // Crop any remaining....
			}
		}

		// Aspect adjustment functions...
		void adjustTargetAspectByHeight(double ar) { adjustSourceAspectByHeight(ar/m_outputAdjustment); }
		void adjustSourceAspectByHeight(double ar) {
			int newHeight = (int)(width()/ar+.5);
			top += ((height()-newHeight)/2);
			bottom = top + newHeight;
		}
		void adjustTargetAspectByWidth(double ar) { adjustSourceAspectByWidth(ar/m_outputAdjustment); }
		void adjustSourceAspectByWidth(double ar) {
			int newWidth = (int)(ar*height()+.5);
			left += ((width()-newWidth)/2);
			right = left + newWidth;
		}
		void adjustTargetAspectByGrowth(double ar) { adjustSourceAspectByGrowth(ar/m_outputAdjustment); }
		void adjustSourceAspectByGrowth(double ar) {
			if (ar < sourceAspect()) adjustSourceAspectByHeight(ar);
			else adjustSourceAspectByWidth(ar);
		}
		void adjustTargetAspectByShrink(double ar) { adjustSourceAspectByShrink(ar/m_outputAdjustment); }
		void adjustSourceAspectByShrink(double ar) {
			if (ar > sourceAspect()) adjustSourceAspectByHeight(ar);
			else adjustSourceAspectByWidth(ar);
		}
		// Adjusts the rectangle to new aspect.  Width is preserved
		// unless doing so causes the rectangle to go outside of r
		// Note: r must be in source space for both functions...
		void adjustTargetAspectSmart(double ar, RECT &boundRect, BOOL preserveWidth = TRUE) { adjustSourceAspectSmart(ar/m_outputAdjustment,boundRect,preserveWidth); }
		void adjustSourceAspectSmart(double ar, RECT &boundRect, BOOL preserveWidth = TRUE) {
			RECT r = {left,top,right,bottom};
			if (preserveWidth) adjustSourceAspectByHeight(ar); else adjustSourceAspectByWidth(ar);
			if (top < boundRect.top || bottom > boundRect.bottom) {
				left=r.left; right=r.right; top=r.top; bottom=r.bottom;
				if (preserveWidth) adjustSourceAspectByWidth(ar); else adjustSourceAspectByHeight(ar);
			}
		}

		void setToClient(HWND hwnd, BOOL useScreenCoords) {
			GetClientRect(hwnd, this);
			if (useScreenCoords) {
				ClientToScreen(hWnd, (POINT *) &left);
				ClientToScreen(hWnd, (POINT *) &right);
			}
		}

		void DebugDump(FILE *f) {
			fprintf(f,"L:%04i R:%04i T:%04i B:%04i [SA: %.4lf TA: %.4lf Adj:%.4lf]\n",left,right,top,bottom,sourceAspect(),targetAspect(),m_outputAdjustment);
		}
};

#endif
