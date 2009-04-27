/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2001 Torbjörn Jansson.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
/////////////////////////////////////////////////////////////////////////////

/**
 * @file DSGraph.h interface for the CDShowGraph class.
 */

#if !defined(AFX_DSGRAPH_H__21DAAF0A_220E_4F8F_A4F6_988DCB31AAE1__INCLUDED_)
#define AFX_DSGRAPH_H__21DAAF0A_220E_4F8F_A4F6_988DCB31AAE1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "exception.h"
#include "DShowBaseSource.h"
#include "DShowAudioControls.h"
#include "DShowSeeking.h"

//if you get an error here, that means you have not checked out the DSRend filter
//or compiled it atleast once.
//it has to be checked out in the same directory as dscaler is checked out in.
#include "..\..\..\DSRend\DSRend.h"

class CTreeSettingsPage;

/**
 * Class that manages a direct show filter graph and the filters in it.
 */
class CDShowGraph
{
public:
    /**
     * Video format settins.
     */
    class CVideoFormat
    {
    public:
        CVideoFormat()
            :m_Width(0),m_Height(0),m_bForceYUY2(FALSE),m_FieldFmt(DSREND_FIELD_FORMAT_AUTO)
        {
        }
        BOOL operator==(CVideoFormat &fmt);
        operator std::string();
        void operator=(std::string &str);

        std::string m_Name;
        ///Width
        long m_Width;
        ///Height
        long m_Height;
        ///Make dsrend filter only accept YUY2, currently unused
        BOOL m_bForceYUY2;
        ///Field format, currently unused
        DSREND_FIELD_FORMAT m_FieldFmt;
    };

    ///failure codes from ChangeRes
    enum eChangeRes_Error
    {
        ///format changed
        SUCCESS,
        ///no filter graph
        ERROR_NO_GRAPH,
        ///failed to change format, changed back to old
        ERROR_CHANGED_BACK,
        ///failed to change format and coud not restor old format
        ERROR_FAILED_TO_CHANGE_BACK
    };

    /**
     * Creates a filtergraph with a capture device as source.
     * @throws CDShowException
     */
    CDShowGraph(std::string device, std::string deviceName, std::string AudioDevice, BOOL bConnectAudio);

    /**
     * Creates a filtergraph with a file as source
     * @throws CDShowException
     */
    CDShowGraph(std::string filename, std::string AudioDevice);
    ///Destructor
    virtual ~CDShowGraph();

    /**
     * @return pointer to the source device or NULL if there is no source
     */
    CDShowBaseSource* getSourceDevice();
    void getConnectionMediatype(AM_MEDIA_TYPE *pmt);
    BOOL GetFields(long *pcFields, FieldBuffer *ppFields,BufferInfo &info,DWORD dwLateness);

    /**
     * Get number of dropped frames.
     * This function return the total number of dropped frames from the
     * renderer filter and the source
     * @return number of dropped frames
     */
    long getDroppedFrames();

    void ConnectGraph();
    void start();
    void pause();
    void stop();
    FILTER_STATE getState() {return m_GraphState;}

    /**
     * Creates a propertypage for a filter.
     * This function creates a propertypage for the filter indicated by index.
     * To get all pages call this function with index set to 0 and then
     * increase it until the function returns FALSE.
     * The caller is responsibel of deleting the returnd propertypage with
     * delete.
     *
     * @param index filter index
     * @param ppPage pointer to created propertypage
     * @param bHasSubPages TRUE if specified filter has subpages
     * @return TRUE if the propertypage was successfully created
     */
    BOOL getFilterPropertyPage(int index, CComPtr<ISpecifyPropertyPages>& pSpecifyPages);

    /**
     * This function works almost the same as getFilterPropertyPage() but returns the subpage.
     * @return TRUE if successfull
     */
    BOOL getFilterSubPage(int filterIndex, int subIndex, CComPtr<ISpecifyPropertyPages>& pSpecifyPages);

    /**
     * Changes connection settings to dsrend filter.
     *
     * @throws CDShowException
     * @param fmt new format to change to
     * @return
     */
    eChangeRes_Error ChangeRes(CDShowGraph::CVideoFormat fmt);

    /**
     * Checks if a resolution is valid and can be selected.
     * @param fmt video format to check
     * @return TRUE if valid
     */
    BOOL IsValidRes(CDShowGraph::CVideoFormat fmt);

    /**
     * Disables the graph reference clock.
     * @throws CDShowException
     */
    void DisableClock();

    /**
     * Restored the old clock after a call to DisableClock().
     * @throws CDShowException
     */
    void RestoreClock();

    /**
     * @return
     */
    CDShowAudioControls *GetAudioControls();

    CDShowSeeking *GetSeeking();

private:
    void InitGraph();
    void CreateRenderer();

    void FindStreamConfig();

    /// updates m_filter vector with filters in the graph
    void BuildFilterList();

    ///Custom video renderer. Used for transfering the picture to dscaler
    CComPtr<IBaseFilter> m_renderer;

    ///Interface used for geting media samples from the renderer filter
    CComPtr<IDSRendFilter> m_DSRend;
    CComPtr<IQualProp> m_pQualProp;
    CComPtr<IDSRendSettings> m_pDSRendSettings;

    ///IAMStreamConfig interface for the filter connected to our renderer
    CComPtr<IAMStreamConfig> m_pStreamCfg;

    CComPtr<IGraphBuilder> m_pGraph;
    CComPtr<ICaptureGraphBuilder2> m_pBuilder;
    CComPtr<IMediaControl> m_pControl;

    CDShowBaseSource *m_pSource;
    CDShowAudioControls *m_pAudioControlls;
    CDShowSeeking *m_pSeeking;

    FILTER_STATE m_GraphState;

    CComPtr<IReferenceClock> m_pOldRefClk;

    class CFilterPages
    {
    public:
        CComPtr<IBaseFilter> m_pFilter;
        std::vector<CComPtr<IPin> > m_SubPage;
    };
    /// used in getFilterName and showPropertyPage
    std::vector<CFilterPages> m_filters;

    DWORD m_hROT;
#ifdef _DEBUG
    HANDLE m_hLogFile;
#endif
};

#endif // !defined(AFX_DSGRAPH_H__21DAAF0A_220E_4F8F_A4F6_988DCB31AAE1__INCLUDED_)
