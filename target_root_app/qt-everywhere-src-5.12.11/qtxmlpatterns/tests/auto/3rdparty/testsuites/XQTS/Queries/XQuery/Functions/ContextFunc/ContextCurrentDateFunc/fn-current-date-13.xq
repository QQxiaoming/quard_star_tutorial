(:*******************************************************:)
(:Test: fn-current-date-13                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-date" function as part :)
(:of an equal expression (ne operator)                   :) 
(:*******************************************************:)

fn:string(fn:current-date()) ne fn:string(fn:current-date())
