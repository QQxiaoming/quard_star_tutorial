(:*******************************************************:)
(:Test: fn-starts-with-11                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: Fri May 27, 2005                                 :)
(:Purpose: Evaluates The "starts-with" function          :)
(:using it as a argument of a fn:not - returns false     :)
(:*******************************************************:)

fn:not(fn:starts-with("A","A"))