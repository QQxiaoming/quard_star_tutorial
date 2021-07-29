(:*******************************************************:)
(:Test: fn-current-time-18                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-time" function as part :)
(:of a boolean expression ("or" opeartor and fn:true function.:) 
(:*******************************************************:)

fn:string(fn:current-time()) or fn:true()
