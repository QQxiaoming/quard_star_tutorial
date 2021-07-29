(:*******************************************************:)
(:Test: fn-current-time-19                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-time" function as part :)
(:of a boolean expression ("or" opeartor and fn:false function.:) 
(:*******************************************************:)

fn:string(fn:current-time()) or fn:false()
