(:*******************************************************:)
(:Test: fn-current-dateTime-15                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 5, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function as part :)
(:of an equal expression (ge operator)                   :) 
(:*******************************************************:)

fn:string(fn:current-dateTime()) ge fn:string(fn:current-dateTime())