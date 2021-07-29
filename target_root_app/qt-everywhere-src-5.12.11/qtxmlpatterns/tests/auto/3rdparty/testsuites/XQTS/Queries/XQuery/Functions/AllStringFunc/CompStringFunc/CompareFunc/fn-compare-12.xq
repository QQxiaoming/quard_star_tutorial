(:*******************************************************:)
(:Test: fn-compare-12                                    :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 1, 2005                                     :)
(:Purpose: Evaluates The "compare" function              :)
(:using it as a argument of a fn:not - returns false     :)
(:*******************************************************:)

fn:not(fn:compare("A","B"))