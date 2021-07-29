(:*******************************************************:)
(:Test: fn-contains-11                                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: Fri May 27, 2005                                 :)
(:Purpose: Evaluates The "contains" function             :)
(:using it as a argument of a fn:not - returns false     :)
(:*******************************************************:)

fn:not(fn:contains("A","A"))