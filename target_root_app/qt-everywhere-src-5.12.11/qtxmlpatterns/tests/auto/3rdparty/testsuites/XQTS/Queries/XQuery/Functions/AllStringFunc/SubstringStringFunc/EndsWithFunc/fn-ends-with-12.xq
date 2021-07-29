(:*******************************************************:)
(:Test: fn-ends-with-12                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: Fri May 27, 2005                                 :)
(:Purpose: Evaluates The "ends-with" function          :)
(:using it as a argument of a fn:not - returns true      :)
(:*******************************************************:)

fn:not(fn:ends-with("A","B"))