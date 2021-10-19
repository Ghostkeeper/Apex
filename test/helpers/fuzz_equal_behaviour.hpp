/*
 * Library for performing massively parallel computations on polygons.
 * Copyright (C) 2021 Ghostkeeper
 * This library is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for details.
 * You should have received a copy of the GNU Affero General Public License along with this library. If not, see <https://gnu.org/licenses/>.
 */

#ifndef APEX_FUZZ_EQUAL_BEHAVIOUR
#define APEX_FUZZ_EQUAL_BEHAVIOUR

#include <gtest/gtest.h> //To assert equivalence.
#include <random> //Pseudo-random fuzz testing.

namespace apex {

/*!
 * Test runner for a fuzz test to verify that two classes behave the same under
 * the same transformations.
 *
 * This test runner will be given one instance of each class, which must start
 * off in an equivalent state. This test runner will then execute random
 * functions on an instance of each class. These functions come in pairs: One
 * for class A, the other for class B, which must represent an equivalent action
 * on the instance. The runner will then verify at each step that the two
 * instances remain equivalent.
 *
 * To do all this, this runner must have the following information:
 * - A function to compare if the two instances are still equivalent.
 * - A list of pairs of transformations to execute on each instance. One of this
 * pair applies to class A, the other to class B.
 * - An instance of each class as starting state.
 *
 * The transformations to apply to each instance will be pseudo-random, starting
 * from a fixed seed. This way the fuzz test is deterministic and repeatable.
 * This is important to ensure that whenever a test starts failing, it is
 * certain that the change made when the failure starts is the change that made
 * it fail.
 *
 * Some transformations can be defined to be more common than others. This can
 * be useful for transformations like "clear the container", or "multiply by 0"
 * that would erase all data and make other transformations trivial. This would
 * make lots of tests useless, so it is nice to give those a lower chance of
 * occurring.
 *
 * The fuzz test can be specified to run an arbitrary number of transformation
 * steps in the \ref run function. It can also run with the same set of
 * transformations multiple times with different starting points.
 *
 * The runner reports its results via Google Test. Once a transformation results
 * in instances that are no longer equal, it fails the test and doesn't
 * continue. After all, once the two instances are no longer equal, further
 * transformations will likely keep them inequal even if the transformations are
 * working perfectly well.
 */
template<typename A, typename B>
class FuzzEqualBehaviour {
public:
	/*!
	 * Construct a new fuzz tester.
	 * \param is_equivalent A function that evaluates whether an instance of
	 * class A is equivalent to an instance of class B.
	 */
	FuzzEqualBehaviour(const std::function<bool(const A&, const B&)> is_equivalent) : is_equivalent(is_equivalent) {}

	/*!
	 * The function that determines whether two instances of the classes are in
	 * an equivalent state.
	 *
	 * This is to be provided by the test.
	 */
	std::function<bool(const A&, const B&)> is_equivalent;

	/*!
	 * For each of the two instances, provides a transformation.
	 *
	 * The transformation needs to be an equivalent transformation for both
	 * classes. It is expected (if the implementation is correct) that if the
	 * two instances start out in an equivalent state, then applying the
	 * respective transformations to the two instances will keep them in
	 * equivalent states.
	 * \param a_transformation The transformation applied to class A.
	 * \param b_transformation The transformation applied to class B.
	 * \param chance The relative chance for this transformation to be applied.
	 * The chances don't need to sum up to 1. It will simply calculate the
	 * probability for this transformation by dividing this chance by the total
	 * summed chances of all transformations.
	 */
	void add_transformation(std::function<void(A&)> a_transformation, std::function<void(B&)> b_transformation, double chance) {
		double cumulative = chance;
		if(!transformations.empty()) {
			cumulative += transformations.back().cumulative_chance;
		}
		transformations.emplace_back(a_transformation, b_transformation, cumulative);
	}

	/*!
	 * Run a fuzz test. This actually executes the tests.
	 *
	 * For a proper test, the two instances need to be in equivalent states.
	 * \param a_start The initial state of an instance of class A.
	 * \param b_start The initial state of an instance of class B.
	 * \param steps How many transformations to try. More steps will cause the
	 * test to take longer, but will also improve the coverage of the test.
	 */
	void run(A& a_start, B& b_start, const size_t steps) {
		//Some pre-checks to verify the validity of the test.
		ASSERT_GT(transformations.size(), 0) << "We must have at least 1 transformation to repeatedly apply to these instances.";
		ASSERT_TRUE(is_equivalent(a_start, b_start)) << "The two instances must start off equivalent.";

		//Random number generator.
		std::default_random_engine random_engine(transformations.size()); //Seed with the number of transformations.
		std::uniform_real_distribution rng(0.0, transformations.back().cumulative_chance); //We'll generate random numbers in the range of the sum of all chances.

		for(size_t step = 0; step < steps; ++step) {
			//Choose a random transformation, based on the pseudo-random number generator and the weighted chances of each transformation.
			const double random_number = rng(random_engine);
			size_t chosen_index = 0;
			while(random_number > transformations[chosen_index].cumulative_chance) {
				++chosen_index;
			}

			//Execute the chosen transformation.
			transformations[chosen_index].transform_a(a_start);
			transformations[chosen_index].transform_b(b_start);

			ASSERT_TRUE(is_equivalent(a_start, b_start)) << "After transforming with option " << chosen_index << ", the two instances must remain equivalent.";
		}
	}

protected:
	/*!
	 * Represents a possible transformation that can be applied to both
	 * instances.
	 *
	 * The two transformation functions are supposed to be equivalent. If the
	 * instances start in an equivalent state, they need to remain equivalent
	 * when both of them are transformed.
	 */
	struct TransformationOption {
		/*!
		 * Construct a new option.
		 * \param transform_a The function that transforms instances of class A.
		 * \param transform_b The function that transforms instances of class B.
		 * \param cumulative_chance The cumulative chance of all options up to
		 * and including this option.
		 */
		TransformationOption(std::function<void(A&)> transform_a, std::function<void(B&)> transform_b, double cumulative_chance) :
			transform_a(transform_a),
			transform_b(transform_b),
			cumulative_chance(cumulative_chance) {}

		/*!
		 * The function that transforms instances of class A.
		 */
		std::function<void(A&)> transform_a;

		/*!
		 * The function that transforms instances of class B.
		 */
		std::function<void(B&)> transform_b;

		/*!
		 * The cumulative chance at which this function gets executed. This is
		 * not the chance that this specific transformation gets executed, but
		 * the sum of the chances of all transformations up to and including
		 * this one in the transformation list.
		 *
		 * This makes it simple to randomly choose a transformation with
		 * weighted probabilities.
		 */
		double cumulative_chance;
	};

	/*!
	 * The list of transformations that can be executed.
	 */
	std::vector<TransformationOption> transformations;
};

}

#endif //APEX_FUZZ_EQUAL_BEHAVIOUR