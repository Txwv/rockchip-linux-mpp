/*
 *
 * Copyright 2015 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define MODULE_TAG "H265E_SYNTAX"

#include <string.h>

#include "h265e_codec.h"
#include "h265e_syntax_new.h"

static void fill_picture_parameters(const H265eCtx *h,
                                    H265ePicParams *pp)
{
    const H265ePps *pps = (H265ePps *)&h->pps;
    const H265eSps *sps = (H265eSps *)&h->sps;

    memset(pp, 0, sizeof(H265ePicParams));

    pp->pic_width  = h->cfg->prep.width;
    pp->pic_height = h->cfg->prep.height;
    pp->hor_stride  = h->cfg->prep.hor_stride;
    pp->ver_stride = h->cfg->prep.ver_stride;
    pp->pps_id = h->slice->m_ppsId;
    pp->sps_id = pps->m_SPSId;
    pp->vps_id = sps->m_VPSId;

    pp->wFormatAndSequenceInfoFlags = (sps->m_chromaFormatIdc               <<  0) |
                                      (sps->m_colorPlaneFlag                <<  2) |
                                      ((sps->m_bitDepthY - 8)               <<  3) |
                                      ((sps->m_bitDepthC - 8)               <<  6) |
                                      ((sps->m_bitsForPOC - 4)              <<  9) |
                                      (0                                    << 13) |
                                      (0                                    << 14) |
                                      (0                                    << 15);


    pp->sps_max_dec_pic_buffering_minus1         = sps->m_maxDecPicBuffering[sps->m_maxTLayers - 1] - 1;
    pp->log2_min_luma_coding_block_size_minus3   = sps->m_log2MinCodingBlockSize - 3;
    pp->log2_diff_max_min_luma_coding_block_size = sps->m_log2DiffMaxMinCodingBlockSize;

    pp->log2_min_transform_block_size_minus2     = sps->m_quadtreeTULog2MinSize - 2;
    pp->log2_diff_max_min_transform_block_size   = sps->m_quadtreeTULog2MaxSize  - sps->m_quadtreeTULog2MinSize;

    pp->max_transform_hierarchy_depth_inter      = sps->m_quadtreeTUMaxDepthInter;
    pp->max_transform_hierarchy_depth_intra      = sps->m_quadtreeTUMaxDepthIntra;

    pp->num_short_term_ref_pic_sets              = sps->m_RPSList.m_numberOfReferencePictureSets;
    pp->num_long_term_ref_pics_sps               = sps->m_numLongTermRefPicSPS;

    pp->num_ref_idx_l0_default_active_minus1     = pps->m_numRefIdxL0DefaultActive - 1;
    pp->num_ref_idx_l1_default_active_minus1     = pps->m_numRefIdxL1DefaultActive - 1;
    pp->init_qp_minus26                          = pps->m_picInitQPMinus26;


    pp->CodingParamToolFlags = (sps->m_scalingListEnabledFlag                                 <<  0) |
                               (sps->m_useAMP                                                 <<  1) |
                               (sps->m_bUseSAO                                                <<  2) |
                               (sps->m_usePCM                                                 <<  3) |
                               ((sps->m_usePCM ? (sps->m_pcmBitDepthLuma - 1) : 0)            <<  4) |
                               ((sps->m_usePCM ? (sps->m_pcmBitDepthChroma - 1) : 0)          <<  8) |
                               ((sps->m_usePCM ? (sps->m_pcmLog2MinSize - 3) : 0)             << 12) |
                               ((sps->m_usePCM ? (sps->m_pcmLog2MaxSize - sps->m_pcmLog2MinSize) : 0) << 14) |
                               (sps->m_bPCMFilterDisableFlag                   << 16) |
                               (sps->m_bLongTermRefsPresent                    << 17) |
                               (sps->m_TMVPFlagsPresent                        << 18) |
                               (sps->m_useStrongIntraSmoothing                 << 19) |
                               (0                                              << 20) |
                               (pps->m_outputFlagPresentFlag                   << 21) |
                               (pps->m_numExtraSliceHeaderBits                 << 22) |
                               (pps->m_signHideFlag                            << 25) |
                               (pps->m_cabacInitPresentFlag                    << 26) |
                               (0                                              << 27);

    pp->CodingSettingPicturePropertyFlags =   (pps->m_bConstrainedIntraPred                       <<  0) |
                                              (pps->m_useTransformSkip                            <<  1) |
                                              (pps->m_useDQP                                      <<  2) |
                                              (pps->m_bSliceChromaQpFlag                          <<  3) |
                                              (pps->m_bUseWeightPred                              <<  4) |
                                              (pps->m_useWeightedBiPred                           <<  5) |
                                              (pps->m_transquantBypassEnableFlag                  <<  6) |
                                              (0                                                  <<  7) |
                                              (pps->m_entropyCodingSyncEnabledFlag                <<  8) |
                                              (0                                                  <<  9) |
                                              (0                                                  << 10) |
                                              (pps->m_LFCrossSliceBoundaryFlag                    << 11) |
                                              (pps->m_deblockingFilterOverrideEnabledFlag         << 12) |
                                              (pps->m_picDisableDeblockingFilterFlag              << 13) |
                                              (pps->m_listsModificationPresentFlag                << 14) |
                                              (pps->m_sliceHeaderExtensionPresentFlag             << 15);

    pp->pps_cb_qp_offset                 = pps->m_chromaCbQpOffset;
    pp->pps_cr_qp_offset                 = pps->m_chromaCrQpOffset;
    pp->diff_cu_qp_delta_depth           = pps->m_maxCuDQPDepth;
    pp->pps_beta_offset_div2             = pps->m_deblockingFilterBetaOffsetDiv2;
    pp->pps_tc_offset_div2               = pps->m_deblockingFilterTcOffsetDiv2;
    pp->log2_parallel_merge_level_minus2 = pps->m_log2ParallelMergeLevelMinus2 - 2;
}

static void fill_slice_parameters( const H265eCtx *h,
                                   H265eSlicParams *sp)
{
    MppEncH265Cfg *codec = &h->cfg->codec.h265;
    H265eSlice  *slice = h->slice;
    sp->sli_cut_bmod = 1;
    if (codec->slice_cfg.split_enable) {
        sp->sli_cut = 1;
        if (codec->slice_cfg.split_mode) {
            sp->sli_cut_cnum_m1 =  codec->slice_cfg.slice_size;
        } else {
            sp->sli_cut_byte =  codec->slice_cfg.slice_size;
        }
        sp->sli_max_num_m1 = 50;
        sp->sli_out_mode = 1;
    }



    sp->cbc_init_flg        = slice->m_cabacInitFlag;
    sp->mvd_l1_zero_flg     = slice->m_bLMvdL1Zero;
    sp->merge_up_flag       = codec->merge_cfg.merge_up_flag;
    sp->merge_left_flag     = codec->merge_cfg.merge_left_flag;
    sp->ref_pic_lst_mdf_l0  = slice->ref_pic_list_modification_flag_l0;

    sp->num_refidx_l1_act   = 0;
    sp->num_refidx_l0_act   = 1;

    sp->num_refidx_act_ovrd = (((RK_U32)slice->m_numRefIdx[0] != slice->m_pps->m_numRefIdxL0DefaultActive)
                               || (slice->m_sliceType == B_SLICE &&
                                   (RK_U32)slice->m_numRefIdx[1] != slice->m_pps->m_numRefIdxL1DefaultActive));

    sp->sli_sao_chrm_flg    = slice->m_sps->m_bUseSAO && slice->m_saoEnabledFlagChroma;
    sp->sli_sao_luma_flg    = slice->m_sps->m_bUseSAO && slice->m_saoEnabledFlag;
    sp->sli_tmprl_mvp_en    = slice->m_enableTMVPFlag;

    sp->pic_out_flg         = slice->m_picOutputFlag;
    sp->slice_type            = slice->m_sliceType;
    sp->slice_rsrv_flg        = 0;
    sp->dpdnt_sli_seg_flg   = 0;
    sp->sli_pps_id          = slice->m_pps->m_PPSId;
    sp->no_out_pri_pic      = slice->no_output_of_prior_pics_flag;


    sp->sli_tc_ofst_div2 = slice->m_deblockingFilterTcOffsetDiv2;
    sp->sli_beta_ofst_div2 = slice->m_deblockingFilterBetaOffsetDiv2;
    sp->sli_lp_fltr_acrs_sli = slice->m_LFCrossSliceBoundaryFlag;
    sp->sli_dblk_fltr_dis = slice->m_deblockingFilterDisable;
    sp->dblk_fltr_ovrd_flg = slice->m_deblockingFilterOverrideFlag;
    sp->sli_cb_qp_ofst = slice->m_sliceQpDeltaCb;
    sp->sli_qp = slice->m_sliceQp;
    sp->max_mrg_cnd = slice->m_maxNumMergeCand;

    sp->col_ref_idx = 0;
    sp->col_frm_l0_flg = slice->m_colFromL0Flag;
    sp->merge_left_flag = codec->merge_cfg.merge_left_flag;
    sp->merge_up_flag = codec->merge_cfg.merge_up_flag;
    sp->sli_poc_lsb = (slice->poc - slice->last_idr + (1 << slice->m_sps->m_bitsForPOC)) %
                      (1 << slice->m_sps->m_bitsForPOC);
    sp->sli_hdr_ext_len = slice->slice_header_extension_length;
}

RK_S32 fill_ref_parameters(const H265eCtx *h, H265eSlicParams *sp)
{
    H265eSlice  *slice = h->slice;
    H265eReferencePictureSet* rps = slice->m_rps;
    RK_U32 numRpsCurrTempList = 0;
    RK_S32 ref_num = 0;
    H265eDpbFrm *ref_frame;
    RK_S32 i, j, k;
    sp->dlt_poc_msb_prsnt0 = 0;
    sp->dlt_poc_msb_cycl0 = 0;
    if (slice->m_bdIdx < 0) {
        RK_S32 prev = 0;
        sp->st_ref_pic_flg = 0;
        sp->num_neg_pic = rps->num_negative_pic;
        sp->num_pos_pic = rps->num_positive_pic;
        for (j = 0; j < rps->num_negative_pic; j++) {
            if (0 == j) {
                sp->dlt_poc_s0_m10 = prev - rps->delta_poc[j] - 1;
                sp->used_by_s0_flg = rps->m_used[j];
            } else if (1 == j) {
                sp->dlt_poc_s0_m11 = prev - rps->delta_poc[j] - 1;
                sp->used_by_s0_flg |= rps->m_used[j] << 1;
            } else if (2 == j) {
                sp->dlt_poc_s0_m12 = prev - rps->delta_poc[j] - 1;
                sp->used_by_s0_flg |= rps->m_used[j] << 2;
            } else if (3 == j) {
                sp->dlt_poc_s0_m13 = prev - rps->delta_poc[j] - 1;
                sp->used_by_s0_flg |= rps->m_used[j] << 3;
            }
            prev = rps->delta_poc[j];
        }
    }

    if (slice->m_sps->m_bLongTermRefsPresent) {
        RK_S32 numLtrpInSH = rps->num_long_term_pic;
        RK_S32 numLtrpInSPS = 0;
        RK_S32 counter = 0;
        RK_U32 poc_lsb_lt[3] = { 0, 0, 0 };
        RK_U32 used_by_lt_flg[3] = { 0, 0, 0 };
        RK_U32 dlt_poc_msb_prsnt[3] = { 0, 0, 0 };
        RK_U32 dlt_poc_msb_cycl[3] = { 0, 0, 0 };
        RK_S32 prevDeltaMSB = 0;
        RK_S32 offset = rps->num_negative_pic + rps->num_positive_pic;
        RK_S32 numLongTerm = rps->m_numberOfPictures - offset;
        RK_S32 bitsForLtrpInSPS = 0;
        for (k = rps->m_numberOfPictures - 1; k > rps->m_numberOfPictures - rps->num_long_term_pic - 1; k--) {
            RK_U32 lsb = rps->poc[k] % (1 << slice->m_sps->m_bitsForPOC);
            RK_U32 find_flag = 0;
            for (i = 0; i < (RK_S32)slice->m_sps->m_numLongTermRefPicSPS; i++) {
                if ((lsb == slice->m_sps->m_ltRefPicPocLsbSps[i]) && (rps->m_used[k] == slice->m_sps->m_usedByCurrPicLtSPSFlag[i])) {
                    find_flag = 1;
                    break;
                }
            }

            if (find_flag) {
                numLtrpInSPS++;
            } else {
                counter++;
            }
        }

        numLtrpInSH -= numLtrpInSPS;

        while (slice->m_sps->m_numLongTermRefPicSPS > (RK_U32)(1 << bitsForLtrpInSPS)) {
            bitsForLtrpInSPS++;
        }

        if (slice->m_sps->m_numLongTermRefPicSPS > 0) {
            sp->num_lt_sps = numLtrpInSPS;
        }
        sp->num_lt_pic = numLtrpInSH;
        // Note that the LSBs of the LT ref. pic. POCs must be sorted before.
        // Not sorted here because LT ref indices will be used in setRefPicList()
        sp->poc_lsb_lt0 = 0;
        sp->used_by_lt_flg0 = 0;
        sp->dlt_poc_msb_prsnt0 = 0;
        sp->dlt_poc_msb_cycl0 = 0;
        sp->poc_lsb_lt1 = 0;
        sp->used_by_lt_flg1 = 0;
        sp->dlt_poc_msb_prsnt1 = 0;
        sp->dlt_poc_msb_cycl1 = 0;
        sp->poc_lsb_lt2 = 0;
        sp->used_by_lt_flg2 = 0;
        sp->dlt_poc_msb_prsnt2 = 0;
        sp->dlt_poc_msb_cycl2 = 0;

        for ( i = rps->m_numberOfPictures - 1; i > offset - 1; i--) {
            RK_U32 deltaFlag = 0;

            if ((i == rps->m_numberOfPictures - 1) || (i == rps->m_numberOfPictures - 1 - numLtrpInSPS)) {
                deltaFlag = 1;
            }
            poc_lsb_lt[numLongTerm - 1 - (i - offset)] = rps->m_pocLSBLT[i];
            used_by_lt_flg[numLongTerm - 1 - (i - offset)] = rps->m_used[i];
            dlt_poc_msb_prsnt[numLongTerm - 1 - (i - offset)] = rps->m_deltaPocMSBPresentFlag[i];

            if (rps->m_deltaPocMSBPresentFlag[i]) {
                if (deltaFlag) {
                    dlt_poc_msb_cycl[numLongTerm - 1 - (i - offset)] = rps->m_deltaPOCMSBCycleLT[i];
                } else {
                    RK_S32 differenceInDeltaMSB = rps->m_deltaPOCMSBCycleLT[i] - prevDeltaMSB;
                    mpp_assert(differenceInDeltaMSB >= 0);
                    dlt_poc_msb_cycl[numLongTerm - 1 - (i - offset)] = differenceInDeltaMSB;
                }
            }
            prevDeltaMSB = rps->m_deltaPOCMSBCycleLT[i];
        }

        sp->poc_lsb_lt0 = poc_lsb_lt[0];
        sp->used_by_lt_flg0 = used_by_lt_flg[0];
        sp->dlt_poc_msb_prsnt0 = dlt_poc_msb_prsnt[0];
        sp->dlt_poc_msb_cycl0 = dlt_poc_msb_cycl[0];
        sp->poc_lsb_lt1 = poc_lsb_lt[1];
        sp->used_by_lt_flg1 = used_by_lt_flg[1];
        sp->dlt_poc_msb_prsnt1 = dlt_poc_msb_prsnt[1];
        sp->dlt_poc_msb_cycl1 = dlt_poc_msb_cycl[1];
        sp->poc_lsb_lt2 = poc_lsb_lt[2];
        sp->used_by_lt_flg2 = used_by_lt_flg[2];
        sp->dlt_poc_msb_prsnt2 = dlt_poc_msb_prsnt[2];
        sp->dlt_poc_msb_cycl2 = dlt_poc_msb_cycl[2];
    }

    sp->lst_entry_l0 = 0;
    sp->ref_pic_lst_mdf_l0 = 0;

    if (slice->m_sliceType == I_SLICE) {
        numRpsCurrTempList = 0;
    } else {
        ref_num = rps->num_negative_pic + rps->num_positive_pic + rps->num_long_term_pic;
        for (i = 0; i < ref_num; i++) {
            if (rps->m_used[i]) {
                numRpsCurrTempList++;
            }
        }
    }

    if (slice->m_pps->m_listsModificationPresentFlag && numRpsCurrTempList > 1) {
        H265eRefPicListModification* refPicListModification = &slice->m_RefPicListModification;
        if (slice->m_sliceType != I_SLICE) {
            sp->ref_pic_lst_mdf_l0 = refPicListModification->m_refPicListModificationFlagL0 ? 1 : 0;
            if (sp->ref_pic_lst_mdf_l0) {
                sp->lst_entry_l0 = refPicListModification->m_RefPicSetIdxL0[0];
            }
        }
    }

    sp->recon_pic.bPicEntry[0] = mpp_buffer_get_fd(h->dpb->curr->buf->buf[0]);
    sp->recon_pic.bPicEntry[1] = mpp_buffer_get_fd(h->dpb->curr->buf->buf[1]);
    ref_frame = slice->m_refPicList[0][0];
    if (ref_frame != NULL) {
        sp->ref_pic.bPicEntry[0] = mpp_buffer_get_fd(ref_frame->buf->buf[0]);
        sp->ref_pic.bPicEntry[1] = mpp_buffer_get_fd(ref_frame->buf->buf[1]);
    }
    return  0;
}


RK_S32 fill_rc_parameters(H265eCtx *h, RcParams *rp)
{
    H265eFrmInfo *frms = &h->frms;
    MppEncRcCfg *rc = &h->cfg->rc;
    RcHalCfg    *rc_cfg = &frms->rc_cfg;
    MppEncCodecCfg *codec = &h->cfg->codec;
    MppEncH265Cfg *h265 = &codec->h265;
    RK_U32 i;
    RK_U32 ctu_target_bits_mul_16 = (rc_cfg->bit_target << 4) / frms->mb_per_frame;
    RK_U32 ctu_target_bits;
    RK_S32 negative_bits_error, positive_bits_error;
    memset(rp, 0, sizeof(RcParams));
    if (ctu_target_bits_mul_16 >= 0x100000) {
        ctu_target_bits_mul_16 = 0x50000;
    }

    ctu_target_bits = (ctu_target_bits_mul_16 * frms->mb_wid) >> 4;
    negative_bits_error = 0 - ctu_target_bits / 4;
    positive_bits_error = ctu_target_bits / 4;

    if (rc->rc_mode == MPP_ENC_RC_MODE_FIXQP) {
        rp->pic_qp = h265->qp_init;
        rp->rc_max_qp = h265->qp_init;
        rp->rc_min_qp = h265->qp_init;
        return 0;
    }

    rp->rc_en = 1;
    rp->aqmode_en = 1;
    rp->qp_mode = 1;
    rp->rc_qp_range = 2;

    rp->pic_qp = frms->start_qp;

    if (frms->status.is_intra) {
        rp->rc_max_qp = h265->max_i_qp;
        rp->rc_min_qp = h265->min_i_qp;
    } else {
        rp->rc_max_qp = h265->max_qp;
        rp->rc_min_qp = h265->min_qp;
    }
    rp->frame_type = frms->status.is_intra ? 2 : 0;
    rp->rc_ctu_num = frms->mb_wid;

    rp->ctu_ebits = ctu_target_bits_mul_16;
    rp->bit_error[0] = negative_bits_error;
    rp->bit_error[1] = positive_bits_error;
    rp->bit_error[2] = positive_bits_error;

    rp->bit_error[3] = positive_bits_error;
    rp->bit_error[4] = positive_bits_error;
    rp->bit_error[5] = positive_bits_error;
    rp->bit_error[6] = positive_bits_error;
    rp->bit_error[7] = positive_bits_error;
    rp->bit_error[8] = positive_bits_error;

    rp->qp_adjust[0] = -1;
    rp->qp_adjust[1] = 0;
    rp->qp_adjust[2] = 0;
    rp->qp_adjust[3] = 0;
    rp->qp_adjust[4] = 0;
    rp->qp_adjust[5] = 0;
    rp->qp_adjust[6] = 0;
    rp->qp_adjust[7] = 0;
    rp->qp_adjust[8] = 1;
    for ( i = 0; i < 8; i++) {
        rp->qpmax_area[i] = h265->qpmax_map[i];
        rp->qpmin_area[i] = h265->qpmin_map[i];
    }
    rp->qp_mode = h265->qpmap_mode;
    return  0;
}

RK_S32 h265e_syntax_fill(void *ctx)
{
    H265eCtx *h = (H265eCtx *)ctx;
    H265eSyntax_new *syn = (H265eSyntax_new*)&h->syntax;
    fill_picture_parameters(h, &syn->pp);
    fill_slice_parameters(h, &syn->sp);
    fill_ref_parameters(h, &syn->sp);
    fill_rc_parameters(h, &syn->rp);
    syn->ud.plt_data = NULL;
    if (h->plt_flag) {
        syn->ud.plt_data = (void*)&h->cfg->osd_plt;
        h->plt_flag = 0;
    }
    return 0;
}
