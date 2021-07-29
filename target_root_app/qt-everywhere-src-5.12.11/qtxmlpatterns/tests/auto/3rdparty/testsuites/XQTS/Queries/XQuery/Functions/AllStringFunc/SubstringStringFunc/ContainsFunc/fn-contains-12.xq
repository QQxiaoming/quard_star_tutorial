(:*******************************************************:)
(:Test: fn-contains-12                                   :)
(:Written By: Carmelo Montanez                           :)
(:Date: Fri May 27, 2005                                 :)
(:Purpose: Evaluates The "contains" function             :)
(:using it as a argument of a fn:not - returns true      :)
(:*******************************************************:)

fn:not(fn:contains("A","B"))