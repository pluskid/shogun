/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Written (W) 2012 Heiko Strathmann
 */

#include <shogun/statistics/TwoDistributionsTestStatistic.h>
#include <shogun/features/Features.h>

using namespace shogun;

CTwoDistributionsTestStatistic::CTwoDistributionsTestStatistic() :
		CTestStatistic()
{
	init();
}

CTwoDistributionsTestStatistic::CTwoDistributionsTestStatistic(
		CFeatures* p_and_q,
		index_t q_start) : CTestStatistic()
{
	init();

	m_p_and_q=p_and_q;
	SG_REF(m_p_and_q);

	m_q_start=q_start;
}

CTwoDistributionsTestStatistic::CTwoDistributionsTestStatistic(
		CFeatures* p, CFeatures* q) :
		CTestStatistic()
{
	init();

	m_p_and_q=p->create_merged_copy(q);
	SG_REF(m_p_and_q);

	m_q_start=p->get_num_vectors();
}

CTwoDistributionsTestStatistic::~CTwoDistributionsTestStatistic()
{
	SG_UNREF(m_p_and_q);
}

void CTwoDistributionsTestStatistic::init()
{
	SG_ADD((CSGObject**)&m_p_and_q, "p_and_q", "Concatenated samples p and q",
			MS_NOT_AVAILABLE);
	SG_ADD(&m_q_start, "q_start", "Index of first sample of q",
			MS_NOT_AVAILABLE);

	m_p_and_q=NULL;
	m_q_start=0;
}

SGVector<float64_t> CTwoDistributionsTestStatistic::bootstrap_null()
{
	SG_DEBUG("entering CTwoDistributionsTestStatistic::bootstrap_null()\n");

	/* compute bootstrap statistics for null distribution */
	SGVector<float64_t> results(m_bootstrap_iterations);

	/* memory for index permutations, (would slow down loop) */
	SGVector<index_t> ind_permutation(m_p_and_q->get_num_vectors());
	ind_permutation.range_fill();
	m_p_and_q->add_subset(ind_permutation);

	for (index_t i=0; i<m_bootstrap_iterations; ++i)
	{
		/* idea: merge features of p and q, shuffle, and compute statistic.
		 * This is done using subsets here */

		/* create index permutation and add as subset. This will mix samples
		 * from p and q */
		SGVector<int32_t>::permute_vector(ind_permutation);

		/* compute statistic for this permutation of mixed samples */
		results[i]=compute_statistic();
	}

	/* clean up */
	m_p_and_q->remove_subset();

	SG_DEBUG("leaving CTwoDistributionsTestStatistic::bootstrap_null()\n");
	return results;
}

float64_t CTwoDistributionsTestStatistic::compute_p_value(
		float64_t statistic)
{
	float64_t result=0;

	if (m_null_approximation_method==BOOTSTRAP)
	{
		/* bootstrap a bunch of MMD values from null distribution */
		SGVector<float64_t> values=bootstrap_null();

		/* find out percentile of parameter "statistic" in null distribution */
		CMath::qsort(values);
		float64_t i=CMath::find_position_to_insert(values, statistic);

		/* return corresponding p-value */
		result=1.0-i/values.vlen;
	}
	else
	{
		SG_ERROR("CTwoDistributionsTestStatistics::compute_p_value(): Unknown"
				"method to approximate null distribution!\n");
	}

	return result;
}

float64_t CTwoDistributionsTestStatistic::compute_threshold(
		float64_t alpha)
{
	float64_t result=0;

	if (m_null_approximation_method==BOOTSTRAP)
	{
		/* bootstrap a bunch of MMD values from null distribution */
		SGVector<float64_t> values=bootstrap_null();

		/* return value of (1-alpha) quantile */
		result=values[CMath::floor(values.vlen*(1-alpha))];
	}
	else
	{
		SG_ERROR("CTwoDistributionsTestStatistics::compute_threshold():"
				"Unknown method to approximate null distribution!\n");
	}

	return result;
}
