(:*******************************************************:)
(:Test: fn-current-date-15                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-date" function as part :)
(:of an equal expression (ge operator)                   :) 
(:*******************************************************:)

fn:string(fn:current-date()) ge fn:string(fn:current-date())
