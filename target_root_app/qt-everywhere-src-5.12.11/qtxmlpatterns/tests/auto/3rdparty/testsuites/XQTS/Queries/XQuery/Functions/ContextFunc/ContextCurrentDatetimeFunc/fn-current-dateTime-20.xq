(:*******************************************************:)
(:Test: fn-current-dateTime-20                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 5, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function (string :)
(: value)as an argument to the fn:not function.          :) 
(:*******************************************************:)

fn:not(fn:string(fn:current-dateTime()))