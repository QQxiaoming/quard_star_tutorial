(:*******************************************************:)
(:Test: fn-current-dateTime-19                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 5, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function as part :)
(:of a boolean expression ("or" operator and fn:false function.:) 
(:*******************************************************:)

fn:string(fn:current-dateTime()) or fn:false()
