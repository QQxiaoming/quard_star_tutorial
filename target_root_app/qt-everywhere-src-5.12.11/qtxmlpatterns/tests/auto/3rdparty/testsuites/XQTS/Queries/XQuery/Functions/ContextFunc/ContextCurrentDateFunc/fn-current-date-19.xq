(:*******************************************************:)
(:Test: fn-current-date-19                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-date" function as part :)
(:of a boolean expression ("or" operator and fn:false function.:) 
(:*******************************************************:)

fn:string(fn:current-date()) or fn:false()
