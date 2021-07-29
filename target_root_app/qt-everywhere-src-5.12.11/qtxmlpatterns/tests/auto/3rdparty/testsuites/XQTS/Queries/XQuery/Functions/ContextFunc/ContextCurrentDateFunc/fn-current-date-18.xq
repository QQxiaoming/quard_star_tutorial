(:*******************************************************:)
(:Test: fn-current-date-18                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-date" function as part :)
(:of a boolean expression ("or" operator and fn:true function.:) 
(:*******************************************************:)

fn:string(fn:current-date()) or fn:true()
