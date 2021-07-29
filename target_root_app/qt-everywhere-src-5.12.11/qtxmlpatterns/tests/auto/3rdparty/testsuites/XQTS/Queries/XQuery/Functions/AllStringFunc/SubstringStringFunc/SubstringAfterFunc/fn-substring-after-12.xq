(:*******************************************************:)
(:Test: fn-substring-after-12                           :)
(:Written By: Carmelo Montanez                           :)
(:Date: Fri May 27, 2005                                 :)
(:Purpose: Evaluates The "substring-after" function     :)
(:using it as a argument of a fn:not - returns true      :)
(:*******************************************************:)

fn:not(fn:substring-after("A","B"))