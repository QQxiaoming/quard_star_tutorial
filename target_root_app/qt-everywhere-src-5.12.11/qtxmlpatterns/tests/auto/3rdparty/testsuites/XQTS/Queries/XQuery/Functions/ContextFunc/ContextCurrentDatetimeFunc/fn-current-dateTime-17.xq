(:*******************************************************:)
(:Test: fn-current-dateTime-17                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 5, 2005                              :)
(:Purpose: Evaluates The "current-dateTime" function as part :)
(:of a boolean expression ("and" operator and fn:false function.:) 
(:*******************************************************:)

fn:string(fn:current-dateTime()) and fn:false()
