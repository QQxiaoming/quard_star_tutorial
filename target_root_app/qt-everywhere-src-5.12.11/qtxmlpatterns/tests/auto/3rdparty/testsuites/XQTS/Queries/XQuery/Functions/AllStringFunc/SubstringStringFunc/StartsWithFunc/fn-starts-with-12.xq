(:*******************************************************:)
(:Test: fn-starts-with-12                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: Fri May 27, 2005                                 :)
(:Purpose: Evaluates The "starts-with" function          :)
(:using it as a argument of a fn:not - returns true      :)
(:*******************************************************:)

fn:not(fn:starts-with("A","B"))