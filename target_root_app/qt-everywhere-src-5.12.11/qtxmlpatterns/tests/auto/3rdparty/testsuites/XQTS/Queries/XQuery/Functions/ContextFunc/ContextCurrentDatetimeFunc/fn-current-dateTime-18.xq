(:*******************************************************:)
(:Test: fn-current-dateTime-18                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 5, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function as part :)
(:of a boolean expression ("or" operator and fn:true function.:) 
(:*******************************************************:)

fn:string(fn:current-dateTime()) or fn:true()
