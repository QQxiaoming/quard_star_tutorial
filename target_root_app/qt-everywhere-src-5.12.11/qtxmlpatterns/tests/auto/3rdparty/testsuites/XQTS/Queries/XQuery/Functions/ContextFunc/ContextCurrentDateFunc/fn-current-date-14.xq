(:*******************************************************:)
(:Test: fn-current-date-14                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-date" function as part :)
(:of an equal expression (le operator)                   :) 
(:*******************************************************:)

fn:string(fn:current-date()) le fn:string(fn:current-date())