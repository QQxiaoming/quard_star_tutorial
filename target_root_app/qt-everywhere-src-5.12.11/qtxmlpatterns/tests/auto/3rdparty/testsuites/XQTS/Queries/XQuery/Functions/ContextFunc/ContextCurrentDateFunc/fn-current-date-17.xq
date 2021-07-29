(:*******************************************************:)
(:Test: fn-current-date-17                               :)
(:Written By: Carmelo Montanez                           :)
(:Date: December 5, 2005                                 :)
(:Purpose: Evaluates The "current-date" function as part :)
(:of a boolean expression ("and" operator and fn:false function.:) 
(:*******************************************************:)

fn:string(fn:current-date()) and fn:false()
