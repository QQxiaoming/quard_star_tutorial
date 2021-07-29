(:*******************************************************:)
(:Test: fn-current-time-17                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-time" function as part :)
(:of a boolean expression ("and" opeartor and fn:false function.:) 
(:*******************************************************:)

fn:string(fn:current-time()) and fn:false()
