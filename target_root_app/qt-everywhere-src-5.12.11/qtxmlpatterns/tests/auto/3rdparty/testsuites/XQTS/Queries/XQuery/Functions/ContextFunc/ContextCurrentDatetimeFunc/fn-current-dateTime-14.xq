(:*******************************************************:)
(:Test: fn-current-dateTime-14                           :)
(:Written By: Carmelo Montanez                           :)
(:dateTime: December 5, 2005                             :)
(:Purpose: Evaluates The "current-dateTime" function as part :)
(:of an equal expression (le operator)                   :) 
(:*******************************************************:)

fn:string(fn:current-dateTime()) le fn:string(fn:current-dateTime())